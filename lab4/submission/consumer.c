#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Get number of reads from command line argument
  // Default to 1 read if no arguments given
  int num_reads = 1;
  if(argc > 1) {
    num_reads = atoi(argv[1]);
  }


  // Open device driver
  int device = open("/dev/scullBuffer0", O_RDONLY);
  if(device < 0) {
    perror("Failed to open /dev/scullBuffer0");
    return -1;
  }


  // Read from device
  int i;
  for(i = 0; i < num_reads; i++) {
    char buffer[512];

    // Prompt user to initiate next read
    printf("Press enter to execute read %d...\n", i+1);
    getchar();

    // Read from device driver
    int bytes_read = read(device, &buffer, 512);
    if(bytes_read < 0) {
      perror("Failed to read from /dev/scullBuffer0");
      return -1;
    }

    if(bytes_read == 0) {
      printf("Buffer empty and no producers, could not read yet\n");
      i--;
    } else {
      printf("Read '%s' from device\n", buffer);
    }
  }


  // Close device
  if(close(device) < 0) {
    perror("Failed to close /dev/scullBuffer0");
    return -1;
  }


  // Return 
  return 0;
}

