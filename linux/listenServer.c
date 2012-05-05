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
	    break;
	
      case LOOKUP:  // Fill in "response" packet after using info from "sentPacket"
        fprintf(stderr, "Server received CREAT packet\n");
        // Call lookup() in lfs.c w/ pinum(sentPacket.inum) and name of file to find
        response.inum = lookup(sentPacket.inum, sentPacket.name);
        break;

      case STAT:
        fprintf(stderr, "Server received STAT packet\n");
        break;

      case WRITE:
        fprintf(stderr, "Server received WRITE packet\n");
        break;

      case READ:
        fprintf(stderr, "Server received READ packet\n");
        break;

      case CREAT:
        fprintf(stderr, "Server received CREAT packet\n");
        response.inum = creat_server(sentPacket.inum, sentPacket.type, sentPacket.name);
        break;

      case UNLINK:
        fprintf(stderr, "Server received UNLINK packet\n");
        break;

      case SHUTDOWN:
        fprintf(stderr, "Server received SHUTDOWN packet\n");
        //shutdown_server();
        break;

      case RESPONSE:
        // What case does this happen in? Whend does the client send response method?
	//i don't think it ever will, it's there to keep the compiler happy.
        fprintf(stderr, "Server received RESPONSE packet\n");
        break;
      }
      
      response.method = RESPONSE;
      // FROM P5 desciption: "before returning a success code, the file system 
      // should always fsync() the image" --- Probably want to fsync here (or somewhere)
      if(UDP_Write(sd, &s, (char*)&response, sizeof(MFS_Packet_t) < 0)) {
        fprintf(stderr, "Error sending response from server to client");
      }
      if (sentPacket.method == SHUTDOWN) {
	    shutdown_server();
      }
    } // END else
  } // END while-loop
  fprintf(stderr, "SHOULD NOT GET HERE! THIS WOULD MAKE NO SENSE");
} // END listenOnServer()
