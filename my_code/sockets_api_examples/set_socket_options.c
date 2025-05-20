#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int socket(int domain, int type, int protocol);
int setsockopt(int sockfd, int level, int optname, const void *optval,
               socklen_t optlen);

int main(void) {
  struct addrinfo hints = {0};
  struct addrinfo *res = NULL;
  int sockfd;
  int err;

  err = getaddrinfo("www.example.com", "http", &hints, &res);
  if (err) {
    perror("getaddrinfo");
    exit(1);
  }

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) {
    perror("socket");
    exit(1);
  }

  /* From `man getsockopt` we can read (about SOL_SOCKET):
     When manipulating socket options, the level at which the option resides and
     the name of the option must be specified. To manipulate options at the
     sockets API level, level is specified as SOL_SOCKET. To manipulate options
     at any other level the protocol number of the appropriate protocol
     controlling the option is supplied.

     From `man 7 socket` we can read (about SO_REUSEADDR):
     Indicates that the rules used in validating addresses supplied in
     a bind(2) call should allow reuse of  local addresses.   For  AF_INET
     sockets this means that a socket may bind, except when there is an active
     listening socket bound to the address.  When the listening socket is bound
     to INADDR_ANY with a specific port  then  it is not possible to bind to
     this port for any local address.  Argument is an integer boolean flag.

  */
  int optval = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==
      -1) {
    perror("setsockopt");
    exit(1);
  }

  return 0;
}
