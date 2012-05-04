#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
  
  
  MFS_Shutdown();
  return 0;
}




