#ifndef BLOCK_H
#define BLOCK_H

#include <fstream>
#include <iostream>

using namespace std;

class Block {
 private:
  short blockSize;

 public:
  char* bytes;

  Block();
  Block(short blockSize);
  ~Block();

  void setBlockSize(short newBlockSize);
  short getBlockSize();
  void read(fstream& file);   // throws IOException , EOFException
  void write(fstream& file);  // throws IOException
};

#endif
