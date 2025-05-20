#include <arpa/inet.h>
#include <stdio.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

int main(void) {
  uint16_t host_port = 8080;
  uint16_t net_port = htons(host_port);
  printf("Network byte order: 0x%x\n", net_port);
  host_port = ntohs(net_port);
  printf("   Host byte order: 0x%x\n", host_port);

  return 0;
}
