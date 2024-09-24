#ifndef __TIMESLICER_H
#define __TIMESLICER_H

// TimeSlicer allows you to create a background job to perform while not blocking
// the main GUI thread. You give it the max percentage of CPU it should use, call star()
// and it'll start calling onSlice as many times as it can without using more cpu than requested
//
// To use this class, you need to break down your job into multiple tiny chunks that
// you perform in onSlice. Typical uses include adding files to or filtering entries from
// the database, driving state machines, etc.
// 
// onSlice will be called multiple times per timer.

#include "timerclient.h"

enum {
  GRANULARITY_EXTRALOW  =   20,
  GRANULARITY_LOW       =   50,
  GRANULARITY_MEDIUM    =  100,
  GRANULARITY_HIGH      =  250,
  GRANULARITY_EXTRAHIGH = 1000,
};

class TimeSlicer : public TimerClientI {
  public:
    
    TimeSlicer(int percent_cpu_usage=25, int slice_duration=GRANULARITY_LOW); 
    virtual ~TimeSlicer();

    virtual void timerclient_timerCallback(int id);

    void startSlicer();
    void stopSlicer();
    int isSlicerStarted();

    virtual void onSlicerStart();
    virtual void onSlicerStop();
    virtual void onBeginSliceBatch() {}
    virtual void onEndSliceBatch() {}
    api_dependent *timerclient_getDependencyPtr() { return timeslicer_getDependencyPtr(); }
    virtual api_dependent *timeslicer_getDependencyPtr()=0;
    virtual void setFirstSliceMinTime(int ms) { firstslicetime = ms; }
    virtual int getSliceCount() { return slicecount; }


    // override this to do your work
    virtual void onSlice() {  }

  private:

    virtual void runSlice(DWORD start, DWORD stopwhen);
    float max_cpu_usage;
    int duration;
    int started;
    int firstslicetime;
    int slicecount;
};

class TimeSlicerD : public TimeSlicer, public DependentI {
  public:
    virtual api_dependent *timeslicer_getDependencyPtr() { return this; }    
};

#endif