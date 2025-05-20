#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(void) {
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo, *p; // will point to the results

  memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;      // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;      // fill in my IP for me

  if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  // servinfo now points to a linked list of 1 or more struct addrinfos

  // ... do everything until you don't need servinfo anymore ....
  for (p = servinfo; p != NULL; p = p->ai_next) {
    char ipstr[INET6_ADDRSTRLEN];
    const char *ipver;
    void *addr;
    int port;

    // IPv4
    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      port = ntohs(ipv4->sin_port);
      ipver = "IPv4";
    }
    // IPv6
    else if (p->ai_family == AF_INET6) {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      port = ntohs(ipv6->sin6_port);
      ipver = "IPv6";
    } else {
      continue;
    }

    // Convert IP to string
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    printf("%s: %s:%d\n", ipver, ipstr, port);
  }

  freeaddrinfo(servinfo); // free the linked-list

  return 0;
}
