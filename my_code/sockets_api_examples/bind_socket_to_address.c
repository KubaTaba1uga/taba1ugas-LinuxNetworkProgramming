#include <netdb.h>
#include <netinet/in.h>
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

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

  /* From `man 2 socket` we can read (about AF_INET):
     AF_INET      IPv4 Internet protocols                    ip(7)

     From `man 7 ip` we can read (about INADDR_ANY):
     When  a  process  wants to receive new incoming packets or connections,
     it should bind a socket to a local interface address using bind(2).  In
     this case, only one IP socket may be bound to any given local (address,
     port) pair.  When INADDR_ANY  is  specified  in  the  bind  call, the
     socket will be bound to all local interfaces.  When listen(2) is called on
     an unbound socket, the socket is automatically bound to a random free port
     with the local address  set  to INADDR_ANY.  When connect(2) is called on
     an unbound socket, the socket is automatically bound to a random free port
     or to a usable shared port with the local address set to INADDR_ANY.
     ...
     There are several special addresses: INADDR_LOOPBACK (127.0.0.1) always
     refers to the local host  via  the  loopback device;  INADDR_ANY  (0.0.0.0)
     means any address for binding; INADDR_BROADCAST (255.255.255.255) means any
     host and has the same effect on bind as INADDR_ANY for historical reasons.

   */
  struct sockaddr_in my_addr;
  my_addr.sin_family = AF_INET;
  my_addr.sin_port =
      htons(3490); // Port is in network byte order so we need to convert
  my_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
    perror("bind");
    exit(1);
  }

  close(sockfd);

  return 0;
}
