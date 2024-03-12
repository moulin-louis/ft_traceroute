//
// Created by loumouli on 3/12/24.
//

#include "ft_traceroute.h"

static int parse_argp(const int key, char* arg, struct argp_state* state) {
  switch (key) {
  case 'f': {
    char* end_ptr = NULL;
    const int64_t first_ttl = strtoll(arg, &end_ptr, 10);
    if (end_ptr == arg)
      argp_failure(state, 1, 0, "first_ttl: cant convert");
    if (first_ttl < 0 || first_ttl > 255)
      argp_failure(state, 1, 0, "first_ttl: invalid number");
    trace.first_ttl = first_ttl;
    break;
  }
  case 'm': {
    char* end_ptr = NULL;
    const int64_t max_ttl = strtoll(arg, &end_ptr, 10);
    if (end_ptr == arg)
      argp_failure(state, 1, 0, "max_ttl: cant convert");
    if (max_ttl < 0 || max_ttl > 255)
      argp_failure(state, 1, 0, "max_ttl: invalid number");
    trace.max_ttl = max_ttl;
    break;
  }
  case 'w': {
    char* end_ptr = NULL;
    const int64_t waittime = strtoll(arg, &end_ptr, 10);
    if (end_ptr == arg)
      argp_failure(state, 1, 0, "waittime: cant convert");
    if (waittime < 0 || waittime > 255)
      argp_failure(state, 1, 0, "waittime: invalid number");
    trace.waittime = waittime;
    break;
  }
  case 'z': {
    char* end_ptr = NULL;
    const int64_t sendwait = strtoll(arg, &end_ptr, 10);
    if (end_ptr == arg)
      argp_failure(state, 1, 0, "sendwait: cant convert");
    if (sendwait < 0 || sendwait > 255)
      argp_failure(state, 1, 0, "send_wait: invalid number");
    trace.sendwait = sendwait;
    break;
  }
  case 'q': {
    char* end_ptr = NULL;
    const int64_t nquerries = strtoll(arg, &end_ptr, 10);
    if (end_ptr == arg)
      argp_failure(state, 1, 0, "nquerries: cant convert");
    if (nquerries < 0 || nquerries > 255)
      argp_failure(state, 1, 0, "nquerries: invalid number");
    trace.nquerries = nquerries;
    break;
  }
  case 'p': {
    char* end_ptr = NULL;
    const uint16_t port = strtol(arg, &end_ptr, 10);
    if (end_ptr == arg)
      argp_failure(state, 1, 0, "port: cant convert");
    if (port < 0 || port > UINT16_MAX)
      argp_failure(state, 1, 0, "port: invalid number");
    trace.port = port;
    break;
  }
  case ARGP_KEY_ARG: {
    if (state->arg_num == 1)
      trace.packet_len = atoi(arg);
    break;
  }
  case ARGP_KEY_END: {
    if (state->arg_num < 1)
      argp_usage(state);
    break;
  }
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
    {0},
  };
  struct argp argp = {0};
  argp.options = options;
  argp.parser = parse_argp;
  argp.args_doc = "hostname [packet_len]";
  return argp_parse(&argp, ac, av, 0, 0, NULL);
}
