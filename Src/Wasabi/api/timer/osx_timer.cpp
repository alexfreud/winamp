#include "osx_timer.h"
#include <api/timer/timerclient.h>

timer_api *timerApi = NULL;


TimerApi::TimerApi()
{
  mainEventLoop = GetMainEventLoop(); 
}

static void WasabiTimerProc(EventLoopTimerRef inTimer, void * inUserData)
{
  TimerClient *client = (TimerClient *)inUserData;
  if (client)
    client->timerclient_timerCallback(inTimer);
}


TimerToken TimerApi::timer_add(TimerClient *client, int id, int ms)
{
  EventLoopTimerRef token;
  OSStatus err = InstallEventLoopTimer(mainEventLoop, 
                                       (float)ms/1000.0f,
                                       (float)ms/1000.0f,
                                       WasabiTimerProc,
                                       client,
                                       &token);
  if (err == noErr)
    return token;
  else
    return 0;                                                                             
}

void TimerApi::timer_remove(TimerClient *client, TimerToken token)
{
  RemoveEventLoopTimer(token);
}
