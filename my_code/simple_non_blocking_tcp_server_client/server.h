#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LISTEN_BACKLOG 50

int fcntl(int fd, int cmd, ... /* arg */);

int set_non_blocking(int socket) {
  int flags = fcntl(socket, F_GETFL, 0);
  if (flags == -1) {
    return errno;
  }

  if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1) {
    return errno;
  }

  return 0;
}

static inline int listen_on_ipv4_tcp_socket(const char *addr, int port,
                                            int *socket_fd) {
  int sockfd;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    return errno;
  }

  int optval = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==
      -1) {
    return errno;
  }

  struct sockaddr_in my_addr;
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  inet_pton(AF_INET, addr, &my_addr.sin_addr);

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
    return errno;
  }

  if (listen(sockfd, LISTEN_BACKLOG) == -1) {
    return errno;
  }

  *socket_fd = sockfd;

  return 0;
}

static inline int recv_non_block(uint32_t buf_len, char *buf, int socket_fd) {
  errno = 0;
  memset(buf, 0, buf_len);
  int received_bytes = recv(socket_fd, buf, buf_len - 1, 0);
  if (received_bytes < 0) {
    return -errno;
  } else if (received_bytes > 0) {
    printf("Read %d bytes from %d: %s\n", received_bytes, socket_fd, buf);
  }

  return received_bytes;
}

static inline int send_non_block(int socket_fd) {
  errno = 0;
  int sent_bytes = send(socket_fd, "pong", strlen("pong"), 0);
  if (sent_bytes < 0) {
    return -errno;
  } else if (sent_bytes > 0) {
    printf("Write %d bytes from %d: %s\n", (int)strlen("pong"), socket_fd,
           "pong");
  }

  return sent_bytes;
}
