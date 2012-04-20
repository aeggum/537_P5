#include <stdio.h>
#include "udp.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int
main(int argc, char *argv[])
{
  int sd = UDP_Open(20000);
  assert(sd > -1);

  struct sockaddr_in addr, addr2;
  int rc = UDP_FillSockAddr(&addr, "mumble-07.cs.wisc.edu", 10000);
  assert(rc == 0);
  
  char message[BUFFER_SIZE];
  //We will probably be passing structs rather than char[]'s..
  sprintf(message, "hello world");
  rc = UDP_Write(sd, &addr, message, BUFFER_SIZE);
  printf("CLIENT:: sent message (%d)\n", rc);
  if (rc > 0) {
    int rc = UDP_Read(sd, &addr2, buffer, BUFFER_SIZE);
    printf("CLIENT:: read %d bytes (message: '%s')\n", rc, buffer);
  }
  
  return 0;
}


