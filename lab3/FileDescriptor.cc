#include "FileDescriptor.h"
#include <stdlib.h>
#include <string.h>
#include "Kernel.h"

FileDescriptor::FileDescriptor(FileSystem *newFileSystem,
                               IndexNode &newIndexNode, int newFlags) {
  deviceNumber = -1;
  indexNodeNumber = -1;
  offset = 0;

  flags = newFlags;
  fileSystem = newFileSystem;

  // copy index node info
  newIndexNode.copy(indexNode);
  bytes = new char[fileSystem->getBlockSize()];
  memset(bytes, '\0', fileSystem->getBlockSize());
}

FileDescriptor::~FileDescriptor() {
  if (bytes != NULL) {
    delete[] bytes;
  }
}

void FileDescriptor::setDeviceNumber(short newDeviceNumber) {
  deviceNumber = newDeviceNumber;
}

short FileDescriptor::getDeviceNumber() { return deviceNumber; }

IndexNode *FileDescriptor::getIndexNode() { return &indexNode; }

void FileDescriptor::setIndexNodeNumber(short newIndexNodeNumber) {
  indexNodeNumber = newIndexNodeNumber;
}

short FileDescriptor::getIndexNodeNumber() { return indexNodeNumber; }

int FileDescriptor::getFlags() { return flags; }

char *FileDescriptor::getBytes() { return bytes; }

short FileDescriptor::getMode() { return indexNode.getMode(); }

int FileDescriptor::getSize() { return indexNode.getSize(); }

void FileDescriptor::setSize(int newSize) {
  indexNode.setSize(newSize);

  // write the inode
  fileSystem->writeIndexNode(&indexNode, indexNodeNumber);
}

short FileDescriptor::getBlockSize() { return fileSystem->getBlockSize(); }

int FileDescriptor::getOffset() { return offset; }

void FileDescriptor::setOffset(int newOffset) { offset = newOffset; }

int FileDescriptor::readBlock(short relativeBlockNumber) {
  if ((int)relativeBlockNumber >
      (int)(fileSystem->getBlockSize() / sizeof(bytes)) +
          IndexNode::MAX_DIRECT_BLOCKS - 1) {
    Kernel::setErrno(Kernel::EFBIG);
    return -1;
  }
  int blockOffset;
  int blockSize = fileSystem->getBlockSize();
  if (relativeBlockNumber >= IndexNode::MAX_DIRECT_BLOCKS) {  // if we need to
                                                              // tap into the
                                                              // indirect block
    if (relativeBlockNumber == IndexNode::MAX_DIRECT_BLOCKS) {
      if (indexNode.getBlockAddress(IndexNode::MAX_DIRECT_BLOCKS) ==
          FileSystem::NOT_A_BLOCK) {
        for (int i = 0; i < blockSize; i++) {
          bytes[i] = (char)0;
        }
        return 0;
      }
      return 0;
    }
    char *temp = new char[fileSystem->getBlockSize()];
    int indirectblocknumber =
        indexNode.getBlockAddress(IndexNode::MAX_DIRECT_BLOCKS);
    memset(temp, '\0', fileSystem->getBlockSize());
    fileSystem->read(temp, fileSystem->getDataBlockOffset() +
                               indirectblocknumber);  // read the whole block
    char tempbufr[9];
    for (int i = 0; i < 8; i++) {
      tempbufr[i] =
          temp[((relativeBlockNumber - 10) * 8 +
                i)];  // copy only the area from the relative block number
    }
    tempbufr[8] = '\0';
    int x = atoi(tempbufr);
    if (x == FileSystem::NOT_A_BLOCK) {
      for (int i = 0; i < blockSize; i++) {
        bytes[i] = (char)0;
      }
      free(temp);
      return 0;
    } else {
      memset(bytes, '\0', blockSize);
      fileSystem->read(bytes, fileSystem->getDataBlockOffset() + x);
    }
    free(temp);
    return 0;
  } else {
    blockOffset = indexNode.getBlockAddress(relativeBlockNumber);
    if (blockOffset == FileSystem::NOT_A_BLOCK) {
      // clear the bytes if it's a block that was never written
      for (int i = 0; i < blockSize; i++) {
        bytes[i] = (char)0;
      }
    } else {
      memset(bytes, '\0', blockSize);
      fileSystem->read(bytes, fileSystem->getDataBlockOffset() + blockOffset);
    }
  }
  return 0;
}

int FileDescriptor::writeBlock(short relativeBlockNumber) {
  if ((int)relativeBlockNumber >
      (int)(fileSystem->getBlockSize() / sizeof(bytes)) +
          IndexNode::MAX_DIRECT_BLOCKS - 1) {  // need 1 byte for the terminator
    Kernel::setErrno(Kernel::EFBIG);
    return -1;
  }
  int blockOffset;
  if (relativeBlockNumber >= IndexNode::MAX_DIRECT_BLOCKS) {
    if (relativeBlockNumber == IndexNode::MAX_DIRECT_BLOCKS) {
      blockOffset = indexNode.getBlockAddress(IndexNode::MAX_DIRECT_BLOCKS);
      if (blockOffset == FileSystem::NOT_A_BLOCK) {  // if we have yet to use
                                                     // our indirect block
        blockOffset = fileSystem->allocateBlock();
        if (blockOffset < 0) {
          return -1;
        }
        indexNode.setBlockSize(fileSystem->getBlockSize() / sizeof(bytes) +
                               IndexNode::MAX_DIRECT_BLOCKS);
        indexNode.setBlockAddress(IndexNode::MAX_DIRECT_BLOCKS, blockOffset);
        fileSystem->writeIndexNode(&indexNode, indexNodeNumber);
        char *temp = new char[fileSystem->getBlockSize()];
        memset(temp, '\0', fileSystem->getBlockSize());
        fileSystem->write(temp, fileSystem->getDataBlockOffset() + blockOffset);
        for (int i = 0; i < (int)(fileSystem->getBlockSize() / sizeof(bytes));
             i++) {
          fileSystem->read(temp,
                           fileSystem->getDataBlockOffset() + blockOffset);
          strcat(temp, "16777215");  // //the value will be fasted later to
                                     // FileSystem::NOT_A_BLOCK
          fileSystem->write(temp,
                            fileSystem->getDataBlockOffset() + blockOffset);
        }
        memset(temp, '\0', fileSystem->getBlockSize());
        fileSystem->read(temp, fileSystem->getDataBlockOffset() + blockOffset);
        free(temp);
      }
    }
    char *temp = new char[fileSystem->getBlockSize()];
    int indirectblocknumber =
        indexNode.getBlockAddress(IndexNode::MAX_DIRECT_BLOCKS);
    memset(temp, '\0', fileSystem->getBlockSize());
    fileSystem->read(temp,
                     fileSystem->getDataBlockOffset() + indirectblocknumber);
    char subbuf[9];  // use this to read the vlalue from our indirect block
    memcpy(subbuf, &temp[(relativeBlockNumber - 10) * 8],
           8);  // find the location on the indirect block
    subbuf[9] = '\0';
    int x = atoi(subbuf);  // convert the offset from a string to int
    if (x == FileSystem::NOT_A_BLOCK) {
      blockOffset = fileSystem->allocateBlock();
      if (blockOffset < 0) {
        free(temp);
        return -1;
      }
      memset(subbuf, '\0', 9);
      sprintf(subbuf, "%d", blockOffset);
      for (int i = 0; i < 8; i++) {
        temp[(((relativeBlockNumber - 10) * 8) + i)] = subbuf[i];
      }
      fileSystem->write(temp, fileSystem->getDataBlockOffset() +
                                  indirectblocknumber);  // write the number as
                                                         // a string to the block
      fileSystem->writeIndexNode(&indexNode, indexNodeNumber);
      fileSystem->write(bytes,
                        fileSystem->getDataBlockOffset() +
                            blockOffset);  // write the bytes from the offset
    } else {
      fileSystem->write(bytes, fileSystem->getDataBlockOffset() +
                                   x);  // write the original bytes
    }
    free(temp);
    return 0;
  } else {
    int blockOffset = indexNode.getBlockAddress(relativeBlockNumber);
    if (blockOffset == FileSystem::NOT_A_BLOCK) {
      // allocate a block; quit if we can't
      blockOffset = fileSystem->allocateBlock();
      if (blockOffset < 0) {
        return -1;
      }
      // update the inode
      indexNode.setBlockAddress(relativeBlockNumber, blockOffset);
      // write the inode
      fileSystem->writeIndexNode(&indexNode, indexNodeNumber);
    }
    // write the actual block from bytes
    fileSystem->write(bytes, fileSystem->getDataBlockOffset() + blockOffset);

    return 0;
  }
}
