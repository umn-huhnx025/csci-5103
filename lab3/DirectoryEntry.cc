#include "DirectoryEntry.h"
#include <stdlib.h>
#include <string.h>
/*
 * Constructs an empty DirectoryEntry.
 */
DirectoryEntry::DirectoryEntry() {
  memset(d_name, '\0', 1024);
  d_ino = 0;
}

/**
 * Constructs a DirectoryEntry for the given inode and name.
 * Note that the name is stored internally as a byte[],
 * not as a string.
 * @param ino the inode number for this DirectoryEntry
 * @param name the file name for this DirectoryEntry
 */
DirectoryEntry::DirectoryEntry(short ino, const char* name) {
  setIno(ino);
  setName(name);
}

/**
 * Sets the inode number for this DirectoryEntry
 * @param newIno the new inode number
 */
void DirectoryEntry::setIno(short newIno) { d_ino = newIno; }

/**
 * Gets the inode number for this DirectoryEntry
 * @return the inode number
 */
short DirectoryEntry::getIno() { return d_ino; }

/**
 * Sets the name for this DirectoryEntry
 * @param newName the new name
 */
void DirectoryEntry::setName(const char* newName) {
  memset(d_name, '\0', 1024);
  strcpy(d_name, newName);
  // uu	cout << "SetName in dir:" << d_name;
}

/**
 * Gets the name for this DirectoryEntry
 * @return the name
 */
char* DirectoryEntry::getName() { return d_name; }

/**
 * Writes a DirectoryEntry to the specified byte array at the specified
 * offset.
 * @param buffer the byte array to which the directory entry should be written
 * @param offset the offset from the beginning of the buffer to which the
 * directory entry should be written
 */
void DirectoryEntry::write(char* buffer, int offset) {
  buffer[offset] = (char)(d_ino >> 8);
  buffer[offset + 1] = (char)d_ino;

  int nLen = strlen(d_name);

  for (int i = 0; i < nLen; i++) {
    buffer[offset + 2 + i] = d_name[i];
  }

  buffer[offset + 2 + nLen] = '\0';
  // har * buf = &buffer[offset+2];

  // out << "in DirectoryEntry " << buf << endl;
}

/**
 * Reads a DirectoryEntry from the spcified byte array at the specified
 * offset.
 * @param buffer the byte array from which the directory entry should be read
 * @param offset the offset from the beginning of the buffer from which the
 * directory entry should be read
 */
void DirectoryEntry::read(char* buffer, int offset) {
  memset(d_name, '\0', 1024);

  int hi = buffer[offset] & 0xff;
  int lo = buffer[offset + 1] & 0xff;
  d_ino = (short)(hi << 8 | lo);

  int nLen = MAX_FILENAME_LENGTH;  // strlen(d_name);

  for (int i = 0; i < nLen; i++) {
    d_name[i] = buffer[offset + 2 + i];
  }

  d_name[nLen] = '\0';
}

/**
 * Converts a DirectoryEntry to a printable string.
 * @return the printable string
 */
char* DirectoryEntry::toString() {
  // Not thread safe - KS
  static char buffer[1024];
  sprintf(buffer, "DirectoryEntry[%d, %s]", getIno(), getName());
  return buffer;
}

void DirectoryEntry::copy(DirectoryEntry& de) {
  de.setIno(getIno());
  de.setName(getName());
}

/**
 * A test driver for this class.
 * @exception java.lang.Exception any exception which may occur.
 */
/* static void main( String[] args ) throws Exception
   {
   DirectoryEntry root = new DirectoryEntry( (short)1 , "/" ) ;
   System.out.println( root.toString() ) ;
   }*/
/*
int main(int argc, char ** argv )
{
        DirectoryEntry root((short)1 , "/") ;
        cout << root.toString() << endl;
}*/
