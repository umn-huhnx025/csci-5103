#ifndef KERNEL_H
#define KERNEL_H

#include "DirectoryEntry.h"
#include "FileDescriptor.h"
#include "IndexNode.h"
#include "ProcessContext.h"
#include "Stat.h"

#include <fstream>
#include <iostream>

class Kernel {
 public:
  static char PROGRAM_NAME[512];  // = "Kernel" ;
  const static int EPERM = 1;
  const static int ENOENT = 2;
  const static int EBADF = 9;
  const static int EACCES = 13;
  const static int EEXIST = 17;
  const static int EXDEV = 18;
  const static int ENOTDIR = 20;
  const static int EISDIR = 21;
  const static int EINVAL = 22;
  const static int ENFILE = 23;
  const static int EMFILE = 24;
  const static int EFBIG = 27;
  const static int ENOSPC = 28;
  const static int EROFS = 30;
  const static int EMLINK = 31;
  const static int sys_nerr = 32;
  //		static char * sys_errlist[32];
  static void perror(const char* s);
  static void setErrno(int newErrno);
  static int getErrno();

  const static short S_IFMT = (short)0170000;
  const static short S_IFREG = (short)0100000;
  const static short S_IFMPB = 070000;
  const static short S_IFBLK = 060000;
  const static short S_IFDIR = 040000;
  const static short S_IFMPC = 030000;
  const static short S_IFCHR = 020000;
  const static short S_ISUID = 04000;
  const static short S_ISGID = 02000;
  const static short S_ISVTX = 01000;
  const static short S_IRWXU = 0700;
  const static short S_IRUSR = 0400;
  const static short S_IREAD = 0400;
  const static short S_IWUSR = 0200;
  const static short S_IWRITE = 0200;
  const static short S_IXUSR = 0100;
  const static short S_IEXEC = 0100;
  const static short S_IRWXG = 070;
  const static short S_IRGRP = 040;
  const static short S_IWGRP = 020;
  const static short S_IXGRP = 010;
  const static short S_IRWXO = 07;
  const static short S_IROTH = 04;
  const static short S_IWOTH = 02;
  const static short S_IXOTH = 01;

  static int close(int fd);
  static int creat(char* pathname, short mode);
  static void exit(int status);
  static int lseek(int fd, int offset, int whence);

  const static int O_RDONLY = 0;
  const static int O_WRONLY = 1;
  const static int O_RDWR = 2;

  static int open(char* pathname, int flags);
  static int open(FileDescriptor* fileDescriptor);
  static int read(int fd, char* buf, int count);
  static int readdir(int fd, DirectoryEntry& dirp);
  static int fstat(int fd, Stat& buf);
  static int stat(char* name, Stat& buf);
  static void sync();
  static int write(int fd, char* buf, int count);
  static int writedir(int fd, DirectoryEntry& dirp);

  // File system hard link implementation
  static int link(char* oldpath, char* newpath);
  static int unlink(char* pathname);

  const static int MAX_OPEN_FILE_SYSTEMS = 1;
  const static short ROOT_FILE_SYSTEM = 0;

  static FileSystem* openFileSystems;  // new FileSystem[MAX_OPEN_FILE_SYSTEMS]
                                       // ;
  static bool initialize();
  static void finalize(int status);

 private:
  static ProcessContext process;
  static int processCount;
  static int MAX_OPEN_FILES;
  static FileDescriptor** openFiles;

#ifdef EXIT_FAILURE
#undef EXIT_FAILURE
#endif
#ifdef EXIT_SUCCESS
#undef EXIT_SUCCESS
#endif
  const static int EXIT_FAILURE = 1;
  const static int EXIT_SUCCESS = 0;
  // static IndexNode rootIndexNode = null ; //Removed

  static int check_fd(int fd);
  static int check_fd_for_read(int fd);
  static int check_fd_for_write(int fd);
  static char* getFullPath(char* pathname);

  static IndexNode* getRootIndexNode();
  static short findNextIndexNode(FileSystem* fileSystem, IndexNode& indexNode,
                                 char* name, IndexNode& nextIndexNode);
  static short findIndexNode(char* path, IndexNode& inode);
};

#endif
