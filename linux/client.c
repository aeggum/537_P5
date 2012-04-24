#include <stdio.h>
#include <stdlib.c>
#include "udp.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int
main(int argc, char *argv[])
{
  //expecting 3 args: server [portnum] [file-system image]
  if (argc != 3) {
    printf("Usage: server [portnum] [file-system image]\n");
    exit(-1);
  }

  int port_num = atoi(argv[1]);
  char* fs_image = argv[2];

  //now we would probably do a call to start the server
  //which would do some of the stuff that is below...


  //going to throw in some psuedocode below: 
  
  //check if fs already exists - (open(fs_image, O_RDWR) == -1) would mean it doesn't
  //    if it doesn't, create it - open(fs_image, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU); ..
  //         create checkpont region - ???
  //         create inode map, single root directory with '.' and '..' entries
  //              root inode should be value 0
  //    if it does exist already, read in checkpoint region & inode  map

  //maybe it would be at this point that we do some of the stuff below, with port_num?

  int sd = UDP_Open(20000);
  if (sd < 0) {
    printf("Problem in opening the socket on port %d\n", port_num);
    exit(-1);
  }

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


