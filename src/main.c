#include "ft_traceroute.h"

t_opt trace;
volatile bool timeout;
t_set* sockets;

int main(int ac, const char** av) {
  if (ac == 1) {
    fprintf(stderr, "Usage: ft_traceroute [option] host\n");
    return 1;
  }
  //Init option and sockets set
  if (init_tc(ac, av))
    return 1;

  signal(SIGTERM, handle_quit);
  signal(SIGINT, handle_quit);
  for (uint64_t i = trace.first_ttl; i <= trace.ttl_max; ++i) {
    for (uint64_t j = 0; j < trace.nbr_probes; ++j) {
    }
  }
  cleanup();
  return 0;
}
