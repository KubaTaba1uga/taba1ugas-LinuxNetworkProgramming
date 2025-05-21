#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int socket(int domain, int type, int protocol);

int main(void) {
  struct addrinfo hints = {0};
  struct addrinfo *res = NULL;
  int sockfd;
  int err;

  err = getaddrinfo("www.example.com", "http", &hints, &res);
  if (err) {
    perror("getaddrinfo");
    exit(1);
  }

  /*
    This is the same as:
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
  */
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) {
    perror("socket");
    exit(1);
  }

  freeaddrinfo(res);

  return 0;
}
