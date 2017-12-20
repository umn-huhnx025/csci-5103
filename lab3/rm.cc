/**
 * Removes/unlinkks a file
 *
 * A simple ln program for a simulated file system.
 * @author Paul Rheinberger -> Referencing tee.cc by Ray Ontko
 */

#include <stdlib.h>
#include <string.h>
#include "Kernel.h"

using namespace std;

int main(int argc, char** argv) {
  char PROGRAM_NAME[8];
  strcpy(PROGRAM_NAME, "rm");

  char filename[64];
  memset(filename, '\0', 64);

  // initialize the file system simulator kernel
  if (Kernel::initialize() == false) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }

  if (argc < 2) {
    cout << PROGRAM_NAME << ": usage: " << PROGRAM_NAME << "filename" << endl;
    Kernel::exit(1);
  }

  // give the command line arguments a better name
  strcpy(filename, argv[1]);

  // create the link
  if (Kernel::unlink(filename) < 0) {
    // if Kernel::link fails, exit Kernel
    cout << "Failed to remove file" << endl;
    Kernel::exit(1);
  }

  // exit with success
  Kernel::exit(0);
}
