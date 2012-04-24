#ifndef __LFS_H__
#define  __LFS_H__
#define MAXINODES     4096        //Max inodes in system
#define DIRBYTES      32          //Number of bytes in each directory entry

typedef struct __inode {
  int size;
  int type;
  int inum;
  //need something for direct pointers..
} inode;

typedef struct __directory {
  inode inode;
  //...TODO: Not sure about this struct.
} directory;


#endif
