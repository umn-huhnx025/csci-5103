#include <cstdio>

#include "uthread.h"

void* f(void* arg) {
  printf("thread %d: Arg is %p.\n", uthread_self(), arg);

  void* retval = (void*)0xdeadbeef;
  printf("thread %d: Returning %p\n", uthread_self(), retval);
  return retval;
}

int main() {
  uthread_init(10);

  void* arg = (void*)0x42424242;
  printf("thread %d: Creating new thread. Arg is %p\n", uthread_self(), arg);
  int t1 = uthread_create(f, arg);

  void* retval;
  uthread_join(t1, &retval);
  printf("thread %d: New thread returned value %p\n", uthread_self(), retval);

  return 0;
}
