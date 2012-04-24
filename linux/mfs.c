#include "mfs.h"
#include <string.h>
#include "udp.h"

/*
 * Takes a host nome and a part number and uses those to find
 * the server exporting the file system. 
 */
int MFS_Init(char *hostname, int port) {
  return 0;
}


/*
 * Takes the parent inode number (inode number of a directory) and looks
 * up the entry name in it.  The inode number of name is returned. 
 */
int MFS_Lookup(int pinum, char *name) {
  return 0;
}


/* 
 * Returns some information about the file specified by inum. 
 */
int MFS_Stat(int inum, MFS_Stat_t *m) {
  return 0;
}


/*
 * Writes a block of size 4096 bytes at the block offset specified by
 * block. 
 */
int MFS_Write(int inum, char *buffer, int block) {
  return 0;
}


/*
 * Reads a block specified by block into the buffer from file specified by inum.
 * The routine should work for either a file or directory; directories should return
 * data in the format specified by MFS_DirEnt_t. 
 */
int MFS_Read(int inum, char *buffer, int block) {
  return 0;
}


/*
 * Makes a file of type (MFS_REGULAR_FILE) or a directory of type (MFS_DIRECTORY)
 * in the parent directory specified by pinum of name name. 
 * NOTE: If name already exists, return success (why?)
 */
int MFS_Creat(int pinum, int type, char *name) {
  return 0;
}


/*
 * Removes the file or directory name from the directory specified by pinum.
 * NOTE: If the name doesn't exist, it is not a failure (why?)
 */
int MFS_Unlink(int pinum, char *name) {
  return 0;
}


/*
 * Tells the server to force all of its data structures to disk and shutdown. 
 * Does so by calling exit(0).  Mostly used for testing purposes. 
 */
int MFS_Shutdown() {
  return 0;
}
