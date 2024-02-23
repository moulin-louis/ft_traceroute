#include "ft_traceroute.h"

char timeout;
t_tc trace;

void cleanup(void) {
  close(trace.sck);
  close(trace.icmp_sck);
}

void handle_alarm(int sig) {
  if (sig == SIGALRM)
    timeout = true;
}

void handle_quit(int sig) {
  if (sig == SIGTERM || sig == SIGINT) {
    cleanup();
    exit(sig);
  }
}

int64_t change_ttl(int sock, uint64_t new_ttl) {
  const int retval = setsockopt(sock, IPPROTO_IP, IP_TTL, &new_ttl, sizeof(new_ttl));
  if (retval == -1) {
    perror("setsockopt ttl");
    return 1;
  }
  return 0;
}

int32_t apply_arg(int ac, char** av) {
  (void)trace;
  (void)ac;
  (void)av;
  return 0;
}

int32_t init_tc(int ac, char** av) {
  trace.sck = socket(AF_INET, SOCK_DGRAM, 0);
  if (trace.sck == -1) {
    perror("socket1");
    return 1;
  }
  trace.icmp_sck = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (trace.icmp_sck == -1) {
    perror("socket2");
    return 1;
  }
  trace.first_ttl = 1;
  trace.port = TC_PORT;
  trace.prot = E_UDP;
  trace.ttl = 1;
  trace.ttl_max = 30;
  trace.waittime = 5;
  apply_arg(ac, av);
  trace.dest.sin_family = AF_INET;
  trace.dest.sin_port = htons(trace.port);
  trace.dest.sin_addr.s_addr = inet_addr(av[1]);
  if (setsockopt(trace.sck, IPPROTO_IP, IP_TTL, &trace.first_ttl, sizeof(trace.first_ttl)) == -1) {
    perror("setsockopt_ttl");
    return 1;
  }
  return 0;
}

int64_t send_probe(void) {
  const int64_t retval = sendto(trace.sck, "toto", 4, 0, (struct sockaddr*)&trace.dest, sizeof(trace.dest));
  if (retval == -1) {
    perror("sendto probe");
    return 1;
  }
  return 0;
}

int64_t wait_response(struct icmphdr* icmphdr, struct iphdr* iphdr) {
  char buf[2048] = {0};
  int64_t retval = 0;
  alarm(trace.waittime);
  while (timeout == false) {
    retval = recvfrom(trace.icmp_sck, buf, 2048, MSG_DONTWAIT, NULL, NULL);
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
  ft_memcpy(iphdr, buf, sizeof(iphdr));
  ft_memcpy(icmphdr, buf + sizeof(icmphdr), sizeof(icmphdr));
  return 0;
}

int main(int ac, char** av) {
  if (ac == 1) {
    // display_usage();
    fprintf(stderr, "Wrong nbr of args\n");
    return 1;
  }
  if (init_tc(ac, av))
    return 1;
  fflush(NULL);
  signal(SIGALRM, handle_alarm);
  signal(SIGTERM, handle_quit);
  signal(SIGINT, handle_quit);
  for (uint64_t iter = 0; iter < trace.ttl_max; ++iter) {
    struct iphdr iphdr;
    struct icmphdr icmphdr;

    // send the probe
    send_probe();
    // wait for the response
    wait_response(&icmphdr, &iphdr);
    printf("%lu: ", trace.ttl);
    if (ft_memcmp(&icmphdr, (uint64_t[]){0, 0, 0, 0, 0, 0, 0, 0}, sizeof(icmphdr)) != 0) {
      ft_hexdump(&icmphdr, sizeof(icmphdr), 0);
    }
    else {
      printf("* * *\n");
    }
    fflush(NULL);
    // stop if ICMP port reset
    trace.ttl += 1;
    if (change_ttl(trace.sck, trace.ttl)) {
      break;
    }
    // start over with ttl += 1 otherwose
  }
  cleanup();
  return 0;
}
