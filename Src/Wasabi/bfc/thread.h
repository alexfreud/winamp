#ifndef _THREAD_H
#define _THREAD_H

#include <bfc/common.h>

#if !defined(WIN32) && !defined(LINUX)
#error port me!
#endif

#ifdef WIN32
#define THREADCALL DWORD WINAPI
#endif
#ifdef LINUX
#define THREADCALL void *
#endif

/**
  class Thread is a portable wrapper for an operating system thread. A thread
  is a thread of execution in a process. Ask Google if you don't know what that
  means.

  To use this class, derive from Thread and override the threadProc() method.
  Simply return from this method and the thread will end.

  Our Thread works on the kill-switch concept. You should write your thread
  code to continually check the getKillSwitch() method to determine if you
  should exit. When either getKillSwitch() returns TRUE or your processing is
  done, just return from the method and the thread will end.

  @short Thread object.
  @author Nullsoft
  @see threadProc()
  @see getKillSwitch()
  @see CriticalSection
*/
class Thread {
public:
/**
  Creates a new Thread object. You can optionally pass in the handle of another
  thread. If you do, the getKillSwitch function will check if this thread is
  still running, and return TRUE if it has died. Usually this other thread
  will be the main thread.
  Threads are created in a suspended state. To start them, call start().

  @see threadProc()
  @see getKillSwitch()
  @see ComponentAPI::main_getMainThreadHandle()
  @see start()
*/
  Thread(HANDLE parent_handle=0);
  virtual ~Thread();

/**
  Starts the thread. You must call this at some point after creating the Thread.

  @ret Boolean. TRUE on success, FALSE on failure.
*/
  int start();

/**
  Checks whether the thread is still running.

  @ret Boolean. TRUE if thread is still running, FALSE if it has ended.
*/
  bool running();

/**
  Waits until thread ends. Does not kill the thread, though, so you might want
  to call setKillSwitch(TRUE) to encourage the thread to end.

  @see setKillSwitch()
*/
  int end();	// waits for thread to end, might want to throw killswitch too
  int kill(); // does not wait, simply kills the thread, use with care

/**
  Sets the internal kill switch to k.

  @param k Boolean. TRUE to force getKillSwitch() to return TRUE.
  @see getKillSwitch()
*/
  void setKillSwitch(int k);

  THREADID getThreadId() const;

/**
  Sets the parent handle. If a parent handle is set, the getKillSwitch()
  function will return TRUE when the parent thread ends.

  @param parent_handle The handle of the thread to monitor.
  @param auto_close If TRUE, the parent handle will be closed on shutdown. Normally this is the desired behavior.
*/
  void setParentHandle(HANDLE parent_handle, int auto_close=TRUE);

/**
  Checks if the parent thread is still running (if a handle was provided.)

  @see Thread()
  @see getKillSwitch()
  @ret Boolean, TRUE if parent is still running, FALSE otherwise.
*/
  int parentRunning();	// if parent handle given, is it still running

/**
  Override this method to do your thread's work.
*/
  virtual int threadProc()=0;

  enum {	// these have to match std.cpp
    PRIORITY_IDLE=-32767,
    PRIORITY_LOWEST=-2,
    PRIORITY_BELOW_NORMAL=-1,
    PRIORITY_NORMAL=0,
    PRIORITY_HIGH=1,
    PRIORITY_HIGHEST=2,
    PRIORITY_TIME_CRITICAL=32767,
  };
/**
  Sets the thread's processing priority.

  @param priority The new priority for the thread.
  @see Std::setThreadPriority()
*/
  void setPriority(int priority);
  
protected:
/**
  Your thread should call this at intervals to determine if it should exit.
  @ret Boolean of whether the thread should exit.
  @see threadProc()
*/
  int getKillSwitch();	// check this and return when it's nonzero

private:
  static THREADCALL ThreadProc(void *param);

  int killswitch;
  HANDLE handle, parent_handle;
  int auto_close_parent;
  THREADID threadid;
  bool isrunning;
  int nicekill;
};

#endif
