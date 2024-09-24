#ifndef _CRITSEC_H
#define _CRITSEC_H

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#endif

#include <bfc/common.h>
#include <bfc/bfc_assert.h>

/**
  CriticalSection is a portable object that implements a critical section,
  which is to say, it ensures that no two threads can be in that same
  section of code at the same time. Usually you make them a global object
  or allocate them with new and pass them to both threads.

  @short Critical section class.
  @author Nullsoft
  @ver 1.0
  @see Thread
  @see InCriticalSection
*/

class CriticalSection {
public:
  CriticalSection();
  virtual ~CriticalSection();

/**
  Enters the critical section. If another thread is already in the critical
  section, the calling thread will be blocked until the other thread calls
  leave().
  @see leave()
*/
  void enter();
/**
  Leaves the critical section. If another thread is currently blocked on
  this critical section, it will be unblocked. If multiple threads are blocking
  only one will be unblocked.
  @see enter()
*/
  void leave();

/**
  Calls enter() and leave() in quick succession. Useful to make sure that no
  other thread is in the critical section (although another thread could
  immediately re-enter)
  @see enter()
  @see leave()
*/
  void inout();

private:
#ifdef ASSERTS_ENABLED
  int within;
#endif
#ifdef _WIN32
  CRITICAL_SECTION cs;
#elif defined(__APPLE__)
  MPCriticalRegionID cr;
#endif
  
};

/**
  This is a little helper class to ease the use of class CriticalSection.
  When it is instantiated, it enters a given critical section. When it is
  destroyed, it leaves the given critical section.
    CriticalSection a_cs;
    void blah() {
      InCriticalSection cs(a_cs); // critical section protection begins
      if (test) {
        return 0; // critical section protection ends
      }
      // critical section protection still active!
      doSomething();
      return 1; // critical section protection ends
    }
  
  @author Nullsoft
  @see CriticalSection
*/
class InCriticalSection {
public:
  InCriticalSection(CriticalSection *cs) : m_cs(cs) { m_cs->enter(); }
  InCriticalSection(CriticalSection &cs) : m_cs(&cs) { m_cs->enter(); }
  ~InCriticalSection() { m_cs->leave(); }
private:
  CriticalSection *m_cs;
};

#define _INCRITICALSECTION(id, x) InCriticalSection __I_C_S__##id(x)
#define INCRITICALSECTION(x) _INCRITICALSECTION(__LINE__, x)
#endif
