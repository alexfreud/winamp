#include "api.h"
#include "timerapi.h"
#include "tmultiplex.h"

//timer_api *timerApi = NULL;

TimerApi::TimerApi() 
{
}

TimerApi::~TimerApi() 
{
  multiplex.shutdown();
}

TimerToken TimerApi::timer_add(TimerClient *client, intptr_t id, int ms) 
{
	multiplex.add(client, id, ms);
	return id;
}

void TimerApi::timer_remove(TimerClient *client, TimerToken id) 
{
  multiplex.remove(client, id);
}

#define CBCLASS TimerApi
START_DISPATCH;
  CB(TIMER_API_ADD, timer_add);
  VCB(TIMER_API_REMOVE, timer_remove);
END_DISPATCH;
#undef CBCLASS