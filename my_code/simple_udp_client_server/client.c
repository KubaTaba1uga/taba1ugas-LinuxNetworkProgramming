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
  my_addr.sin_port = htons(9043);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
    perror("bind");
    exit(1);
  }
  struct addrinfo hints = {0};
  struct addrinfo *receiver_addr = NULL;
  int err;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  /* Now you can do:
     nc -l -p 8080
     wsad
     quit
   */
  err = getaddrinfo("127.0.0.1", "3490", &hints, &receiver_addr);
  if (err) {
    perror("getaddrinfo");
    exit(1);
  }

  const uint32_t buffer_size = 1024;
  char buffer[buffer_size];
  uint32_t received_bytes;
  uint32_t sent_bytes;

  int i = 0;
  while (++i) {
    const char *msg = "Hello, World!";
    const char *msg_b = "ping";
    if (i % 2) {
      msg = msg_b;
    }

    sent_bytes = sendto(sockfd, msg, strlen(msg), MSG_NOSIGNAL,
                        receiver_addr->ai_addr, receiver_addr->ai_addrlen);
    if (sent_bytes > 0) {
      printf("Write %d bytes: %.*s\n", (int)strlen(msg), (int)strlen(msg), msg);
    } else if (sent_bytes == 0) {
      printf("Connection closed by peer\n");
    } else {
      perror("recvfrom");
      exit(1);
    }

    if (i % 2) {
      memset(buffer, 0, buffer_size);

      received_bytes =
          recvfrom(sockfd, buffer, buffer_size - 1, MSG_NOSIGNAL, NULL, NULL);
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
    }

    sleep(2);
  }

  freeaddrinfo(receiver_addr);
  close(sockfd);

  return 0;
}
