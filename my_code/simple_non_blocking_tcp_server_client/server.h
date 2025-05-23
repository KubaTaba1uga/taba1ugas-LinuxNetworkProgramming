#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50
#define TIMEOUT 2

#define PRINTF_CONN(conn)                                                      \
  printf("Connection { conn_i=%d, timer_i=%d }\n", conn->conn->fd,             \
         conn->timer->fd);

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

static inline void delete_connection(int *poll_buf_len, struct pollfd *poll_buf,
                                     struct pollfd *del) {
  if (*poll_buf_len == 0) {
    return;
  }

  printf("Deleting connection %d ...\n", del->fd);

  int i = *poll_buf_len;
  while (i-- > 0) {
    if (&poll_buf[i] == del) {
      break;
    }
  }

  if (i > 0) {
    printf("%d", poll_buf[i].fd);
    close(poll_buf[i].fd);
    poll_buf[i] =
        poll_buf[*poll_buf_len - 1]; // Replace with the last descriptor
    *poll_buf_len -= 1;              // Reduce the total count
  }
}

static inline int create_non_blocking_timer(void) {
  struct itimerspec timer_value;

  int sockfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);

  timer_value.it_value.tv_sec = TIMEOUT;
  timer_value.it_value.tv_nsec = 0;
  timer_value.it_interval.tv_sec = 0;
  timer_value.it_interval.tv_nsec = 0;

  errno = 0;
  if (timerfd_settime(sockfd, 0, &timer_value, NULL) != 0) {
    return -errno;
  };

  return sockfd;
}

static inline int refresh_timer(int sockfd) {
  struct itimerspec timer_value;

  timer_value.it_value.tv_sec = TIMEOUT;
  timer_value.it_value.tv_nsec = 0;
  timer_value.it_interval.tv_sec = 0;
  timer_value.it_interval.tv_nsec = 0;

  errno = 0;
  if (timerfd_settime(sockfd, 0, &timer_value, NULL) != 0) {
    return -errno;
  }
  return 0;
}
