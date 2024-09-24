#ifndef __MAIN_TIMERMULTIPLEXER_H
#define __MAIN_TIMERMULTIPLEXER_H

#include "timermul.h"
#include <bfc/depend.h>
#include <bfc/string/StringW.h>
#include <api/timer/timerclient.h>

class VirtualTimer 
{
  public:
    VirtualTimer(TimerClient *_client, intptr_t _id, api_dependent *depend);
    virtual ~VirtualTimer() { }

    TimerClient *client, *mclient;
    api_dependent *dep;
    StringW name;
    intptr_t id;
};

class MainTimerMultiplexer : public TimerMultiplexer, public TimerMultiplexerClient, public DependentViewerI  {
  public:

    MainTimerMultiplexer();
    virtual ~MainTimerMultiplexer();
    
    virtual void add(TimerClient *client, intptr_t id, int ms);
    virtual void remove(TimerClient *client, intptr_t id);
  
    virtual void onMultiplexedTimer(void *data, int skip, int mssincelast);
    virtual void onServerTimer();

    virtual int viewer_onItemDeleted(api_dependent *item);

  private:

    int isValidTimerClientPtr(TimerClient *tc, api_dependent *dep);

    int haveClient(TimerClient *client);
    PtrList<VirtualTimer> timerclients;
    PtrListQuickSortedByPtrVal<TimerClient> masters;
  
};

#endif
