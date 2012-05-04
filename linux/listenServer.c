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
    MFS_Packet_t packet;
    
    int rc = UDP_Read(sd, &s, (char*)&packet, sizeof(MFS_Packet_t));
    if (rc <= 0) 
      continue;
    else {
      MFS_Packet_t response;
      
      //For whatever reason, this switch statement isn't working. 
      //nothing seems to be hit, ever...
      //once this can work, everything should be a lot easier
      switch(packet.method) {
	
      case INIT:
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
      case RESPONSE:
	break;
      }
      
      response.method = RESPONSE;
      rc = UDP_Write(sd, &s, (char*)&response, sizeof(MFS_Packet_t));
      if (packet.method == SHUTDOWN)
	shutdown_server();
    }
  }
}
