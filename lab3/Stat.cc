#include "Stat.h"

Stat::Stat() {
  st_dev = 0;
  st_ino = 0;
  st_mode = 0;
  st_nlink = 0;
  st_uid = 0;
  st_gid = 0;
  st_rdev = 0;
  st_size = 0;
  st_atime = 0;
  st_mtime = 0;
  st_ctime = 0;
}

void Stat::setDev(short newDev) { st_dev = newDev; }

short Stat::getDev() { return st_dev; }

void Stat::setIno(short newIno) { st_ino = newIno; }

short Stat::getIno() { return st_ino; }

void Stat::setMode(short newMode) { st_mode = newMode; }

short Stat::getMode() { return st_mode; }

void Stat::setNlink(short newNlink) { st_nlink = newNlink; }

short Stat::getNlink() { return st_nlink; }

void Stat::setUid(short newUid) { st_uid = newUid; }

short Stat::getUid() { return st_uid; }

void Stat::setGid(short newGid) { st_gid = newGid; }

short Stat::getGid() { return st_gid; }

void Stat::setRdev(short newRdev) { st_rdev = newRdev; }

short Stat::getRdev() { return st_rdev; }

void Stat::setSize(int newSize) { st_size = newSize; }

int Stat::getSize() { return st_size; }

void Stat::setAtime(int newAtime) { st_atime = newAtime; }

int Stat::getAtime() { return st_atime; }

void Stat::setMtime(int newMtime) { st_mtime = newMtime; }

int Stat::getMtime() { return st_mtime; }

void Stat::setCtime(int newCtime) { st_ctime = newCtime; }

int Stat::getCtime() { return st_ctime; }

void Stat::copyIndexNode(IndexNode& indexNode) {
  st_mode = indexNode.getMode();
  st_nlink = indexNode.getNlink();
  st_uid = indexNode.getUid();
  st_uid = indexNode.getGid();
  st_size = indexNode.getSize();
  st_atime = indexNode.getAtime();
  st_mtime = indexNode.getMtime();
  st_ctime = indexNode.getCtime();
}
