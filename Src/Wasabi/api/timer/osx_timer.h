#ifndef NULLSOFT_WASABI_OSX_TIMER_H
#define NULLSOFT_WASABI_OSX_TIMER_H

#include <api/timer/api_timer.h>

class TimerApi : public timer_apiI
{
public:
  TimerApi();
   TimerToken timer_add(TimerClient *client, int id, int ms);
   void timer_remove(TimerClient *client, TimerToken);
private:
     EventLoopRef mainEventLoop;
};


#endif