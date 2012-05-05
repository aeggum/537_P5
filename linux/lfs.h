#include "mfs.h"
#ifndef __LFS_H__
#define  __LFS_H__
#define MAXINODES     4096        //Max inodes in system
#define DIRBYTES      32          //Number of bytes in each directory entry
#define CRSIZE        6
#define BLOCKSIZE     4096
#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

typedef struct __inode {
  int size;
  int type; //MFS_DIRECTORY or MFS_REGULAR_FILE
  int inum;
  //need something for direct pointers..
  int dpointers[14];  //used for direct pointers (LFS)
  //bool used[14];   (don't know how to use booleans in header files)
  int dp_used[14];    //used to tell if direct pointer at [i] is used
} inode;

typedef struct __directory {
  int inums[128];
  char names[128][28];      //could do a char* but this is fixed size
  //the above is my best guess after having read the book
} directory;


//Function prototypes (implement in lfs.c)
int update_CR(int inum);
int start_server(int port, char* path);
int shutdown_server();
int lookup_server(int pinum, char* name);
int stat_server(int inum, MFS_Stat_t *m);   //doesn't compile
int creat_server(int pinum, int type, char *name);
int write_server(int inum, char *buffer, int block);
int read_server(int inum, char *buffer, int block);
int unlink_server(int pinum, char *name);
#endif

