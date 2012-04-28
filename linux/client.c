#include <stdio.h>
#include <stdlib.c>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int
main(int argc, char *argv[])
{
  printf("inside of client.c\n");
  
  //expecting 3 args: server [portnum] [file-system image]
  if (argc != 3) {
    printf("Usage: server [portnum] [file-system image]\n");
    exit(-1);
  }

  int port_num = atoi(argv[2]);  //as mentioned in mfs.c
  char* host = argv[1];
  
  MFS_Init(host, port_num);
  MFS_Creat(0, MFS_DIRECTORY, "TestDir");
  int inode = MFS_Lookup(0, "TestDir");
  int i;
  for(i = 0; i < 1800; ++i) {
      char buffer[2000];
      sprintf(buffer, "file_%d", i);
      int ret = MFS_Creat(inode, MFS_REGULAR_FILE, buffer);
      if(ret == 0) {
	printf("Successfully created: %s", buffer);
      }
      else {
	printf("Failed to create: %s", buffer);
      }
    }
  
  
  //TODO: More to set-up.
  MFS_Shutdown();
  return -1;
    
  /*int sd = UDP_Open(20000);
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
  
  return 0;*/
}


