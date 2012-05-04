#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "lfs.h"

#define BUFFER_SIZE (4096)

int
main(int argc, char *argv[]) {
  /* Check and parse arguments */
  if (argc != 3) {
    printf("Usage: server [portnum] [file-system-image]\n");
    exit(-1);
  }

  int port_num = atoi(argv[1]);
  char* fs_image = argv[2];

  printf("new port number: %d\n", port_num);

  /* Start the server: open file image and copy checkpoint region into memory */
  /* In lfs.c */
  start_server(port_num, fs_image);

  return 0;
  
}
