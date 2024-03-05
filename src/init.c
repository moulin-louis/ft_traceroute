//
// Created by loumouli on 3/4/24.
//

#include "ft_traceroute.h"

static int64_t apply_arg(const int ac, char** av) {
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
  trace.nbr_probes = DEFAULT_NBR_PROBES;
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

  trace.probes = ft_set_new(sizeof(t_probe));
  if (trace.probes == NULL)
    return 1;

  return change_ttl(trace.sck, 1);
}
