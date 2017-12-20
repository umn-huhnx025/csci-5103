#include <cstdio>

#include "uthread.h"

void* spin(void* arg) {
  while (1) {
    printf("Current thread running in spin function is %d\n", uthread_self());
    /*Busy work to slow down counsel output*/
    for (int i = 0; i < 9999999; i++) {
      for (int i = 0; i < 20; i++)
        ;
    }
  }
  return (void*)0x1111;
}

void* Suspend_resume_terminate(void* arg) {
  printf("Creating 2 new threads that will just spin\n");
  usleep(2);
  int t1 = uthread_create(spin, (void*)0x1111);
  int t2 = uthread_create(spin, (void*)0x1111);
  /*Busy work to slow down console output this is used many times*/
  for (int i = 0; i < 9999999; i++) {
    for (int i = 0; i < 60; i++)
      ;
  }
  printf("now going to suspend thread %d\n", t1);
  sleep(1);
  uthread_suspend(t1);
  for (int i = 0; i < 9999999; i++) {
    for (int i = 0; i < 60; i++)
      ;
  }
  printf("now going to resume thread %d\n", t1);
  sleep(1);
  uthread_resume(t1);
  for (int i = 0; i < 9999999; i++) {
    for (int i = 0; i < 60; i++)
      ;
  }
  printf("now going to terminate thread %d\n", t1);
  uthread_terminate(t1);
  sleep(1);
  for (int i = 0; i < 9999999; i++) {
    for (int i = 0; i < 60; i++)
      ;
  }
  printf("now going to terminate thread %d\n", t2);
  uthread_terminate(t2);
  sleep(1);
  return (void*)0x1111;
}

int main() {
  uthread_init(10);
  void* retval;
  int mt = uthread_create(Suspend_resume_terminate, (void*)0x1111);
  uthread_join(mt, &retval);
  printf("End of test\n");
  return 0;
}
