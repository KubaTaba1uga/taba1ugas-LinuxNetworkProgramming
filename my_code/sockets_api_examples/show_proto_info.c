#include <netdb.h>
#include <stdio.h>

/*
  All possible protocols can be found in /etc/protocols.
 */

int main(void) {
  struct protoent *proto;
  proto = getprotobyname("tcp");
  if (proto) {
    printf("Protocol number for TCP: %d\n", proto->p_proto);
  }

  proto = getprotobyname("udp");
  if (proto) {
    printf("Protocol number for UDP: %d\n", proto->p_proto);
  }

  proto = getprotobyname("rdp");
  if (proto) {
    printf("Protocol number for RDP: %d\n", proto->p_proto);
  }

  return 0;
}
