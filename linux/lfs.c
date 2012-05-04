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

/**
 * updates the checkpoint region.  as long as the inum is valid,
 * the method will set the offset to the correct location (4096*inum)
 * on disk and then write the address of the imap on disk.
 *
 * Then, it will update the next block..though i'm not totally sure how
 * that is working. 
 */
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

/**
 * Fairly simple method that finds the inode that is associated with
 * the given inode number. Does this by getting the block that the inode
 * is in, and the seeking to that block and reading from the block at that
 * location into the inode.
 * 
 * Returns 0 on success, -1 if an invalid inum is passed in.
 */
int find_inode(int inum, inode* node) {
  if (inum < 0 || inum >= MAXINODES) 
    return -1;
  
  int inode_block = imap[inum];
  lseek(fd, inode_block*BLOCKSIZE, SEEK_SET);
  read(fd, node, sizeof(inode));

  return 0;
}


/**
 * Builds the directory block.  First initializes a directory (to nothing), 
 * and then, if first_block is true, it sets the directory with the first 
 * block information.
 * Then, it seeks to where the next block is (global), writes what's at 
 * the directory into the block and increments the next_block variable.
 *
 * Returns the block that was written to (1 less than next_block)
 */
int build_dir_block(int first_block, int inum, int pinum) {
  directory d;
  int i;
  for (i = 0; i < 128; i++) {
    d.inums[i] = -1;
    strcpy(d.names[i], "DNE\0");
  }
  
  if (first_block) {
    d.inums[0] = inum;
    strcpy(d.names[0], ".\0");
    d.inums[1] = pinum;
    strcpy(d.names[1], "..\0");
  }

  lseek(fd, next_block*BLOCKSIZE, SEEK_SET);
  write(fd, &d, BLOCKSIZE);
  next_block++;
  
  return next_block - 1;
}


/**
 * starts the server. First checks if the file has already been created, 
 * and if not, goes through the entire set-up process, from inodes to 
 * setting the root directory.  If the file already exists, this method
 * will just seek to it.
 * When done with setup, the file will be seeked to and we will start
 * listening on it.  
 * 
 * I do not think the method should ever return.
 */
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
      imap[i] = -1;      //sets the ENTIRE imap to -1 on init
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
  return -1;
}


/**
 * Probably the simplest method in the library, this function returns
 * the relevant information about the file specified by inum. 
 * It puts the information into the struct passed in. 
 * TODO: Get this method in the h file so it can be used. 
 *       I can't get the method to compile, in the h file. 
 */
int stat_server(int inum, MFS_Stat_t *m) {
  //return information about file given by inum
  //...so we should get the inode related to inum, then fill in m
  //to fill in MFS_Stat_t, there is type and size int fields only
  inode node; 
  if (find_inode(inum, &node) == -1) 
    return -1;    //if the inum doesn't exist, return -1
  
  m->type = node.type;
  m->size = node.type;
  
  return 0;
}


/**
 * Complicated, and I don't understand it all and will comment when 
 * I am awake and can. ...
 */
int creat_server(int pinum, int type, char *name) {
  //if server already exists, return a success
  if (lookup(pinum, name) != -1) return 0;
  inode parent;
  if (find_inode(pinum, &parent) == -1) return -1;
  if (parent.type != MFS_DIRECTORY) return -1;

  int inum = -1;
  int i;
  for (i = 0; i < MAXINODES; i++) {
    if (imap[i] == -1) {
      inum = i;
      break;
    }
  }

  if (inum == -1) return -1;
  
  int b, e; 
  directory block;
  for (b = 0; b < 14; b++) {
    if (parent.dp_used[b]) {
      lseek(fd, parent.dpointers[b]*BLOCKSIZE, SEEK_SET);
      read(fd, &block, BLOCKSIZE);

      for (e = 0; e < 128; e++) {
	if (block.inums[e] == -1) 
	  goto found_parent_slot;
      }
    }
    else {
      int bl = build_dir_block(0, inum, -1);
      parent.size += BLOCKSIZE;
      parent.dp_used[b] = 1;
      parent.dpointers[b] = bl;
      b--;
    }
  }
  
  return -1;   //when the directory is full
  
 found_parent_slot:
  lseek(fd, imap[pinum]*BLOCKSIZE, SEEK_SET);
  write(fd, &parent, BLOCKSIZE);
  block.inums[e] = inum;
  strcpy(block.names[e], name);
  lseek(fd, parent.dpointers[b]*BLOCKSIZE, SEEK_SET);
  write(fd, &block, BLOCKSIZE);


  //create inode
  inode n;
  n.inum = inum;
  n.size = 0;
  for (i = 0; i < 14; i++) {
    n.dp_used[i] = 0;
    n.dpointers[i] = -1;
  }
  n.type = type;
  if (type == MFS_DIRECTORY) {
    n.dp_used[0] = 1;
    n.dpointers[0] = next_block;
    
    build_dir_block(1, inum, pinum);
    
    // update file size
    n.size += BLOCKSIZE;
  }
  else if (type != MFS_DIRECTORY && type != MFS_REGULAR_FILE) {
    return -1;
  }

  // update imap
  imap[inum] = next_block;

  // write inode
  lseek(fd, next_block*BLOCKSIZE, SEEK_SET);
  write(fd, &n, sizeof(inode));
  next_block++;
  
  // write checkpoint region
  update_CR(inum);

  return 0;
}


/**
 * Shuts down the server.  Synchronize the file and then exit
 * the program.  The return statement should never be reached.
 */
int shutdown_server() {
  fsync(fd);
  exit(0);
  return -1;
}
