#ifndef __TIMER_MULTIPLEXER_H
#define __TIMER_MULTIPLEXER_H

#include <bfc/common.h>
#include <bfc/ptrlist.h>
#include <api/dependency/api_dependentviewer.h>

// FG> not too sure how to get a callback for attribute change, if anyone wants to change it be my guest ;)
#define RESOLUTION_CHECK_DELAY 1000 // check for resolution changes every second

// if uioptions CfgItem not found, use this value for resolution
#define DEF_RES 20

// below MAX_TIMER_DELAY, timer are multiplexed using a 'wheel' algorithm (mem used = MAX_TIMER_DELAY/resolution * sizeof(PtrList) + ntimers*sizeof(MultiplexedTimer), but fast (no lookup) )
// above MAX_TIMER_DELAY, resolution drops to MAX_TIMER_DELAY/LOW_RES_DIV and uses ntimers*sizeof(MultiplexedTimer) bytes
#define MAX_TIMER_DELAY 1000  // keep this dividable by LOW_RES_DIV please
#define LOW_RES_DIV        4

class CfgItem;
class api_config;

class TimerMultiplexerClient {
  public:
    virtual void onMultiplexedTimer(void *data, int skip, int mssincelast)=0;
};

class MultiplexedTimer {
  public:
    MultiplexedTimer(int _ms, void *_data) : ms(_ms), data(_data) {
      nexttick=0;
      flag=0;
      lost = 0;
      lastmscount=0;
      lastdelay=0;
    }
    virtual ~MultiplexedTimer() { }

    int ms;
    void *data;
    DWORD nexttick; // only used by low precision timers
    int flag; // only used by hi precision timers
    float lost; // only used by hi precision timers
    DWORD lastmscount;
    int lastdelay;
};

class TimerMultiplexer : public ifc_dependentviewer
{
  public:

    TimerMultiplexer();
    virtual ~TimerMultiplexer();

    virtual void setClient(TimerMultiplexerClient *client);

    virtual void onServerTimer();

    virtual void addTimer(int ms, void *data);
    virtual void removeTimer(void *data);
    virtual void setResolution(int ms);

    virtual void shutdown();
    virtual int getNumTimers();
    virtual int getNumTimersLP();

  private:

    void checkResolution(DWORD now);
    void resetTimer(int newresolution);
    void resetWheel();
    void distributeAll();
    void distribute(MultiplexedTimer *t);
    void runCurSlice(DWORD now);
    void runTimer(DWORD now, DWORD last, MultiplexedTimer *t, PtrList<MultiplexedTimer> *slice, int pos);
    void removeFromWheel(MultiplexedTimer *t);
    void runLowPrecisionTimers(DWORD now);
    void removeFromLowPrecision(MultiplexedTimer *t);
    void doShutdown();
    PtrList<MultiplexedTimer> *getSlice(int n);

    TimerMultiplexerClient *client;
    int resolution;
    bool check_resolution;
    int timerset;

    int curslice;
    int nslices;
    int justexited;
    int firstevent;

    PtrList< PtrList< MultiplexedTimer > > wheel;
    PtrList< MultiplexedTimer > timers;
    PtrList< MultiplexedTimer > lptimers;
    MultiplexedTimer *running_timer;
		int dependentViewer_callback(ifc_dependent *item, const GUID *classguid, int cb, intptr_t param1 = 0, intptr_t param2 = 0, void *ptr = NULL, size_t ptrlen = 0);

  CfgItem *uioptions;
	RECVS_DISPATCH;
};

class MultiplexerServer {
public:
  MultiplexerServer(TimerMultiplexer *mux, UINT tid) : m_mux(mux), m_tid(tid) {}
  virtual ~MultiplexerServer() {}
  TimerMultiplexer *getMultiplexer() { return m_mux; }
  UINT_PTR getId() { return m_tid; }
  void setId(UINT_PTR id) { m_tid = id; }
private:
  TimerMultiplexer *m_mux;
  UINT_PTR m_tid;
};

class MultiplexerServerComparatorTID {
public:
  // comparator for sorting
  static int compareItem(MultiplexerServer *p1, MultiplexerServer* p2) {
    if (p1->getId() < p2->getId()) return -1;
    if (p1->getId() > p2->getId()) return 1;
    return 0;
  }
  // comparator for searching
  static int compareAttrib(const wchar_t *attrib, MultiplexerServer *item) {
    if (*((UINT *)attrib) < item->getId()) return -1;
    if (*((UINT *)attrib) < item->getId()) return 1;
    return 0;
  }
};

class MultiplexerServerComparatorMux{
public:
  // comparator for sorting
  static int compareItem(MultiplexerServer *p1, MultiplexerServer* p2) {
    if (p1->getMultiplexer() < p2->getMultiplexer()) return -1;
    if (p1->getMultiplexer() > p2->getMultiplexer()) return 1;
    return 0;
  }
  // comparator for searching
  static int compareAttrib(const wchar_t *attrib, MultiplexerServer *item) {
    if ((TimerMultiplexer *)attrib < item->getMultiplexer()) return -1;
    if ((TimerMultiplexer *)attrib < item->getMultiplexer()) return 1;
    return 0;
  }
};

#endif
