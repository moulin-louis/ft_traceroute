#include "ft_traceroute.h"

t_opt trace;
volatile bool timeout;
t_set* sockets;
fd_set readfds;

uint64_t send_probe(t_probe* probe) {
  uint8_t packet[trace.packet_len];

  ft_memset(packet, 0, sizeof(packet));
  const int64_t retval =
    sendto(probe->sck, packet, sizeof(packet), 0, (struct sockaddr*)&probe->dest, sizeof(probe->dest));
  if (retval == -1) {
    perror("sendto");
    return 1;
  }
  clock_gettime(CLOCK_MONOTONIC, &probe->send_time);
  return 0;
}

uint64_t handle_error(t_probe* probe) {
  probe->msg.msg_name = NULL;
  probe->msg.msg_namelen = 0;
  probe->msg.msg_iov = NULL;
  probe->msg.msg_iovlen = 0;
  probe->msg.msg_flags = 0;
  probe->msg.msg_control = &probe->packet;
  probe->msg.msg_controllen = sizeof(probe->packet);

  const int64_t retval = recvmsg(probe->sck, &probe->msg, MSG_ERRQUEUE);
  if (retval == -1) {
    perror("recvmsg");
    return 1;
  }
  for (probe->cmsg = CMSG_FIRSTHDR(&probe->msg); probe->cmsg; probe->cmsg = CMSG_NXTHDR(&probe->msg, probe->cmsg)) {
    if (probe->cmsg->cmsg_level != SOL_IP) // We only want IP level errors
      continue;
    if (probe->cmsg->cmsg_type != IP_RECVERR) // We only want IP_RECVERR errors
      continue;
    probe->sock_err = (struct sock_extended_err*)CMSG_DATA(probe->cmsg);
    if (probe->sock_err == NULL) // We need data to continue
      continue;
    if (probe->sock_err->ee_origin != SO_EE_ORIGIN_ICMP) // We only want ICMP errors
      continue;
    probe->recv_addr = *(struct sockaddr_in*)SO_EE_OFFENDER(probe->sock_err);
    if (probe->sock_err->ee_type == ICMP_TIME_EXCEEDED && probe->sock_err->ee_code == ICMP_EXC_TTL) {
      probe->received = true;
      clock_gettime(CLOCK_MONOTONIC, &probe->recv_time);
      return 1; // TTL exceeded keep going
    }
    if (probe->sock_err->ee_type == ICMP_DEST_UNREACH && probe->sock_err->ee_code == ICMP_PORT_UNREACH) {
      probe->received = true;
      clock_gettime(CLOCK_MONOTONIC, &probe->recv_time);
      return 2; // port unreachable, we are done
    }
  }
  return 0; // Other error, we stop
}


uint64_t grab_answer(t_probe* probe) {
  socklen_t len = sizeof(probe->recv_addr);
  int64_t retval =
    recvfrom(probe->sck, probe->packet, sizeof(probe->packet), 0, (struct sockaddr*)&probe->recv_addr, &len);
  if (retval == -1) {
    // handle potential ICMP error
    return handle_error(probe);
  }
  probe->received = true;
  clock_gettime(CLOCK_MONOTONIC, &probe->recv_time);
  return 0;
}

int main(int ac, char** av) {
  struct timeval timeout;

  FD_ZERO(&readfds);
  if (ac == 1) {
    fprintf(stderr, "Usage: ft_traceroute [option] host\n");
    return 1;
  }
  // Init option and sockets set
  if (init_tc(ac, av))
    return 1;
  printf("traceroute to %s (%s), %ld hops max, %ld byte packets\n", av[1], inet_ntoa(trace.ip_addr.sin_addr),
         trace.max_ttl, trace.packet_len);
  timeout.tv_sec = trace.waittime;
  timeout.tv_usec = 0;
  signal(SIGTERM, handle_quit);
  signal(SIGINT, handle_quit);
  for (uint64_t i = trace.first_ttl - 1; i < trace.max_ttl; ++i) {
    const uint16_t port = htons(trace.port + i);
    for (uint64_t j = 0; j < trace.nquerries; ++j) {
      t_probe* probe = ft_set_get(sockets, i * trace.nquerries + j);
      probe->dest.sin_port = port;
      if (send_probe(probe)) {
        cleanup();
        return 1;
      }
    }
  }
  while (true) {
    const int64_t retval = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
    if (retval == -1) {
      perror("select");
      cleanup();
      return 1;
    }
    if (retval == 0) // timeout reached
      break;
    for (uint64_t i = 0; i < trace.nbr_total_probes; ++i) {
      t_probe* probe = ft_set_get(sockets, i);
      if (FD_ISSET(probe->sck, &readfds) == false)
        continue;
      uint64_t retval = grab_answer(probe);
      if (retval == 3 || retval == 1)
        break;
    }
    // reset the readfds
    FD_ZERO(&readfds);
    for (uint64_t i = 0; i < trace.nbr_total_probes; ++i) {
      const t_probe* probe = ft_set_get(sockets, i);
      if (probe->received == false)
        FD_SET(probe->sck, &readfds);
    }
  }
  print_result();
  cleanup();
  return 0;
}
