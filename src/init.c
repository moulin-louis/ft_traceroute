//
// Created by loumouli on 3/4/24.
//

#include "ft_traceroute.h"
static uint64_t init_probe(void* ptr);
static uint64_t clean_probe(void* ptr);

void cleanup(void) { ft_set_destroy(sockets, clean_probe); }

void init_default() {
  trace.first_ttl = DEFAULT_FIRST_TLL;
  trace.max_ttl = DEFAULT_MAX_TTL;
  trace.packet_len = DEFAULT_SIZE_PACKET;
  trace.nquerries = DEFAULT_NBR_PROBES;
  trace.port = DEFAULT_UDP_PORT;
  trace.waittime = DEFAULT_WAITTIME;
  trace.sendwait = DEFAULT_WAIT_PROBE;
  trace.nbr_total_probes = (trace.max_ttl - (trace.first_ttl - 1)) * trace.nquerries;
}

int64_t init_sockets() {
  sockets = ft_set_new(sizeof(t_probe));
  if (sockets == NULL) {
    perror("ft_set_new:");
    return 1;
  }
  if (ft_set_reserve(sockets, trace.nbr_total_probes)) {
    perror("ft_set_reserve:");
    return 1;
  }
  ft_memset(sockets->data, 0, sockets->capacity * sockets->nbytes_data);
  sockets->len = trace.nbr_total_probes;
  if (ft_set_iter(sockets, init_probe)) {
    ft_set_destroy(sockets, clean_probe);
    perror("ft_set_iter:");
    return 1;
  }
  for (uint64_t i = trace.first_ttl - 1; i < trace.max_ttl; ++i) {
    for (uint64_t j = 0; j < trace.nquerries; ++j) {
      t_probe* probe = ft_set_get(sockets, i * trace.nquerries + j);
      probe->ttl = trace.first_ttl + i;
      if (setsockopt(probe->sck, SOL_IP, IP_TTL, &probe->ttl, sizeof(probe->ttl)) == -1) {
        printf("probe->sck = %d\n", probe->sck);
        perror("setsockopt ttl");
        return 1;
      }
    }
  }
  return 0;
}

void print_options(void) {
  printf("firs_ttl = %ld\n", trace.first_ttl);
  printf("ttl_max = %ld\n", trace.max_ttl);
  printf("nbr_probes = %ld\n", trace.nquerries);
  printf("waittime = %ld\n", trace.waittime);
  printf("wait_prob = %ld\n", trace.sendwait);
  printf("port = %u\n", trace.port);
  printf("packet size = %ld\n", trace.packet_len);
}

int64_t init_tc(int ac, char** av) {
  (void)ac;
  init_default();
  if (parse_opt(ac, av))
    return 1;
  strcpy((char*)trace.hostname, av[1]);
  if (hostname_to_sockaddr((char*)trace.hostname, &trace.ip_addr)) {
    perror("host_to_sockaddr");
    return 1;
  }
  if (init_sockets())
    return 1;
  return 0;
}

static uint64_t init_probe(void* ptr) {
  t_probe* probe = ptr;
  ft_memset(probe, 0, sizeof(t_probe));
  probe->sck = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
  if (probe->sck == -1) {
    perror("socket");
    return 1;
  }
  if (setsockopt(probe->sck, SOL_IP, IP_RECVERR, &(int[]){1}, sizeof(int)) == -1) {
    perror("setsockopt recverr");
    return 1;
  }
  FD_SET(probe->sck, &readfds);
  probe->dest.sin_family = AF_INET;
  probe->dest.sin_addr = trace.ip_addr.sin_addr;
  return 0;
}

static uint64_t clean_probe(void* ptr) {
  const t_probe* probe = ptr;
  if (probe->sck)
    close(probe->sck);
  return 0;
}
