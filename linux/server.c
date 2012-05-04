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
  
  /*int sd = UDP_Open(10000);
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
    }*/
}


//MOVED THE MOTHOD BELOW TO ITS OWN FILE TO HELP WITH COMPILATION (I HOPE)

//TODO I think this method is something like we will need.  
//Call a method to open a port and keep it open, listening.

/*void listenOnServer(int port_num) {
  int sd = UDP_Open(port_num);
  if (sd < 0) {   //TODO: It's failing here, so set-up isn't right
    printf("Error in opening a socket on port_num %d\n", port_num);
    exit(-1);
  }

  printf("starting the server (listening) in server.c\n");

  while(1) {
    struct sockaddr_in s;
    MFS_Packet_t packet;
    int rc = UDP_Read(sd, &s, (char*)&packet, sizeof(MFS_Packet_t));
    if (rc <= 0) 
      continue;
    else {
      MFS_Packet_t response;
      
      switch(packet.method) {
	
      case INIT:
	//
	break;
      case LOOKUP:
	//
	break;
      case STAT:
	//
	break;
      case WRITE:
	//
	break;
      case READ:
	//
	break;
      case CREAT:
	// 
	break;
      case UNLINK: 
	//
	break;
      case SHUTDOWN:
	//shutdown_server();
	break;
      }

      response.method = RESPONSE;
      rc = UDP_Write(sd, &s, (char*)&response, sizeof(MFS_Packet_t));
      if (packet.method == SHUTDOWN)
	shutdown_server(); // TODO: implement this.  is it just an exit? fsync then exit?
    }
  }
  }*/
