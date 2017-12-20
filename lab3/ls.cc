/**
 * A simple directory listing program for a simulated file system.
 * @author Ray Ontko -> Converted to C++ by Kwangsung
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DirectoryEntry.h"
#include "Kernel.h"
#include "Stat.h"

/**
 * Print a listing for a particular file.
 * This is a convenience method.
 * @param name the name to print
 * @param stat the stat containing the file's information
 */
void print(char *name, Stat &stat) {
  // a buffer to fill with a line of output
  //  StringBuffer s = new StringBuffer() ;
  char temp[8192];
  char ino[32];

  memset(temp, '\0', 8192);
  memset(ino, '\0', 32);

  // append the inode number in a field of 5
  sprintf(ino, "%d", stat.getIno());

  for (unsigned i = 0; i < 5 - strlen(ino); i++) {
    strcat(temp, " ");
  }

  strcat(temp, ino);
  strcat(temp, " ");

  // append the size in a field of 10
  sprintf(ino, "%d", stat.getSize());

  //	cout << "Inode number : " << ino << endl;

  for (unsigned i = 0; i < 10 - strlen(ino); i++) {
    strcat(temp, " ");
  }

  strcat(temp, ino);
  strcat(temp, " ");

  //	cout << "name : " << name << endl;

  // append the name
  strcat(temp, name);

  // print the buffer
  cout << temp << endl;
}

int main(int argc, char **argv) {
  char PROGRAM_NAME[8];
  strcpy(PROGRAM_NAME, "ls");

  char temp[512];
  char name[512];

  if (Kernel::initialize() == false) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }

  // for each path-name given
  for (int i = 1; i < argc; i++) {
    strcpy(name, argv[i]);
    int status = 0;

    //		cout << name << endl;

    // stat the name to get information about the file or directory
    Stat stat;
    status = Kernel::stat(name, stat);

    if (status < 0) {
      Kernel::perror(PROGRAM_NAME);
      Kernel::exit(1);
    }

    // mask the file type from the mode
    short type = (short)(stat.getMode() & Kernel::S_IFMT);

    // if name is a regular file, print the info
    if (type == Kernel::S_IFREG) {
      print(name, stat);
    }

    // if name is a directory open it and read the contents
    else if (type == Kernel::S_IFDIR) {
      // open the directory
      int fd = Kernel::open(name, Kernel::O_RDONLY);

      if (fd < 0) {
        Kernel::perror(PROGRAM_NAME);
        cout << PROGRAM_NAME << ": unable to open \"" << name
             << "\" for reading";
        Kernel::exit(1);
      }

      // print a heading for this directory
      cout << endl << name << ":" << endl;

      // create a directory entry structure to hold data as we read
      DirectoryEntry directoryEntry;
      int count = 0;

      // while we can read, print the information on each entry
      while (true) {
        //	memcpy(temp, '\0', 511);
        // read an entry; quit loop if error or nothing read
        status = Kernel::readdir(fd, directoryEntry);
        if (status <= 0) {
          break;
        }

        //				cout << directoryEntry.toString() <<
        // endl;

        // get the name from the entry
        char *entryName = directoryEntry.getName();

        // call stat() to get info about the file
        //				memcpy(temp, '\0', 512);

        strcpy(temp, name);
        strcat(temp, "/");
        strcat(temp, entryName);
        // name, entryName);

        status = Kernel::stat(temp, stat);
        if (status < 0) {
          Kernel::perror(PROGRAM_NAME);
          Kernel::exit(1);
        }

        // print the entry information
        print(entryName, stat);
        count++;

        memset(temp, '\0', 512);
      }

      // check to see if our last read failed
      if (status < 0) {
        Kernel::perror("main");
        cout << "main: unable to read directory entry from /";
        Kernel::exit(2);
      }

      // close the directory
      Kernel::close(fd);

      // print a footing for this directory
      cout << "total files: " << count << endl;
    }
  }

  // exit with success if we process all the arguments
  Kernel::exit(0);
}
