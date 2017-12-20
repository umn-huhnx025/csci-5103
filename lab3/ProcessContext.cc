#include "ProcessContext.h"
#include <stdlib.h>
#include <string.h>

ProcessContext::ProcessContext() {
  /**
   * Number of last error.
   * <p>
   * Simulates the unix system variable:
   * <pre>
   *   extern int errno;
   * </pre>
   */
  errno = 0;
  /**
   * The uid for the process.
   */
  uid = 1;

  /**
   * The gid for the process.
   */
  gid = 1;

  /**
   * The working directory for the process.
   */
  memset(dir, '\0', 512);
  strcpy(dir, "/root");

  /**
   * The umask for the process.
   */
  umask = 0000;

  /**
   * The maximum number of files a process may have open.
   */
  MAX_OPEN_FILES = 0;

  /**
   * The array of file descriptors for open files.
   * The integer file descriptors for kernel method calls
   * are indexes into this array.
   */
  //	FileDescriptor[] openFiles =  new FileDescriptor[MAX_OPEN_FILES] ;

  /**
   * Construct a process context.  By default, uid=1, gid=1, dir="/root",
   * and umask=0000.
   */
  openFiles = NULL;
}

/**
 * Construct a process context and specify uid, gid, dir, and umask.
 */
ProcessContext::ProcessContext(short newUid, short newGid, char* newDir,
                               short newUmask) {
  errno = 0;
  uid = newUid;
  gid = newGid;
  strcpy(dir, newDir);
  umask = newUmask;
  MAX_OPEN_FILES = 0;
  openFiles = NULL;
}

void ProcessContext::init(short newUid, short newGid, char* newDir,
                          short newUmask) {
  errno = 0;
  uid = newUid;
  gid = newGid;
  strcpy(dir, newDir);
  umask = newUmask;
  MAX_OPEN_FILES = 0;
  openFiles = NULL;
}

ProcessContext::~ProcessContext() {
  if (openFiles != NULL) {
    delete[] openFiles;
  }
}

int ProcessContext::getOpenFilesCount() {
  int nCnt = 0;
  for (int i = 0; i < MAX_OPEN_FILES; i++) {
    if (openFiles[i] != NULL) {
      nCnt++;
    }
  }

  return nCnt;
}

// Not used now. - KS
bool ProcessContext::setFileDescriptor(int fd, FileDescriptor* file) {
  // f(file == NULL)
  //
  // if(openFiles[fd] != NULL)
  //{
  //}
  //

  // f(openFiles.size >= MAX_OPEN_FILES)
  //
  ////No More file open allowed;
  //
  // lse

  openFiles[fd] = file;
  //
  return true;
}

void ProcessContext::setMaxOpenFiles(int nMax) {
  MAX_OPEN_FILES = nMax;

  if (openFiles != NULL) {
    delete openFiles;
  }

  openFiles = new FileDescriptor*[nMax];

  for (int i = 0; i < nMax; i++) {
    openFiles[i] = NULL;
  }
}

/**
 * Set the process uid.
 */
void ProcessContext::setUid(short newUid) { uid = newUid; }

/**
 * Get the process uid.
 */
short ProcessContext::getUid() { return uid; }

/**
 * Set the process gid.
 */
void ProcessContext::setGid(short newGid) { gid = newGid; }

/**
 * Get the process gid.
 */
short ProcessContext::getGid() { return gid; }

/**
 * Set the process working directory.
 */
void ProcessContext::setDir(char* newDir) { strcpy(dir, newDir); }

/**
 * Get the process working directory.
 */
char* ProcessContext::getDir() { return dir; }

/**
 * Set the process umask.
 */
void ProcessContext::setUmask(short newUmask) { umask = newUmask; }

/**
 * Get the process umask.
 */
short ProcessContext::getUmask() { return umask; }
