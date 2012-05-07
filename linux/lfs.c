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

int imap[MAXINODES];   //Inode map points to addresses of Inodes (size 4096)
int next_block;        //next block in the AS to be written (ie end of the log)
int fd;

/**
 * updates the checkpoint region.  as long as the inum is valid,
 * the method will set the offset to the correct location (4096*inum)
 * on disk and then write the address of the imap on disk.
 *
 * EXPLAINATION: Every file image starts with int[4096] array, this is the imap.
 * We seek to beginFile + inum*sizeof(int) (the Inode we want to update) then 
 * overwrite it with the new address of the Inode we're refering to
 *
 * The "next_block" refers to the address of the next block of memory we can 
 * write to.  This block could be located anywhere in memory but our "next_block"
 * int is always located directly after the imap[] and holds the address we want
 * to write to next
 *
 * Sorry if you already knew that; I don't know how recent those comments were.
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
  if (fd == -1) {  //file does not exist already, START CREATION
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
  } // END NEW FILE CREATION
  else {
    lseek(fd, 0, SEEK_SET);
    read(fd, imap, sizeof(int)*MAXINODES);
    read(fd, &next_block, sizeof(int));
  }
 
  printf("calling to listen on port: %d\n", port);
  listenOnServer(port);
  return 0;
}



/**
 * Takes in the parent inode and looks up the entry name in the parent. 
 * The inode number of name is returned. 
 */
int lookup_server(int pinum, char* name) {
  // RETURN -1 on (invalid pinum, name does not exist in pinum)
  inode *pinode = malloc(sizeof(inode));  // Create a temp inode for the directory
  directory direct;
  //get the inode associated with the passed in pinum
  if (find_inode(pinum, pinode) != 0) 
    return -1;  

  int i;
  int j;
  for (i = 0; i < 14; i++) {
    //if the direct pointer at that location is used
    if (pinode->dp_used[i] == 1) {
      lseek(fd, pinode->dpointers[i]*BLOCKSIZE, SEEK_SET);
      read(fd, &direct, BLOCKSIZE);
      for(j = 0; j < 128; j++) {
        if(strcmp(direct.names[j], name) == 0) {
          return direct.inums[j];
        }
      }
    }
  }
  return -1;
}


/**
 * Probably the simplest method in the library, this function returns
 * the relevant information about the file specified by inum. 
 * 
 * It finds the relevant inode (from inum) and fills in the passed 
 * in struct's information with the information from the inode
 */
int stat_server(int inum, MFS_Stat_t *m) {
  inode node; 
  if (find_inode(inum, &node) == -1) 
    return -1;    //if the inum doesn't exist, return -1
  
  m->type = node.type;
  m->size = node.size;
  
  return 0;
}


/**
 * Takes in the parent's inode number and the type of file to create, 
 * MFS_DIRECTORY or MFS_REGULAR_FILE, as well as the name of the parent.
 * The parent needs to be a directory.  
 *
 * Otherwise, tries to find a slot and then makes a file in the parent's 
 * directory.  More comments are listed throughout. 
 *
 * Note: poor naming convention
 * Couldn't use creat itself as there is of course the syscall of the same name
 */
int creat_server(int pinum, int type, char *name) {
  //if server already exists, return a success (as specified)
  inode parent;
  if (find_inode(pinum, &parent) == -1) return -1;  //invalid pinum
  if (parent.type != MFS_DIRECTORY) return -1;  //can't make a file in a file
  if (lookup_server(pinum, name) != -1) return 0;
  
  int inum = -1;
  int i;
  for (i = 0; i < MAXINODES; i++) {  //look for empty slots in imap
    if (imap[i] == -1) {
      inum = i;
      break;  //found one!  get out
    }
  }

  //imap is completely full, return error
  if (inum == -1) return -1;
  
  int b, e; 
  directory block;
  /*
   * look at the direct ponters of parent, try to find an open slot to place
   * the new child.  each time it finds a parent that doesn't match, it does 
   * some other stuff. (ie. builds the directory block)
   */
  for (b = 0; b < 14; b++) {
    if (parent.dp_used[b]) {
      lseek(fd, parent.dpointers[b]*BLOCKSIZE, SEEK_SET);
      read(fd, &block, BLOCKSIZE);

      for (e = 0; e < 128; e++) {
	if (block.inums[e] == -1) {
    
	  //now we have found a slot for the file to go...
          lseek(fd, imap[pinum]*BLOCKSIZE, SEEK_SET);
	  write(fd, &parent, BLOCKSIZE);
	  block.inums[e] = inum;
	  strcpy(block.names[e], name);
	  lseek(fd, parent.dpointers[b]*BLOCKSIZE, SEEK_SET);
	  write(fd, &block, BLOCKSIZE);
	  
	  
	  //create inode, init direct pointers
	  inode n;
	  n.inum = inum;
	  n.size = 0;
	  for (i = 0; i < 14; i++) {
	    n.dp_used[i] = 0;
	    n.dpointers[i] = -1;
	  }
	  n.type = type;
	  
	  //must do some more work if we're making a directory
	  if (type == MFS_DIRECTORY) {
	    n.dp_used[0] = 1;
	    n.dpointers[0] = next_block;
	    
	    build_dir_block(1, inum, pinum);
	    
	    // update the file size
	    n.size += BLOCKSIZE;
	  }
	  
	  //there probably aren't any tests against this but nonetheless
	  if (type != MFS_DIRECTORY && type != MFS_REGULAR_FILE) {
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
}
 

/**
 * First checks for three error cases: 1) invalid inum, 2) invalid file type,
 * 3) invalid block number (outside of the valid range).  After that, we 
 * write the buffer into memory by seeking to where it needs to be written, then
 * setting up information in the inode we're writing out.  last, it writes the 
 * inode out to memory and updates the checkpoint region and imap. 
 */
//TODO: Actually write the file size out correctly.  That may be one of the last few bugs. 
int write_server(int inum, char *buffer, int block) {
  inode node;
  //invalid inum
  if (find_inode(inum, &node) != 0) {
    fprintf(stderr, "WRITE: invalid inum\n");
    return -1;  
  }

  //invalid file type (directory)
  if (node.type != MFS_REGULAR_FILE) {
    fprintf(stderr, "WRITE: invalid file type\n");
    return -1;
  }

  //invalid block number (think that this keeps the size in check)
  if (block < 0 || block > 13) {
    fprintf(stderr, "WRITE: invalid block number\n");
    return -1;
  }

  //write buffer into memory (write)
  //update inode.dpointers[block] to be address of next_block
  //update the inode size, next_block, imap, update_CR()
  

  // write buffer 
  lseek(fd, next_block*BLOCKSIZE, SEEK_SET);
  write(fd, buffer, BLOCKSIZE);
  node.dpointers[block] = next_block;

  if (!node.dp_used[block]) {
    node.dp_used[block] = 1;
  }
  
  // update the file size to be largest x of (dp_used[x] == 1) * BLOCKSIZE
  if((block + 1) * BLOCKSIZE > node.size)
    node.size = (block + 1) * BLOCKSIZE;

  // Write the Inode data
  lseek(fd, imap[inum]*BLOCKSIZE, SEEK_SET);
  write(fd, &node, sizeof(inode));

  //update the checkpoint region
  next_block++;
  update_CR(inum);

  return 0;
}


/**
 * First checks for error cases, checking first for an invalid inum passed in, then
 * a block number out of range (< 0 or > 13). 
 * From there, if the file is a regular file, it seeks to the block given by its' direct
 * pointer, reads it into buffer and then returns.  
 * If the file is a directory, things are a little more complicated.  We get the parent
 * inode and search through the directories of the parent and fills in a DirEnt struct
 * when a match is found.  It finishes by copying the memory using a memcpy.
 */
int read_server(int inum, char *buffer, int block) {
  inode node;
  //invalid inum
  if (find_inode(inum, &node) != 0) 
    return -1;
  
  //invalid block number
  if (block < 0 || block > 13) 
    return -1;

  if (node.type == MFS_REGULAR_FILE) {
    lseek(fd, node.dpointers[block]*BLOCKSIZE, SEEK_SET);
    read(fd, buffer, BLOCKSIZE);
    fprintf(stderr, "Reading from Inode %d file-block %d, physical block %d\n", node.inum, block, node.dpointers[block]);
    fprintf(stderr, "Inode %d dpointer[%d] is %d\n", node.inum, block, node.dpointers[block]);
    //node.dpointers[block];
  }
  else  {    //MFS_DIRECTORY
    MFS_DirEnt_t dirEnt;    //name and inum
    int pinum = node.dpointers[1];      //get parent inum
    inode pnode;
    find_inode(pinum, &pnode);          //get the parent inode
    
    directory direct;
    //get the inode associated with the passed in pinum                                         
    int i;
    int j;
    for (i = 0; i < 14; i++) {
      //if the direct pointer at that location is used                                          
      if (pnode.dp_used[i] == 1) {
	lseek(fd, pnode.dpointers[i]*BLOCKSIZE, SEEK_SET);
	read(fd, &direct, BLOCKSIZE);
	for(j = 0; j < 128; j++) {
	  if(direct.inums[j] == inum) {
	    strcpy(dirEnt.name, direct.names[j]);
	    dirEnt.inum = inum;
	  }
	}
      }
    }

    memcpy(buffer, &dirEnt, sizeof(MFS_DirEnt_t));

  }
  
  return 0;
}


/**
 * Romeves the file or directory (if empty) 'name' from the directory specified by 
 * pinum.  It gets the parent inode, makes certain it is a directory first.  If no errors
 * it fills a directory struct with the contents of the pinodes' directory.   
 * It gets the inode of the child we want to remove.  
 *
 * We then search through the directory mentioned earlier.  If we find a match of
 * the name, if it's a regular file, we clear the directory entry and write to disk. 
 * If it's a directory itself, we check if it is empty. If it's not, an error is 
 * returned.  If it is, we clear it just the same, write to disk.
 *
 * Lastly we clear the imap of the inode number removed and update the CR, and return.
 */
int unlink_server(int pinum, char *name) {
  inode pinode;
  // Invalid pinum
  if (find_inode(pinum, &pinode) != 0) {
    fprintf(stderr, "UNLINK: invalid inum\n");
    return -1;  
  }
  // Check pinum refers to a directory
  if(pinode.type != MFS_DIRECTORY) {
    return -1;
  }
  
  //find and fill in the directory relating to the parent
  directory direct;
  lseek(fd, pinode.dpointers[0]*BLOCKSIZE, SEEK_SET);
  read(fd, &direct, BLOCKSIZE);
  
  //get the inode number of the child we want to remove/unlink
  int inum = lookup_server(pinum, name);
  if(inum < 0) return 0;
  
  //get the inode of the child to remove/unlink
  inode node;
  find_inode(inum, &node);

  int i;
  int j;
  //look through directory, looking for match of the name
  for(j = 0; j < 128; j++) {
    if(strcmp(direct.names[j], name) == 0) {
      
      //if regular file, just clear and write to disk. 
      if(node.type == MFS_REGULAR_FILE) {
        strcpy(direct.names[j], "DNE\0");
        direct.inums[j] = -1;
        lseek(fd, pinode.dpointers[0]*BLOCKSIZE, SEEK_SET);
        write(fd, &direct, BLOCKSIZE);
      }
      
      //if directory, search through it in same manner, then clear and write to disk.
      else {
        directory tempDir;
        lseek(fd, node.dpointers[0]*BLOCKSIZE, SEEK_SET);
        read(fd, &tempDir, BLOCKSIZE);
        for(i = 2; i < 128; i++) {
          if(tempDir.inums[i] != -1) {
            return -1; 
          }
        }
        strcpy(direct.names[j], "DNE\0");
        direct.inums[j] = -1;
        lseek(fd, pinode.dpointers[0]*BLOCKSIZE, SEEK_SET);
        write(fd, &direct, BLOCKSIZE);
      }
    }
  }
  
  //update the imap. by now we have removed the node at inum, or returned an error.
  imap[inum] = -1; 
  update_CR(inum);
  return 0;
}


/**
 * Shuts down the server.  Synchronize the file and then exit
 * the program.  The return statement should never be reached.
 */
int shutdown_server() {
  fsync(fd);
  // close(fd) ? Maybe want to close the file and check return code?
  exit(0);
  return -1;
}
