//
// Created by loumouli on 3/4/24.
//

#include "ft_traceroute.h"

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
}

uint64_t init_probes() {
  trace.probes = ft_set_new(sizeof(t_probe));
  if (trace.probes == NULL)
    return 1;
  if (ft_set_reserve(trace.probes, trace.nbr_probes))
    return 1;
  for (uint64_t idx = 0; idx < trace.nbr_probes; ++idx) {
    t_probe probe;
    ft_memset(&probe, 0, sizeof(probe));
    if (ft_set_push(trace.probes, &probe, sizeof(probe)))
      return 1;
  }
  return 0;
}

uint64_t init_packet(void) {
  trace.size_packet = sizeof(struct iphdr) + sizeof(struct udphdr) + trace.size_probe;
  trace.packet = calloc(1, trace.size_packet);
  if (trace.packet == NULL) {
    perror("malloc");
    return 1;
  }
  struct iphdr* ip_hdr =  (struct iphdr*)trace.packet;
  ip_hdr->version = 4;
  ip_hdr->ihl = 5;
  ip_hdr->tos = 0;
  ip_hdr->tot_len = trace.size_packet;
  ip_hdr->frag_off = 0;
  ip_hdr->protocol = IPPROTO_UDP;
  ip_hdr->saddr = trace.dest.sin_addr.s_addr;
  ip_hdr->daddr = trace.dest.sin_addr.s_addr;
  return 0;
}

//A function that return my address ip
in_addr_t get_my_ip(void) {
  in_addr_t result = 0;
  struct ifaddrs* ifaddr;
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return 0;
  }
  for (const struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    if (ifa->ifa_addr->sa_family == AF_INET) {
      const struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
      if (addr->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
        result = addr->sin_addr.s_addr;
        break;
      }
    }
  }
  freeifaddrs(ifaddr);
  return result;
}

int64_t init_tc(const int ac, const char** av) {
  (void)ac;
  trace.sck = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  if (trace.sck == -1) {
    perror("socket");
    return 1;
  }
  trace.icmp_sck = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (trace.icmp_sck == -1) {
    perror("socket_icmp");
    return 1;
  }
  if (setsockopt(trace.sck, IPPROTO_IP, IP_HDRINCL, (uint64_t[]){1}, sizeof(uint64_t)) == -1) {
    perror("setsockopt");
    return 1;
  }
  // setsockopt(trace.icmp_sck, SOL_IP, );
  init_default();
  //init dest
  hostname_to_sockaddr(av[1], &trace.dest);
  trace.dest.sin_family = AF_INET;
  //init src
  trace.src.sin_family = AF_INET;
  trace.src.sin_addr.s_addr = get_my_ip();
  if (init_probes())
    return 1;
  if (init_packet())
    return 1;
  return change_ttl(trace.sck, 1);
}
