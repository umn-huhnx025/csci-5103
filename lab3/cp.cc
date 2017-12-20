/**
 * A simple copy program for a simulated file system.
 * @author Ray Ontko -> Converted to C++ by Kwangsung
 */
#include <stdlib.h>
#include <string.h>
#include "DirectoryEntry.h"
#include "Kernel.h"
#include "Stat.h"

#define BUF_SIZE 4096
#define OUTPUT_MODE 0700

int main(int argc, char** argv) {
  char PROGRAM_NAME[8];
  strcpy(PROGRAM_NAME, "cp");

  // initialize the file system simulator kernel
  if (Kernel::initialize() == false) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }
  // print a helpful message if no command line arguments are given
  if (argc != 3) {
    cout << PROGRAM_NAME << ": usage: " << PROGRAM_NAME
         << " input-file output-file";
    Kernel::exit(1);
  }

  // give the parameters more meaningful names
  char in_name[64];
  char out_name[64];

  strcpy(in_name, argv[1]);
  strcpy(out_name, argv[2]);

  // open the input file
  int in_fd = Kernel::open(in_name, Kernel::O_RDONLY);

  if (in_fd < 0) {
    Kernel::perror(PROGRAM_NAME);
    cout << PROGRAM_NAME << ": unable to open input file \"" << in_name << "\"";
    Kernel::exit(2);
  }

  // open the output file
  int out_fd = Kernel::creat(out_name, OUTPUT_MODE);
  if (out_fd < 0) {
    Kernel::perror(PROGRAM_NAME);
    cout << PROGRAM_NAME << ": unable to open output file \"" << in_name
         << "\"";
    Kernel::exit(3);
  }

  // read and write buffers full of data while we can
  int rd_count = 0;
  char buffer[BUF_SIZE];
  memset(buffer, '\0', BUF_SIZE);

  while (true) {
    // read a buffer full from the input file
    rd_count = Kernel::read(in_fd, buffer, BUF_SIZE);

    // if error or nothing read, quit the loop
    if (rd_count <= 0) {
      break;
    }

    // write whatever we read to the output file
    int wr_count = Kernel::write(out_fd, buffer, rd_count);

    // if error or nothing written, give message and exit
    if (wr_count <= 0) {
      Kernel::perror(PROGRAM_NAME);
      cout << PROGRAM_NAME << ": error during write to output file";
      Kernel::exit(4);
    }
  }

  // close the files
  Kernel::close(in_fd);
  Kernel::close(out_fd);

  // check to see if the final read was successful; exit accordingly
  if (rd_count == 0) {
    Kernel::exit(0);
  } else {
    Kernel::perror(PROGRAM_NAME);
    cout << PROGRAM_NAME << ": error during read from input file";
    Kernel::exit(5);
  }
}
