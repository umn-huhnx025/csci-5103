#include "Kernel.h"
#include <stdlib.h>
#include <string.h>
#include <cstdlib>

/**
 * Prints a system error message.  The actual text written
 * to stderr is the
 * given string, followed by a colon, a space, the message
 * text, and a newline.  It is customary to give the name of
 * the program as the argument to perror.
 * @param s the program name
 */

// char * Kernel::sys_errlist[32];
char Kernel::PROGRAM_NAME[512];
ProcessContext Kernel::process;
FileSystem* Kernel::openFileSystems;
int Kernel::processCount;
FileDescriptor** Kernel::openFiles;
int Kernel::MAX_OPEN_FILES;

const char* g_sys_errlist[32] = {NULL,
                                 "Not owner",
                                 "No such file or directory",
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 "Bad file number",
                                 NULL,
                                 NULL,
                                 NULL,
                                 "Permission denied",
                                 NULL,
                                 NULL,
                                 NULL,
                                 "File exists",
                                 "Cross-device link",
                                 NULL,
                                 "Not a directory",
                                 "Is a directory",
                                 "Invalid argument",
                                 "File table overflow",
                                 "Too many open files",
                                 NULL,
                                 NULL,
                                 "File too large",
                                 "No space left on device",
                                 NULL,
                                 "Read-only file system",
                                 "Too many links"};

void Kernel::perror(const char* s) {
  bool support_error = false;
  char error[512];

  memset(error, '\0', 512);

  if ((process.errno > 0) && (process.errno < sys_nerr)) {
    support_error = true;
    strcpy(error, g_sys_errlist[process.errno]);
  }

  if (support_error == false) {
    cout << s << ": unknown errno " << process.errno << endl;
  } else {
    cout << s << ": " << error << endl;
  }
}

/**
 * Set the value of errno for the current process.
 * <p>
 * Simulates the unix variable:
 * <pre>
 *   extern int errno ;
 * </pre>
 * @see getErrno
 */
void Kernel::setErrno(int newErrno) { process.errno = newErrno; }

/**
 * Get the value of errno for the current process.
 * <p>
 * Simulates the unix variable:
 * <pre>
 *   extern int errno ;
 * </pre>
 * @see setErrno
 */
int Kernel::getErrno() { return process.errno; }

/**
 * Closes the specified file descriptor.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int close(int fd);
 * </pre>
 * @param fd the file descriptor of the file to close
 * @return Zero if the file is closed; -1 if the file descriptor
 * is invalid.
 */
int Kernel::close(int fd) {
  // check fd
  int status = check_fd(fd);
  if (status < 0) {
    return status;
  }

  // Get filesystem and inode
  FileSystem* fileSystem = openFileSystems;
  IndexNode indexNode;
  short indexNodeNumber = process.openFiles[fd]->getIndexNodeNumber();

  process.openFiles[fd]->getIndexNode()->copy(indexNode);
  short numberLinks = indexNode.getNlink();

  // If no one else using the file
  if (numberLinks == 0) {
    // free any blocks currently allocated to the file
    int blockSize = fileSystem->getBlockSize();
    int blocks = (indexNode.getSize() + blockSize - 1) / blockSize;
    for (int i = 0; i < blocks; i++) {
      int address = indexNode.getBlockAddress(i);
      if (address != FileSystem::NOT_A_BLOCK) {
        fileSystem->freeBlock(address);
        indexNode.setBlockAddress(i, FileSystem::NOT_A_BLOCK);
      }
    }

    // update the inode to size 0
    indexNode.setSize(0);

    // write the inode to the file system.
    fileSystem->writeIndexNode(&indexNode, indexNodeNumber);
  }

  // remove the file descriptor from the kernel's list of open files
  for (int i = 0; i < MAX_OPEN_FILES; i++) {
    if (openFiles[i] == process.openFiles[fd]) {
      delete openFiles[i];
      openFiles[i] = NULL;
      break;
    }
  }
  // ??? is it an error if we didn't find the open file?

  // remove the file descriptor from the list.
  process.openFiles[fd] = NULL;
  return 0;
}

/**
 * Creates a file or directory with the specified mode.
 * <p>
 * Creates a new file or prepares to rewrite an existing file.
 * If the file does not exist, it is given the mode specified.
 * If the file does exist, it is truncated to length zero.
 * The file is opened for writing and its file descriptor is
 * returned.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int creat(const char *pathname, mode_t mode);
 * </pre>
 * @param pathname the name of the file or directory to create
 * @param mode the file or directory protection mode for the new file
 * @return the file descriptor (a non-negative integer); -1 if
 * a needed directory is not searchable, if the file does not
 * exist and the directory in which it is to be created is not
 * writable, if the file does exist and is unwritable, if the
 * file is a directory, or if there are already too many open
 * files.
 * @exception java.lang.Exception if any underlying action causes
 * an exception to be thrown
 */
int Kernel::creat(char* pathname, short mode) {
  // get the full path
  char* fullPath = getFullPath(pathname);
  char dirname[1024];
  memset(dirname, '\0', 1024);
  strcpy(dirname, "/");

  FileSystem* fileSystem = openFileSystems;
  IndexNode currIndexNode;
  IndexNode prevIndexNode;
  IndexNode emptyIndexNode;
  getRootIndexNode()->copy(currIndexNode);

  // out << "CurIndexNode " << currIndexNode.toString() << endl;

  //	cout << currIndexNode.toString();

  short indexNodeNumber = FileSystem::ROOT_INDEX_NODE_NUMBER;

  char* token = NULL;
  token = strtok(fullPath, "/");
  char name[512];  // = "." ; // start at root node
  memset(name, '\0', 512);
  strcpy(name, ".");  // may be not needed.

  while (1) {
    if (token != NULL) {
      memset(name, '\0', 512);
      strcpy(name, token);

      //			cout << name << endl;
      // check to see if the current node is a directory

      //			cout <<currIndexNode.toString() << endl;

      if ((currIndexNode.getMode() & S_IFMT) != S_IFDIR) {
        // return (ENOTDIR) if a needed directory is not a directory
        process.errno = ENOTDIR;
        return -1;
      }

      // get the next inode corresponding to the token
      currIndexNode.copy(prevIndexNode);

      // Init CurIndexNode
      emptyIndexNode.copy(currIndexNode);
      // prevIndexNode = currIndexNode ;
      //   currIndexNode = new IndexNode() ;
      indexNodeNumber = findNextIndexNode(openFileSystems, prevIndexNode, name,
                                          currIndexNode);
    } else {
      break;
    }

    token = strtok(NULL, "/");

    if (token != NULL) {
      strcat(dirname, name);
      strcat(dirname, "/");
    }
  }

  // ??? we need to set some fields in the file descriptor

  int flags = O_WRONLY;  // ???
  FileDescriptor* fileDescriptor = NULL;

  if (indexNodeNumber < 0) {
    // file does not exist.  We check to see if we can create it.

    // check to see if the prevIndexNode (a directory) is writeable
    // ??? tbd
    // return (EACCES) if the file does not exist and the directory
    // in which it is to be created is not writable
    currIndexNode.setMode(mode);
    currIndexNode.setNlink((short)1);

    // allocate the next available inode from the file system
    short newInode = fileSystem->allocateIndexNode();

    if (newInode == -1) {
      return -1;
    }

    // Do writeIndex first -KS
    fileSystem->writeIndexNode(&currIndexNode, newInode);

    //		cout << "new file des " << currIndexNode.toString() << endl;

    fileDescriptor = new FileDescriptor(fileSystem, currIndexNode, flags);
    // assign inode for the new file
    fileDescriptor->setIndexNodeNumber(newInode);

    // System.out.println( "newInode = " + newInode ) ;
    // open the directory
    // ??? it would be nice if we had an "open" that took an inode
    // instead of a name for the dir
    // System.out.println( "dirname = " + dirname.toString() ) ;
    //		cout << "dir name and name " << dirname << " " << name << endl;
    int dir = open(dirname, O_RDWR);
    if (dir < 0) {
      perror(PROGRAM_NAME);
      cout << PROGRAM_NAME << ": unable to open directory for writing" << endl;
      return -1;
    }

    // scan past the directory entries less than the current entry
    // and insert the new element immediately following

    //		cout << "New dir name " << name << endl;
    int status = 0;
    DirectoryEntry newDirectoryEntry(newInode, name);
    DirectoryEntry currentDirectoryEntry;

    while (true) {
      // read an entry from the directory
      status = readdir(dir, currentDirectoryEntry);

      if (status < 0) {
        cout << PROGRAM_NAME << ": error reading directory in creat";
        exit(EXIT_FAILURE);
      } else if (status == 0) {
        // if no entry read, write the new item at the current
        // location and break
        writedir(dir, newDirectoryEntry);
        break;
      } else {
        // if current item > new item, write the new item in
        // place of the old one and break
        if (strcmp(currentDirectoryEntry.getName(),
                   newDirectoryEntry.getName()) > 0) {
          int seek_status =
              lseek(dir, -DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
          //					cout << "lseek status" <<
          // seek_status
          //<<  endl;
          if (seek_status < 0) {
            cout << PROGRAM_NAME << ": error during seek in creat";
            exit(EXIT_FAILURE);
          }

          writedir(dir, newDirectoryEntry);
          break;
        }
      }
    }

    // copy the rest of the directory entries out to the file
    DirectoryEntry nextDirectoryEntry;

    while (status > 0) {
      // read next item
      status = readdir(dir, nextDirectoryEntry);

      if (status > 0) {
        // in its place
        int seek_status = lseek(dir, -DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
        if (seek_status < 0) {
          cout << PROGRAM_NAME << ": error during seek in creat";
          exit(EXIT_FAILURE);
        }
      }
      // write current item
      //			cout << currentDirectoryEntry.toString() <<
      // endl;
      writedir(dir, currentDirectoryEntry);

      //			cout << "Old : " <<
      // currentDirectoryEntry.toString() << endl;
      // current item = next item
      nextDirectoryEntry.copy(currentDirectoryEntry);

      //			cout << "Next : " <<
      // nextDirectoryEntry.toString() << endl;
    }

    // close the directory
    close(dir);
  } else {
    // file does exist ( indexNodeNumber >= 0 )

    // if it's a directory, we can't truncate it
    if ((currIndexNode.getMode() & S_IFMT) == S_IFDIR) {
      // return (EISDIR) if the file is a directory
      process.errno = EISDIR;
      return -1;
    }

    // check to see if the file is writeable by the user
    // ??? tbd
    // return (EACCES) if the file does exist and is unwritable

    // free any blocks currently allocated to the file
    int blockSize = fileSystem->getBlockSize();
    int blocks = (currIndexNode.getSize() + blockSize - 1) / blockSize;

    for (int i = blocks - 1; i >= 0; i--) {
      if (i > IndexNode::MAX_DIRECT_BLOCKS) {  // free the indirect blocks if
                                               // any
        char* temp = new char[fileSystem->getBlockSize()];
        memset(temp, '\0', fileSystem->getBlockSize());
        int indirectblocknumber = currIndexNode.getBlockAddress(
            IndexNode::MAX_DIRECT_BLOCKS);  // get indirect block adress
        fileSystem->read(
            temp, fileSystem->getDataBlockOffset() + indirectblocknumber);
        char tempbufr[9];
        for (int j = 0; j < 8; j++) {
          tempbufr[j] = temp[((i - 10) * 8 + j)];  // copy only the section from
                                                   // the indirectblock starting
                                                   // from i
        }
        tempbufr[8] = '\0';
        int x = atoi(tempbufr);
        if (x != FileSystem::NOT_A_BLOCK) {
          //  cout << "free indirect block block" << x << endl;
          fileSystem->freeBlock(x);
        }
        free(temp);
      } else {
        int address = currIndexNode.getBlockAddress(i);
        if (address != FileSystem::NOT_A_BLOCK) {
          //  cout << "free block" << address << endl;
          fileSystem->freeBlock(address);
          currIndexNode.setBlockAddress(i, FileSystem::NOT_A_BLOCK);
        }
      }
    }

    // update the inode to size 0
    currIndexNode.setSize(0);

    // write the inode to the file system.
    fileSystem->writeIndexNode(&currIndexNode, indexNodeNumber);

    // set up the file descriptor
    fileDescriptor = new FileDescriptor(fileSystem, currIndexNode, flags);
    // assign inode for the new file
    fileDescriptor->setIndexNodeNumber(indexNodeNumber);
  }

  return open(fileDescriptor);
}

/**
 * Terminate the current "process".  Any open files will be closed.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   exit(int status);
 * </pre>
 * <p>
 * Note: If this is the last process to terminate, this method
 * calls finalize().
 * @param status the exit status
 * @exception java.lang.Exception if any underlying
 * Exception is thrown
 */
void Kernel::exit(int status) {
  //	cout << "exit" <<endl;
  // close anything that might be open for the current process
  for (int i = 0; i < process.getOpenFilesCount(); i++) {
    if (process.openFiles[i] != NULL) {
      close(i);
    }
  }

  // terminate the process
  processCount--;

  // if this is the last process to end, call finalize
  if (processCount <= 0) {
    finalize(status);
  }
}

/**
 * Set the current file pointer for a file.
 * The current file position is updated based on the values of
 * offset and whence.  If whence is 0, the new position is
 * offset bytes from the beginning of the file.  If whence is
 * 1, the new position is the current position plus the value
 * of offset.  If whence is 2, the new position is the size
 * of the file plus the offset value.  Note that offset may be
 * negative if whence is 1 or 2, as long as the resulting
 * position is not less than zero.  It is valid to position
 * past the end of the file, but it is not valid to read
 * past the end of the file.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   lseek( int filedes , int offset , int whence );
 * </pre>
 * @param fd the file descriptor
 * @param offset the offset
 * @param whence 0 = from beginning of file; 1 = from
 * current position ; 2 = from end of file
 */
int Kernel::lseek(int fd, int offset, int whence) {
  // check fd
  int status = check_fd(fd);
  if (status < 0) return status;

  FileDescriptor* file = process.openFiles[fd];

  int newOffset;
  if (whence == 0)
    newOffset = offset;
  else if (whence == 1)
    newOffset = file->getOffset() + offset;
  else if (whence == 2)
    newOffset = file->getSize() + offset;
  else {
    // bad whence value
    process.errno = EINVAL;
    return -1;
  }

  if (newOffset < 0) {
    // bad offset value
    process.errno = EINVAL;
    return -1;
  }

  file->setOffset(newOffset);
  return newOffset;
}

/**
 * Opens a file or directory for reading, writing, or
 * both reading and writing.
 * <p>
 * The file is positioned at the beginning (byte 0).
 * The returned file descriptor must be used for subsequent
 * calls for other input and output functions on the file.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int open(const char *pathname, int flags );
 * </pre>
 * @param pathname the name of the file or directory to create
 * @param flags the flags to use when opening the file: O_RDONLY,
 * O_WRONLY, or O_RDWR.
 * @return the file descriptor (a non-negative integer); -1 if
 * the file does not exist, if one of the necessary directories
 * does not exist or is unreadable, if the file is not readable
 * (resp. writable), or if too many files are open.
 * @exception java.lang.Exception if any underlying action causes
 * an exception to be thrown
 */
int Kernel::open(char* pathname, int flags) {
  // get the full path name
  char* fullPath = getFullPath(pathname);

  IndexNode indexNode;
  short indexNodeNumber = findIndexNode(fullPath, indexNode);

  if (indexNodeNumber < 0) {
    return -1;
  }

  // ??? return (Exxx) if the file is not readable
  // and was opened O_RDONLY or O_RDWR

  // ??? return (Exxx) if the file is not writable
  // and was opened O_WRONLY or O_RDWR

  // set up the file descriptor
  FileDescriptor* fileDescriptor =
      new FileDescriptor(openFileSystems, indexNode, flags);
  fileDescriptor->setIndexNodeNumber(indexNodeNumber);

  return open(fileDescriptor);
}

/**
 * Open a file using a FileDescriptor.  The open and create
 * methods build a file descriptor and then invoke this method
 * to complete the open process.
 * <p>
 * This is a convenience method for the simulator kernel.
 * @param fileDescriptor the file descriptor
 * @return the file descriptor index in the process open file
 * list assigned to this open file
 */
int Kernel::open(FileDescriptor* fileDescriptor) {
  // scan the kernel open file list for a slot
  // and add our new file descriptor
  int kfd = -1;
  for (int i = 0; i < MAX_OPEN_FILES; i++) {
    if (openFiles[i] == NULL) {
      kfd = i;
      openFiles[kfd] = fileDescriptor;
      break;
    }
  }

  if (kfd == -1) {
    // return (ENFILE) if there are already too many open files
    process.errno = ENFILE;
    delete fileDescriptor;
    return -1;
  }

  // scan the list of open files for a slot
  // and add our new file descriptor
  int fd = -1;
  for (int i = 0; i < process.getMaxOpenFiles(); i++) {
    if (process.openFiles[i] == NULL) {
      fd = i;
      process.openFiles[fd] = fileDescriptor;
      break;
    }
  }

  if (fd == -1) {
    // remove the file from the kernel list
    openFiles[kfd] = NULL;
    // return (EMFILE) if there isn't room left
    process.errno = EMFILE;
    delete fileDescriptor;
    return -1;
  }

  // return the index of the file descriptor for now open file
  return fd;
}

/**
 * Read bytes from a file.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int read(int fd, void *buf, size_t count);
 * </pre>
 * @param fd the file descriptor of a file open for reading
 * @param buf an array of bytes into which bytes are read
 * @param count the number of bytes to read from the file
 * @return the number of bytes actually read; or -1 if an error occurs.
 * @exception java.lang.Exception if any underlying action causes
 * Exception to be thrown
 */
int Kernel::read(int fd, char* buf, int count) {
  // check fd
  int status = check_fd_for_read(fd);
  if (status < 0) return status;

  FileDescriptor* file = process.openFiles[fd];
  int offset = file->getOffset();
  int size = file->getSize();
  int blockSize = file->getBlockSize();
  char* bytes = file->getBytes();
  int readCount = 0;
  if (count >= IndexNode::MAX_DIRECT_BLOCKS) count--;
  for (int i = 0; i < count; i++) {  // acount for the index node allocation
    // if we read to the end of the file, stop reading
    if (offset >= size) break;
    // if this is the first time through the loop,
    // or if we're at the beginning of a block, load the data block
    if ((i == 0) || ((offset % blockSize) == 0)) {
      status = file->readBlock((short)(offset / blockSize));
      if (status < 0) return status;
    }
    // copy a byte from the file buffer to the read buffer
    buf[i] = bytes[offset % blockSize];
    offset++;
    readCount++;
  }
  // update the offset
  file->setOffset(offset);

  // return the count of bytes read
  return readCount;
}

/**
 * Reads a directory entry from a file descriptor for an open directory.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int readdir(unsigned int fd, struct dirent *dirp ) ;
 * </pre>
 * Note that count is ignored in the unix call.
 * @param fd the file descriptor for the directory being read
 * @param dirp the directory entry into which data should be copied
 * @return number of bytes read; 0 if end of directory; -1 if the file
 * descriptor is invalid, or if the file is not opened for read access.
 * @exception java.lang.Exception if any underlying action causes
 * Exception to be thrown
 */
int Kernel::readdir(int fd, DirectoryEntry& dirp) {
  // check fd
  int status = check_fd_for_read(fd);
  if (status < 0) {
    return status;
  }

  FileDescriptor* file = process.openFiles[fd];

  // check to see if the file is a directory
  if ((file->getMode() & S_IFMT) != S_IFDIR) {
    // return (ENOTDIR) if a needed directory is not a directory
    process.errno = ENOTDIR;
    return -1;
  }

  //	cout << "offset " << file->getOffset() << " size "  << file->getSize()
  //<< endl;

  // return 0 if at end of directory
  //	cout << "getOffset()" <<  file->getOffset() << " getSize " <<
  // file->getSize();
  if (file->getOffset() >= file->getSize()) {
    //		cout << "we could see 48 offset not happen in java"<< endl;
    return 0;
  }

  // read a block, if needed
  status = file->readBlock((short)(file->getOffset() / file->getBlockSize()));
  if (status < 0) {
    return status;
  }

  // read bytes from the block into the DirectoryEntry
  // OKS Probelm here
  dirp.read(file->getBytes(), file->getOffset() % file->getBlockSize());

  //	cout << dirp.toString()  << endl;
  int newOffset = file->getOffset() + DirectoryEntry::DIRECTORY_ENTRY_SIZE;
  file->setOffset(newOffset);

  //	cout << "new offset " << newOffset << endl;
  // return the size of a DirectoryEntry
  return DirectoryEntry::DIRECTORY_ENTRY_SIZE;
}

/**
 * Obtain information for an open file.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int fstat(int filedes, struct stat *buf);
 * </pre>
 * @exception java.lang.Exception if any underlying action causes
 * Exception to be thrown
 */
int Kernel::fstat(int fd, Stat& buf) {
  // check fd
  int status = check_fd(fd);
  if (status < 0) return status;

  FileDescriptor* fileDescriptor = process.openFiles[fd];
  short deviceNumber = fileDescriptor->getDeviceNumber();
  short indexNodeNumber = fileDescriptor->getIndexNodeNumber();

  IndexNode indexNode;
  fileDescriptor->getIndexNode()->copy(indexNode);

  // copy information to buf
  buf.st_dev = deviceNumber;
  buf.st_ino = indexNodeNumber;
  buf.copyIndexNode(indexNode);

  return 0;
}

/**
 * Obtain information about a named file.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int stat(const char *name, struct stat *buf);
 * </pre>
 * @exception java.lang.Exception if any underlying action causes
 * Exception to be thrown
 */
int Kernel::stat(char* name, Stat& buf) {
  // a buffer for reading directory entries
  DirectoryEntry directoryEntry;

  // get the full path
  char* fullPath = getFullPath(name);

  //	cout << "Stat Full Path: " << fullPath << endl;

  // find the index node
  IndexNode indexNode;

  short indexNodeNumber = findIndexNode(fullPath, indexNode);
  if (indexNodeNumber < 0) {
    // return ENOENT
    process.errno = ENOENT;
    return -1;
  }

  // copy information to buf
  buf.st_dev = ROOT_FILE_SYSTEM;
  buf.st_ino = indexNodeNumber;
  buf.copyIndexNode(indexNode);

  return 0;
}

/**
 * First commits inodes to buffers, and then buffers to disk.
 * <p>
 * Simulates unix system call:
 * <pre>
 *   int sync(void);
 * </pre>
 */
void Kernel::sync() {
  // write out superblock if updated
  // write out free list blocks if updated
  // write out inode blocks if updated
  // write out data blocks if updated

  // at present, all changes to inodes, data blocks,
  // and free list blocks
  // are written as they go, so this method does nothing.
}

/**
 * Write bytes to a file.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int write(int fd, const void *buf, size_t count);
 * </pre>
 * @exception java.lang.Exception if any underlying action causes
 * Exception to be thrown
 */
int Kernel::write(int fd, char* buf, int count) {
  //	cout << "Kernel::write Count : " << count << endl;
  // check fd
  int status = check_fd_for_write(fd);
  if (status < 0) {
    return status;
  }

  FileDescriptor* file = process.openFiles[fd];

  // return (ENOSPC) if the device containing the file system
  // referred to by fd has not room for the data

  int offset = file->getOffset();
  int size = file->getSize();
  int blockSize = file->getBlockSize();
  char* bytes = file->getBytes();
  int writeCount = 0;
  for (int i = 0; i < count; i++) {
    // if this is the first time through the loop,
    // or if we're at the beginning of a block,
    // load or allocate a data block
    if ((i == 0) || ((offset % blockSize) == 0)) {
      status = file->readBlock((short)(offset / blockSize));
      if (status < 0) {
        return status;
      }
    }
    // copy a byte from the write buffer to the file buffer
    bytes[offset % blockSize] = buf[i];
    offset++;
    // if we get to the end of a block, write it out
    if ((offset % blockSize) == 0) {
      status = file->writeBlock((short)((offset - 1) / blockSize));
      if (status < 0) {
        return status;
      }

      // update the file size if it grew
      if (offset > size) {
        file->setSize(offset);
        size = offset;
      }
    }
    writeCount++;
  }

  // write the last block if we wrote anything to it
  if ((offset % blockSize) > 0) {
    status = file->writeBlock((short)((offset - 1) / blockSize));

    if (status < 0) {
      //			cout << "here " << endl;
      return status;
    }
  }

  // update the file size if it grew
  if (offset > size) {
    file->setSize(offset);
  }

  // update the offset
  file->setOffset(offset);

  // return the count of bytes written
  return writeCount;
}

/**
 * Writes a directory entry from a file descriptor for an
 * open directory.
 * <p>
 * Simulates the unix system call:
 * <pre>
 *   int readdir(unsigned int fd, struct dirent *dirp ) ;
 * </pre>
 * Note that count is ignored in the unix call.
 * @param fd the file descriptor for the directory being read
 * @param dirp the directory entry into which data should be copied
 * @return number of bytes read; 0 if end of directory; -1 if the file
 * descriptor is invalid, or if the file is not opened for read access.
 * @exception java.lang.Exception if any underlying action causes
 * Exception to be thrown
 */
int Kernel::writedir(int fd, DirectoryEntry& dirp) {
  // check fd
  int status = check_fd_for_write(fd);
  if (status < 0) return status;

  FileDescriptor* file = process.openFiles[fd];

  // check to see if the file is a directory
  if ((file->getMode() & S_IFMT) != S_IFDIR) {
    // return (ENOTDIR) if a needed directory is not a directory
    process.errno = ENOTDIR;
    return -1;
  }

  short blockSize = file->getBlockSize();
  // allocate or read a block
  status = file->readBlock((short)(file->getOffset() / blockSize));
  if (status < 0) {
    return status;
  }

  //	cout << "dir written " << dirp.toString() << " offset" <<
  // file->getOffset() << endl;

  // write bytes from the DirectoryEntry into the block
  dirp.write(file->getBytes(), file->getOffset() % blockSize);

  // write the updated block
  status = file->writeBlock((short)(file->getOffset() / blockSize));
  if (status < 0) {
    return status;
  }

  // cout <<"From dir write " <<  dirp.toString() << endl;

  // update the file size
  file->setOffset(file->getOffset() + DirectoryEntry::DIRECTORY_ENTRY_SIZE);
  if (file->getOffset() > file->getSize()) {
    file->setSize(file->getOffset());
  }

  // return the size of a DirectoryEntry
  return DirectoryEntry::DIRECTORY_ENTRY_SIZE;
}

/**
 * Simulates the UNIX system call:
 *   `ln oldpath newpath`
 * @param oldpath the target file
 * @param newpath the link file to be created
 *
 * Most code was copied from Kernel::creat()
 */
int Kernel::link(char* oldpath, char* newpath) {
  // Get the full path of old path argument
  char* full_oldPath = getFullPath(oldpath);
  char old_directory[1024];
  memset(old_directory, '\0', 1024);
  strcpy(old_directory, "/");

  // Set up inode variables
  IndexNode currIndexNode;
  IndexNode prevIndexNode;
  IndexNode emptyIndexNode;
  getRootIndexNode()->copy(currIndexNode);

  // Initialize inode number to the root inode number
  short indexNodeNumber = FileSystem::ROOT_INDEX_NODE_NUMBER;

  // Tokenize full old path into directories
  char* token = strtok(full_oldPath, "/");
  char old_filename[512];  // = "." ; // start at root node
  memset(old_filename, '\0', 512);
  strcpy(old_filename, ".");  // may be not needed.

  // Get inode data for oldpath file
  while (1) {
    if (token != NULL) {
      memset(old_filename, '\0', 512);
      strcpy(old_filename, token);

      if ((currIndexNode.getMode() & S_IFMT) != S_IFDIR) {
        // return (ENOTDIR) if a needed directory is not a directory
        process.errno = ENOTDIR;
        return -1;
      }

      // get the next inode corresponding to the token
      currIndexNode.copy(prevIndexNode);

      // Init CurIndexNode
      emptyIndexNode.copy(currIndexNode);
      // prevIndexNode = currIndexNode ;
      //   currIndexNode = new IndexNode() ;
      indexNodeNumber = findNextIndexNode(openFileSystems, prevIndexNode,
                                          old_filename, currIndexNode);
    } else {
      break;
    }

    token = strtok(NULL, "/");

    if (token != NULL) {
      strcat(old_directory, old_filename);
      strcat(old_directory, "/");
    }
  }

  // Update inode variables
  IndexNode old_indexNode;
  currIndexNode.copy(old_indexNode);
  int old_indexNodeNumber = indexNodeNumber;

  // Get the full path of old path argument
  char* full_newPath = getFullPath(newpath);
  char new_directory[1024];
  memset(new_directory, '\0', 1024);
  strcpy(new_directory, "/");

  // Reset inode variables
  getRootIndexNode()->copy(currIndexNode);
  indexNodeNumber = FileSystem::ROOT_INDEX_NODE_NUMBER;

  // Tokenize full new path into directories
  token = strtok(full_newPath, "/");
  char new_filename[512];  // = "." ; // start at root node
  memset(new_filename, '\0', 512);
  strcpy(new_filename, ".");  // may be not needed.

  while (1) {
    if (token != NULL) {
      memset(new_filename, '\0', 512);
      strcpy(new_filename, token);

      if ((currIndexNode.getMode() & S_IFMT) != S_IFDIR) {
        // return (ENOTDIR) if a needed directory is not a directory
        process.errno = ENOTDIR;
        return -1;
      }

      // get the next inode corresponding to the token
      currIndexNode.copy(prevIndexNode);

      // Init CurIndexNode
      emptyIndexNode.copy(currIndexNode);
      // prevIndexNode = currIndexNode ;
      //   currIndexNode = new IndexNode() ;
      indexNodeNumber = findNextIndexNode(openFileSystems, prevIndexNode,
                                          new_filename, currIndexNode);
    } else {
      break;
    }

    token = strtok(NULL, "/");

    if (token != NULL) {
      strcat(new_directory, new_filename);
      strcat(new_directory, "/");
    }
  }

  // Update inode variables
  IndexNode new_indexNode;
  currIndexNode.copy(new_indexNode);
  int new_indexNodeNumber = indexNodeNumber;

  if (new_indexNodeNumber >= 0) {
    // file does exist
    cout << PROGRAM_NAME << ": Failed to create hardlink '" << newpath
         << "': File exists" << endl;
    process.errno = EEXIST;
    return -1;
  }

  // System.out.println( "newInode = " + newInode ) ;
  // open the directory
  // ??? it would be nice if we had an "open" that took an inode
  // instead of a name for the dir
  // System.out.println( "dirname = " + dirname.toString() ) ;
  //		cout << "dir name and name " << dirname << " " << name << endl;
  int dir = open(new_directory, O_RDWR);
  if (dir < 0) {
    perror(PROGRAM_NAME);
    cout << PROGRAM_NAME << ": unable to open directory for writing" << endl;
    return -1;
  }

  // Update the number of links to the inode
  short numberLinks = old_indexNode.getNlink();
  numberLinks++;
  old_indexNode.setNlink(numberLinks);

  // Persist the inode on disk
  openFileSystems->writeIndexNode(&old_indexNode, old_indexNodeNumber);

  // scan past the directory entries less than the current entry
  // and insert the new element immediately following

  //		cout << "New dir name " << name << endl;
  int status = 0;
  DirectoryEntry newDirectoryEntry(old_indexNodeNumber, new_filename);
  DirectoryEntry currentDirectoryEntry;

  while (true) {
    // read an entry from the directory
    status = readdir(dir, currentDirectoryEntry);

    if (status < 0) {
      cout << PROGRAM_NAME << ": error reading directory in creat";
      exit(EXIT_FAILURE);
    } else if (status == 0) {
      // if no entry read, write the new item at the current
      // location and break
      writedir(dir, newDirectoryEntry);
      break;
    } else {
      // if current item > new item, write the new item in
      // place of the old one and break
      if (strcmp(currentDirectoryEntry.getName(), newDirectoryEntry.getName()) >
          0) {
        int seek_status = lseek(dir, -DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
        //					cout << "lseek status" <<
        // seek_status
        //<<  endl;
        if (seek_status < 0) {
          cout << PROGRAM_NAME << ": error during seek in creat";
          exit(EXIT_FAILURE);
        }

        writedir(dir, newDirectoryEntry);
        break;
      }
    }
  }

  // copy the rest of the directory entries out to the file
  DirectoryEntry nextDirectoryEntry;

  while (status > 0) {
    // read next item
    status = readdir(dir, nextDirectoryEntry);

    if (status > 0) {
      // in its place
      int seek_status = lseek(dir, -DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
      if (seek_status < 0) {
        cout << PROGRAM_NAME << ": error during seek in link" << endl;
        exit(EXIT_FAILURE);
      }
    }
    // write current item
    //			cout << currentDirectoryEntry.toString() <<
    // endl;
    writedir(dir, currentDirectoryEntry);

    //			cout << "Old : " <<
    // currentDirectoryEntry.toString() << endl;
    // current item = next item
    nextDirectoryEntry.copy(currentDirectoryEntry);

    //			cout << "Next : " <<
    // nextDirectoryEntry.toString() << endl;
  }

  // close the directory
  close(dir);

  // Return success
  return 0;
}

int Kernel::unlink(char* pathname) {
  // get the full path
  char* fullPath = getFullPath(pathname);
  char dirname[1024];
  memset(dirname, '\0', 1024);
  strcpy(dirname, "/");

  FileSystem* fileSystem = openFileSystems;
  IndexNode currIndexNode;
  IndexNode prevIndexNode;
  IndexNode emptyIndexNode;
  getRootIndexNode()->copy(currIndexNode);

  // out << "CurIndexNode " << currIndexNode.toString() << endl;

  //	cout << currIndexNode.toString();

  short indexNodeNumber = FileSystem::ROOT_INDEX_NODE_NUMBER;

  char* token = NULL;
  token = strtok(fullPath, "/");
  char name[512];  // = "." ; // start at root node
  memset(name, '\0', 512);
  strcpy(name, ".");  // may be not needed.

  while (1) {
    if (token != NULL) {
      memset(name, '\0', 512);
      strcpy(name, token);

      //			cout << name << endl;
      // check to see if the current node is a directory

      //			cout <<currIndexNode.toString() << endl;

      if ((currIndexNode.getMode() & S_IFMT) != S_IFDIR) {
        // return (ENOTDIR) if a needed directory is not a directory
        process.errno = ENOTDIR;
        return -1;
      }

      // get the next inode corresponding to the token
      currIndexNode.copy(prevIndexNode);

      // Init CurIndexNode
      emptyIndexNode.copy(currIndexNode);
      // prevIndexNode = currIndexNode ;
      //   currIndexNode = new IndexNode() ;
      indexNodeNumber = findNextIndexNode(openFileSystems, prevIndexNode, name,
                                          currIndexNode);
    } else {
      break;
    }

    token = strtok(NULL, "/");

    if (token != NULL) {
      strcat(dirname, name);
      strcat(dirname, "/");
    }
  }

  // Check if file was found in filesystem
  if (indexNodeNumber < 0) {
    process.errno = ENOENT;
    cout << PROGRAM_NAME << ": File " << pathname << " does not exist: ";
    return -1;
  }

  // Check if file is a directory
  if ((currIndexNode.getMode() & S_IFMT) == S_IFDIR) {
    process.errno = EISDIR;
    cout << PROGRAM_NAME << ": File " << pathname << " is a directory: ";
    return -1;
  }

  // Update the number of links to the inode
  short numberLinks = currIndexNode.getNlink();
  numberLinks--;
  currIndexNode.setNlink(numberLinks);
  fileSystem->writeIndexNode(&currIndexNode, indexNodeNumber);

  // Check if file is currently open in a process
  int isOpen = 0;
  for (int i = 0; i < MAX_OPEN_FILES; i++) {
    if (openFiles[i] && openFiles[i]->getIndexNodeNumber() == indexNodeNumber) {
      // TODO MARK TO DELETE LATER
      isOpen = 1;
      break;
    }
  }

  // Check if file is no longer needed
  if (!isOpen && numberLinks == 0) {
    // free any blocks currently allocated to the file
    int blockSize = fileSystem->getBlockSize();
    int blocks = (currIndexNode.getSize() + blockSize - 1) / blockSize;
    for (int i = 0; i < blocks; i++) {
      int address = currIndexNode.getBlockAddress(i);
      if (address != FileSystem::NOT_A_BLOCK) {
        fileSystem->freeBlock(address);
        currIndexNode.setBlockAddress(i, FileSystem::NOT_A_BLOCK);
      }
    }

    // update the inode to size 0
    currIndexNode.setSize(0);

    // write the inode to the file system.
    fileSystem->writeIndexNode(&currIndexNode, indexNodeNumber);
  }

  // Open directory
  int dir = open(dirname, O_RDWR);
  if (dir < 0) {
    perror(PROGRAM_NAME);
    cout << PROGRAM_NAME << ": unable to open directory for writing" << endl;
    return -1;
  }

  // scan past the directory entries less than the current entry
  // and insert the new element immediately following

  //		cout << "New dir name " << name << endl;
  int status = 0;
  DirectoryEntry currentDirectoryEntry;
  DirectoryEntry nextDirectoryEntry;
  IndexNode directoryIndexNode;

  while (true) {
    // read an entry from the directory
    status = readdir(dir, currentDirectoryEntry);

    if (status < 0) {
      cout << PROGRAM_NAME << ": error reading directory in unlink";
      exit(EXIT_FAILURE);
    } else if (status == 0) {
      break;
    } else {
      // Skip over entry if it is the file being removed
      if (strcmp(currentDirectoryEntry.getName(), name) == 0) {
        int seek_status = lseek(dir, -DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
        //					cout << "lseek status" <<
        // seek_status
        //<<  endl;
        if (seek_status < 0) {
          cout << PROGRAM_NAME << ": error during seek in creat";
          exit(EXIT_FAILURE);
        }

        status = readdir(dir, currentDirectoryEntry);
        break;
      }
    }
  }

  while (status > 0) {
    // read next item
    status = readdir(dir, nextDirectoryEntry);

    if (status == 0) {
      // Write the final entry to the directory
      writedir(dir, nextDirectoryEntry);

      FileDescriptor* directory = process.openFiles[dir];

      // Subtract directory entry size from directory
      int directorySize = directory->getSize();
      directorySize -= DirectoryEntry::DIRECTORY_ENTRY_SIZE;
      directory->setSize(directorySize);

      // Update directory
      directory->getIndexNode()->copy(directoryIndexNode);

      // Subtract directory entry size from inode
      int inodeSize = directoryIndexNode.getSize();
      inodeSize -= DirectoryEntry::DIRECTORY_ENTRY_SIZE;
      directoryIndexNode.setSize(inodeSize);

      // Update inode
      fileSystem->writeIndexNode(&directoryIndexNode,
                                 directory->getIndexNodeNumber());

      break;
    } else if (status > 0) {
      // in its place
      // Go back 2 directory entries because we skipped the removed file
      int seek_status =
          lseek(dir, -2 * DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
      if (seek_status < 0) {
        cout << PROGRAM_NAME << ": error during seek in creat";
        exit(EXIT_FAILURE);
      }
    }
    // write current item
    //			cout << currentDirectoryEntry.toString() <<
    // endl;
    writedir(dir, nextDirectoryEntry);

    int seek_status = lseek(dir, DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
    if (seek_status < 0) {
      cout << PROGRAM_NAME << ": error during seek in creat";
      exit(EXIT_FAILURE);
    }
  }

  // close the directory
  close(dir);

  return 0;
}

/*
   to be done:
   int access(const char *pathname, int mode);
   int link(const char *oldpath, const char *newpath);
   int unlink(const char *pathname);
   int rename(const char *oldpath, const char *newpath);
   int symlink(const char *oldpath, const char *newpath);
   int lstat(const char *file_name, struct stat *buf);
   int chmod(const char *path, mode_t mode);
   int fchmod(int fildes, mode_t mode);
   int chown(const char *path, uid_t owner, gid_t group);
   int fchown(int fd, uid_t owner, gid_t group);
   int lchown(const char *path, uid_t owner, gid_t group);
   int utime(const char *filename, struct utimbuf *buf);
   int readlink(const char *path, char *buf, size_t bufsiz);
   int chdir(const char *path);
   mode_t umask(mode_t mask);
 */

/**
 * This is an internal variable for the simulator which always
 * points to the
 * current ProcessContext.  If multiple processes are implemented,
 * then this variable will "point" to different processes at
 * different times.
 */

/**
 * Initialize the file simulator kernel.  This should be the
 * first call in any simulation program.  You can think of this
 * as the method which "boots" the kernel.
 * This method opens the "filesys.conf" file (or the file named
 * by the system property "filesys.conf") and reads any properties
 * given in that file, including the filesystem.root.filename and
 * filesystem.root.mode ("r", "rw").
 */
bool Kernel::initialize() {
  // check to see if the name of an alternate configuration
  // file has been specified.  This can be done, for example,
  //   java -Dfilesys.conf=myfile.txt program-name parameter ...

  // tring propertyFileName = System.getProperty( "filesys.conf" ) ;
  // f ( propertyFileName == null )
  // propertyFileName = "filesys.conf" ;
  // roperties properties = new Properties() ;
  // ry
  //
  // FileInputStream in = new FileInputStream( propertyFileName ) ;
  // properties.load( in ) ;
  // in.close() ;
  //
  // atch( FileNotFoundException e )
  //
  // System.err.println( PROGRAM_NAME + ": error opening properties file" ) ;
  // System.exit( EXIT_FAILURE ) ;
  //
  // atch( IOException e )
  //
  // System.err.println( PROGRAM_NAME + ": error reading properties file" ) ;
  // System.exit( EXIT_FAILURE ) ;
  //
  strcpy(PROGRAM_NAME, "Kernel");

  char rootFileSystemFilename[32];  // = "filesys.dat";
  char rootFileSystemMode[32];

  memset(rootFileSystemFilename, '\0', 32);
  memset(rootFileSystemMode, '\0', 32);

  strcpy(rootFileSystemFilename, "filesys.dat");
  strcpy(rootFileSystemMode, "rw");

  // get the current process properties
  short uid = 1;
  short gid = 1;
  short umask = 0002;
  char dir[32];

  memset(dir, '\0', 32);

  strcpy(dir, "/root");
  MAX_OPEN_FILES = 20;  ///"kernel.max_open_files" , "20" ) ) ;

  // create open file array
  openFiles = new FileDescriptor*[MAX_OPEN_FILES];

  for (int i = 0; i < MAX_OPEN_FILES; i++) {
    openFiles[i] = NULL;
  }

  // create the first process
  // ocess = new ProcessContext(uid, gid, dir, umask);
  process.init(uid, gid, dir, umask);
  process.setMaxOpenFiles(10);
  processCount++;

  // open the root file system
  openFileSystems = new FileSystem(rootFileSystemFilename, rootFileSystemMode);

  return openFileSystems->is_ready();
}

/**
 * End the simulation and exit.
 * Terminates any remaining "processes", flushes all file system blocks
 * to "disk", and exit the simulation program.  This method is generally
 * called by exit() when the last process terminates.  However,
 * it may also be called directly to gracefully end the simlation.
 * @param status the status to use with System.exit()
 * @exception java.lang.Exception if any underlying operation
 * causes and exception to be thrown.
 */
void Kernel::finalize(int status) {
  // exit() any remaining processes
  /*	if(process != NULL)
          {
                  delete process;
          }*/

  if (openFileSystems != NULL) {
    openFileSystems->close();
    delete openFileSystems;
  }

  if (openFiles != NULL) {
    delete[] openFiles;
    //		delete openFiles;
    // Need to also remove assigne FileDescriptor
  }

  // flush file system blocks
  sync();

  // terminate the program
  // it(status) ;
  std::exit(status);
}

/*
   Some internal methods.
 */

/**
 * Check to see if the integer given is a valid file descriptor
 * index for the current process.  Sets errno to EBADF if invalid.
 * <p>
 * This is a convenience method for the simulator kernel;
 * it should not be called by user programs.
 * @param fd the file descriptor index
 * @return zero if the file descriptor index is valid; -1 if the file
 * descriptor index is not valid
 */
int Kernel::check_fd(int fd) {
  // look for the file descriptor in the open file list
  if (fd < 0 || fd >= process.getOpenFilesCount() ||
      process.openFiles[fd] == NULL) {
    // return (EBADF) if file descriptor is invalid
    process.errno = EBADF;
    return -1;
  }

  return 0;
}

/**
 * Check to see if the integer given is a valid file descriptor
 * index for the current process, and if so, whether the file is
 * open for reading.  Sets errno to EBADF if invalid or not open
 * for reading.
 * <p>
 * This is a convenience method for the simulator kernel;
 * it should not be called by user programs.
 * @param fd the file descriptor index
 * @return zero if the file descriptor index is valid; -1 if the file
 * descriptor index is not valid or is not open for reading.
 */
int Kernel::check_fd_for_read(int fd) {
  int status = check_fd(fd);
  if (status < 0) return -1;

  FileDescriptor* fileDescriptor = process.openFiles[fd];
  int flags = fileDescriptor->getFlags();
  if ((flags != O_RDONLY) && (flags != O_RDWR)) {
    // return (EBADF) if the file is not open for reading
    process.errno = EBADF;
    return -1;
  }

  return 0;
}

/**
 * Check to see if the integer given is a valid file descriptor
 * index for the current process, and if so, whether the file is
 * open for writing.  Sets errno to EBADF if invalid or not open
 * for writing.
 * <p>
 * This is a convenience method for the simulator kernel;
 * it should not be called by user programs.
 * @param fd the file descriptor index
 * @return zero if the file descriptor index is valid; -1 if the file
 * descriptor index is not valid or is not open for writing.
 */
int Kernel::check_fd_for_write(int fd) {
  int status = check_fd(fd);
  if (status < 0) return -1;

  FileDescriptor* fileDescriptor = process.openFiles[fd];
  int flags = fileDescriptor->getFlags();
  if ((flags != O_WRONLY) && (flags != O_RDWR)) {
    // return (EBADF) if the file is not open for writing
    process.errno = EBADF;
    return -1;
  }

  return 0;
}

/**
 * Get the full path for a file by adding
 * the working directory for the current process
 * to the beginning of the given path name
 * if necessary.
 * @param pathname the given path name
 * @return the resulting fully qualified path name
 */
char* Kernel::getFullPath(char* pathname) {
  // Not thread safe.
  static char fullPath[1024];
  memset(fullPath, '\0', 1024);

  // make sure the path starts with a slash
  if (pathname[0] == '/') {
    strcpy(fullPath, pathname);
  } else {
    sprintf(fullPath, "%s/%s", process.getDir(), pathname);
  }
  return fullPath;
}

IndexNode* Kernel::getRootIndexNode() {
  // if( rootIndexNode == null )
  return openFileSystems->getRootIndexNode();
}

short Kernel::findNextIndexNode(FileSystem* fileSystem, IndexNode& indexNode,
                                char* name, IndexNode& nextIndexNode) {
  // if stat isn't a directory give an error
  if ((indexNode.getMode() & S_IFMT) != S_IFDIR) {
    // return (ENOTDIR) if a needed directory is not a directory
    process.errno = ENOTDIR;
    return -1;
  }

  FileDescriptor* fileDescriptor =
      new FileDescriptor(fileSystem, indexNode, O_RDONLY);

  //	cout << fileDescriptor.getOffset()<< endl;
  int fd = open(fileDescriptor);

  if (fd < 0) {
    // process.errno = ???
    return -1;
  }

  // create a buffer for reading directory entries
  DirectoryEntry directoryEntry;

  int status = 0;
  short indexNodeNumber = -1;
  // while there are more directory blocks to be read
  while (true) {
    // read a directory entry
    status = readdir(fd, directoryEntry);
    //		cout << directoryEntry.toString() << endl;

    if (status <= 0) {
      //			cout << "here!!!" << endl;
      // we got to the end of the directory, or
      // encountered an error, so quit
      break;
    }

    //		cout << "Entry Name : " <<  directoryEntry.getName() << "  " <<
    // name;
    if (strcmp(directoryEntry.getName(), name) == 0) {
      indexNodeNumber = directoryEntry.getIno();
      // read the inode block
      fileSystem->readIndexNode(&nextIndexNode, indexNodeNumber);
      // we're done searching
      break;
    }
  }

  // close the file since we're done with it
  int close_status = close(fd);
  if (close_status < 0) {
    // process.errno = ???
    return -1;
  }

  // if we encountered an error reading, return error
  if (status < 0) {
    // process.errno = ???
    return -1;
  }

  // if we got to the directory without finding the name, return error
  if (status == 0) {
    process.errno = ENOENT;
    return -1;
  }

  // return index node number if success
  return indexNodeNumber;
}

// get the inode for a file which is expected to exist
short Kernel::findIndexNode(char* path, IndexNode& inode) {
  // start with the root file system, root inode
  // FileSystem fileSystem = openFileSystems[ ROOT_FILE_SYSTEM ] ;
  IndexNode indexNode;
  getRootIndexNode()->copy(indexNode);

  short indexNodeNumber = FileSystem::ROOT_INDEX_NODE_NUMBER;

  // parse the path until we get to the end
  char* token;
  token = strtok(path, "/");
  char name[512];  // = "." ; // start at root node
  memset(name, '\0', 512);

  while (token != NULL) {
    //		cout << token << endl;

    if (strcmp(token, "") != 0) {
      // check to see if it is a directory
      if ((indexNode.getMode() & S_IFMT) != S_IFDIR) {
        // return (ENOTDIR) if a needed directory is not a directory
        process.errno = ENOTDIR;
        return -1;
      }

      // check to see if it is readable by the user
      // ??? tbd
      // return (EACCES) if a needed directory is not readable

      IndexNode nextIndexNode;
      // get the next index node corresponding to the token
      indexNodeNumber =
          findNextIndexNode(openFileSystems, indexNode, token, nextIndexNode);
      if (indexNodeNumber < 0) {
        // return ENOENT
        process.errno = ENOENT;
        return -1;
      }

      nextIndexNode.copy(indexNode);
    }

    token = strtok(NULL, "/");
  }
  // copy indexNode to inode
  indexNode.copy(inode);
  return indexNodeNumber;
}
