#include "precomp_wasabi_bfc.h"
#include "critsec.h"
// uncomment this if needed
//#define CS_DEBUG

CriticalSection::CriticalSection() {
#ifdef WIN32
  InitializeCriticalSection(&cs);
#elif defined(__APPLE__)
  MPCreateCriticalRegion(&cr);
#elif defined(LINUX)
  pthread_mutex_t recursive = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  cs.mutex = recursive;
#endif

#ifdef ASSERTS_ENABLED
#ifdef CS_DEBUG
  within = 0;
#endif
#endif
}

CriticalSection::~CriticalSection() {
#ifdef CS_DEBUG
#ifdef ASSERTS_ENABLED
  ASSERT(!within);
#endif
#endif
#ifdef WIN32
  DeleteCriticalSection(&cs);
#elif defined(__APPLE__)
  MPDeleteCriticalRegion(cr);
#elif defined(LINUX)
  pthread_mutex_destroy(&cs.mutex);
#endif
}

void CriticalSection::enter() {
#ifdef WIN32
  EnterCriticalSection(&cs);
#elif defined(__APPLE__)
  MPEnterCriticalRegion(cr, kDurationForever);
#elif defined(LINUX)
  pthread_mutex_lock(&cs.mutex);
#endif

#ifdef CS_DEBUG
#ifdef ASSERTS_ENABLED
  ASSERT(!within);
  within = 1;
#endif
#endif
}

void CriticalSection::leave() {
#if defined(CS_DEBUG) && defined(ASSERTS_ENABLED)
  ASSERT(within);
  within = 0;
#endif

#ifdef WIN32
  LeaveCriticalSection(&cs);
#elif defined(__APPLE__)
  MPExitCriticalRegion(cr);
#elif defined(LINUX)
  pthread_mutex_unlock(&cs.mutex);
#endif
}

void CriticalSection::inout() {
  enter();
  leave();
}
