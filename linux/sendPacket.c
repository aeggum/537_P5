#include <stdio.h>
#include "lfs.h"
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "udp.h"
#include "mfs.h"

/**
 * method that sends a message (through a packet).  Used by the client.
 * The server will take the 'response' packet and fill in the important 
 * bits of information, depending on the call (done in response packet)
 */
int sendPacket(char* hostname, int port_num, MFS_Packet_t *sent, MFS_Packet_t *response) {
  int sd = UDP_Open(0);
  if (sd < 0) {
    perror("Error in opening connection at port 0.\n");
    return -1;
  }

  //this code taken or slightly modified from original client
  struct sockaddr_in addr, addr2;
  int rc = UDP_FillSockAddr(&addr, hostname, port_num);
  if (rc < 0) {
    perror("Error looking up host at port");
    return -1;
  }

  //set up things for the select call (3 second timeout)
  fd_set read_fds;
  struct timeval tv;
  tv.tv_sec = 3;    //3 seconds exactly, 0 microseconds                             
  tv.tv_usec = 0;

  /* Client should send a packet to server then be chillin waiting for a response */
  while(1) {
    FD_ZERO(&read_fds);      //clear rfds to 0                              
    FD_SET(sd, &read_fds);   //adds the file descriptor to the set                  
    
    // Send a packet to the server
    UDP_Write(sd, &addr, (char*)sent, sizeof(MFS_Packet_t));

    // Chillin waiting for a response (select described in P5 description)
    if (select(sd + 1, &read_fds, NULL, NULL, &tv)) {   //waits for an fd to become "ready"

      // Read a server response into "response packet"
      rc = UDP_Read(sd, &addr2, (char*)response, sizeof(MFS_Packet_t)); 
      
      //rc is the return of recvfrom (see man page)
      if (rc >= 0) { 
        UDP_Close(sd);
        return 0;
      }
    } //end if(select())                                                            

  } //end while(1)                                                                  
}

