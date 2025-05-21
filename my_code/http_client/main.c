/* Here are our goals:
    -  We want to write a program which gets the address of a WWW site (e.g.
       httpstat.us) as the argument and fetches the document.
    -  The program outputs the document to stdout; the program uses TCP to
       connect to the HTTP server.
*/
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    puts("Usage: http_client <website url>");
    exit(1);
  }

  struct addrinfo hints = {0};
  struct addrinfo *res = NULL;
  int sockfd;
  int err;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  puts("Connecting...");

  err = getaddrinfo(argv[1], "http", &hints, &res);
  if (err) {
    perror("getaddrinfo");
    exit(1);
  }

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) {
    perror("socket");
    exit(1);
  }

  int optval = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==
      -1) {
    perror("setsockopt");
    exit(1);
  }

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    perror("connect");
    exit(1);
  }

  puts("Connected");
  puts("Sending http request...");

  const char http_request[256];
  sprintf((char *)http_request,
          "GET / HTTP/1.1\nHost: %s\nConnection: close\n\n", argv[1]);
  ssize_t send_bytes = send(sockfd, http_request, sizeof(http_request), 0);
  if (send_bytes > 0) {
    printf("Send %d bytes\n", (int)send_bytes);
  } else if (send_bytes == 0) {
    printf("Connection closed by peer\n");
    exit(1);
  } else {
    perror("send failed");
    exit(1);
  }

  puts("Receiving http response...");

  char buffer[1024];
  while (true) {
    ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if (recv_bytes > 0) {
      printf("%.*s", (int)recv_bytes, buffer);
    } else if (recv_bytes == 0) {
      printf("\nConnection closed by peer\n");
      break;
    } else {
      perror("recv failed");
      exit(1);
    }
  }

  close(sockfd);
  freeaddrinfo(res);

  return 0;
}
