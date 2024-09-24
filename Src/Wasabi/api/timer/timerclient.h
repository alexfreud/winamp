#ifndef __TIMER_CLIENT_H
#define __TIMER_CLIENT_H

#include <bfc/dispatch.h>
#include <bfc/common.h>
#include <bfc/depend.h>
#include <map>

#define DEFERREDCB_TIMERID -2

class TimerClient;

#ifdef _WIN32
typedef UINT_PTR TimerToken ;
#elif defined(__APPLE__)
typedef EventLoopTimerRef TimerToken;
#else
#error port me!
#endif

typedef struct {
  TimerClient *origin;
  intptr_t param1;
  intptr_t param2;
} deferred_callback;

class NOVTABLE TimerClient : public Dispatchable 
{
protected:
  TimerClient() { }	

public:
  int timerclient_setTimer(int id, int ms);
  int timerclient_killTimer(int id);
  void timerclient_postDeferredCallback(intptr_t param1, intptr_t param2=0, int mindelay=0);
  int timerclient_onDeferredCallback(intptr_t param1, intptr_t param2);
  void timerclient_timerCallback(TimerToken token);

  TimerClient *timerclient_getMasterClient(); 
  void timerclient_onMasterClientMultiplex();
  api_dependent *timerclient_getDependencyPtr(); 
  void timerclient_setSkipped(int s);
  int timerclient_getSkipped();
  void timerclient_setTimerDelay(int td);
  int timerclient_getTimerDelay();
  const wchar_t *timerclient_getName();

  enum {
    TIMERCLIENT_TIMERCALLBACK   = 101,
    TIMERCLIENT_SETTIMER        = 110,
    TIMERCLIENT_KILLTIMER       = 120,
    TIMERCLIENT_GETMASTERCLIENT = 130,
    TIMERCLIENT_ONMASTERMUX     = 140,
    TIMERCLIENT_GETDEPPTR       = 150,
    TIMERCLIENT_SETSKIPPED      = 160,
    TIMERCLIENT_GETSKIPPED      = 170,
    TIMERCLIENT_SETTIMERDELAY   = 180,
    TIMERCLIENT_GETTIMERDELAY   = 190,
    TIMERCLIENT_POSTDEFERREDCB  = 200,
    TIMERCLIENT_ONDEFERREDCB    = 210,
    TIMERCLIENT_GETNAME         = 220,
  };
};

inline void TimerClient::timerclient_timerCallback(TimerToken token) {
  _voidcall(TIMERCLIENT_TIMERCALLBACK, token);
}

inline int TimerClient::timerclient_setTimer(int id, int ms) {
  return _call(TIMERCLIENT_SETTIMER, 0, id, ms);
}

inline int TimerClient::timerclient_killTimer(int id) {
  return _call(TIMERCLIENT_KILLTIMER, 0, id);
}

inline TimerClient *TimerClient::timerclient_getMasterClient() {
  return _call(TIMERCLIENT_GETMASTERCLIENT, (TimerClient *)NULL);
}

inline void TimerClient::timerclient_onMasterClientMultiplex() {
  _voidcall(TIMERCLIENT_ONMASTERMUX);
}

inline api_dependent *TimerClient::timerclient_getDependencyPtr() {
  return _call(TIMERCLIENT_GETDEPPTR, (api_dependent *)NULL);
}

inline void TimerClient::timerclient_setSkipped(int s) {
  _voidcall(TIMERCLIENT_SETSKIPPED, s);
}

inline int TimerClient::timerclient_getSkipped() {
  return _call(TIMERCLIENT_GETSKIPPED, 0);
}

inline void TimerClient::timerclient_setTimerDelay(int td) {
  _voidcall(TIMERCLIENT_SETTIMERDELAY, td);
}

inline int TimerClient::timerclient_getTimerDelay() {
  return _call(TIMERCLIENT_GETTIMERDELAY, 0);
}

inline void TimerClient::timerclient_postDeferredCallback(intptr_t param1, intptr_t param2, int mindelay) {
  _voidcall(TIMERCLIENT_POSTDEFERREDCB, param1, param2, mindelay);
}

inline int TimerClient::timerclient_onDeferredCallback(intptr_t param1, intptr_t param2) {
  return _call(TIMERCLIENT_ONDEFERREDCB, 0, param1, param2);
}

inline const wchar_t *TimerClient::timerclient_getName() {
  return _call(TIMERCLIENT_GETNAME, (const wchar_t *)NULL);
}

class NOVTABLE TimerClientI : public TimerClient {
protected:
  TimerClientI();

public:
  virtual ~TimerClientI();

  virtual int timerclient_setTimer(intptr_t id, int ms);
  virtual int timerclient_killTimer(intptr_t id);

  // override this to catch your timer events
  virtual void timerclient_timerCallback(int id) { }

  virtual TimerClient *timerclient_getMasterClient() { return NULL; }
  virtual void timerclient_onMasterClientMultiplex() { };
  virtual api_dependent *timerclient_getDependencyPtr()=0;
  virtual void timerclient_setSkipped(int s) { skipped = s; }
  virtual int timerclient_getSkipped() { return skipped; }
  virtual void timerclient_setTimerDelay(int td) { timerdelay = td; }
  virtual int timerclient_getTimerDelay() { return timerdelay; }
  virtual void timerclient_postDeferredCallback(intptr_t param1, intptr_t param2=0, int mindelay=0);
  virtual int timerclient_onDeferredCallback(intptr_t param1, intptr_t param2) { return 1; };
  virtual const wchar_t *timerclient_getName() { return NULL; }
  
protected:
  RECVS_DISPATCH;

private:
  virtual void timerclient_handleDeferredCallback(TimerToken token);

  int skipped;
  int timerdelay;
  int disallowset;
	PtrList<deferred_callback> cbs;
#ifdef _WIN32
	class TokenMap 
	{
	public:
		void delItem(TimerToken) {}
		bool reverseGetItem(intptr_t id, TimerToken *token)
		{
			*token = id;
			return true;
		}
		bool getItem(TimerToken token, intptr_t *id)
		{
			*id = token;
			return true;
		}
		void addItem(TimerToken, intptr_t) {}
	};
#else
	typedef std::map<TimerToken, intptr_t> TokenMap;
#endif
  TokenMap tokens;
};

class NOVTABLE TimerClientDI : public TimerClientI, public DependentI {
protected:
  TimerClientDI() { }

public:
  api_dependent *timerclient_getDependencyPtr() { return this; }
};


#endif
