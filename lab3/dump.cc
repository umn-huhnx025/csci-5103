/**
 * a simple dump program
 * prints the offset, hexvalue, and decimal value for each byte in a
 * file, for all files mentioned on the command line.
 * @author Ray Ontko -> Converted to C++ by Kwangsung
 */
#include <stdlib.h>
#include <string.h>
#include "Kernel.h"

int main(int argc, char** argv) {
  char PROGRAM_NAME[8];
  strcpy(PROGRAM_NAME, "dump");

  if (argc != 2) {
    cout << PROGRAM_NAME << ": usage: " << PROGRAM_NAME << " filesystem-file"
         << endl;
    return -1;
  }

  ifstream is(argv[1]);

  if (is.is_open() == false) {
    cout << "error: unable to open input file " << argv[1] << endl;
    return -1;
  }

  // while we are able to read bytes from it
  int c;
  int off = 0;
  char hex[3];

  while (is.read(reinterpret_cast<char*>(&c), 1)) {
    //		cout << "here"<< endl;
    if (c > 0) {
      sprintf(hex, "%02x", c);
      // harToHex(c, hex);
      cout << off << " " << hex << " " << c;

      if (c >= 32 && c < 127) {
        cout << " " << (char)c;
      }

      cout << endl;
    }

    off++;
  }

  is.close();
}
