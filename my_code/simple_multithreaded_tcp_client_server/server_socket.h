#include <arpa/inet.h>
#include <errno.h>

#define LISTEN_BACKLOG 50

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

static inline void set_socket_timeouts(int sockfd) {
  struct timeval timeout;
  timeout.tv_sec = 2; // 2 seconds
  timeout.tv_usec = 2;

  // Set receive timeout
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  // Set send timeout
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}
