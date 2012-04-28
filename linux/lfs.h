#ifndef __LFS_H__
#define  __LFS_H__
#define MAXINODES     4096        //Max inodes in system
#define DIRBYTES      32          //Number of bytes in each directory entry
#define CRSIZE        6
#define BLOCKSIZE     4096

typedef struct __inode {
  int size;
  int type;
  int inum;
  //need something for direct pointers..
  int dpointers[14];  //used for direct pointers (LFS)
  //bool used[14];   (don't know how to use booleans in header files)
  int dp_used[14];    //used to tell if direct pointer at [i] is used
} inode;

typedef struct __directory {
  int inums[128];
  char names[128][28];      //could do a char* but this is fixed..
  //the above is my best guess after having read the book
} directory;


//Function prototypes
int update_CR(int inum, int fd);
int start_server(int port, char* path);
int shutdown_server();

#endif
