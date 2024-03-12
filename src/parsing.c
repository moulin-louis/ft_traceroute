//
// Created by loumouli on 3/12/24.
//

#include "ft_traceroute.h"

static int parse_argp(const int key, char* arg, struct argp_state* state) {
  switch (key) {
  case 'f':
    trace.first_ttl = atoi(arg);
    break;
  case 'm':
    trace.max_ttl = atoi(arg);
    break;
  case 'w':
    trace.waittime = atoi(arg);
    break;
  case 'z':
    trace.sendwait = atoi(arg);
    break;
  case 'q':
    trace.nquerries = atoi(arg);
    break;
  case 'p':
    trace.port = atoi(arg);
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num == 1)
      trace.packet_len = atoi(arg);
    break;
  case ARGP_KEY_END:
    if (state->arg_num < 1)
      argp_usage(state);
    break;
  case 'v':
    printf("Version 1.0, Louis MOULIN, loumouli\n");
    break;
  }
  return 0;
}

int32_t parse_opt(int ac, char** av) {
  const struct argp_option options[] = {
    {"first_ttl", 'f', "NUM", 0, "Specifies with what TTL to start. Defaults to 1.", 0},
    {"max_ttl", 'm', "NUM", 0,
     "Specifies the maximum number of hops (max time-to-live value) traceroute will probe. The default is 30.", 0},
    {"waittime", 'w', "NUM", 0, "Set the time (in seconds) to wait for a response to a probe (default 5.0 sec).", 0},
    {"sendwait", 'z', "NUM", 0, "Minimal time interval between probes (default 0).", 0},
    {"nquerries", 'q', "NUM", 0, "Sets the number of probe packets per hop. The default is 3.", 0},
    {"port", 'p', "NUM", 0,
     "Specifies the destination port base traceroute will use (the destination port number will be incremented by each "
     "probe).",
     0},
    {"version", 'v', 0, 0, "Print version info", 0},
    {0},
  };
  struct argp argp = {0};
  argp.options = options;
  argp.parser = parse_argp;
  argp.args_doc = "hostname [packet_len]";
  return argp_parse(&argp, ac, av, 0, 0, NULL);
}
