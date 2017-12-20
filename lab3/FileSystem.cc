#include "FileSystem.h"
#include <stdlib.h>
#include <string.h>
#include "BitBlock.h"
#include "Kernel.h"
#include "SuperBlock.h"

FileSystem::FileSystem(char* newFilename, char* newMode) {
  blockSize = 0;
  blockCount = 0;
  freeListBlockOffset = 0;
  inodeBlockOffset = 0;
  dataBlockOffset = 0;

  currentFreeListBitNumber = 0;
  currentFreeListBlock = -1;

  currentIndexNodeNumber = 0;
  currentIndexNodeBlock = -1;
  indexBlockBytes = NULL;

  memset(filename, '\0', 256);
  strcpy(filename, newFilename);

  if (open() == false) {
    cout << "failed to open file for our file system" << endl;
  }
}

FileSystem::~FileSystem() {
  if (freeListBitBlock != NULL) {
    delete freeListBitBlock;
  }

  if (indexBlockBytes != NULL) {
    delete[] indexBlockBytes;
  }
}

/**
 * Get the blockSize for this FileSystem.
 * @return the block size in bytes
 */
short FileSystem::getBlockSize() { return blockSize; }

int FileSystem::getFreeListBlockOffset() { return freeListBlockOffset; }

int FileSystem::getInodeBlockOffset() { return inodeBlockOffset; }

int FileSystem::getDataBlockOffset() { return dataBlockOffset; }

/**
 * Get the rootIndexNode for this FileSystem.
 * @return the root index node
 */
IndexNode* FileSystem::getRootIndexNode() {
  //	cout << rootIndexNode.toString() << endl;
  return &rootIndexNode;
}

bool FileSystem::is_ready() { return isSystemReady; }

/**
 * Open a backing file for this FileSystem and read the superblock.
 */
bool FileSystem::open() {
  // Default R/W
  file.open(filename);  //, mode);

  if (file.is_open() == false) {
    isSystemReady = false;
    cout << "Failed to open filesystem file :" << filename << endl;
    return false;
  }

  // read the block size and other information from the superblock
  SuperBlock superBlock;
  superBlock.read(file);
  blockSize = superBlock.getBlockSize();
  blockCount = superBlock.getBlocks();
  // ??? inodeCount
  freeListBlockOffset = superBlock.getFreeListBlockOffset();
  inodeBlockOffset = superBlock.getInodeBlockOffset();
  dataBlockOffset = superBlock.getDataBlockOffset();

  freeListBitBlock = new BitBlock(blockSize);

  // initialize index block buffer
  indexBlockBytes = new char[blockSize];

  // read the root index node
  readIndexNode(&rootIndexNode, ROOT_INDEX_NODE_NUMBER);

  // cout << rootIndexNode.toString() << endl;

  isSystemReady = true;

  return true;
}

/**
 * Close the backing file for this FileSystem, if any.
 * @exception java.io.IOException if the closing the backing
 * file causes any IOException to be thrown
 */
void FileSystem::close() {
  //	cout << "I'm in FileSystem::Close();" << endl;
  if (file.is_open()) {
    file.close();
  }
}

/**
 * Read bytes into a buffer from the specified absolute block number
 * of the file system.
 * @param bytes the byte buffer into which the block should be read
 * @param blockNumber the absolute block number which should be read
 * @exception java.io.IOException if there are any exceptions during
 * the read from the underlying "file system" file.
 */
void FileSystem::read(char* bytes, int blockNumber) {
  file.seekg(blockNumber * blockSize);
  file.read(bytes, blockSize);
}

/**
 * Write bytes from a buffer to the specified absolute block number
 * of the file system.
 * @param bytes the byte buffer from which the block should be written
 * @param blockNumber the absolute block number which should be written
 * @exception java.io.IOException if there are any exceptions during
 * the write to the underlying "file system" file.
 */
void FileSystem::write(char* bytes, int blockNumber) {
  file.seekp(blockNumber * blockSize);
  file.write(bytes, blockSize);
}

/**
 * Mark a data block as being free in the free list.
 * @param dataBlockNumber the data block which is to be marked free
 * @exception java.io.IOException if any exception occurs during an
 * operation on the underlying "file system" file.
 */
void FileSystem::freeBlock(int dataBlockNumber) {
  loadFreeListBlock(dataBlockNumber);
  freeListBitBlock->resetBit(dataBlockNumber % (blockSize * 8));

  file.seekp((freeListBlockOffset + currentFreeListBlock) * blockSize);
  freeListBitBlock->write(file);
}

/**
 * Allocate a data block from the list of free blocks.
 * @return the data block number which was allocated; -1 if no blocks
 * are available
 * @exception java.io.IOException if any exception occurs during an
 * operation on the underlying "file system" file.
 */
int FileSystem::allocateBlock() {
  // from our current position in the free list block,
  // scan until we find an open position.  If we get back to
  // where we started, there are no free blocks and we return
  // -1.
  int save = currentFreeListBitNumber;

  while (true) {
    loadFreeListBlock(currentFreeListBitNumber);
    bool allocated =
        freeListBitBlock->isBitSet(currentFreeListBitNumber % (blockSize * 8));
    int previousFreeListBitNumber = currentFreeListBitNumber;
    currentFreeListBitNumber++;

    // if curr bit number >= data block count, set to 0
    if (currentFreeListBitNumber >= (blockCount - dataBlockOffset)) {
      currentFreeListBitNumber = 0;
    }

    if (!allocated) {
      freeListBitBlock->setBit(previousFreeListBitNumber % (blockSize * 8));
      file.seekp((freeListBlockOffset + currentFreeListBlock) * blockSize);
      freeListBitBlock->write(file);
      return previousFreeListBitNumber;
    }

    if (save == currentFreeListBitNumber) {
      Kernel::setErrno(Kernel::ENOSPC);
      return -1;
    }
  }
}

/**
 * Loads the block containing the specified data block bit into
 * the free list block buffer.  This is a convenience method.
 * @param dataBlockNumber the data block number
 * @exception java.io.IOException
 */
void FileSystem::loadFreeListBlock(int dataBlockNumber) {
  int neededFreeListBlock = dataBlockNumber / (blockSize * 8);

  if (currentFreeListBlock != neededFreeListBlock) {
    file.seekg((freeListBlockOffset + neededFreeListBlock) * blockSize);
    freeListBitBlock->read(file);
    currentFreeListBlock = neededFreeListBlock;
  }
}

/**
 * Allocate an index node for the file system.
 * @return the inode number for the next available index node;
 * -1 if there are no index nodes available.
 * @exception java.io.IOException if there is an exception during
 * an operation on the underlying "file system" file.
 */
short FileSystem::allocateIndexNode() {
  // from our current position in the index node block list,
  // scan until we find an open position.  If we get back to
  // where we started, there are no free inodes and we return
  // -1.
  short save = currentIndexNodeNumber;
  IndexNode temp;

  while (true) {
    readIndexNode(&temp, currentIndexNodeNumber);
    short previousIndexNodeNumber = currentIndexNodeNumber;
    currentIndexNodeNumber++;

    if (currentIndexNodeNumber >= ((dataBlockOffset - inodeBlockOffset) *
                                   (blockSize / IndexNode::INDEX_NODE_SIZE))) {
      currentIndexNodeNumber = 0;
    }

    if (temp.getNlink() == 0) {
      // ??? should we update nlinks here?
      return previousIndexNodeNumber;
    }
    if (save == currentIndexNodeNumber) {
      // ??? it seems like we should give a different error here
      Kernel::setErrno(Kernel::ENOSPC);
      return -1;
    }
  }
}

/**
 * Reads an index node at the index node location specified.
 * @param indexNode the index node
 * @param indexNodeNumber the location
 * @execption java.io.IOException if any exception occurs in an
 * underlying operation on the "file system" file.
 */
void FileSystem::readIndexNode(IndexNode* indexNode, short indexNodeNumber) {
  loadIndexNodeBlock(indexNodeNumber);
  indexNode->read(indexBlockBytes,
                  (indexNodeNumber * IndexNode::INDEX_NODE_SIZE) % blockSize);
}

/**
 * Writes an index node at the index node location specified.
 * @param indexNode the index node
 * @param indexNodeNumber the location
 * @execption java.io.IOException if any exception occurs in an
 * underlying operation on the "file system" file.
 */
void FileSystem::writeIndexNode(IndexNode* indexNode, short indexNodeNumber) {
  loadIndexNodeBlock(indexNodeNumber);
  indexNode->write(indexBlockBytes,
                   (indexNodeNumber * IndexNode::INDEX_NODE_SIZE) % blockSize);
  write(indexBlockBytes, inodeBlockOffset + currentIndexNodeBlock);
}

/**
 * Loads the block containing the specified index node into
 * the index node block buffer.  This is a convenience method.
 * @param indexNodeNumber the index node number
 * @exception java.io.IOException
 */
void FileSystem::loadIndexNodeBlock(short indexNodeNumber) {
  short neededIndexNodeBlock =
      (short)(indexNodeNumber / (blockSize / IndexNode::INDEX_NODE_SIZE));

  if (currentIndexNodeBlock != neededIndexNodeBlock) {
    read(indexBlockBytes, inodeBlockOffset + neededIndexNodeBlock);
    currentIndexNodeBlock = neededIndexNodeBlock;
  }
}
