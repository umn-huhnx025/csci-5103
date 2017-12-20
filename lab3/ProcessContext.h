#ifndef PROCESS_CONTEXT_H
#define PROCESS_CONTEXT_H

#include <fstream>
#include <iostream>
#include "FileDescriptor.h"
//#include <unordered_map>

using namespace std;
class ProcessContext {
 public:
  int errno;
  int MAX_OPEN_FILES;
  FileDescriptor** openFiles;  //= new FileDescriptor[MAX_OPEN_FILES];
  //		unordered_map<int, FileDescriptor *> openFiles;

 private:
  short uid;
  short gid;
  char dir[512];
  short umask;

 public:
  ProcessContext();
  ~ProcessContext();
  ProcessContext(short newUid, short newGid, char* newDir, short newUmask);

  void init(short newUid, short newGid, char* newDir, short newUmask);
  //-KS
  int getMaxOpenFiles() { return MAX_OPEN_FILES; }
  void setMaxOpenFiles(int nMax);
  bool setFileDescriptor(int fd, FileDescriptor* file);
  int getOpenFilesCount();

  void setUid(short newUid);
  short getUid();
  void setGid(short newGid);
  short getGid();
  void setDir(char* newDir);
  char* getDir();
  void setUmask(short newUmask);
  short getUmask();
};

#endif
