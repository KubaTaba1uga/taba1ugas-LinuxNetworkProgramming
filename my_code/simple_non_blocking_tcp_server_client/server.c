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
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "server.h"

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

  struct pollfd fds[16];
  memset(&fds, 0, sizeof(fds));

  init_cons_queue();
  /*
     The server socket is the first entry in the pollfd array, which is
     dynamically updated as clients connect or disconnect.
  */
  fds[0].fd = socket_fd;  // Monitor server socket
  fds[0].events = POLLIN; // Monitor for incoming connections

  int nfds = 1; // Start with two monitored socket

  while (true) {
    puts("Processing event loop...");

    int activity = poll(fds, nfds, -1); // Wait indefinitely
    if (activity < 0) {
      perror("poll");
      break;
    }

    // Add new connection
    if (fds[0].revents & POLLIN) {
      puts("Creating new connection...");
      int sock_client = accept(fds[0].fd, &client_addr, &client_addr_size);
      if (nfds < 15) {
        fds[nfds].fd = sock_client;
        fds[nfds].events = POLLIN; // Monitor for incoming data
        printf("Created new connection %d\n", fds[nfds].fd);
        nfds++;
        fds[nfds].fd = create_non_blocking_timer(); // Monitor connection time
        fds[nfds].events = POLLIN; // Monitor for incoming timer fire
        printf("Created new timer %d for %d\n", fds[nfds].fd, fds[nfds - 1].fd);
        insert_connection((struct Connection){
            .timer = &fds[nfds],
            .conn = &fds[nfds - 1],
        });

        nfds++;
      }
    }

    struct Connection *next, *conn = STAILQ_FIRST(&conns_queue);
    while (conn) {
      next = STAILQ_NEXT(conn, _next);
      bool do_delete = false;
      PRINTF_CONN(conn);

      if (conn->conn->revents & (POLLHUP | POLLIN | POLLERR)) {
        uint32_t buf_size = 1024;
        char buf[buf_size];

        int received_bytes = recv_non_block(buf_size, buf, conn->conn->fd);
        if (received_bytes < 0) {
          perror("recv_non_block");
          return errno;
        } else if (received_bytes == 0) {
          do_delete = true;
        } else if (strstr(buf, "ping")) {
          int sent_bytes = send_non_block(conn->conn->fd);
          if (sent_bytes < 0) {
            perror("send_non_block");
            return errno;
          } else if (sent_bytes == 0) {
            do_delete = true;
          }
        }
      }

      if (conn->timer->revents & (POLLHUP | POLLIN | POLLERR)) {
        puts("Timer fired!!!!");
        do_delete = true;
      }

      if (do_delete) {
        delete_connection(&nfds, fds, conn->conn);
        delete_connection(&nfds, fds, conn->timer);
        STAILQ_REMOVE(&conns_queue, conn, Connection, _next);
        free(conn);
      }

      conn = next;
    }
  }

  close(socket_fd);

  return 0;
}
