#include "BitBlock.h"

BitBlock::BitBlock(short blockSize) : Block(blockSize) {}

void BitBlock::setBit(int whichBit) {
  bytes[whichBit / 8] |= (char)(1 << (whichBit % 8));
}

void BitBlock::setBit(int whichBit, bool value) {
  if (value) {
    setBit(whichBit);
  } else {
    resetBit(whichBit);
  }
}

bool BitBlock::isBitSet(int whichBit) {
  return (bytes[whichBit / 8] & (char)(1 << (whichBit % 8))) != 0;
}

void BitBlock::resetBit(int whichBit) {
  bytes[whichBit / 8] &= ~(char)(1 << (whichBit % 8));
}
