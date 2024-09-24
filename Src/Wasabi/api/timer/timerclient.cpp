#include "api.h"
#include <api/timer/api_timer.h>
#include <api/timer/timerclient.h>

#define CBCLASS TimerClientI
START_DISPATCH;
// this one doesn't map directly so that we can catch deferredcb timers
VCB(TIMERCLIENT_TIMERCALLBACK, timerclient_handleDeferredCallback);
CB(TIMERCLIENT_GETMASTERCLIENT, timerclient_getMasterClient);
VCB(TIMERCLIENT_ONMASTERMUX, timerclient_onMasterClientMultiplex);
CB(TIMERCLIENT_GETDEPPTR, timerclient_getDependencyPtr);
CB(TIMERCLIENT_GETSKIPPED, timerclient_getSkipped);
VCB(TIMERCLIENT_SETSKIPPED, timerclient_setSkipped);
VCB(TIMERCLIENT_POSTDEFERREDCB , timerclient_postDeferredCallback);
CB(TIMERCLIENT_ONDEFERREDCB , timerclient_onDeferredCallback);
CB(TIMERCLIENT_GETNAME, timerclient_getName);
END_DISPATCH;

TimerClientI::TimerClientI()
		: skipped(0), timerdelay(0), disallowset(0)
{ }

TimerClientI::~TimerClientI()
{
	disallowset = 1;
	if (cbs.getNumItems() > 0)
	{
		TimerToken token;
		if (tokens.reverseGetItem(DEFERREDCB_TIMERID, &token)) // TODO: it would be nice to have a combo get/del, so we don't have to search twice
		{
			WASABI_API_TIMER->timer_remove(this, token);
			tokens.delItem(token);
		}
	}
	cbs.deleteAll();
}

int TimerClientI::timerclient_setTimer(intptr_t id, int ms)
{
	if (disallowset) return 0;
	ASSERTPR(id > 0, "A timer id cannot be <= 0");
	ASSERTPR(id != DEFERREDCB_TIMERID, "please chose another timer id");
	TimerToken token = WASABI_API_TIMER->timer_add(this, id, ms);
	tokens.addItem(token, id);
	return 1;
}

int TimerClientI::timerclient_killTimer(intptr_t id)
{
	TimerToken token;
	if (tokens.reverseGetItem(id, &token)) // TODO: it would be nice to have a combo get/del, so we don't have to search twice
	{
		WASABI_API_TIMER->timer_remove(this, token);
		tokens.delItem(token);
	}
	return 1;
}

void TimerClientI::timerclient_postDeferredCallback(intptr_t param1, intptr_t param2, int mindelay)
{
	if (disallowset) return ;
	for (int i = 0;i < cbs.getNumItems();i++)
	{
		deferred_callback *cb = cbs.enumItem(i);
		if (cb->param1 == param1 && cb->param2 == param2)
		{
			cbs.removeByPos(i);
			break;
		}
	}
	deferred_callback *c = new deferred_callback;
	c->origin = this;
	c->param1 = param1;
	c->param2 = param2;
	cbs.addItem(c);
	TimerToken token = WASABI_API_TIMER->timer_add(this, DEFERREDCB_TIMERID, MAX(1, mindelay));
	tokens.addItem(token, DEFERREDCB_TIMERID);
}

void TimerClientI::timerclient_handleDeferredCallback(TimerToken token)
{
	// let deriving class handle it. note we call into here even if
	// it's the deferred cb id, since older versions did that too,
	// expecting the user to call down on the method.

#ifdef WIN64
	LPARAM id;
#else
	intptr_t id;
#endif
	if (tokens.getItem(token, &id))
	{
		timerclient_timerCallback((int)id);

		// process our deferred cb id
		if (id == DEFERREDCB_TIMERID)
		{
			WASABI_API_TIMER->timer_remove(this, token);
			PtrList<deferred_callback> temp(cbs);
			cbs.removeAll();
			foreach(temp)
			deferred_callback *c = temp.getfor();
			c->origin->timerclient_onDeferredCallback(c->param1, c->param2);
			delete c;
			endfor
		}
	}
}
