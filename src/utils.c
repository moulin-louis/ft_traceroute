//
// Created by loumouli on 3/4/24.
//

#include "ft_traceroute.h"

void handle_quit(const int sig) {
  if (sig == SIGTERM || sig == SIGINT) {
    cleanup();
    exit(sig);
  }
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

int32_t hostname_to_sockaddr(const char* hostname, void* result_ptr) {
  struct addrinfo hints;
  struct addrinfo* result;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  const int retval = getaddrinfo(hostname, NULL, &hints, &result);
  if (retval != 0)
    return retval;
  memcpy(result_ptr, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);
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

double calculate_rtt(const struct timespec start_time, const struct timespec end_time) {
  // Convert to ms the difference between the 2 timespec
  return (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0;
}
