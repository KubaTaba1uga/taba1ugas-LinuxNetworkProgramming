/* Here are our goals for the TCP server:
    -  We receive a "ping" message from the client.
    -  We send back a "pong" message to the client.
*/
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

#define LISTEN_BACKLOG 50

int main(void) {
  int sockfd;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
  my_addr.sin_port = htons(net_port);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  inet_pton(AF_INET, net_addr, &my_addr.sin_addr);

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
    perror("bind");
    exit(1);
  }

  if (listen(sockfd, LISTEN_BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  while (true) {
    // Create new_fd
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1) {
      perror("accept");
      exit(1);
    }

    // Use new_fd
    char buffer[1024];
    do {
      memset(buffer, 0, sizeof(buffer));

      ssize_t read_bytes = recv(new_fd, buffer, sizeof(buffer) - 1, 0);
      if (read_bytes > 0) {
        printf("Read %d bytes: %.*s\n", (int)read_bytes, (int)read_bytes,
               buffer);
        if (strstr(buffer, "ping")) {
          ssize_t write_bytes = send(new_fd, "pong", strlen("pong"), 0);
          /* puts("Wrote"); */
          if (write_bytes > 0) {
            printf("Write %d bytes: %s\n", (int)write_bytes, "pong");
          } else if (write_bytes == 0) {
            printf("Connection closed by peer\n");
          } else {
            perror("send failed");
            exit(1);
          }
        }
      } else if (read_bytes == 0) {
        printf("Connection closed by peer\n");
        break;
      } else {
        perror("recv failed");
        continue;
      }

      buffer[read_bytes] = 0;

    } while (!strstr(buffer, "quit"));

    // Cleanup new fd
    close(new_fd);
  }

  close(sockfd);

  return 0;
}
