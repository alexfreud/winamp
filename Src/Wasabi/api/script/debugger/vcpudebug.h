#ifndef __VCPUDEBUG_H
#define __VCPUDEBUG_H

#include <bfc/ptrlist.h>
#include <bfc/stack.h>
#include <bfc/reentryfilter.h>

class MakiJITD;

class VCPUDebugger {
  public:
    VCPUDebugger();
    virtual ~VCPUDebugger();

    void trace();

    int getVIP(); // instruction pointer
    int getVSD(); // script descriptor (id)
    int getVSP(); // variables stack pointer
    int getVCC(); // call stack pointer

    int filterEvent(int vcpuid, int eventid); // if this returns 1, you should return immediatly and not call eventComplete!
    void eventComplete();

    int isActive();

    MakiJITD *createJITD(int vcpuid);
    MakiJITD *getJITD(int vcpuid);
    const char *getCodeBlock(int vcpuid);

  private:

    PtrList<MakiJITD> jitds;
    ReentryFilterObject reentryfilter;
    ReentryFilter filter;
    Stack<int> scopestack;
};

#endif
