#include <cstdio>

#include "uthread.h"

void* test_yield1(void* arg) {
  printf("thread %d: Before yield. Yielding...\n", uthread_self());

  // Commenting the following line out will probably lead to both print
  // statements happening consecutively.
  uthread_yield();

  printf("thread %d: After yield. Done.\n", uthread_self());
  return 0;
}

void* test_yield2(void* arg) {
  printf("thread %d: This should happen in between.\n", uthread_self());
  return 0;
}

int main() {
  uthread_init(10);
  int t1 = uthread_create(test_yield1, 0);
  int t2 = uthread_create(test_yield2, 0);
  uthread_join(t1, nullptr);
  uthread_join(t2, nullptr);
  return 0;
}
