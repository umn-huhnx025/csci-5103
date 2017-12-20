#ifndef FILE_DESCRIPTOR_H
#define FILE_DESCRIPTOR_H

#include <fstream>
#include <iostream>
#include "FileSystem.h"
#include "IndexNode.h"

using namespace std;

class FileDescriptor {
 private:
  FileSystem *fileSystem;
  IndexNode indexNode;
  short deviceNumber;
  short indexNodeNumber;
  int flags;
  int offset;
  char *bytes;

 public:
  //		FileDescriptor( short newDeviceNumber , short newIndexNodeNumber
  //,  int newFlags);
  FileDescriptor(FileSystem *newFileSystem, IndexNode &newIndexNode,
                 int newFlags);
  ~FileDescriptor();
  void setDeviceNumber(short newDeviceNumber);
  short getDeviceNumber();
  IndexNode *getIndexNode();
  void setIndexNodeNumber(short newIndexNodeNumber);
  short getIndexNodeNumber();
  int getFlags();
  char *getBytes();
  short getMode();
  int getSize();
  void setSize(int newSize);
  short getBlockSize();
  int getOffset();
  void setOffset(int newOffset);
  int readBlock(short relativeBlockNumber);
  int writeBlock(short relativeBlockNumber);
};

#endif
