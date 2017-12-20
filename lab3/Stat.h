#ifndef STAT_H
#define STAT_H

#include <fstream>
#include <iostream>
#include "IndexNode.h"

using namespace std;

class Stat {
 public:
  short st_dev;
  short st_ino;
  short st_mode;
  short st_nlink;
  short st_uid;
  short st_gid;
  short st_rdev;
  int st_size;
  int st_atime;
  int st_mtime;
  int st_ctime;

  Stat();
  void setDev(short newDev);
  short getDev();
  void setIno(short newIno);
  short getIno();
  void setMode(short newMode);
  short getMode();
  void setNlink(short newNlink);
  short getNlink();
  void setUid(short newUid);
  short getUid();
  void setGid(short newGid);
  short getGid();
  void setRdev(short newRdev);
  short getRdev();
  void setSize(int newSize);
  int getSize();
  void setAtime(int newAtime);
  int getAtime();
  void setMtime(int newMtime);
  int getMtime();
  void setCtime(int newCtime);
  int getCtime();
  void copyIndexNode(IndexNode& indexNode);
};

#endif
