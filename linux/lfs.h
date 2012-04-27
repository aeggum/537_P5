#ifndef __LFS_H__
#define  __LFS_H__
#define MAXINODES     4096        //Max inodes in system
#define DIRBYTES      32          //Number of bytes in each directory entry

typedef struct __inode {
  int size;
  int type;
  int inum;
  //need something for direct pointers..
  int blocks[14];  //used for direct pointers (LFS)
  //bool used[14];   (don't know how to use booleans in header files)
  int used[14];    //used to tell if direct pointer at [i] is used
} inode;

typedef struct __directory {
  inode inode;
  //Will need something about directories here.  Have to read the chapter.
} directory;


#endif
