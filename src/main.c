#include "ft_traceroute.h"

bool timeout;
t_tc trace;

void cleanup(void) {
  close(trace.sck);
  close(trace.icmp_sck);
}

void handle_quit(const int sig) {
  if (sig == SIGTERM || sig == SIGINT) {
    cleanup();
    exit(sig);
  }
  if (sig == SIGALRM)
    timeout = true;
}

int64_t get_time(double* time) {
  struct timeval tst;

  if (gettimeofday(&tst, NULL) == -1) {
    perror("gettimeofday");
    return 1;
  }
  *time = tst.tv_sec * 1000 + tst.tv_usec / 1000;
  return 0;
}

int64_t ip_to_hostname(const char* ip, char* result_str) {
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &(sa.sin_addr));
  const int retval = getnameinfo((struct sockaddr*)&sa, sizeof(sa), result_str, NI_MAXHOST, NULL, 0, 0);
  if (retval != 0)
    return retval;
  return 0;
}

int64_t change_ttl(const int sock, const uint64_t new_ttl) {
  const int retval = setsockopt(sock, IPPROTO_IP, IP_TTL, &new_ttl, sizeof(new_ttl));
  if (retval == -1) {
    perror("setsockopt ttl");
    return 1;
  }
  return 0;
}

int64_t apply_arg(const int ac, char** av) {
  (void)trace;
  (void)ac;
  (void)av;
  return 0;
}

int64_t init_tc(const int ac, char** av) {
  trace.sck = socket(AF_INET, SOCK_DGRAM, 0);
  if (trace.sck == -1) {
    perror("socket");
    return 1;
  }
  trace.icmp_sck = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (trace.icmp_sck == -1) {
    perror("socket_icmp");
    return 1;
  }
  trace.first_ttl = DEFAULT_FIRST_TLL;
  trace.ttl_max = DEFAULT_MAX_TTL;
  trace.ttl = DEFAULT_FIRST_TLL;
  trace.size_packet = DEFAULT_SIZE_PACKET;
  trace.port = DEFAULT_UDP_PORT;
  trace.prot = DEFAULT_PROT;
  trace.waittime = DEFAULT_WAITTIME;
  trace.wait_prob = DEFAULT_WAIT_PROBE;
  apply_arg(ac, av);
  trace.dest.sin_family = AF_INET;
  trace.dest.sin_port = htons(trace.port);
  if (inet_pton(AF_INET, av[1], &trace.dest.sin_addr) != 1) {
    perror("inet_pton");
    return 1;
  }
  return change_ttl(trace.sck, 1);
}

int64_t send_probe(void) {
  uint8_t probe[trace.size_packet];
  gettimeofday(&trace.start_time, NULL);
  const int64_t retval = sendto(trace.sck, probe, trace.size_packet, 0, (struct sockaddr*)&trace.dest, sizeof(trace.dest));
  if (retval == -1) {
    perror("sendto probe");
    return 1;
  }
  return 0;
}

int64_t wait_response(struct icmphdr* icmphdr, struct sockaddr_in* src) {
  char buf[2048] = {0};
  socklen_t len = sizeof(struct sockaddr_in);

  ft_memset(src, 0, sizeof(*src));
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
  gettimeofday(&trace.end_time, NULL);
  ft_memcpy(icmphdr, buf + sizeof(struct iphdr), sizeof(icmphdr));
  return 0;
}

void print_result(const struct icmphdr* icmphdr, const struct sockaddr_in* src) {
  printf(" %lu  ", trace.ttl);
  if (ft_memcmp(icmphdr, (uint64_t[]){0, 0, 0, 0, 0, 0, 0, 0}, sizeof(*icmphdr)) == 0) {
    printf("* * *\n");
    return;
  }
  char* ip_str = inet_ntoa(src->sin_addr);
  char hostname_str[HOST_NAME_MAX];
  ip_to_hostname(ip_str, hostname_str);
  printf("%s ", strlen(hostname_str) ? hostname_str : ip_str);
  printf("(%s) ", ip_str);
  const double rtt_ms = ((trace.end_time.tv_sec - trace.start_time.tv_sec) * 1000000L + (trace.end_time.tv_usec - trace.start_time.tv_usec)) / 1000.0;
  printf("%.3f ms", rtt_ms);
  printf("\n");
}

int main(int ac, char** av) {
  if (ac == 1) {
    // display_usage();
    fprintf(stderr, "Wrong nbr of args\n");
    return 1;
  }
  if (init_tc(ac, av))
    return 1;
  printf("traceroute to %s (%s), %ld hops max, %ld byte packets\n", av[1], inet_ntoa(trace.dest.sin_addr), trace.ttl_max, trace.size_packet);
  signal(SIGALRM, handle_quit);
  signal(SIGTERM, handle_quit);
  signal(SIGINT, handle_quit);
  for (uint64_t iter = 0; iter < trace.ttl_max; ++iter) {
    struct icmphdr icmphdr;
    struct sockaddr_in src;
    // send the probe
    if (send_probe())
      break;
    // wait for the response
    if (wait_response(&icmphdr, &src))
      break;
    print_result(&icmphdr, &src);
    if (icmphdr.code == 3 && icmphdr.type == 3)
      break;
    // start over with ttl += 1 otherwose
    trace.ttl += 1;
    if (change_ttl(trace.sck, trace.ttl))
      break;
    sleep(trace.wait_prob);
  }
  cleanup();
  return 0;
}
