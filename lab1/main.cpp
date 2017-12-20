#include <cstdio>

#include "uthread.h"

void* f(void* arg) {
  printf("f: Thread %d, arg = %p\n", uthread_self(), arg);
  return (void*)(uthread_self() * 0x1000L);
}

void* test_join(void* arg) {
  printf("test_join: Thread %d, arg = %p\n", uthread_self(), arg);
  int tid = uthread_create(f, (void*)0x6868);
  void* retval;
  printf("Join waiting for thread %d\n", tid);
  uthread_join(tid, &retval);
  printf("Joined thread %d, retval = %p\n", tid, retval);
  return (void*)17;
}

void* long_test(void* arg) {
  int x;
  printf("just got here\n");
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 500000; j++) {
      x++;
    }
  }
  printf("Done!\n");
  return (void*)17;
}

int main() {
  int threads[4];
  uthread_init(9999);
  void* retval;
  threads[0] = uthread_create(long_test, (void*)0x1111);
  threads[1] = uthread_create(long_test, (void*)0x1111);
  threads[2] = uthread_create(long_test, (void*)0x1111);
  threads[3] = uthread_create(long_test, (void*)0x1111);
  for(int i=0; i<4; i++) {
    uthread_join(threads[i],&retval);
  }
  return 0;
}
