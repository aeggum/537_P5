#include "mfs.h"
#include "lfs.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "listenServer.h"

int imap[MAXINODES];   //4096, as defined in lfs.h
int next_block;        //next block in the AS to be written
bool new;
int fd;

int update_CR(int inum) {
  //update the inode table
  if (inum  != -1) {
    lseek(fd, inum*sizeof(int), SEEK_SET);
    write(fd, &imap[inum], sizeof(int));
  }
  
  //update the next block
  lseek(fd, MAXINODES*sizeof(int), SEEK_SET);
  write(fd, &next_block, sizeof(int));
  
  return 0;
}

int start_server(int port, char* path) {
  fd = open(path, O_RDWR);
  printf("inside the start_server() method\n");
  if (fd == -1) {  //file does not exist already
    new = true;
    fd = open(path, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    if (fd == -1) return -1;
    next_block = CRSIZE;
    
    
    int i;
    for(i = 0; i < MAXINODES; i++) {
      imap[i] = -1;      //sets the imap to -1 on init
    }
        

    lseek(fd, 0, SEEK_SET);
    write(fd, imap, sizeof(int)*MAXINODES);
    write(fd, &next_block, sizeof(int));

    inode root_node;            //root node is a directory
    root_node.inum = 0;
    root_node.size = BLOCKSIZE;
    root_node.type = MFS_DIRECTORY;
    root_node.dp_used[0] = 1;
    root_node.dpointers[0] = next_block;
        

    for (i = 1; i < 14; i++) {
      root_node.dp_used[i] = 0;
      root_node.dpointers[i] = -1;
    }
    
    directory base_block;       //fill in the root directory
    base_block.inums[0] = 0;
    base_block.inums[1] = 0;
    strcpy(base_block.names[0], ".\0");
    strcpy(base_block.names[1], "..\0");

    for (i = 2; i < 128; i++) {
      base_block.inums[i] = -1;
      strcpy(base_block.names[i], "DNE\0");
    }
    
    //write the base block
    lseek(fd, next_block*BLOCKSIZE, SEEK_SET);
    write(fd, &base_block, sizeof(directory));
    next_block++;

    imap[0] = next_block;
  
    //write inode
    lseek(fd, next_block*BLOCKSIZE, SEEK_SET);
    write(fd, &root_node, sizeof(inode));
    next_block++;
   
    update_CR(0);
  } 
  else {
    new = false;
    lseek(fd, 0, SEEK_SET);
    read(fd, imap, sizeof(int)*MAXINODES);
    read(fd, &next_block, sizeof(int));
  }
 
  printf("calling to listen on port: %d\n", port);
  listenOnServer(port);
  return 0;
}



int lookup(int pinum, char* name) {
  //TODO
  return 0;
}

//return -1 only if the inum doesn't exist.
int stat_server(int inum, MFS_Stat_t *m) {
  //return information about file given by inum
  //...so we should get the inode related to inum, then fill in m
  //to fill in MFS_Stat_t, there is type and size int fields only
  
  return 0;
}

int shutdown_server() {
  fsync(fd);
  exit(0);
  return -1;
}
