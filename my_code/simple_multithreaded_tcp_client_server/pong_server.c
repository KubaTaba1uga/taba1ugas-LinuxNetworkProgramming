/* Here are our goals for the improved TCP server:
    -  We create a TCP socket using socket(), bind it to localhost, and
   listen().
    -  The server can handle multiple clients concurrently.
    -  We implement a simple dispatcher-worker architecture using threads:
        -  The dispatcher accepts new connections using accept().
        -  Each accepted connection is assigned to a separate worker thread.
    -  We introduce a timer mechanism:
        -  The timer periodically checks all active connections.
        -  If a connection is inactive or stale for too long, it is closed.
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
#include "server_socket.h"
#include "server_thread.h"

int main(void) {
  int sockfd;
  int err;

  puts("Creating socket...");
  init_task_queue();

  err = listen_on_ipv4_tcp_socket(net_addr, net_port, &sockfd);
  if (err) {
    printf("Listen error: %s\n", strerror(err));
    exit(err);
  }

  puts("Creating threads...");

  err = create_threads(4);
  if (err) {
    printf("Create threads error: %s\n", strerror(err));
    exit(err);
  }

  while (true) {
    puts("Waiting for new connection...");
    // Create new_fd
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1) {
      perror("accept");
      exit(1);
    }

    set_socket_timeouts(new_fd);

    err = insert_task((struct HandleConnectionTask){.accepted_fd = new_fd});
    if (err) {
      printf("Insert task error: %s\n", strerror(err));
      exit(err);
    }

    wakeup_threads();

    printf("Connection enqueued for %d\n", new_fd);
  }

  return 0;
}
