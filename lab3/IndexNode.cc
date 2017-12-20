
#include "IndexNode.h"
#include <stdlib.h>
#include <string.h>

#define NOT_A_BLOCK 0x00FFFFFF

int IndexNode::MAX_FILE_BLOCKS = 11;

IndexNode::IndexNode() {
  mode = 0;
  nlink = 0;
  uid = 0;
  gid = 0;
  size = 0;

  for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
    directBlocks[i] = NOT_A_BLOCK;
  }

  indirectBlock = NOT_A_BLOCK;        // Not yet implemented.
  doubleIndirectBlock = NOT_A_BLOCK;  // Not yet implemented.
  tripleIndirectBlock = NOT_A_BLOCK;  // Not yet implemented.
  atime = 0;                          // Not yet implemented.
  mtime = 0;                          // Not yet implemented.
  ctime = 0;                          // Not yet implemented.
}

/**
 * Sets the mode for this IndexNode.
 * This is the file type and file protection information.
 */
void IndexNode::setMode(short newMode) { mode = newMode; }

/**
 * Gets the mode for this IndexNode.
 * This is the file type and file protection information.
 */
short IndexNode::getMode() { return mode; }

/**
 * Set the number of links for this IndedNode.
 * @param newNlink the number of links
 */
void IndexNode::setNlink(short newNlink) { nlink = newNlink; }

/**
 * Get the number of links for this IndexNode.
 * @return the number of links
 */
short IndexNode::getNlink() { return nlink; }

void IndexNode::setUid(short newUid) { uid = newUid; }

short IndexNode::getUid() { return uid; }

short IndexNode::getGid() { return gid; }

void IndexNode::setGid(short newGid) { gid = newGid; }

/**
 * Sets the size for this IndexNode.
 * This is the number of bytes in the file.
 */
void IndexNode::setSize(int newSize) { size = newSize; }

/**
 * Gets the size for this IndexNode.
 * This is the number of bytes in the file.
 */
int IndexNode::getSize() { return size; }

/**
 * Gets the address corresponding to the specified
 * sequential block of the file.
 * @param block the sequential block number
 * @return the address of the block, a number between zero and one
 * less than the number of blocks in the file system
 * @exception java.lang.Exception if the block number is invalid
 */
int IndexNode::getBlockAddress(int block) {
  if (block >= 0 && block < MAX_DIRECT_BLOCKS) {
    return (directBlocks[block]);
  } else if (block == MAX_DIRECT_BLOCKS) {
    return indirectBlock;
  } else {
    cout << "invalid block address get: " << block << endl;
  }
  return 0;
}

/**
 * Sets the address corresponding to the specified sequential
 * block of the file.
 * @param block the sequential block number
 * @param address the address of the block, a number between zero and one
 * less than the number of blocks in the file system
 * @exception java.lang.Exception if the block number is invalid
 */
void IndexNode::setBlockAddress(int block, int address) {
  if (block >= 0 && block < MAX_DIRECT_BLOCKS) {
    directBlocks[block] = address;
  } else if (block == MAX_DIRECT_BLOCKS) {
    indirectBlock = address;
  } else {
    cout << "invalid block address set: " << block << endl;
  }
}

void IndexNode::setBlockSize(short size) { MAX_FILE_BLOCKS = size; }

void IndexNode::setAtime(int newAtime) { atime = newAtime; }

int IndexNode::getAtime() { return atime; }

void IndexNode::setMtime(int newMtime) { mtime = newMtime; }

int IndexNode::getMtime() { return mtime; }

void IndexNode::setCtime(int newCtime) { ctime = newCtime; }

int IndexNode::getCtime() { return ctime; }

/**
 * Writes the contents of an index node to a byte array.
 * This is used to copy the bytes which correspond to the
 * disk image of the index node onto a block buffer so that
 * they may be written to the file system.
 * @param buffer the buffer to which bytes should be written
 * @param offset the offset from the beginning of the buffer
 * at which bytes should be written
 */
void IndexNode::write(char* buffer, int offset) {
  // write the mode info
  buffer[offset] = (unsigned char)(mode >> 8);
  buffer[offset + 1] = (unsigned char)mode;

  // write nlink
  buffer[offset + 2] = (unsigned char)(nlink >> 8);
  buffer[offset + 3] = (unsigned char)nlink;

  // write uid
  buffer[offset + 4] = (unsigned char)(uid >> 8);
  buffer[offset + 5] = (unsigned char)uid;

  // write gid
  buffer[offset + 6] = (unsigned char)(gid >> 8);
  buffer[offset + 7] = (unsigned char)gid;

  // write the size info
  buffer[offset + 8] = (unsigned char)(size >> 24);
  buffer[offset + 8 + 1] = (unsigned char)(size >> 16);
  buffer[offset + 8 + 2] = (unsigned char)(size >> 8);
  buffer[offset + 8 + 3] = (unsigned)(size);

  // write the directBlocks info 3 bytes at a time
  for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
    buffer[offset + 12 + 3 * i] = (unsigned char)(directBlocks[i] >> 16);
    buffer[offset + 12 + 3 * i + 1] = (unsigned char)(directBlocks[i] >> 8);
    buffer[offset + 12 + 3 * i + 2] = (unsigned char)(directBlocks[i]);
  }
  buffer[offset + 16 + 3 * MAX_DIRECT_BLOCKS] =
      (unsigned char)(indirectBlock >> 16);
  buffer[offset + 16 + 3 * MAX_DIRECT_BLOCKS + 1] =
      (unsigned char)(indirectBlock >> 8);
  buffer[offset + 16 + 3 * MAX_DIRECT_BLOCKS + 2] =
      (unsigned char)(indirectBlock);

  // leave room for indirectBlock, doubleIndirectBlock, tripleIndirectBlock

  // leave room for atime, mtime, ctime
}

/**
 * Reads the contents of an index node from a byte array.
 * This is used to copy the bytes which correspond to the
 * disk image of the index node from a block buffer that
 * has been read from the file system.
 * @param buffer the buffer from which bytes should be read
 * @param offset the offset from the beginning of the buffer
 * at which bytes should be read
 */
void IndexNode::read(char* buffer, int offset) {
  int b3;
  int b2;
  int b1;
  int b0;

  // read the mode info
  b1 = buffer[offset] & 0xff;
  b0 = buffer[offset + 1] & 0xff;
  mode = (short)(b1 << 8 | b0);

  // read the nlink info
  b1 = buffer[offset + 2] & 0xff;
  b0 = buffer[offset + 3] & 0xff;
  nlink = (short)(b1 << 8 | b0);

  // read the uid info
  b1 = buffer[offset + 4] & 0xff;
  b0 = buffer[offset + 5] & 0xff;
  uid = (short)(b1 << 8 | b0);

  // read the gid info
  b1 = buffer[offset + 6] & 0xff;
  b0 = buffer[offset + 7] & 0xff;
  gid = (short)(b1 << 8 | b0);

  // read the size info
  b3 = buffer[offset + 8] & 0xff;
  b2 = buffer[offset + 8 + 1] & 0xff;
  b1 = buffer[offset + 8 + 2] & 0xff;
  b0 = buffer[offset + 8 + 3] & 0xff;
  size = b3 << 24 | b2 << 16 | b1 << 8 | b0;

  // read the block address info 3 bytes at a time
  for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
    b2 = buffer[offset + 12 + i * 3] & 0xff;
    b1 = buffer[offset + 12 + i * 3 + 1] & 0xff;
    b0 = buffer[offset + 12 + i * 3 + 2] & 0xff;
    directBlocks[i] = b2 << 16 | b1 << 8 | b0;
  }

  b2 = buffer[offset + 16 + MAX_DIRECT_BLOCKS * 3] & 0xff;
  b1 = buffer[offset + 16 + MAX_DIRECT_BLOCKS * 3 + 1] & 0xff;
  b0 = buffer[offset + 16 + MAX_DIRECT_BLOCKS * 3 + 2] & 0xff;
  indirectBlock = b2 << 16 | b1 << 8 | b0;

  // leave room for indirectBlock, doubleIndirectBlock, tripleIndirectBlock

  // leave room for atime, mtime, ctime
}

/**
 * Converts an index node into a printable string.
 * @return the printable string
 */
char* IndexNode::toString() {
  // Not thread safe. -KS
  static char buffer[1024];
  char temp[512];
  memset(buffer, '\0', 1024);
  memset(temp, '\0', 512);
  sprintf(buffer, "IndexNode[%#o,{", mode);

  for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
    if (i > 0) {
      strcat(buffer, ",");
    }

    sprintf(temp, "%d", directBlocks[i]);
    strcat(buffer, temp);
  }

  strcat(buffer, "}]");
  return buffer;
}

void IndexNode::copy(IndexNode& indexNode) {
  indexNode.mode = mode;
  indexNode.nlink = nlink;
  indexNode.uid = uid;
  indexNode.gid = gid;
  indexNode.size = size;

  for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
    indexNode.directBlocks[i] = directBlocks[i];
  }

  indexNode.indirectBlock = indirectBlock;
  indexNode.doubleIndirectBlock = doubleIndirectBlock;
  indexNode.tripleIndirectBlock = tripleIndirectBlock;
  indexNode.atime = atime;
  indexNode.mtime = mtime;
  indexNode.ctime = ctime;
}
/*
int main(int argc, char ** argv )
{
        //byte[] buffer = new byte[512] ;
        char * buffer = new char[512];

        IndexNode root;
        root.setMode((short)040000) ;
        root.setBlockAddress(0, 33 ) ;
        cout << root.toString()<<endl;

        IndexNode copy;
        root.write(buffer, 0) ;
        copy.read(buffer, 0) ;
        cout <<copy.toString()<<endl;
}*/
