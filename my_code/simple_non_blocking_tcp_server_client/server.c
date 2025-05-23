/* Here are our goals for the improved single-threaded TCP server:
    -  We create a TCP socket using socket(), bind it to localhost, and
   listen().
    -  We use poll() to manage multiple client connections without threads:
        -  The server tracks the listening socket and all accepted client
   sockets.
        -  When poll() indicates a client socket is readable, we handle incoming
   data.
    -  For each client connection, we track its last activity time.
    -  We use timerfd to implement a periodic timeout mechanism:
        -  A timerfd is added to the poll set.
        -  On expiration, we scan all connections for inactivity.
        -  Stale (inactive) client connections are closed.
    -  The server responds to "ping" with "pong", and ignores other messages.
*/
#include <arpa/inet.h>
#include <assert.h>
#include <bits/time.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "server.h"
#include "server_conn.h"
#include "server_fd.h"

int main(void) {
  struct sockaddr client_addr;
  socklen_t client_addr_size = sizeof(client_addr);
  int socket_fd;
  int err;

  puts("Creating socket...");

  err = listen_on_ipv4_tcp_socket(net_addr, net_port, &socket_fd);
  if (err) {
    printf("Listen error: %s\n", strerror(err));
    exit(err);
  }

  set_non_blocking(socket_fd);

  init_fds_queue();
  init_cons_queue();
  /*
     The server socket is the first entry in the pollfd array, which is
     dynamically updated as clients connect or disconnect.
  */
  insert_fd(&(struct pollfd){.fd = socket_fd, .events = POLLIN, .revents = 0});

  while (true) {
    puts("Processing event loop...");

    int activity = mypoll();
    if (activity < 0) {
      perror("mypoll");
      break;
    }

    // Add new connection
    if (fds_buf.fds[0].revents & POLLIN) {
      puts("Creating new connection...");
      int sock_client =
          accept(fds_buf.fds[0].fd, &client_addr, &client_addr_size);
      int fd_timer = create_non_blocking_timer();

      err = insert_fd(
          &(struct pollfd){.fd = sock_client, .events = POLLIN, .revents = 0});
      assert(err == 0);

      insert_fd(
          &(struct pollfd){.fd = fd_timer, .events = POLLIN, .revents = 0});
      assert(err == 0);

      err = insert_connection(&(struct Connection){
          .timer_fd = fd_timer,
          .conn_fd = sock_client,
      });
      assert(err == 0);
    }

    // Process exsisting connections
    struct Connection *next, *conn = STAILQ_FIRST(&conns_queue);
    while (conn) {
      next = STAILQ_NEXT(conn, _next);
      bool do_delete = false;

      struct pollfd *connfd = find_fd(conn->conn_fd);
      struct pollfd *timerfd = find_fd(conn->timer_fd);
      assert(connfd != NULL);
      assert(timerfd != NULL);
      if (connfd->revents & (POLLHUP | POLLIN | POLLERR)) {
        puts("Request fired!!!");
        uint32_t buf_size = 1024;
        char buf[buf_size];

        int received_bytes = recv_non_block(buf_size, buf, conn->conn_fd);
        if (received_bytes < 0) {
          perror("recv_non_block");
          return errno;
        } else if (received_bytes == 0) {
          do_delete = true;
        } else if (strstr(buf, "ping")) {
          int sent_bytes = send_non_block(conn->conn_fd);
          if (sent_bytes < 0) {
            perror("send_non_block");
            return errno;
          } else if (sent_bytes == 0) {
            do_delete = true;
          }
        }
        refresh_timer(conn->timer_fd);
      }
      if (timerfd->revents & (POLLHUP | POLLIN | POLLERR)) {
        puts("Timer fired!!!!");
        do_delete = true;
      }

      if (do_delete) {
        remove_fd(conn->conn_fd);
        remove_fd(conn->timer_fd);
        STAILQ_REMOVE(&conns_queue, conn, Connection, _next);
        free(conn);
      }

      conn = next;
    }
  }

  close(socket_fd);

  return 0;
}
