#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

#include <fstream>
#include <iostream>

using namespace std;

class SuperBlock {
 private:
  unsigned short blockSize;
  int blocks;
  int freeListBlockOffset;
  int inodeBlockOffset;
  int dataBlockOffset;

 public:
  SuperBlock();
  void setBlockSize(short newBlockSize);
  short getBlockSize();
  void setBlocks(int newBlocks);
  int getBlocks();
  void setFreeListBlockOffset(int newFreeListBlockOffset);
  int getFreeListBlockOffset();
  void setInodeBlockOffset(int newInodeBlockOffset);
  int getInodeBlockOffset();
  void setDataBlockOffset(int newDataBlockOffset);
  int getDataBlockOffset();
  void write(fstream& file);
  void read(fstream& file);
};

#endif
