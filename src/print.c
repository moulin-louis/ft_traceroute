//
// Created by loumouli on 3/12/24.
//

#include "ft_traceroute.h"

void init_ptr(t_probe** probe, t_probe** end_probe, const uint8_t ttl) {
  uint64_t i = 0;
  for (; i < sockets->len; ++i) {
    *probe = ft_set_get(sockets, i);
    if ((*probe)->ttl == ttl)
      break;
  }
  *end_probe = ft_set_get(sockets, i);
  for (; i < trace.nbr_total_probes; ++i, *end_probe = ft_set_get(sockets, i)) {
    if ((*end_probe)->ttl != ttl)
      break;
  }
}

uint64_t print_info_ttl(const uint8_t ttl) {
  t_probe* probe = NULL;
  t_probe* end_probe = NULL;
  const char* old_ip = "";
  bool end = false;
  char old_hostname[HOST_NAME_MAX] = {0};

  init_ptr(&probe, &end_probe, ttl);
  if (probe->sock_err)
    if (probe->sock_err->ee_type == ICMP_DEST_UNREACH && probe->sock_err->ee_code == ICMP_PORT_UNREACH)
      end = true;
  printf("%2u ", ttl);
  for (; probe != end_probe; probe++) {
    char* ip_str = inet_ntoa(probe->recv_addr.sin_addr);
    char hostname[HOST_NAME_MAX] = {0};
    ip_to_hostname(ip_str, hostname);
    const double rtt = calculate_rtt(probe->send_time, probe->recv_time);

    if (probe->received == false) {
      printf("* ");
      continue;
    }
    if (strcmp(hostname, old_hostname) != 0 || strcmp(ip_str, old_ip) != 0) {
      printf("%s ", ft_strlen(hostname) ? hostname : ip_str);
      printf("(%s) ", ip_str);
    }
    if (rtt < 0.0f || rtt > trace.waittime * 1000)
      printf("* ");
    else
      printf("%.3f ms ", rtt);
    old_ip = ip_str;
    strcpy(old_hostname, hostname);
  }
  printf("\n");
  fflush(NULL);
  if (end)
    return 2;
  return 0;
}

int64_t print_result(void) {
  for (uint64_t i = trace.first_ttl; i <= trace.max_ttl; ++i) {
    const int retval = print_info_ttl(i);
    if (retval == 2)
      break;
  }
  return 0;
}
