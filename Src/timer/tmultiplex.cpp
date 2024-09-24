#include "tmultiplex.h"
#include <api/timer/timerclient.h>
#include <assert.h>
VirtualTimer::VirtualTimer(TimerClient *_client, intptr_t _id, api_dependent *depend) :
  client(_client), id(_id), dep(depend)
{
  name = client->timerclient_getName();
  mclient = client->timerclient_getMasterClient();
}

MainTimerMultiplexer::MainTimerMultiplexer() {
  setClient(this);
}

MainTimerMultiplexer::~MainTimerMultiplexer() {
/*
  foreach(timerclients)
    VirtualTimer *vt = timerclients.getfor();
    //DebugString("TIMER MULTIPLEXER WARNING: TimerClient %X (%s) was not deregistered\n", vt->client, vt->name.getValue());
  endfor;  
*/
// NOTE: if you get a crash here, someone probably had a timer event outstanding
// or didn't call down in timerclient_timerCallback()

// Also this is guaranteed to happen if one of your timerclient objects (ie: a wnd) was not deleted
// eventho your DLL has been unloaded. the watched pointer will remain watched instead of unregistering
// itself from its viewers. DependentViewerI (one of our direct ancestors) will try to dereference that pointer in
// order to signal it that a viewer was detached, and it will crash.

  timerclients.deleteAll();
}

void MainTimerMultiplexer::add(TimerClient *client, intptr_t id, int ms) {
  remove(client, id);
  api_dependent *d = client->timerclient_getDependencyPtr();
  assert(d != NULL);
  VirtualTimer *t = new VirtualTimer(client, id, d);
  timerclients.addItem(t);
  viewer_addViewItem(d);
  if (t->mclient) {
    d = t->mclient->timerclient_getDependencyPtr();
    ASSERT(d != NULL);
    viewer_addViewItem(d);
  }
  addTimer(ms, static_cast<void *>(t));
}

void MainTimerMultiplexer::remove(TimerClient *client, intptr_t id) {
  while (masters.haveItem(client)) masters.removeItem(client);
  for (int i=0;i<timerclients.getNumItems();i++) {
    VirtualTimer *t = timerclients.enumItem(i);
    masters.removeItem(t->mclient);//BU store mclient on VirtualTimer now
    if (t->client == client && (t->id == id || id == -1)) {
      viewer_delViewItem(t->dep);
      timerclients.removeByPos(i);
      removeTimer(static_cast<void *>(t));
      delete t;
      i--;
    }
  }
}

int MainTimerMultiplexer::isValidTimerClientPtr(TimerClient *tc, api_dependent *dep) {
//   try {
  __try {
      api_dependent *d = tc->timerclient_getDependencyPtr();
      if (d != dep) return 0;
   //} catch (...) {
  } __except (1) {
     return 0;
   } 
   return 1;
}

void MainTimerMultiplexer::onMultiplexedTimer(void *data, int skip, int mssincelasttimer) {
  assert(data != NULL);
  VirtualTimer *t = static_cast<VirtualTimer *>(data);
  if (!isValidTimerClientPtr(t->client, t->dep)) {
    //DebugString("TIMER MULTIPLEXER WARNING: TimerClient %X (%s) is no longer valid! (%d)\n", t->client, t->name.getValue(), t->id);
    remove(t->client, -1);
	t->client = 0;
  } else {
    TimerClient *mc = t->client->timerclient_getMasterClient();
    if (mc) masters.addItem(mc);
    t->client->timerclient_setSkipped(skip);
    t->client->timerclient_setTimerDelay(mssincelasttimer);
    t->client->timerclient_timerCallback(t->id); 
    // -----------------------------------------------------------------------
    // WARNING
    // 
    // below this line, you can no longer assume that t is pointing at valid
    // memory, because timerCallback can eventually call remove
    // -----------------------------------------------------------------------
  } 
}

void MainTimerMultiplexer::onServerTimer() {
  TimerMultiplexer::onServerTimer();
  TimerClient *last = NULL;
  for (int i=0;i<masters.getNumItems();i++) {
    TimerClient *t = masters.enumItem(i);
    if (t == last) continue;
    t->timerclient_onMasterClientMultiplex();
    last = t;
  }
  masters.removeAll();
}

int MainTimerMultiplexer::haveClient(TimerClient *client) {
  for (int i=0;i<timerclients.getNumItems();i++) 
	{
		VirtualTimer *vt = timerclients.enumItem(i);
		if (vt && vt->client == client)
			return 1;
	}
  return 0;
}

int MainTimerMultiplexer::viewer_onItemDeleted(api_dependent *item) {
  for (int i=0;i<timerclients.getNumItems();i++) {
    VirtualTimer *vt = timerclients.enumItem(i);
    if (vt->dep == item) {
      remove(vt->client, -1);
      return 1;
    }
  }
  return 1;
}
