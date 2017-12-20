#include "tcb.h"

TCBList TCB::ReadyList;
TCBList TCB::TerminatedList;
TCBList TCB::SuspendList;
std::map<int, TCB*> TCB::TCBMap;
TCBList::iterator TCB::iter;
TCB* TCB::RunningThread;

TCB::TCB() {
  static int current_tid = 0;
  tid = current_tid++;
  freeable_sp = malloc(TCB::INITIAL_STACK_SIZE);
  sp = (char*)freeable_sp + TCB::INITIAL_STACK_SIZE - sizeof(int);
  retval = (void**)malloc(sizeof(void**));
  returned = false;
  TCBMap.emplace(tid, this);
}

TCB::~TCB() {
  TCB::TCBMap.erase(tid);
  free(retval);
  free(freeable_sp);
}
