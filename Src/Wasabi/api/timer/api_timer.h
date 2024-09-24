#ifndef __TIMER_API_H
#define __TIMER_API_H

// Under linux, the Timer API requires the Linux API

#include <bfc/dispatch.h>
#include <bfc/platform/platform.h>

class TimerClient;

#ifdef _WIN32
typedef UINT_PTR TimerToken ;
#elif defined(__APPLE__)
typedef EventLoopTimerRef TimerToken;
#else
#error port me!
#endif

class timer_api : public Dispatchable 
{
  public:
    TimerToken timer_add(TimerClient *client, intptr_t id, int ms);
    void timer_remove(TimerClient *client, TimerToken token);

  enum {
    TIMER_API_ADD = 1,
    TIMER_API_REMOVE = 11,
  };
};

inline TimerToken timer_api::timer_add(TimerClient *client, intptr_t id, int ms) 
{
  return _call(TIMER_API_ADD, (TimerToken)0, client, id, ms);
}

inline void timer_api::timer_remove(TimerClient *client, TimerToken token) 
{
  _voidcall(TIMER_API_REMOVE, client, token);
}

// {3130D81C-AE1F-4954-9765-698473B627B0}
static const GUID timerApiServiceGuid = 
{ 0x3130d81c, 0xae1f, 0x4954, { 0x97, 0x65, 0x69, 0x84, 0x73, 0xb6, 0x27, 0xb0 } };

extern timer_api *timerApi;

#endif