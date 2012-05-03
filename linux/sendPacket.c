#include <stdio.h>
#include "lfs.h"
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "udp.h"
#include "mfs.h"


int sendPacket(char* hostname, int port_num, MFS_Packet_t *sent, MFS_Packet_t *response) {
  int sd = UDP_Open(0);
  if (sd < 0) {
    perror("Error in opening connection at port 0.\n");
    return -1;
  }

  struct sockaddr_in addr, addr2;
  int rc = UDP_FillSockAddr(&addr, hostname, port_num);
  if (rc < 0) {
    perror("Error looking up host at port");
    return -1;
  }

  fd_set read_fds;
  struct timeval tv;
  tv.tv_sec = 3;    //3 seconds exactly, 0 microseconds                             
  tv.tv_usec = 0;

  //want client to accept messages forever..                                        
  while(1) {
    FD_ZERO(&read_fds);      //clear rfds to 0                                      
    FD_SET(sd, &read_fds);   //adds the file descriptor to the set                  
    UDP_Write(sd, &addr, (char*)response, sizeof(MFS_Packet_t));
    if (select(sd + 1, &read_fds, NULL, NULL, &tv)) {
      rc = UDP_Read(sd, &addr2, (char*)response, sizeof(MFS_Packet_t));
      if (rc > 0) {
        UDP_Close(sd);
        return 0;
      }
      else {
	//do something else                                                         
      }
    } //end if(select())                                                            
  } //end while(1)                                                                  

}
