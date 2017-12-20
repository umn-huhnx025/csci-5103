#include "SuperBlock.h"
#include <stdlib.h>
#include <string.h>
SuperBlock::SuperBlock() {}

void SuperBlock::setBlockSize(short newBlockSize) { blockSize = newBlockSize; }

short SuperBlock::getBlockSize() { return blockSize; }

void SuperBlock::setBlocks(int newBlocks) { blocks = newBlocks; }

int SuperBlock::getBlocks() { return blocks; }

/**
 * Set the freeListBlockOffset (in blocks)
 * @param newFreeListBlockOffset the new offset in blocks
 */
void SuperBlock::setFreeListBlockOffset(int newFreeListBlockOffset) {
  freeListBlockOffset = newFreeListBlockOffset;
}

/**
 * Get the free list block offset
 * @return the free list block offset
 */
int SuperBlock::getFreeListBlockOffset() { return freeListBlockOffset; }

/**
 * Set the inodeBlockOffset (in blocks)
 * @param newInodeBlockOffset the new offset in blocks
 */
void SuperBlock::setInodeBlockOffset(int newInodeBlockOffset) {
  inodeBlockOffset = newInodeBlockOffset;
}

/**
 * Get the inode block offset (in blocks)
 * @return inode block offset in blocks
 */
int SuperBlock::getInodeBlockOffset() { return inodeBlockOffset; }

/**
 * Set the dataBlockOffset (in blocks)
 * @param newDataBlockOffset the new offset in blocks
 */
void SuperBlock::setDataBlockOffset(int newDataBlockOffset) {
  dataBlockOffset = newDataBlockOffset;
}

/**
 * Get the dataBlockOffset (in blocks)
 * @return the offset in blocks to the data block region
 */
int SuperBlock::getDataBlockOffset() { return dataBlockOffset; }

/**
 * writes this SuperBlock at the current position of the specified file.
 */
void SuperBlock::write(fstream& file) {
  if (file.is_open() == true) {
    file.write(reinterpret_cast<char*>(&blockSize), sizeof(short));
    file.write(reinterpret_cast<char*>(&blocks), sizeof(int));
    file.write(reinterpret_cast<char*>(&freeListBlockOffset), sizeof(int));
    file.write(reinterpret_cast<char*>(&inodeBlockOffset), sizeof(int));
    file.write(reinterpret_cast<char*>(&dataBlockOffset), sizeof(int));

    int nSize = sizeof(short) + sizeof(int) * 4;
    int dummySize = blockSize - nSize;

    char* dummy = new char[dummySize];
    memset(dummy, '\0', dummySize);
    file.write(dummy, dummySize);
    delete[] dummy;
  }
}

/**
 * reads this SuperBlock at the current position of the specified file.
 */
void SuperBlock::read(fstream& file) {
  if (file.is_open() == true) {
    file.read(reinterpret_cast<char*>(&blockSize), sizeof(short));
    file.read(reinterpret_cast<char*>(&blocks), sizeof(int));
    file.read(reinterpret_cast<char*>(&freeListBlockOffset), sizeof(int));
    file.read(reinterpret_cast<char*>(&inodeBlockOffset), sizeof(int));
    file.read(reinterpret_cast<char*>(&dataBlockOffset), sizeof(int));
  }
}
