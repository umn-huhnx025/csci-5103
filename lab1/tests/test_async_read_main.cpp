#include <fcntl.h>
#include <stdio.h>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "uthread.h"

// Tests the functionality of async_read(). Creates a Hello World file with
// filename given by the arg argument. Then reads the file to the buffer by
// calling async_read().
void* async_read_routine(void* arg) {
  char buffer[100];
  int fd;

  // Create Hello, World file.txt
  std::ofstream o((char*)arg);
  o << "Hello, World!" << std::endl;

  // Open file.txt for reading
  fd = open((char*)arg, O_RDONLY);

  // "Asynchronously" read the file to the buffer
  if ((async_read(fd, buffer, 14)) == -1) {
    printf("Thread %d: Failed read\n", uthread_self());
    return (void*)-1;
  }

  // Loop to take more time
  int x;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 500000; j++) {
      x++;
    }
  }

  // Print out file contents
  printf("Thread %d: File contents: %s \n", uthread_self(), buffer);

  close(fd);
  remove((char*)arg);

  return (void*)0;
}

// Tests the functionality of async_read() by calling async_read_routine only
// once.
long async_read_test_single() {
  int tid;
  void* retval;

  // Create uthread running async_read_routine()
  tid = uthread_create(async_read_routine, (void*)"file.txt");

  // Join uthread created above
  uthread_join(tid, &retval);

  // Return return value from async_read_routine
  return (long)retval;
}

// Tests the functionality of async_read() by calling async_read_routine for
// multiple uthreads.
long async_read_test_multiple() {
  int tid_list[5];
  void* retval;

  // Create uthread's running async_read_routine()
  tid_list[0] = uthread_create(async_read_routine, (void*)"file1.txt");
  tid_list[1] = uthread_create(async_read_routine, (void*)"file2.txt");
  tid_list[2] = uthread_create(async_read_routine, (void*)"file3.txt");
  tid_list[3] = uthread_create(async_read_routine, (void*)"file4.txt");
  tid_list[4] = uthread_create(async_read_routine, (void*)"file5.txt");

  // For each uthread created above
  for (int i = 0; i < 5; i++) {
    // Join uthread
    uthread_join(tid_list[i], &retval);

    // Return -1 if uthread returns -1
    if ((long)retval == -1) {
      return -1;
    }
  }

  return 0;
}

int main() {
  uthread_init(9999);

  // Test async_read()
  async_read_test_single();
  async_read_test_multiple();

  return 0;
}
