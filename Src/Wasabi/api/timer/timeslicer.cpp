#include "precomp.h"
#include "timeslicer.h"

#define TIMER_SLICE 0x7816

TimeSlicer::TimeSlicer(int percent_cpu_usage/* =50 */, int slice_duration/* =-1 */) {
  max_cpu_usage = MIN((float)percent_cpu_usage, 99.0f) / 100.0f;
  duration = slice_duration;
  started = 0;
  slicecount = 0;
  firstslicetime = -1;
}

TimeSlicer::~TimeSlicer() {
}

void TimeSlicer::startSlicer() {
  if (started) return;
  started = 1;
  timerclient_setTimer(TIMER_SLICE, duration);
  onSlicerStart();
  timerclient_timerCallback(TIMER_SLICE);
}

void TimeSlicer::stopSlicer() {
  if (!started) return;
  started = 0;
  timerclient_killTimer(TIMER_SLICE);
  onSlicerStop();
  firstslicetime = -1;
}

void TimeSlicer::onSlicerStop() {
}

void TimeSlicer::onSlicerStart() {
}

int TimeSlicer::isSlicerStarted() {
  return started;
}

void TimeSlicer::timerclient_timerCallback(int id) {
  if (id == TIMER_SLICE) {
    DWORD now = Std::getTickCount();
    runSlice(now, now + (int)((float)duration * max_cpu_usage));
  } else
    TimerClient::timerclient_timerCallback(id);
}

void TimeSlicer::runSlice(DWORD start, DWORD stopwhen) {
  DWORD now = Std::getTickCount();
  slicecount = 0;
  onBeginSliceBatch();
  if (slicecount == 0 && firstslicetime != -1) stopwhen = now + firstslicetime;
  while (Std::getTickCount() < stopwhen) {
    onSlice();
    slicecount++;
    if (!started) break;	// got aborted
  }
  onEndSliceBatch();
}
