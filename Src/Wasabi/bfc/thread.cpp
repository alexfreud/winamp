#include "precomp_wasabi_bfc.h"

#include "thread.h"

#if !defined(WIN32) && !defined(LINUX)
#error port me!
#endif

#pragma warning(push)
#pragma warning(disable : 4229)

THREADCALL Thread::ThreadProc(void *param) {
  Thread *th = static_cast<Thread*>(param);
  th->isrunning = true;
  int ret = th->threadProc();
  th->isrunning = false;
  return (THREADCALL)ret;
}

#pragma warning(pop)

Thread::Thread(HANDLE _parent_handle) :
  isrunning(false),
  killswitch(0),
  handle(0), parent_handle(0), nicekill(1),
  auto_close_parent(FALSE)
{
  setParentHandle(_parent_handle);
#ifdef WIN32
  handle = CreateThread(NULL, 0, ThreadProc, (LPVOID)this, CREATE_SUSPENDED, &threadid);
#endif
}

Thread::~Thread() {
  if (handle != 0) {
    if (isrunning) {
      setKillSwitch(TRUE);
      end();
      // FUCKO: might need to get rough here
      // FG> this serves no purpose, end() will not return unless the thread exits nicely
      #ifdef WIN32
        CloseHandle(handle);
      #elif defined(LINUX)
        pthread_kill(handle, SIGTERM);
      #endif
    }
  }
  setParentHandle(0);
}

int Thread::start() {
  if (running()) return 1;
#ifdef WIN32
  return !(ResumeThread(handle) == 0xffffffff);
#endif
#ifdef LINUX
  pthread_create(&handle, NULL, ThreadProc, (LPVOID)this);
  return 1;
#endif
}

bool Thread::running() {
  return isrunning;
}

int Thread::end() {
  while (running()) Sleep(66);
  return 1;
}

int Thread::kill() {
#ifdef WIN32
  CloseHandle(handle);
#elif defined(LINUX)
  pthread_kill(handle, SIGTERM);
#endif
  handle = 0;
  isrunning = false;
  return 1;
}

void Thread::setKillSwitch(int k) {
  killswitch = k;
}

int Thread::getKillSwitch() {
  if (killswitch) return 1;
  if (parent_handle != 0 && !parentRunning()) return 1;
  return 0;
}

THREADID Thread::getThreadId() const {
  return threadid;
}

void Thread::setParentHandle(HANDLE _parent_handle, int _auto_close) {
  if (parent_handle != 0 && auto_close_parent) {
#ifdef WIN32
    CloseHandle(parent_handle);
#else
//#error port me!
  DebugString("setParentHandle???\n");
#endif
  }
  parent_handle = _parent_handle;
  auto_close_parent = _auto_close;
}

int Thread::parentRunning() {
  if (parent_handle == 0) return -1;
#ifdef WIN32
  DWORD exitcode;
  int r = GetExitCodeThread(parent_handle, &exitcode);
  if (r == 0 || exitcode != STILL_ACTIVE) return 0;
  return 1;
#else
//#error port me!
  DebugString("parentRunning???\n");
#endif
}

void Thread::setPriority(int priority) {
    Wasabi::Std::setThreadPriority(priority, handle);
}
