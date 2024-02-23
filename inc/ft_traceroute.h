//
// Created by loumouli on 2/22/24.
//

#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include "libft.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

// PORT LIKELY UNUSED
#define TC_PORT 33434

// Which protocol used to send for probes
typedef enum {
  E_ICMP = 1 << 0, // Use ICMP request
  E_UDP = 1 << 1, // Use UDP packet
} E_PROT;

typedef struct {
  uint64_t ttl; // current ttl (from 1 to ttl max)
  uint64_t first_ttl; // first ttl to use(default to 1)
  uint64_t ttl_max; // max ttl (default to 30)
  uint16_t port; // port used to send the udp packet (default to TC_PORT)
  uint64_t waittime; // time in seconds to wait for a ICMP response
  E_PROT prot; // protocul used (default to E_UDP)
  int sck; // socket to send the probe packet
  int icmp_sck; // socket used to receive ICMP response
  struct sockaddr_in dest;
} t_tc;

#endif // FT_TRACEROUTE_H
