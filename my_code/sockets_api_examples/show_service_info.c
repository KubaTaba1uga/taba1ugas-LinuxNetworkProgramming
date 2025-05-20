#include <netdb.h>
#include <stdio.h>

/*
  All possible protocols can be found in /etc/services.
 */

int main(void) {
  struct servent *service;
  service = getservbyname("http", "tcp");
  if (service) {
    printf("Protocol number for HTTP/TCP: %d\n",
           // Convert values between host and network byte order.
           ntohs(service->s_port));
  }

  service = getservbyname("git", "tcp");
  if (service) {
    printf("Protocol number for GIT/TCP: %d\n", ntohs(service->s_port));
  }

  service = getservbyname("rtsp", "udp");
  if (service) {
    printf("Protocol number for RTSP/UDP: %d\n", ntohs(service->s_port));
  }

  return 0;
}
