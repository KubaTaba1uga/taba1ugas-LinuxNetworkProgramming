#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int socket(int domain, int type, int protocol);
int setsockopt(int sockfd, int level, int optname, const void *optval,
               socklen_t optlen);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int main(void) {
  int sockfd;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

  struct sockaddr_in my_addr;
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(3490);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
    perror("bind");
    exit(1);
  }

  const uint32_t buffer_size = 1024;
  char buffer[buffer_size];
  int32_t received_bytes;

  struct sockaddr_storage sender_addr;
  socklen_t sender_addr_len = sizeof(sender_addr);
  while (true) {
    memset(buffer, 0, buffer_size);

    received_bytes =
        recvfrom(sockfd, buffer, buffer_size - 1, MSG_NOSIGNAL,
                 (struct sockaddr *)&sender_addr, &sender_addr_len);
    if (received_bytes > 0) {
      printf("Read %d bytes: %s\n", received_bytes, buffer);
    } else if (received_bytes == 0) {
      printf("Connection closed by peer\n");
      continue;
    } else {
      perror("recvfrom");
      exit(1);
      continue;
    }

    if (strstr(buffer, "ping")) {
      received_bytes = sendto(sockfd, "pong", strlen("pong"), MSG_NOSIGNAL,
                              (struct sockaddr *)&sender_addr, sender_addr_len);
      if (received_bytes > 0) {
        printf("Write %d bytes: %s\n", received_bytes, "pong");
      } else if (received_bytes == 0) {
        printf("Connection closed by peer\n");
        continue;
      } else {
        perror("sendto");
        exit(1);
        continue;
      }
    }
  }

  close(sockfd);

  return 0;
}
