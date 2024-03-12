//
// Created by loumouli on 2/22/24.
//

#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include "libft.h"

#include <arpa/inet.h>
#include <bits/local_lim.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <ifaddrs.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/udp.h>

#define DEFAULT_FIRST_TLL 1
#define DEFAULT_MAX_TTL 30
#define DEFAULT_SIZE_PACKET 60
#define DEFAULT_NBR_PROBES 3
#define DEFAULT_UDP_PORT 33434
#define DEFAULT_PROT E_UDP
#define DEFAULT_WAITTIME 5
#define DEFAULT_WAIT_PROBE 0

// Which protocol used to send for probes
typedef enum {
  E_ICMP = 1 << 0, // Use ICMP request
  E_UDP = 1 << 1, // Use UDP packet
} E_PROT;

typedef struct {
  int sck;  //socket to send the probe packet and receive error message
  uint8_t ttl; // current ttl of the probe
  struct sockaddr_in dest; // dest address info
  struct sockaddr_in recv_addr; // address of the "responder"
  struct timespec send_time; // time when the packet was sent
  struct timespec recv_time; // time when the packet was received
  double rtt; //round trip time in ms
} t_probe;

typedef struct {
  uint64_t ttl; // current ttl (from 1 to `ttl_max`)
  uint64_t first_ttl; // first ttl to use (default to 1)
  uint64_t ttl_max; // max ttl (default to 30)
  uint64_t size_probe; // size of the probe (default to 60)
  uint64_t nbr_probes; // number of probes to send for each ttl (default to 3)
  uint16_t port; // port used to send the udp packet (default to TC_PORT)
  uint64_t waittime; // time in seconds to wait for a ICMP response (default to 5)
  uint64_t wait_prob; // tine in second to wait between probe sending (default to 0)
  uint64_t nbr_total_probes; // total number of probes to send
  E_PROT prot; // protocul used (default to E_UDP)
} t_opt;

extern t_opt trace;
extern volatile bool timeout;
extern t_set* sockets;

// Init a t_tc struct based on argc/argv
int64_t init_tc(const int ac, const char** av);

// cleanup the trace struct (close both socket)
void cleanup(void);

// handle signal (SIGTERM, SIGQUIT, SIGALARM)
void handle_quit(int sig);

// Convert an ip string to an hostname
int64_t ip_to_hostname(const char* ip, char* result_str);

// Change the TTL of a given socket to a given ttl
int64_t change_ttl(int sock, uint64_t new_ttl);

// Convert a given hostname in ASCII to a already-allocated sockaddr
int32_t hostname_to_sockaddr(const char* hostname, void* result_ptr);

//calculate rtt between 2 timeval
double calculate_rtt(struct timeval start_time, struct timeval end_time);

// Calculate the checksum of a given ip header
uint16_t checksum_ip(uint16_t* addr, uint64_t count);

#endif // FT_TRACEROUTE_H
