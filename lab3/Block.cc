#include "Block.h"

Block::Block() {
  blockSize = 0;
  bytes = NULL;
}

Block::Block(short blockSize) { setBlockSize(blockSize); }

Block::~Block() {
  if (bytes != NULL) {
    delete[] bytes;
  }
}

void Block::setBlockSize(short newBlockSize) {
  blockSize = newBlockSize;
  bytes = new char[blockSize]();
}

short Block::getBlockSize() { return blockSize; }

void Block::read(fstream& file) {
  if (file.is_open() == true) {
    file.read(bytes, blockSize);
  }
}

void Block::write(fstream& file) {
  if (file.is_open() == true) {
    file.write(bytes, blockSize);
  }
}
