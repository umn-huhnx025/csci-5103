/**
 * Creates a hard link to target with filename link
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
  strcpy(PROGRAM_NAME, "ln");

  char target[64];
  memset(target, '\0', 64);

  char link[64];
  memset(link, '\0', 64);

  // initialize the file system simulator kernel
  if (Kernel::initialize() == false) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }

  if (argc < 3) {
    cout << PROGRAM_NAME << ": usage: " << PROGRAM_NAME
         << "target_name link_name" << endl;
    Kernel::exit(1);
  }

  // give the command line arguments a better name
  strcpy(target, argv[1]);
  strcpy(link, argv[2]);

  // create the link
  if (Kernel::link(target, link) < 0) {
    // if Kernel::link fails, exit Kernel
    cout << "Failed to create link" << endl;
    Kernel::exit(1);
  }

  // exit with success
  Kernel::exit(0);
}
