#ifndef __TIMER_SVC_H
#define __TIMER_SVC_H

#include <api/timer/api_timer.h>
#include "tmultiplex.h"

class TimerApi : public timer_api
{
public:
	TimerApi();
	~TimerApi();
	virtual TimerToken timer_add(TimerClient *client, intptr_t id, int ms);
	virtual void timer_remove(TimerClient *client, TimerToken token = -1);

protected:
	MainTimerMultiplexer multiplex;
	RECVS_DISPATCH;
};


#endif
