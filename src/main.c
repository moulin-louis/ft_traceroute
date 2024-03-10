#include "ft_traceroute.h"

t_tc trace;

int64_t send_probe(void) {
  uint8_t probe[trace.size_packet];

  ft_memset(probe, 0, sizeof(probe));
  const int64_t ret = sendto(trace.sck, probe, trace.size_packet, 0, (struct sockaddr*)&trace.dest, sizeof(trace.dest));
  if (ret == -1) {
    perror("sendto probe");
    return 1;
  }
  return 0;
}

int64_t grab_packet(struct icmphdr* icmphdr, struct sockaddr_in* src) {
  uint8_t buf[sizeof(struct iphdr) + sizeof(struct icmp)] = {0};
  socklen_t len = sizeof(struct sockaddr_in);

  const int64_t retval = recvfrom(trace.icmp_sck, buf, sizeof(buf), 0, (struct sockaddr*)src, &len);
  if (retval == -1) {
    perror("recvfrom");
    return 1;
  }
  ft_memcpy(icmphdr, buf + sizeof(struct iphdr), sizeof(struct icmphdr));
  return 0;
}

int64_t print_result(void) {
  char hostname_str[HOST_NAME_MAX] = {0};
  const t_probe* probe = ft_set_get(trace.probes, 0);
  char* ip_str = inet_ntoa(probe->src.sin_addr);

  ip_to_hostname(ip_str, hostname_str);

  printf(" %lu  ", trace.ttl);
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
  probe = ft_set_get(trace.probes, 0);
  if (probe->icmphdr.type == ICMP_DEST_UNREACH && probe->icmphdr.code == ICMP_PORT_UNREACH)
    return 0;
  return 2;
}

int64_t handle_probes(void) {
  struct timeval timeout;
  fd_set readfds;
  uint64_t nbr_packet = 0;

  timeout.tv_sec = trace.waittime;
  timeout.tv_usec = 0;
  // Reset all probes
  ft_memset(trace.probes->data, 0, trace.probes->len * trace.probes->nbytes_data);
  // Send all probes
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    gettimeofday(&((t_probe*)ft_set_get(trace.probes, idx))->start_time, NULL);
    if (send_probe())
      return 1; // Return ERROR code
    usleep(2000);
  }
  // Wait up to timeout for probes
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
      if (grab_packet(&probe->icmphdr, &probe->src))
        return 1;
      gettimeofday(&probe->end_time, NULL);
      probe->rtt = (((probe->end_time.tv_sec - probe->start_time.tv_sec) * 1000000L + (probe->end_time.tv_usec - probe->start_time.tv_usec)) / 1000.0) - 2;
      nbr_packet += 1;
    }
    if (nbr_packet == trace.nbr_probes)
      break;
  }
  return print_result();
}

int main(int ac, const char** av) {
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
