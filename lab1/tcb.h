#pragma once

#include <ucontext.h>
#include <list>
#include <map>

enum State {
  STATE_READY,
  STATE_RUNNING,
  STATE_SUSPENDED,
  STATE_TERMINATED,
};

class TCB;

typedef std::list<TCB*> TCBList;

class TCB {
 public:
  int tid;
  ucontext_t context;
  void *sp, *freeable_sp;
  State state;
  void* arg;
  void** retval;
  bool returned;

  static const int INITIAL_STACK_SIZE = 4096;

  static TCBList ReadyList;
  static TCBList TerminatedList;
  static TCBList SuspendList;
  static TCB* RunningThread;
  static std::map<int, TCB*> TCBMap;
  static TCBList::iterator iter;

  TCB();
  ~TCB();
};
