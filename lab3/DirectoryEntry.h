#ifndef DIRECTORY_ENTRY_H
#define DIRECTORY_ENTRY_H
#include <fstream>
#include <iostream>

using namespace std;

class DirectoryEntry {
 public:
  const static int MAX_FILENAME_LENGTH = 14;
  const static int DIRECTORY_ENTRY_SIZE = MAX_FILENAME_LENGTH + 2;
  unsigned short d_ino;
  char d_name[1024];

  DirectoryEntry();
  DirectoryEntry(short ino, const char* name);
  void setIno(short newIno);
  short getIno();
  void setName(const char* newName);
  char* getName();
  void write(char* buffer, int offset);
  void read(char* buffer, int offset);
  char* toString();

  void copy(DirectoryEntry& de);
};

#endif
