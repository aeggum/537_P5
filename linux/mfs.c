#include "mfs.h"
#include <stdbool.h>
#include <string.h>
#include "udp.h"
#include "sendPacket.h"

//global variables that will be initialized in MFS_INIT
bool init_done = false;
char *server_name;
int server_port;

int length_check(char* name) {
  //length should be 28, so 27 not counting null termination
  if (strlen(name) > 27) {   
    return -1;
  }

  return 0;
}

/*
 * Takes a host name and a part number and uses those to find
 * the server exporting the file system. 
 */
int MFS_Init(char *hostname, int port) {
  if (hostname == NULL || (port < 0))
    return -1;
  
  server_name = malloc(sizeof(hostname + 1)); //+1 needed for null terminator
  strcpy(server_name, hostname);
  server_port = port;
  init_done = true;
  return 0;
}


/*
 * Takes the parent inode number (inode number of a directory) and looks
 * up the entry name in it.  The inode number of name is returned. 
 */
int MFS_Lookup(int pinum, char *name) {
  if (!init_done) return -1;              //init not yet called
  if (length_check(name) < 0) return -1;  //name too long
  if (name == NULL) return -1;
  
  MFS_Packet_t send, receive;
  send.inum = pinum;
  strcpy((char*)&(send.name), name);
  send.method = LOOKUP;
  int rc = sendPacket(server_name, server_port, &send, &receive);
  if (rc < 0)
    return -1;
  
  return receive.inum;
}


/* 
 * Returns some information about the file specified by inum. 
 */
int MFS_Stat(int inum, MFS_Stat_t *m) {
  if (!init_done) return -1;
  
  MFS_Packet_t send, receive;
  send.inum = inum;
  send.method = STAT;
  
  int rc = sendPacket(server_name, server_port, &send, &receive);
  if (rc < 0) 
    return -1;

  if(receive.inum < 0) {
    return -1;
  }
  //now copy the contents from the received packet into m
  memcpy(m, &(receive.stat), sizeof(MFS_Stat_t));
  return receive.inum;
}


/*
 * Writes a block of size 4096 bytes at the block offset specified by
 * block. 
 */
int MFS_Write(int inum, char *buffer, int block) {
  if (!init_done) return -1;
  
  MFS_Packet_t send, receive;
  send.method = WRITE;
  send.inum = inum;
  memcpy(send.buffer, buffer, 4096);
  send.block = block;

  int rc = sendPacket(server_name, server_port, &send, &receive);
  if (rc < 0) 
    return -1;
  
  //return 0 on success, -1 on failure
  return receive.inum;
}


/*
 * Reads a block specified by block into the buffer from file specified by inum.
 * The routine should work for either a file or directory; directories should return
 * data in the format specified by MFS_DirEnt_t. 
 */
int MFS_Read(int inum, char *buffer, int block) {
  if (!init_done) return -1;

  MFS_Packet_t send, receive;
  send.method = READ;
  send.inum = inum;
  send.block = block;
  
  int rc = sendPacket(server_name, server_port, &send, &receive);
  if (rc < 0) 
    return -1;
  
  if (receive.inum > -1) 
    memcpy(buffer, receive.buffer, 4096);
  
  return receive.inum;
}


/*
 * Makes a file of type (MFS_REGULAR_FILE) or a directory of type (MFS_DIRECTORY)
 * in the parent directory specified by pinum of name name. 
 * NOTE: If name already exists, return success (why?)
 */
int MFS_Creat(int pinum, int type, char *name) {
  if (!init_done) return -1;
  if (length_check(name) < 0) return -1;
  
  MFS_Packet_t send, receive;
  send.inum = pinum;
  send.type = type;
  send.method = CREAT;
  
  strcpy(send.name, name);
  if (sendPacket(server_name, server_port, &send, &receive) < 0) {
	fprintf(stderr, "sendPacket error");
    return -1;
  }
  return receive.inum;
}


/*
 * Removes the file or directory name from the directory specified by pinum.
 * NOTE: If the name doesn't exist, it is not a failure (why?)
 */
int MFS_Unlink(int pinum, char *name) {
  if (!init_done) return -1;
  if (length_check(name) < 0) return -1;
  
  MFS_Packet_t send, receive;
  send.method = UNLINK;
  send.inum = pinum;
  strcpy(send.name, name);

  int rc = sendPacket(server_name, server_port, &send, &receive);
  if (rc < 0)
    return -1;

  //return 0 on success, -1 on failure (as above)
  return receive.inum;
}


/*
 * Tells the server to force all of its data structures to disk and shutdown. 
 * Does so by calling exit(0).  Mostly used for testing purposes. 
 */
int MFS_Shutdown() {
  if (!init_done) return -1;
  
  MFS_Packet_t send, receive;
  send.method = SHUTDOWN;
  
  int rc = sendPacket(server_name, server_port, &send, &receive);
  if (rc < 0)
    return -1;
  
  return 0;
}
