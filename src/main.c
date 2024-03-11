#include "ft_traceroute.h"

t_tc trace;
volatile bool timeout;

int64_t send_probe(const uint16_t port, const uint64_t ttl, const uint64_t id) {
  struct iphdr* ip_hdr =  (struct iphdr*)trace.packet;
  // struct udphdr* udp_hdr = (struct udphdr*)(packet + sizeof(struct ip));

  if (change_ttl(trace.sck, ttl))
    return 1;
  ip_hdr->id = htons(id);
  ip_hdr->ttl = ttl;
  ip_hdr->check = checksum((uint16_t*)ip_hdr, sizeof(struct iphdr));
  trace.dest.sin_port = port;
  const int64_t ret = sendto(trace.sck, trace.packet, trace.size_packet, 0, (struct sockaddr*)&trace.dest, sizeof(trace.dest));
  if (ret == -1) {
    perror("sendto probe");
    return 1;
  }
  return 0;
}

int64_t grab_packet(uint8_t* buf, struct sockaddr_in* src) {
  socklen_t len = sizeof(struct sockaddr_in);

  const int64_t retval = recvfrom(trace.icmp_sck, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)src, &len);
  if (retval == -1) {
    if (errno == EWOULDBLOCK || errno == EAGAIN)
      return 2;
    perror("recvfrom");
    return 1;
  }
  return 0;
}

int64_t print_result(void) {
  char hostname_str[HOST_NAME_MAX] = {0};
  const t_probe* probe = ft_set_get(trace.probes, 0);
  char* ip_str = inet_ntoa(probe->src.sin_addr);

  ip_to_hostname(ip_str, hostname_str);

  printf(" %lu  ", trace.ttl);
  printf("%s ", ft_strlen(hostname_str) ? hostname_str : ip_str);
  printf("(%s) ", ip_str);
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    probe = ft_set_get(trace.probes, idx);
    if (probe->rtt == 0) {
      printf("* ");
      continue;
    }
    printf("%.3f ms ", probe->rtt);
  }
  printf("\n");
  probe = ft_set_get(trace.probes, 0);
  if (probe->icmphdr.type == ICMP_DEST_UNREACH && probe->icmphdr.code == ICMP_PORT_UNREACH)
    return 0;
  return 2;
}

int main(int ac, const char** av) {
  if (ac == 1) {
    fprintf(stderr, "Usage: ft_traceroute [option] host\n");
    return 1;
  }
  if (init_tc(ac, av))
    return 1;
  //print source address
  printf("traceroute to %s (%s), %ld hops max, %ld byte packets\n", av[1], inet_ntoa(trace.dest.sin_addr),
         trace.ttl_max, trace.size_probe);
  signal(SIGTERM, handle_quit);
  signal(SIGINT, handle_quit);
  // send each probes
  for (uint64_t i = trace.first_ttl; i <= trace.ttl_max; ++i) {
    const uint16_t port = htons(trace.port + i);
    for (uint64_t j = 0; j < trace.nbr_probes; ++j) {
      if (send_probe(port, i, i * j)) {
        cleanup();
        return 1;
      }
    }
  }
  signal(SIGALRM, handle_quit);
  alarm(trace.waittime);
  while (timeout == false) {
    uint8_t packet[sizeof(struct ip) + sizeof(struct icmp)];
    struct sockaddr_in src;
    ft_memset(packet, 0, sizeof(packet));
    const uint64_t retval = grab_packet(packet, &src);
    if (retval == 1) {
      cleanup();
      return 1;
    }
    if (retval == 2)
      continue;
    printf("identification %d\n", ntohs(((struct ip*)packet)->ip_id));
  }
  cleanup();
  return 0;
}
