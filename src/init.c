//
// Created by loumouli on 3/4/24.
//

#include "ft_traceroute.h"
static uint64_t init_probe(void* ptr);
static uint64_t clean_probe(void* ptr);

void cleanup(void) { printf("cleanup called\n"); ft_set_destroy(sockets, clean_probe); }

void init_default() {
  trace.first_ttl = DEFAULT_FIRST_TLL;
  trace.ttl_max = DEFAULT_MAX_TTL;
  trace.ttl = DEFAULT_FIRST_TLL;
  trace.size_probe = DEFAULT_SIZE_PACKET;
  trace.nbr_probes = DEFAULT_NBR_PROBES;
  trace.port = DEFAULT_UDP_PORT;
  trace.prot = DEFAULT_PROT;
  trace.waittime = DEFAULT_WAITTIME;
  trace.wait_prob = DEFAULT_WAIT_PROBE;
  trace.nbr_total_probes = (trace.ttl_max - (trace.first_ttl - 1)) * trace.nbr_probes;
}

int64_t init_sockets() {
  sockets = ft_set_new(sizeof(t_probe));
  if (sockets == NULL) {
    perror("ft_set_new:");
    return 1;
  }
  for (uint64_t i = 0; i < trace.nbr_total_probes; ++i) {
    t_probe probe = {0};
    printf("i = %ld\n", i);
    if (ft_set_insert(sockets, &probe, i)) {
      perror("ft_set_push:");
      return 1;
    }
  }
  if (ft_set_iter(sockets, init_probe)) {
    ft_set_destroy(sockets, clean_probe);
    perror("ft_set_iter:");
    return 1;
  }
  uint64_t i = 0;
  for (; i <= sockets->len; ++i) {
    t_probe* probe = ft_set_get(sockets, i);
    probe->ttl = trace.first_ttl + i;
    if (setsockopt(probe->sck, IPPROTO_IP, IP_TTL, &probe->ttl, sizeof(probe->ttl)) == -1) {
      printf("probe->sck = %d\n", probe->sck);
      perror("setsockopt ttl");
      return 1;
    }
  }
  printf("%ld sockets created\n", i);
  return 0;
}

int64_t init_tc(const int ac, const char** av) {
  (void)ac;
  (void)av;
  init_default();
  if (init_sockets())
    return 1;
  return 0;
}

static uint64_t init_probe(void* ptr) {
  t_probe* probe = ptr;
  ft_memset(probe, 0, sizeof(t_probe));
  probe->sck = socket(AF_INET, SOCK_DGRAM, 0);
  if (probe->sck == -1) {
    perror("socket");
    return 1;
  }
  if (setsockopt(probe->sck, SOL_IP, IP_RECVERR, &(int){1}, sizeof(int)) == -1) {
    perror("setsockopt recverr");
    return 1;
  }
  probe->dest.sin_family = AF_INET;
  probe->dest.sin_port = htons(trace.port);
  return 0;
}

static uint64_t clean_probe(void* ptr) {
  const t_probe* probe = ptr;
  if (probe->sck)
    close(probe->sck);
  return 0;
}
