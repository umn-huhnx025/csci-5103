/**
 * Reads standard input and writes to standard output
 * and the named file.
 * A simple tee program for a simulated file system.
 * @author Ray Ontko -> Converted to C++ by Kwangsung
 */
#include <stdlib.h>
#include <string.h>
#include "Kernel.h"

#define BUF_SIZE 4096
#define OUTPUT_MODE 0100700
using namespace std;

int main(int argc, char** argv) {
  char PROGRAM_NAME[8];
  strcpy(PROGRAM_NAME, "tee");

  char name[64];
  memset(name, '\0', 64);

  // initialize the file system simulator kernel
  if (Kernel::initialize() == false) {
    cout << "Failed to initialized Kernel" << endl;
    Kernel::exit(1);
  }

  if (argc < 2) {
    cout << PROGRAM_NAME << ": usage: " << PROGRAM_NAME << " output-file"
         << endl;
    Kernel::exit(1);
  }

  // give the command line argument a better name
  strcpy(name, argv[1]);

  // create the output file
  int out_fd = Kernel::creat(name, OUTPUT_MODE);
  if (out_fd < 0) {
    Kernel::perror(PROGRAM_NAME);
    cout << PROGRAM_NAME << ": unable to open output file \"" << name << "\""
         << endl;
    Kernel::exit(2);
  }

  // create a buffer for reading from standard input
  //	char buffer[BUF_SIZE] ;
  //	memset(buffer, '\0', BUF_SIZE);

  char c;

  // while we can, read from standard input
  int wr_count = 0;

  while (1)  // cin.eof())
  {
    // read a buffer full of data from standard input
    //		cin.get(buffer, BUF_SIZE);

    // if we reach the end (-1), quit the loop
    // if(cin.eof())
    //{
    //	break ;
    //}
    c = cin.get();

    if (cin.eof()) {
      break;
    }

    // write what we read to the output file; if error, exit
    wr_count = Kernel::write(out_fd, &c, 1);  // buffer, strlen(buffer));
    if (wr_count <= 0) {
      cout << endl;
      Kernel::perror(PROGRAM_NAME);
      cout << PROGRAM_NAME << ": error during write to output file" << endl;
      Kernel::exit(3);
    }

    // write what we read to standard output
    cout << c;  //"  -> " << wr_count << "bytes written" << endl;
  }

  // close the output file
  Kernel::close(out_fd);

  // exit with success
  Kernel::exit(0);
}
