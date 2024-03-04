#include "ft_traceroute.h"

bool timeout;
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

int64_t wait_response(struct icmphdr* icmphdr, struct sockaddr_in* src) {
  char buf[2048] = {0};
  socklen_t len = sizeof(struct sockaddr_in);

  alarm(trace.waittime);
  while (timeout == false) {
    const int64_t retval = recvfrom(trace.icmp_sck, buf, 2048, MSG_DONTWAIT, (struct sockaddr*)src, &len);
    if (retval == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        continue;
      perror("recvfrom");
      return 1;
    }
    break;
  }
  if (timeout == true)
    timeout = false;
  ft_memcpy(icmphdr, buf + sizeof(struct iphdr), sizeof(struct icmphdr));
  return 0;
}

void print_result(const struct sockaddr_in* src, double rtts[trace.nbr_probes]) {
  printf(" %lu  ", trace.ttl);
  char* ip_str = inet_ntoa(src->sin_addr);
  char hostname_str[HOST_NAME_MAX];
  ft_memset(hostname_str, 0, sizeof(hostname_str));
  ip_to_hostname(ip_str, hostname_str);
  printf("%s ", strlen(hostname_str) ? hostname_str : ip_str);
  printf("(%s) ", ip_str);
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    if (rtts[idx] > 5000) {
      printf("* ");
      continue;
    }
    printf("%.3f ms ", rtts[idx]);
  }
  printf("\n");
}

bool all_same_results(const struct sockaddr_in srcs[trace.nbr_probes],
                      const struct icmphdr icmphdrs[trace.nbr_probes]) {
  const struct sockaddr_in sample_addr = srcs[0];
  const struct icmphdr sample_icmp = icmphdrs[0];
  for (uint64_t idx = 1; idx < trace.nbr_probes; ++idx) {
    if (ft_memcmp(&sample_addr, &srcs[idx], sizeof(struct sockaddr_in)) != 0)
      return false;
  }
  for (uint64_t idx = 1; idx < trace.nbr_probes; ++idx) {
    if (ft_memcmp(&sample_icmp, &icmphdrs[idx], sizeof(struct icmphdr)) != 0)
      return false;
  }
  return true;
}

int64_t handle_probes(void) {
  double rtts[trace.nbr_probes];
  struct timeval start_times[trace.nbr_probes];
  struct timeval end_times[trace.nbr_probes];
  struct sockaddr_in srcs[trace.nbr_probes];
  struct icmphdr icmphdrs[trace.nbr_probes];

  ft_memset(rtts, 0, sizeof(rtts));
  ft_memset(start_times, 0, sizeof(start_times));
  ft_memset(end_times, 0, sizeof(end_times));
  ft_memset(srcs, 0, sizeof(srcs));
  ft_memset(icmphdrs, 0, sizeof(icmphdrs));
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    gettimeofday(&start_times[idx], NULL);
    if (send_probe())
      return 1; // Return ERROR code
  }
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    if (wait_response(&icmphdrs[idx], &srcs[idx]))
      return 1; // Return ERROR code
    gettimeofday(&end_times[idx], NULL);
    rtts[idx] = ((end_times[idx].tv_sec - start_times[idx].tv_sec) * 1000000L +
                 (end_times[idx].tv_usec - start_times[idx].tv_usec)) /
      1000.0;
  }
  print_result(&srcs[0], rtts);
  if (icmphdrs[0].type == ICMP_DEST_UNREACH && icmphdrs[0].code == ICMP_PORT_UNREACH)
    return 0; // Return SUCCESS code
  return 2; // Return TIMEOUT code
}

int main(int ac, char** av) {
  if (ac == 1) {
    // display_usage();
    fprintf(stderr, "Wrong nbr of args\n");
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
