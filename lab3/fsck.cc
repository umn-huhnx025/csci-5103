#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Kernel.h"

// This program checks the file system's health. It checks that each inode
// reports the correct number of links and that each disk block is correctly
// marked allocated or free. The exit code is the sum of the following errors:
//                 0       No errors
//                 1       Number of links for an inode is incorrect
//                 2       Disk block is incorrectly marked free/allocated

// checkSubdirectories iteratively traverses the inodes and ensures each inode
// has the correct number of links.
int checkSubdirectories();

// countLinks returns the number of files in the whole filesystem that share the
// inode number ino. Returns -1 if an error occurred.
short countLinks(short ino);
short countLinksAux(short ino, char* path);

// recursiveLs recursively lists all paths in the file system beginning at path.
// It works just like countLinks, but doesn't check anything, it only lists path
// names.
int recursiveLs(char* path);
int recursiveLsAux(char* path, int indent);

int main(int argc, char* argv[]) {
  int result = 0;
  if (!Kernel::initialize()) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }

  // recursiveLs("/");
  // printf("\n");

  result = checkSubdirectories();

  Kernel::exit(result);
}

int checkSubdirectories() {
  // Allocate new BitBlock
  // For each inode
  // // Count number of links, make sure it's the same as what the inode says
  // // For each data block
  // // // Set bit in new list
  // Now, the new list should be a perfect representation of free/allocated
  // blocks
  // Get the freeListBitBlock from the FileSystem
  // Compare the two lists, report any differences

  // Generic error handling variable
  int error;

  // Keep track of if we've encountered an error so far
  int errorRet = 0;

  FileSystem* fs = Kernel::openFileSystems;
  IndexNode in;

  // Get free list
  int blockSize = fs->getBlockSize();
  BitBlock freeListBitBlock(blockSize);
  fs->read(freeListBitBlock.bytes, fs->getFreeListBlockOffset());

  // New free list we will construct as we iterate over all the inodes. This
  // should be a perfect representation of free/allocated blocks.
  BitBlock newFreeList(blockSize);

  int maxInodes = (fs->getDataBlockOffset() - fs->getInodeBlockOffset()) *
                  (blockSize / IndexNode::INDEX_NODE_SIZE);
  // printf("Max inodes: %d\n", numInodes);
  for (int i = 0; i < maxInodes; i++) {
    // printf("Checking inode %d\n", i);

    // Get the file for inode i
    fs->readIndexNode(&in, i);
    FileDescriptor* des = new FileDescriptor(fs, in, Kernel::O_RDONLY);
    int fd = Kernel::open(des);
    if (fd < 0) {
      Kernel::perror("fsck");
      return -1;
    }

    Stat s;
    if ((error = Kernel::fstat(fd, s)) < 0) {
      Kernel::perror("fsck");
      return -1;
    }

    if ((error = Kernel::close(fd)) < 0) {
      Kernel::perror("fsck");
      return -1;
    }

    // If no mode bits are set, then this must not be an allocated inode. This
    // probably isn't a foolproof check, but given the state of the base code, I
    // think it's a decent enough assumption to make.
    if (!s.st_mode) continue;

    // Mark used blocks as allocated in new free list
    for (int b = 0; b < IndexNode::MAX_FILE_BLOCKS; b++) {
      int address = in.getBlockAddress(b);
      if (address != FileSystem::NOT_A_BLOCK) {
        // printf("Block %d of inode %d is allocated (%#x)\n", b, i, address);
        newFreeList.setBit(address % (blockSize * 8));
      }
    }

    // Check number of links is correct
    short nlinks = s.st_nlink;
    short count = countLinks(i);
    if (count < 0) {
      return count;
    }
    if (nlinks != count) {
      printf("inode %d actually has %d links, inode reports %d links\n", i,
             count, nlinks);
      errorRet |= 0x1;
    }
  }

  // Compare the two free lists and report any differences
  for (int b = 0; b < blockSize * 8; b++) {
    if (newFreeList.isBitSet(b) != freeListBitBlock.isBitSet(b)) {
      errorRet |= 0x2;
      if (newFreeList.isBitSet(b)) {
        printf("Block %d should be marked allocated, but is not\n", b);
      } else {
        printf("Block %d should be marked free, but is not\n", b);
      }
    }
  }

  return errorRet;
}

short countLinks(short ino) {
  // printf("Counting names with inode %d\n", ino);
  return countLinksAux(ino, "/");
}

short countLinksAux(short ino, char* path) {
  // This is a recursive function that opens one file at each level of the tree.
  // If some path ever exceeds the maximum open file limit, we'll need to find a
  // different way to do this.

  // Number of links to inode ino in directory path
  int n = 0;
  int error;

  int fd = Kernel::open(path, Kernel::O_RDONLY);
  if (fd < 0) {
    Kernel::perror(path);
    return -1;
  }

  // Check inode of this path
  Stat s;
  if ((error = Kernel::fstat(fd, s)) < 0) {
    Kernel::perror(path);
    return -1;
  }
  if (s.st_ino == ino) {
    // printf("  %s\n", path);
    n++;
  }

  int r;
  DirectoryEntry d;
  char buf[1024];
  while (true) {
    // Read the next entry in this directory
    r = Kernel::readdir(fd, d);

    // End of directory
    if (r == 0) break;

    // Some error occurred
    if (r < 0) {
      Kernel::perror(buf + 1);
      return -1;
    }

    // Strip a leading "/" if path starts with "//", unless path == "/"
    char* fmt;
    if (!(strncmp(path, "/", strlen(path))))
      fmt = "%s%s";
    else
      fmt = "%s/%s";
    sprintf(buf, fmt, path, d.d_name);

    // printf("Counting files in %s\n", buf + 1);

    // Check for . and .., don't make recursive call on these or else it'll loop
    // forever
    if (!strncmp(d.d_name, ".", 1) || !strncmp(d.d_name, "..", 2)) {
      if (d.d_ino == ino) {
        // printf("  %s\n", buf);
        n++;
      }
      continue;
    } else {
      // Get metadata on the next file in this directory
      int fd2 = Kernel::open(buf, Kernel::O_RDONLY);
      if (fd2 < 0) {
        Kernel::perror(buf);
        return -1;
      }
      if ((error = Kernel::fstat(fd2, s)) < 0) {
        Kernel::perror(buf);
        return -1;
      }
      if ((error = Kernel::close(fd2)) < 0) {
        Kernel::perror(buf);
        return -1;
      }

      // Check inode
      if ((short)(s.st_mode & Kernel::S_IFMT) == Kernel::S_IFDIR) {
        // printf("%s is a directory, counting files there\n", buf + 1);

        // Looking at a directory, count links in the next subdirectory
        short next = countLinksAux(ino, buf);
        if (next < 0) {
          return -1;
        } else {
          n += next;
        }
      } else {
        // Not a directory, check this file's inode
        if (s.st_ino == ino) {
          // printf("  %s\n", buf);
          n++;
        }
      }
    }
  }

  // Can't close the directory while we're traersing it.
  if ((error = Kernel::close(fd)) < 0) {
    Kernel::perror(path);
    return -1;
  }

  return n;
}

int recursiveLs(char* path) {
  printf("inode: name\n");
  return recursiveLsAux(path, 0);
}

int recursiveLsAux(char* path, int indent) {
  int error = 0;
  DirectoryEntry d;
  Stat s;

  int fd;
  if ((fd = Kernel::open(path, Kernel::O_RDONLY)) < 0) {
    Kernel::perror(path);
    return fd;
  }
  int f;
  if ((f = Kernel::fstat(fd, s)) < 0) {
    Kernel::perror(path);
    return -1;
  }

  for (int i = 0; i < indent; i++) printf("  ");
  printf("%d: %s\n", s.st_ino, path);

  if ((short)(s.st_mode & Kernel::S_IFMT) != Kernel::S_IFDIR) {
    int c;
    if ((c = Kernel::close(fd)) < 0) {
      Kernel::perror(path);
      return c;
    }
    return error;
  }

  char buf[1024];
  int r;
  while (true) {
    // Read the next entry in this directory
    r = Kernel::readdir(fd, d);

    // End of directory
    if (r == 0) break;

    // Some error occurred
    if (r < 0) {
      Kernel::perror(path);
      return r;
    }

    // Ignore '.' and '..' files
    if (!strncmp(d.d_name, ".", 1) || !strncmp(d.d_name, "..", 2)) continue;

    // buf holds the name of the next file in the directory at path
    char* fmt;
    if (!(strncmp(path, "/", strlen(path))))
      fmt = "%s%s";
    else
      fmt = "%s/%s";
    sprintf(buf, fmt, path, d.d_name);

    error |= recursiveLsAux(buf, indent + 1);
  }

  int c;
  if ((c = Kernel::close(fd)) < 0) {
    Kernel::perror(path);
    return c;
  }

  return error;
}
