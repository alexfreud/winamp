#include <bfc/platform/platform.h>
#include "timermul.h"

#include <api.h>
#include <api/config/items/attribs.h>
#include <api/config/items/cfgitem.h>

				// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
				const GUID uioptions_guid =
				{ 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };

PtrListQuickSorted<MultiplexerServer, MultiplexerServerComparatorTID> servers_tid;
PtrListQuickSorted<MultiplexerServer, MultiplexerServerComparatorTID> servers_mux;

TimerMultiplexer::TimerMultiplexer() {
  timerset = 0;
  nslices = 0;
  resolution = -1;
  check_resolution = true;
  client = NULL;
  curslice = 0;
  running_timer = NULL;
  uioptions = NULL;
  justexited = 0;
  firstevent = 1;
  resetTimer(50); // initial, is changed for config value on first event
}

TimerMultiplexer::~TimerMultiplexer() {
  doShutdown();
}

void TimerMultiplexer::setClient(TimerMultiplexerClient *_client) {
  client = _client;
}

void TimerMultiplexer::addTimer(int ms, void *data) {
  //if (ms < 0) { DebugString("Timer with negative delay set, ignored coz the time machine service isn't ready yet\n"); }
  MultiplexedTimer *t = new MultiplexedTimer(ms, data);
  if (ms >= MAX_TIMER_DELAY) {
    lptimers.addItem(t);
    t->nexttick = Wasabi::Std::getTickCount() + t->ms;
  } else {
    timers.addItem(t);
    if (nslices > 0)
      distribute(t);
  }
}

void TimerMultiplexer::removeTimer(void *data) {

  if (running_timer && running_timer->data == data)
    running_timer = NULL;

  int i;
  for (i=0;i<timers.getNumItems();i++) {
    MultiplexedTimer *t = timers.enumItem(i);
    if (t->data == data) {
      removeFromWheel(t);
      timers.removeByPos(i);
      delete t;
      return;
    }
  }
  for (i=0;i<lptimers.getNumItems();i++) {
    MultiplexedTimer *t = lptimers.enumItem(i);
    if (t->data == data) {
      removeFromLowPrecision(t);
      delete t;
      return;
    }
  }
}

void TimerMultiplexer::setResolution(int ms) {
  resolution = ms;
}

void TimerMultiplexer::shutdown() {
  doShutdown();
}

void TimerMultiplexer::doShutdown() {
  timers.deleteAll();
  wheel.deleteAll();
  lptimers.deleteAll();
  if (timerset) {
    MultiplexerServer *s = servers_mux.findItem((const wchar_t *)this);
    if (s) {
#ifdef WIN32
      KillTimer(NULL, s->getId());
#elif defined(LINUX)

#endif
    }
    timerset = 0;
  }
}

void TimerMultiplexer::checkResolution(DWORD now) {
	if (check_resolution == true)
	{
		if (WASABI_API_CONFIG)
		{
			if (uioptions == NULL)  
			{
				uioptions = WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
				if (uioptions)
				{
					ifc_dependent *ui_change = uioptions->getDependencyPtr();
					ui_change->dependent_regViewer(this, 1);
				}
			}
			check_resolution = uioptions?false:true;
			int nresolution = uioptions ? _intVal(uioptions, L"Multiplexed timers resolution") : DEF_RES;

			nresolution = MAX(10, MIN(MAX_TIMER_DELAY/LOW_RES_DIV, nresolution));
			if (nresolution != resolution) {
				resetTimer(nresolution);
				resolution = nresolution;
				resetWheel();
			}
		}
	}
}

VOID CALLBACK timerMultiplexerServerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
  MultiplexerServer *s = servers_tid.findItem((const wchar_t *)&idEvent);
  if (s) s->getMultiplexer()->onServerTimer();
}

void TimerMultiplexer::resetTimer(int newresolution) {
  if (timerset) {
    MultiplexerServer *s = servers_mux.findItem((const wchar_t *)this);
    if (s)
      KillTimer(NULL, s->getId());
  }

  // linux port implements settimer
  UINT_PTR id = SetTimer(NULL, 0, newresolution, timerMultiplexerServerProc);
  MultiplexerServer *s = servers_mux.findItem((const wchar_t *)this);
  if (!s) {
    s = new MultiplexerServer(this, (UINT)id);
    servers_mux.addItem(s);
    servers_tid.addItem(s);
  } else {
    s->setId(id);
    servers_tid.sort();
  }
  timerset = 1;
}

PtrList<MultiplexedTimer> *TimerMultiplexer::getSlice(int n) {
  ASSERT(nslices > 0);
  return wheel.enumItem(n % nslices);
}

void TimerMultiplexer::resetWheel() {

  wheel.deleteAll();

  nslices = MAX_TIMER_DELAY / resolution;

  for (int i=0;i<nslices;i++)
    wheel.addItem(new PtrList< MultiplexedTimer >);

  curslice = 0;
  distributeAll();
}

void TimerMultiplexer::distributeAll() {
  for (int i=0;i<timers.getNumItems();i++) {
    distribute(timers.enumItem(i));
  }
}

void TimerMultiplexer::distribute(MultiplexedTimer *t) {
  ASSERT(t != NULL);

  int delay = t->ms;

  int slice = delay / resolution + curslice;
  PtrList<MultiplexedTimer> *l = getSlice(slice);

  ASSERT(l != NULL);

  l->addItem(t);
}

void TimerMultiplexer::onServerTimer() {

  justexited = 0;

  DWORD now = Wasabi::Std::getTickCount();

  checkResolution(now);

  runCurSlice(now);

  if ((curslice % (nslices/LOW_RES_DIV)) == 0) { // execute low precision timers every MAX_TIMER_DELAY/LOW_RES_DIV
    runLowPrecisionTimers(now);
  }

  if (!justexited) {
    curslice++;
    curslice %= nslices;
  }

  justexited = 1;

  if (firstevent) {
    firstevent = 0;
    checkResolution(Wasabi::Std::getTickCount());
  }
}

void TimerMultiplexer::runCurSlice(DWORD now) {
  //DebugString("Running slice %d\n", curslice);
  PtrList<MultiplexedTimer> *slice = getSlice(curslice);
  ASSERT(slice != NULL);

  // mark them clean
  int i;
  for (i=0;i<slice->getNumItems();i++)
    slice->enumItem(i)->flag = 0;

  // run events
  int n;
  do {
    n = 0;
    for (i=0;i<slice->getNumItems();i++) {
      MultiplexedTimer *t = slice->enumItem(i);
      if (t == NULL) break; // do not remove this line even if you think it's useless
      // t might have been removed by a previous runTimer in this slice, so see if it's still here and if not, ignore
      if (!timers.haveItem(t)) { slice->removeItem(t); i--; continue; }
      if (t->flag == 1) continue;
      t->flag = 1;
      int lastdelay = MAX(0, (int)(now - t->lastmscount));
      DWORD last = t->lastmscount;
      if (last == 0) last = now;
      t->lastmscount = now;
      t->lastdelay = lastdelay;
      running_timer = t;
      runTimer(now, last, t, slice, i);
// -----------------------------------------------------------------------
// WARNING
//
// below this line, you can no longer assume that t is pointing at valid
// memory, because runTimer can eventually call removeTimer
// -----------------------------------------------------------------------
      n++;
    }
  } while (n > 0);
}

void TimerMultiplexer::runTimer(DWORD now, DWORD last, MultiplexedTimer *t, PtrList<MultiplexedTimer> *slice, int pos) {

  int nextslice = curslice + t->ms / resolution;
  int spent = now - last;
  int lost = spent - t->ms;

  if (lost > 0) {
    t->lost += (float)lost / (float)t->ms;
  }

  PtrList<MultiplexedTimer> *next = getSlice(nextslice);
  ASSERT(next != NULL);

  if (slice == next) {
    nextslice++;
    next = getSlice(nextslice);
  }

  slice->removeByPos(pos);
  next->addItem(t);

  int skip = (int)t->lost;
  t->lost -= (int)t->lost;
  if (client) {
    client->onMultiplexedTimer(t->data, skip, t->lastdelay);
// -----------------------------------------------------------------------
// WARNING
//
// below this line, you can no longer assume that t is pointing at valid
// memory, because onMultiplexedTimer can eventually call removeTimer
// -----------------------------------------------------------------------
  }
}

void TimerMultiplexer::removeFromWheel(MultiplexedTimer *t) {
  for (int i=0;i<nslices;i++) {
    PtrList<MultiplexedTimer> *slice = getSlice(i);
    for (int j=0;j<slice->getNumItems();j++) {
      if (slice->enumItem(j) == t) {
        slice->removeByPos(j);
        j--;
      }
    }
  }
}

void TimerMultiplexer::removeFromLowPrecision(MultiplexedTimer *t) {
  for (int i=0;i<lptimers.getNumItems();i++) {
    if (lptimers.enumItem(i) == t) {
      lptimers.removeByPos(i);
      i--;
    }
  }
}

void TimerMultiplexer::runLowPrecisionTimers(DWORD now) {
  int restart;
  do {
    restart = 0;
    for (int i=0;i<lptimers.getNumItems();i++) {
      MultiplexedTimer *t = lptimers.enumItem(i);
      if (t->nexttick < now) {
        if (client) {
          running_timer = t;
          t->lost += (now - t->nexttick) / t->ms;
          int skip = (int)t->lost;
          t->lost -= skip; // remove integer part
          DWORD last = t->lastmscount;
          t->lastdelay = now-last;
          t->lastmscount = now;
          t->nexttick = t->nexttick+(t->ms)*(skip+1);
          client->onMultiplexedTimer(t->data, skip, t->lastdelay);
// -----------------------------------------------------------------------
// WARNING
//
// below this line, you can no longer assume that t is pointing at valid
// memory, because onMultiplexedTimer can eventually call removeTimer
// -----------------------------------------------------------------------
        }
        if (running_timer == NULL) { // onMultiplexedTimer called removeTimer
          restart =1;
          break;
        }
      }
    }
  } while (restart);
}


int TimerMultiplexer::getNumTimers() {
  return timers.getNumItems();
}

int TimerMultiplexer::getNumTimersLP() {
  return lptimers.getNumItems();
}

int TimerMultiplexer::dependentViewer_callback(ifc_dependent *item, const GUID *classguid, int cb, intptr_t param1, intptr_t param2 , void *ptr, size_t ptrlen)
{
	if (param1 == CfgItem::Event_ATTRIBUTE_CHANGED)
	{
		check_resolution=true;
	}
	else if (param1 == CfgItem::Event_ATTRIBUTE_REMOVED)
	{
		uioptions=0;
	}
	return 1;
}

#define CBCLASS TimerMultiplexer
START_DISPATCH;
CB(DEPENDENTVIEWER_CALLBACK, dependentViewer_callback)
END_DISPATCH;
#undef CBCLASS
