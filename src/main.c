#include "ft_traceroute.h"

t_opt trace;
volatile bool timeout;
t_set* sockets;

uint64_t send_probe(t_probe* probe) {
  uint8_t packet[trace.size_probe];

  ft_memset(packet, 0, sizeof(packet));
  const int64_t retval = sendto(probe->sck, packet, sizeof(packet), 0, (struct sockaddr*)&probe->dest, sizeof(probe->dest));
  if (retval == -1) {
    perror("sendto");
    return 1;
  }
  return 0;
}

int main(int ac, const char** av) {
  if (ac == 1) {
    fprintf(stderr, "Usage: ft_traceroute [option] host\n");
    return 1;
  }
  // Init option and sockets set
  if (init_tc(ac, av))
    return 1;

  signal(SIGTERM, handle_quit);
  signal(SIGINT, handle_quit);
  for (uint64_t i = trace.first_ttl; i <= trace.ttl_max; ++i) {
    uint16_t port = htons(trace.port + i);
    for (uint64_t j = 0; j < trace.nbr_probes; ++j) {
      t_probe* probe = ft_set_get(sockets, i + j);
      probe->dest.sin_port = port;
      if (send_probe(probe)) {
        cleanup();
        return 1;
      }
    }
  }
  printf("all probes sent\n");
  cleanup();
  return 0;
}
