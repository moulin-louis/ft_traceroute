#include "ft_traceroute.h"

int main(int ac, char** av) {
  struct sockaddr_in dest;
  struct sockaddr_in rcv;
  socklen_t len_recv;
  if (ac == 1) {
    fprintf(stderr, "Wrong nbr of args\n");
    return 1;
  }
  int sckfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sckfd == -1)
    return 1;
  int ttl = 1;
  if (setsockopt(sckfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
    fprintf(stderr, "error setsockopt\n");
    perror("setsockopt");
    return 1;
  }
  ft_memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(TC_PORT);
  dest.sin_addr.s_addr = inet_addr(av[1]);
  int retval = sendto(sckfd, "toto", 4, 0, (struct sockaddr*)&dest, sizeof(dest));
  if (retval == -1) {
    fprintf(stderr, "error sendto\n");
    perror("sendto");
    return 1;
  }
  char buf[2048];
  ft_memset(buf, 0, sizeof(buf));
  printf("%d bytes send\n", retval);

  retval = recvfrom(sckfd, buf, 2048, 0, (struct sockaddr*)&rcv, &len_recv);
  if (retval == -1) {
    fprintf(stderr, "error recvfrom\n");
    perror("recvfrom");
    return 1;
  }
  printf("%d bytes read\n", retval);
  close(sckfd);
  return 0;
}
