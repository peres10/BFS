#ifndef BFS_FILE_H
#include "bfs_file.h"
#endif

/***
  create_C: creates a contiguous file
    parameters:
      @in: filename, max blocks, type (C or D)
     @out: return OK
    errors:
     from bytemap, inode, dir operations
     errors from disk driver						***/

static int file_create_C( char *name, unsigned int nblocks, char type) {
  int ercode;
  // get the disk block entry for this file
  ercode=bmap_ops.getfree(2,nblocks);
  if (ercode < 0) return ercode;
  unsigned int dm2set=ercode;

  // get the inode entry for this file
  ercode=bmap_ops.getfree(0,1);
  if (ercode < 0) return ercode;
  unsigned int inode2set=ercode;

  // create the dentry
  ercode=dir_ops.create(name,inode2set);
  if (ercode < 0) return ercode;

  // mark the disk block entry for this file
  ercode=bmap_ops.set(2,dm2set,nblocks,1);
  if (ercode < 0) return ercode; // This would be a bug!

  // mark the inode bmap entry for this file
  ercode=bmap_ops.set(0,inode2set,1,1);
  if (ercode < 0) return ercode; // This would be a bug!

  // save the data in the inode itself

  union sml_lrg ino;

  // Read the inode whose number is inode2set
  ercode=inode_ops.read(inode2set,&ino);
  if (ercode < 0) return ercode; // This would be a bug!

  ino.smlino.isvalid= 1;
  ino.smlino.type= type;
  ino.smlino.size= 0;
  ino.smlino.start= dm2set;
  ino.smlino.end= dm2set;

  // Save the inode 
  ercode=inode_ops.write(inode2set,&ino);
  if (ercode < 0) return ercode; // This would be a bug!

  return 0;
 }


/***
  create_I: creates an indexed file
    parameters:
      @in: filename
     @out: return OK
    errors:
     from bytemap, inode, dir operations
     errors from disk driver						***/

static int file_create_I(char *name) {
  int ercode;

  // get the inode entry for this file
  ercode=bmap_ops.getfree(1,1);
  if (ercode < 0) return ercode;
  unsigned int inode2set=ercode;

  // create the dentry
  ercode=dir_ops.create(name,inode2set);
  if (ercode < 0) return ercode;

  // mark the inode bmap entry for this file
  ercode=bmap_ops.set(1,inode2set,1,1);
  if (ercode < 0) return ercode; // This would be a bug!

  // save the data in the inode itself

  union sml_lrg ino;

  // Read the inode whose number is inode2set
  ercode=inode_ops.read(inode2set,&ino);
  if (ercode < 0) return ercode; // This would be a bug!

  // inode deallocate should clean the on disk image, no need here
  ino.lrgino.isvalid= 1;
  //ino.lrgino.size= 0;

  // Save the inode 
  ercode=inode_ops.write(inode2set,&ino);
  if (ercode < 0) return ercode; // This would be a bug!

  return 0;
 }


struct file_operations file_ops= {
        .create_C= file_create_C,
        .create_I= file_create_I
};
