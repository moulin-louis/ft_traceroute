//
// Created by loumouli on 2/22/24.
//

#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include "libft.h"

#include <argp.h>
#include <arpa/inet.h>
#include <bits/local_lim.h>
#include <bsd/string.h>
#include <errno.h>
#include <ifaddrs.h>
#include <linux/errqueue.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define DEFAULT_FIRST_TLL 1
#define DEFAULT_MAX_TTL 30
#define DEFAULT_SIZE_PACKET 60
#define DEFAULT_NBR_PROBES 3
#define DEFAULT_UDP_PORT 33434
#define DEFAULT_WAITTIME 5
#define DEFAULT_WAIT_PROBE 0

typedef struct {
  int sck; // socket to send the probe packet and receive error message
  uint8_t ttl; // current ttl of the probe
  struct sockaddr_in dest; // dest address info
  struct sockaddr_in recv_addr; // address of the "responder"
  struct timespec send_time; // time when the packet was sent
  struct timespec recv_time; // time when the packet was received
  struct msghdr msg; // message received
  uint8_t packet[1024]; // packet received
  struct cmsghdr* cmsg; // control message
  struct sock_extended_err* sock_err; // error message
  bool received; // true if the probe was received
} t_probe;

typedef struct {
  uint64_t first_ttl; // first ttl to use (default to 1)
  uint64_t max_ttl; // max ttl (default to 30)
  uint64_t packet_len; // size of the probe (default to 60)
  uint64_t nquerries; // number of probes to send for each ttl (default to 3)
  uint16_t port; // port used to send the udp packet (default to TC_PORT)
  uint64_t waittime; // time in seconds to wait for a ICMP response (default to 5)
  uint64_t sendwait; // tine in second to wait between probe sending (default to 0)
  uint64_t nbr_total_probes; // total number of probes to send
  uint8_t hostname[HOST_NAME_MAX]; // hostname to trace
  struct sockaddr_in ip_addr; // ip address info
} t_opt;

extern t_opt trace;
extern t_set* sockets;
extern fd_set readfds;

// Init a t_tc struct based on argc/argv
int64_t init_tc(int ac, char** av);

// Use argp to parse command line options
int32_t parse_opt(int ac, char** av);

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

// calculate rtt between 2 timeval
double calculate_rtt(struct timespec start_time, struct timespec end_time);

// print the result of the trace
int64_t print_result(void);

#endif // FT_TRACEROUTE_H
