#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)

typedef struct __MFS_Stat_t {
  int type;   // MFS_DIRECTORY or MFS_REGULAR
  int size;   // bytes
  // note: no permissions, access times, etc.
  } MFS_Stat_t;

// TODO: Need some sort of check point region structure, i think?
//typedef struct __MFS_ChkPtRgn {
  // NEED:	Pointer to end of log (size of the log in bytes?)
  //		Array of pointers to pieces of the inode map

  //} MFS_ChkPtRgn

// TODO: Need an Inode structure (I didn't see one yet)
    /*typedef struct __MFS_Inode {
  // NEED:	Inode number? [This is the index of the Inode map where each is located]
  //		Size
  //		Type (regular or directory)
  //		Arr[14] pointers to data blocks (4KB) for the file

  } MFS_Inode*/

typedef struct __MFS_DirEnt_t {
  char name[28];  // up to 28 bytes of name in directory (including \0)
  int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;


//can use an enum to tell which method we will be using
enum method {
  INIT, 
  LOOKUP, 
  STAT, 
  WRITE,
  READ,
  CREAT,
  UNLINK,
  SHUTDOWN,
  RESPONSE
};

typedef struct __MFS_Packet_t {
  enum method method;
  MFS_Stat_t stat;
  int inum;
  int type;
  int block;
  char name[28];
  char buffer[MFS_BLOCK_SIZE];
  
} MFS_Packet_t;

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__
