#include <stdio.h>
#include "udp.h"

#define BUFFER_SIZE (4096)

int
main(int argc, char *argv[])
{
  int sd = UDP_Open(10000);
  assert(sd > -1);

  printf("waiting in loop\n");
  
  while (1) {
    struct sockaddr_in s;
    char buffer[BUFFER_SIZE];
    int rc = UDP_Read(sd, &s, buffer, BUFFER_SIZE);
    if (rc > 0) {
      printf("SERVER:: read %d bytes (message: '%s')\n", rc, buffer);
      char reply[BUFFER_SIZE];
      sprintf(reply, "reply");
      rc = UDP_Write(sd, &s, reply, BUFFER_SIZE);
    }
  }
  
  return 0;
}


//TODO: I think this method is something like we will need.  
//Call a method to open a port and keep it open, listening.
void listenOnServer(int port_num) {
  int sd = UDP_Open(port_num);
  if (sd < 0) {
    printf("Error in opening a socket on port_num %d\n", port_num);
    exit(-1);
  }

  printf("starting the server (listening) in server.c\n");
  while(1) {
    
  }
}
