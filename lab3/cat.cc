/**
 * Reads a sequence of files and writes them to standard output.
 * A simple cat program for a simulated file system.
 * @author Ray Ontko -> Converted to C++ by Kwangsung
 */

#include <stdlib.h>
#include <string.h>
#include "Kernel.h"

#define BUF_SIZE 4096

int main(int argc, char** argv) {
  char PROGRAM_NAME[8];
  memset(PROGRAM_NAME, '\0', 8);
  strcpy(PROGRAM_NAME, "cat");

  // initialize the file system simulator kernel
  if (Kernel::initialize() == false) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }

  if (argc <= 1) {
    cout << PROGRAM_NAME << ": usage: " << PROGRAM_NAME << " input-file ..."
         << endl;
    Kernel::exit(1);
  }

  char name[512];
  char buffer[BUF_SIZE];

  memset(name, '\0', 512);
  // display a helpful message if no arguments are given
  // for each filename specified
  for (int i = 1; i < argc; i++) {
    strcpy(name, argv[i]);

    // open the file for reading
    int in_fd = Kernel::open(name, Kernel::O_RDONLY);
    if (in_fd < 0) {
      Kernel::perror(PROGRAM_NAME);
      cout << PROGRAM_NAME << " : unable to open input file " << name;
      Kernel::exit(2);
    }

    // create a buffer for reading data
    memset(buffer, '\0', BUF_SIZE);

    // read data while we can
    int rd_count;
    while (true) {
      // read a buffer full of data
      memset(buffer, '\0', BUF_SIZE);
      rd_count = Kernel::read(in_fd, buffer, BUF_SIZE);

      // if we encounter an error or get to the end, quit the loop
      if (rd_count <= 0) {
        break;
      }

      // write whatever we read to standard output
      cout << buffer;
      ///     System.out.write( buffer , 0 , rd_count ) ;
    }

    // close the input file
    Kernel::close(in_fd);

    // exit with failure if we encounter an error
    if (rd_count < 0) {
      Kernel::perror(PROGRAM_NAME);
      cout << PROGRAM_NAME << ": error during read from input file";
      Kernel::exit(3);
    }
  }

  // exit with success if we read all the files without error
  Kernel::exit(0);
}
