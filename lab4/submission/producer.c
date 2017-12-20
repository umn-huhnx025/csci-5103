#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Get number of writes from command line argument
  // Default to 1 write if no arguments given
  int num_writes = 1;
  if(argc > 1) {
    num_writes = atoi(argv[1]);
  }


  // Open device driver
  int device = open("/dev/scullBuffer0", O_WRONLY);
  if(device < 0) {
    perror("Failed to open /dev/scullBuffer0");
    return -1;
  }


  // Write to device
  int i;
  for(i = 0; i < num_writes; i++) {
    // Format string to be written to device
    char buffer[25];
    snprintf(buffer, 25, "Hello, world! %10d", i+1);

    // Prompt user to initiate next write
    printf("Press enter to execute write %d...\n", i+1);
    getchar();

    // Write to device driver
    int bytes_written = write(device, buffer, 25);
    if(bytes_written < 0) {
      perror("Failed to write to /dev/scullBuffer0");
      return -1;
    }
    
    if(bytes_written == 0) {
      printf("Buffer full and no consumers, could not write yet\n");
      i--; 
    } else {
      printf("Wrote 'Hello, world! %10d' to device.\n", i+1);
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
