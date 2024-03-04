//
// Created by loumouli on 3/4/24.
//

#include "ft_traceroute.h"

void cleanup(void) {
  close(trace.sck);
  close(trace.icmp_sck);
}

void handle_quit(const int sig) {
  if (sig == SIGTERM || sig == SIGINT) {
    cleanup();
    exit(sig);
  }
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
