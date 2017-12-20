#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <fstream>
#include <iostream>
#include "BitBlock.h"
#include "IndexNode.h"

using namespace std;

class FileSystem {
 private:
  fstream file;
  char filename[256];
  //		char mode[128];
  short blockSize;
  int blockCount;
  int freeListBlockOffset;
  int inodeBlockOffset;
  int dataBlockOffset;

  IndexNode rootIndexNode;

  int currentFreeListBitNumber;
  int currentFreeListBlock;
  BitBlock* freeListBitBlock;

  short currentIndexNodeNumber;
  short currentIndexNodeBlock;
  char* indexBlockBytes;

  void loadFreeListBlock(int dataBlockNumber);
  void loadIndexNodeBlock(short indexNodeNumber);

 public:
  const static short ROOT_INDEX_NODE_NUMBER = 0;
  const static int NOT_A_BLOCK = 0x00FFFFFF;

  FileSystem(char* newFilename, char* newMode);
  ~FileSystem();
  short getBlockSize();
  int getFreeListBlockOffset();
  int getInodeBlockOffset();
  int getDataBlockOffset();
  IndexNode* getRootIndexNode();

  bool open();
  void close();
  void read(char* bytes, int blockNumber);
  void write(char* bytes, int blockNumber);

  void freeBlock(int dataBlockNumber);
  int allocateBlock();

  short allocateIndexNode();
  void readIndexNode(IndexNode* indexNode, short indexNodeNumber);
  void writeIndexNode(IndexNode* indexNode, short indexNodeNumber);

  bool is_ready();
  bool isSystemReady;
};

#endif
