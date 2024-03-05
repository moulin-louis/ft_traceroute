#include "ft_traceroute.h"

// bool timeout;
t_tc trace;

int64_t send_probe(void) {
  uint8_t probe[trace.size_packet];

  ft_memset(&probe, 0, sizeof(probe));
  const int64_t retval =
    sendto(trace.sck, probe, trace.size_packet, 0, (struct sockaddr*)&trace.dest, sizeof(trace.dest));
  if (retval == -1) {
    perror("sendto probe");
    return 1;
  }
  return 0;
}

int64_t grab_packet(struct icmphdr* icmphdr, struct sockaddr_in* src) {
  char buf[2048] = {0};
  socklen_t len = sizeof(struct sockaddr_in);

  const int64_t retval = recvfrom(trace.icmp_sck, buf, 2048, 0, (struct sockaddr*)src, &len);
  if (retval == -1) {
    perror("recvfrom");
    return 1;
  }
  ft_memcpy(icmphdr, buf + sizeof(struct iphdr), sizeof(struct icmphdr));
  return 0;
}

void print_result(void) {
  const t_probe* probe = ft_set_get(trace.probes, 0);
  printf(" %lu  ", trace.ttl);
  char* ip_str = inet_ntoa(probe->src.sin_addr);
  char hostname_str[HOST_NAME_MAX];
  ft_memset(hostname_str, 0, sizeof(hostname_str));
  ip_to_hostname(ip_str, hostname_str);
  printf("%s ", ft_strlen(hostname_str) ? hostname_str : ip_str);
  printf("(%s) ", ip_str);

  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    probe = ft_set_get(trace.probes, idx);
    if (probe->rtt == 0) {
      printf("* ");
      continue;
    }
    printf("%.3f ms ", probe->rtt);
  }
  printf("\n");
}

int64_t handle_probes(void) {
  struct timeval timeout;
  fd_set readfds;
  uint64_t nbr_packet = 0;

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  // Init X probes in the set
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    t_probe probe;
    ft_memset(&probe, 0, sizeof(probe));
    ft_set_push(trace.probes, &probe, sizeof(probe));
  }
  // Send each probe and update their timestamp
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    gettimeofday(&((t_probe*)ft_set_get(trace.probes, idx))->start_time, NULL);
    if (send_probe())
      return 1; // Return ERROR code
  }
  while (true) {
    FD_ZERO(&readfds);
    FD_SET(trace.icmp_sck, &readfds);
    const int retval = select(trace.icmp_sck + 1, &readfds, NULL, NULL, &timeout);
    if (retval == -1) {
      perror("select");
      return 1; // Return ERROR code
    }
    if (retval == 0)
      break; // Break cause of timeout
    if (FD_ISSET(trace.icmp_sck, &readfds)) {
      t_probe* probe = ft_set_get(trace.probes, nbr_packet);
      grab_packet(&probe->icmphdr, &probe->src);
      gettimeofday(&probe->end_time, NULL);
      probe->rtt = ((probe->end_time.tv_sec - probe->start_time.tv_sec) * 1000000L +
                    (probe->end_time.tv_usec - probe->start_time.tv_usec)) /
        1000.0;
      ft_set_push(trace.probes, &probe, sizeof(probe));
      nbr_packet += 1;
    }
  }
  print_result();
  const t_probe* probe = ft_set_get(trace.probes, 0);
  if (probe->icmphdr.type == ICMP_DEST_UNREACH && probe->icmphdr.code == ICMP_PORT_UNREACH) {
    return 0; // Return SUCCESS code
  }
  ft_set_clear(trace.probes);
  return 2;
}

int main(int ac, char** av) {
  if (ac == 1) {
    fprintf(stderr, "Usage: ft_traceroute [option] host\n");
    return 1;
  }
  if (init_tc(ac, av))
    return 1;
  printf("traceroute to %s (%s), %ld hops max, %ld byte packets\n", av[1], inet_ntoa(trace.dest.sin_addr),
         trace.ttl_max, trace.size_packet);
  signal(SIGALRM, handle_quit);
  signal(SIGTERM, handle_quit);
  signal(SIGINT, handle_quit);
  for (uint64_t iter = 0; iter < trace.ttl_max; ++iter) {
    const int retval = handle_probes();
    if (retval == 1 || retval == 0)
      break;
    trace.ttl += 1;
    if (change_ttl(trace.sck, trace.ttl))
      break;
    sleep(trace.wait_prob);
  }
  cleanup();
  return 0;
}
