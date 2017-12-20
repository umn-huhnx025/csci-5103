/**
 * An mkdir for a simulated file system.
 * @author Ray Ontko -> Converted to C++ by Kwangsung
 */
#include <stdlib.h>
#include <string.h>
#include "DirectoryEntry.h"
#include "Kernel.h"
#include "Stat.h"

int main(int argc, char** argv) {
  char PROGRAM_NAME[8];
  strcpy(PROGRAM_NAME, "mkdir");

  char name[64];
  char parentName[64];

  memset(name, '\0', 64);
  memset(parentName, '\0', 64);

  // initialize the file system simulator kernel
  if (Kernel::initialize() == false) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }
  // print a helpful message if no command line arguments are given
  if (argc < 2) {
    cout << PROGRAM_NAME << ": too few arguments" << endl;
    Kernel::exit(1);
  }

  // create a buffer for writing directory entries
  char directoryEntryBuffer[DirectoryEntry::DIRECTORY_ENTRY_SIZE];
  memset(directoryEntryBuffer, '\0', DirectoryEntry::DIRECTORY_ENTRY_SIZE);

  // for each argument given on the command line
  for (int i = 1; i < argc; i++) {
    // given the argument a better name
    strcpy(name, argv[i]);
    int status = 0;

    // call creat() to create the file
    int newDir = Kernel::creat(name, Kernel::S_IFDIR);

    if (newDir < 0) {
      Kernel::perror(PROGRAM_NAME);
      cout << PROGRAM_NAME << ": \"" << name << "\"" << endl;
      Kernel::exit(2);
    }

    // get file info for "."
    Stat selfStat;
    status = Kernel::fstat(newDir, selfStat);
    if (status < 0) {
      Kernel::perror(PROGRAM_NAME);
      Kernel::exit(3);
    }

    //		cout <<"Ino " <<  selfStat.getIno() << endl;

    // add entry for "."
    DirectoryEntry self(selfStat.getIno(), ".");
    memset(directoryEntryBuffer, '\0', DirectoryEntry::DIRECTORY_ENTRY_SIZE);
    self.write(directoryEntryBuffer, 0);

    //		cout << "From mkdir " << self.toString() << endl;
    status = Kernel::write(newDir, directoryEntryBuffer,
                           DirectoryEntry::DIRECTORY_ENTRY_SIZE);
    if (status < 0) {
      Kernel::perror(PROGRAM_NAME);
      Kernel::exit(4);
    }

    // get file info for ".."
    Stat parentStat;
    sprintf(parentName, "%s/..", name);
    Kernel::stat(parentName, parentStat);

    // add entry for ".."
    DirectoryEntry parent(parentStat.getIno(), "..");
    memset(directoryEntryBuffer, '\0', DirectoryEntry::DIRECTORY_ENTRY_SIZE);
    parent.write(directoryEntryBuffer, 0);

    cout << directoryEntryBuffer << endl;

    status = Kernel::write(newDir, directoryEntryBuffer,
                           DirectoryEntry::DIRECTORY_ENTRY_SIZE);
    if (status < 0) {
      Kernel::perror(PROGRAM_NAME);
      Kernel::exit(5);
    }

    // call close() to close the file
    status = Kernel::close(newDir);
    if (status < 0) {
      Kernel::perror(PROGRAM_NAME);
      Kernel::exit(6);
    }

    //		Stat stat;
    //      status = Kernel::stat( name , stat ) ;

    //		if(status == -1)
    //		{
    //			cout <<"shit\n" <<endl;
    //		}
  }

  // exit with success if we process all the arguments
  Kernel::exit(0);
}
