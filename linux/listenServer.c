#include <stdio.h>
#include "udp.h"
#include "lfs.h"
#include "mfs.h"


//Call a method to open a port and keep it open, listening.                   
void listenOnServer(int port_num) {
  int sd = UDP_Open(port_num);
  if (sd < 0) {   
    printf("Error in opening a socket on port_num %d\n", port_num);
    exit(-1);
  }

    
  printf("starting the server (listening) in server.c\n");
  while(1) {
    struct sockaddr_in s;
    MFS_Packet_t sentPacket;
    
    // Read in a packet sent from client via sendPacket
    int rc = UDP_Read(sd, &s, (char*)&sentPacket, sizeof(MFS_Packet_t));
    if (rc <= 0) // rc = -1 is the code for error; I'm not sure what if() continue
      continue;  // does. You'll have to explain that to me.
    else {
      MFS_Packet_t response;
      
      // This switch is working now right?...
      // Going to want to do something more in each case obviously
      switch(sentPacket.method) {
	
      case INIT:
        fprintf(stderr, "Server received INIT packet\n");
        // NULL CASE
	    break;
	
      case LOOKUP:
        fprintf(stderr, "Server received LOOKUP packet\n");
        response.inum = lookup_server(sentPacket.inum, sentPacket.name);
        break;

      case STAT:
        response.inum = stat_server(sentPacket.inum, &response.stat);
        fprintf(stderr, "Server received STAT packet\n");
        break;

      case WRITE:
        fprintf(stderr, "Server received WRITE packet\n");
	response.inum = write_server(sentPacket.inum, sentPacket.buffer, sentPacket.block);
        break;

      case READ:
        fprintf(stderr, "Server received READ packet\n");
	response.inum = read_server(sentPacket.inum, response.buffer, sentPacket.block);
        break;

      case CREAT:
        fprintf(stderr, "Server received CREAT packet\n");
        response.inum = creat_server(sentPacket.inum, sentPacket.type, sentPacket.name);
        break;

      case UNLINK:
        response.inum = unlink_server(sentPacket.inum, sentPacket.name);
        fprintf(stderr, "Server received UNLINK packet\n");
        break;

      case SHUTDOWN:
        fprintf(stderr, "Server received SHUTDOWN packet\n");
        //shutdown_server();
        break;

      case RESPONSE:
        // NULL CASE
        fprintf(stderr, "Server received RESPONSE packet\n");
        break;
      }
      
      response.method = RESPONSE;
      // FROM P5 desciption: "before returning a success code, the file system 
      // should always fsync() the image" --- Probably want to fsync here (or somewhere)
      // Fixed missing bracket that screwed stuff up, sorry about that
      if(UDP_Write(sd, &s, (char*)&response, sizeof(MFS_Packet_t)) < 0) {
        fprintf(stderr, "Error sending response from server to client");
      }
      if (sentPacket.method == SHUTDOWN) {
	    shutdown_server();
      }
    } // END else
  } // END while-loop
  fprintf(stderr, "SHOULD NOT GET HERE! THIS WOULD MAKE NO SENSE");
} // END listenOnServer()
