/* Here are our goals for the TCP client:
    -  We send a "ping" message to the server.
    -  We receive a "pong" message from the server.
    -  We print the received message to stdout.
  Tcp client takes number of secons so we can test server timeout.
  It is initially set for 2 seconds so values like 1 and 2 shouldn't trigger it.
*/
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

#define LISTEN_BACKLOG 50

int main(int argc, const char *argv[]) {
  struct addrinfo hints = {0};
  struct addrinfo *res = NULL;
  int sockfd;
  int err;

  if (argc != 2) {
    printf("Usage: ping_client <number of seconds to sleep>\n");
    exit(1);
  }

  uint32_t sleeping_time = atoi(argv[1]);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  char net_port_buf[64];
  memset(net_port_buf, 0, sizeof(net_port_buf));
  snprintf(net_port_buf, sizeof(net_port_buf) - 1, "%d", net_port);

  err = getaddrinfo(net_addr, net_port_buf, &hints, &res);
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

  int i = 0;
  while (++i) {
    const char *msg_a = "Hello, World!";
    const char *msg_b = "ping";
    const char *msg = msg_a;

    if (i % 2) {
      msg = msg_b;
    }

    /* MSG_NOSIGNAL means - Don’t send me SIGPIPE if I try to write to a closed
       socket. Just return -1 and set errno = EPIPE.
    */
    ssize_t write_bytes = send(sockfd, msg, strlen(msg), MSG_NOSIGNAL);
    /* puts("Wrote"); */
    if (write_bytes > 0) {
      printf("Write %d bytes: %s\n", (int)write_bytes, msg);
    } else if (write_bytes == 0) {
      printf("Connection closed by peer\n");
    } else {
      perror("send failed");
      exit(1);
    }

    if (i % 2) {
      char read_buf[1024];
      memset(read_buf, 0, sizeof(read_buf));
      ssize_t read_bytes =
          recv(sockfd, read_buf, sizeof(read_buf) - 1, MSG_NOSIGNAL);
      if (read_bytes > 0) {
        printf("Read %d bytes: %.*s\n", (int)read_bytes, (int)read_bytes,
               read_buf);
      } else if (read_bytes == 0) {
        printf("Connection closed by peer\n");
      } else {
        perror("receive failed");
        exit(1);
      }
    }

    sleep(sleeping_time);
  }

  close(sockfd);
  freeaddrinfo(res);

  return 0;
}
