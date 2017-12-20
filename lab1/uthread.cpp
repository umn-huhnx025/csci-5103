#include "uthread.h"

#include <aio.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <algorithm>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/*This is the function that switches contexts.
  It traps the current state of the working thread and puts it on the ready
  list. Next it pops the front thread in the ready list and switchs to it's
  context. If a thread is switched AFTER the first call to uthread_switch, the
  flag is set so that the thread can immedaitly break from the function and
  return to it's former execuation */
void uthread_switch(int signum) {
  TCB* running = TCB::RunningThread;
  volatile int flag = true;
  if (running) {
    running->state = STATE_READY;
    TCB::ReadyList.push_back(running);
    getcontext(&running->context);
    if (flag == false) return;
  }
  if (TCB::ReadyList.size()) {
    TCB* next = TCB::ReadyList.front();
    TCB::ReadyList.pop_front();
    next->state = STATE_RUNNING;
    TCB::RunningThread = next;
    flag = false;
    setcontext(&next->context);
  }
}

void stub(void* (*func)(void*), void* arg) {
  TCB* tcb = TCB::TCBMap.at(uthread_self());
  *tcb->retval = (*func)(arg);
  tcb->returned = true;
  // printf("stub: thread %d's function returned %p\n", tcb->tid, *tcb->retval);
  uthread_terminate(tcb->tid);
}

int uthread_create(void* (*start_routine)(void*), void* arg) {
  TCB* running = TCB::TCBMap.at(uthread_self());
  if (running != TCB::RunningThread) {
    fprintf(stderr,
            "Error: uthread_create: You may only create if you are the running "
            "thread.\n");
    return -1;
  }
  sigaddset(&running->context.uc_sigmask, SIGVTALRM);

  TCB* tcb = new TCB();
  // printf("Created thread %d\n", tcb->tid);
  getcontext(&tcb->context);

  tcb->context.uc_mcontext.gregs[REG_RIP] = (long long)stub;
  tcb->context.uc_mcontext.gregs[REG_RDI] = (long long)start_routine;
  tcb->context.uc_mcontext.gregs[REG_RSI] = (long long)arg;
  tcb->context.uc_mcontext.gregs[REG_RSP] = (long long)tcb->sp;

  tcb->state = STATE_READY;
  TCB::ReadyList.push_back(tcb);
  // printf("Added thread %d to the ReadyList\n", tcb->tid);

  sigdelset(&running->context.uc_sigmask, SIGVTALRM);

  return tcb->tid;
}

int uthread_yield(void) {
  TCB* tcb = TCB::TCBMap.at(uthread_self());
  if (tcb != TCB::RunningThread) {
    fprintf(stderr,
            "Error: uthread_yield: You may only yield if you are the running "
            "thread.\n");
    return -1;
  }
  sigaddset(&tcb->context.uc_sigmask, SIGVTALRM);
  uthread_switch(0);

  // printf("yield: Cleaning up terminated threads\n");
  TCB::TerminatedList.clear();

  sigdelset(&tcb->context.uc_sigmask, SIGVTALRM);
  return 0;
}

int uthread_self(void) { return TCB::RunningThread->tid; }

int uthread_join(int tid, void** retval) {
  if (TCB::TCBMap.find(tid) == TCB::TCBMap.end()) {
    fprintf(stderr,
            "Error: uthread_join: Cannot join thread %d because it does not "
            "exist.\n",
            tid);
    return -1;
  }
  TCB* tcb = TCB::TCBMap.at(tid);
  while (!tcb->returned) uthread_yield();
  if (retval) *retval = *tcb->retval;
  return 0;
}

int uthread_init(int time_slice) {
  struct sigaction sa;
  struct itimerval timer;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &uthread_switch;
  sigaction(SIGVTALRM, &sa, NULL);

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = time_slice;

  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = time_slice;

  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  // Give us somewhere to go back to
  // printf("Initialized main thread\n");
  TCB* main_thread = new TCB();
  main_thread->state = STATE_RUNNING;
  TCB::RunningThread = main_thread;
  getcontext(&main_thread->context);

  return 0;
}
/* Terminates a thread by thread id. */
int uthread_terminate(int tid) {
  if (TCB::TCBMap.find(tid) == TCB::TCBMap.end()) {
    fprintf(stderr,
            "Error: uthread_terminate: Cannot terminate thread %d because it "
            "does not exist.\n",
            tid);
    return -1;
  }
  TCB* tcb = TCB::TCBMap.at(tid);
  sigaddset(&tcb->context.uc_sigmask, SIGVTALRM);
  switch (tcb->state) {
    case STATE_READY:
      TCB::ReadyList.remove(tcb);
      break;
    case STATE_RUNNING:
      tcb->state = STATE_TERMINATED;
      TCB::TerminatedList.push_back(tcb);
      TCB::RunningThread = nullptr;
      uthread_switch(0);
      break;
    case STATE_SUSPENDED:
      TCB::SuspendList.remove(tcb);
      break;
    case STATE_TERMINATED:
      TCB::TerminatedList.remove(tcb);
      break;
    default:
      fprintf(
          stderr,
          "Error: uthread_terminate: Thread %d is in an unknown state (%d).\n",
          tid, tcb->state);
      sigdelset(&tcb->context.uc_sigmask, SIGVTALRM);
      return -1;
  }
  sigdelset(&tcb->context.uc_sigmask, SIGVTALRM);
  return 0;
}
/* Suspends a thread by thread id. Returns 0 on success and -1 on failure */
int uthread_suspend(int tid) {
  if (TCB::TCBMap.find(tid) == TCB::TCBMap.end()) {
    fprintf(stderr,
            "Error: uthread_suspend: Cannot suspend thread %d because it "
            "does not exist.\n",
            tid);
    return -1;
  }
  TCB* tcb = TCB::TCBMap.at(tid);
  sigaddset(&tcb->context.uc_sigmask, SIGVTALRM);
  if (tcb->state == STATE_RUNNING) {
    tcb->state = STATE_SUSPENDED;
    TCB::SuspendList.push_back(tcb);
    TCB::RunningThread = nullptr;
    getcontext(&tcb->context);
  } else if (tcb->state == STATE_READY) {
    tcb->state = STATE_SUSPENDED;
    TCB::ReadyList.remove(tcb);
    TCB::SuspendList.push_back(tcb);
  } else {
    fprintf(stderr,
            "Threads can only be suspended if they are running or ready!");
    sigdelset(&tcb->context.uc_sigmask, SIGVTALRM);
    return -1;
  }
  sigdelset(&tcb->context.uc_sigmask, SIGVTALRM);
  return 0;
}
/* resumes a suspend thread by it's id and return 0. If the
   thread is not in the suspend state, it will return -1 */
int uthread_resume(int tid) {
  if (TCB::TCBMap.find(tid) == TCB::TCBMap.end()) {
    fprintf(stderr,
            "Error: uthread_resume: Cannot resume thread %d because it "
            "does not exist.\n",
            tid);
    return -1;
  }
  TCB* tcb = TCB::TCBMap.at(tid);
  sigaddset(&tcb->context.uc_sigmask, SIGVTALRM);
  if (tcb->state != STATE_SUSPENDED) {
    fprintf(stderr, "Threads can only be resumed if they were suspended\n");
    return -1;
  }
  TCB::SuspendList.remove(tcb);
  TCB::ReadyList.push_back(tcb);
  tcb->state = STATE_READY;
  sigdelset(&tcb->context.uc_sigmask, SIGVTALRM);
  return 0;
}

// Asynchronous read wrapper function. The async_read function is a wrapper
// around the read system call to give the illusion of a synchronous read to
// the calling thread without suspending the whole process. async_read()
// attempts to read up to nbytes bytes from file descriptor filedes into the
// buffer starting at buf. See man read for a description of the read system
// call.
//
// On success, returns the number of bytes read from filedes and written to
// buf. On failure, returns -1 and sets errno appropriately.
ssize_t async_read(int filedes, void* buf, size_t nbytes) {
  ssize_t num_read;
  aiocb* a_read = new aiocb();

  // Initialize asynchronous I/O control block struct
  a_read->aio_fildes = filedes;
  a_read->aio_offset = 0;
  a_read->aio_buf = buf;
  a_read->aio_nbytes = nbytes;
  a_read->aio_reqprio = 0;
  a_read->aio_sigevent.sigev_notify = SIGEV_NONE;
  a_read->aio_lio_opcode = LIO_READ;

  // On aio_read() failure, return -1
  if (aio_read(a_read) == -1) {
    return -1;
  }

  // While the asynchronous I/O read is in progress
  while (aio_error(a_read) == EINPROGRESS) {
    // Yield uthread
    uthread_yield();
  }

  // On asynchronous I/O read cancelation or failure, return -1
  if (aio_error(a_read) != 0) {
    return -1;
  }

  // On aio_return() failure, return -1
  if ((num_read = aio_return(a_read)) == -1) {
    return -1;
  }

  // On success, return the number of bytes read
  return num_read;
}
