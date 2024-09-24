#ifndef __MAKIDEBUGAPI_H
#define __MAKIDEBUGAPI_H

#include <api/script/debugger/api_makidebug.h>
#include <api/script/debugger/vcpudebug.h>

class MakiDebuggerApi : public api_makiDebuggerI {
  public:
    MakiDebuggerApi();
    virtual ~MakiDebuggerApi();

    virtual void debugger_trace();
    virtual int debugger_isActive();
    virtual int debugger_getVIP();
    virtual int debugger_getVSD();
    virtual int debugger_getVCC();
    virtual int debugger_getVSP();
    virtual int debugger_filterEvent(int vcpuid, int eventid);
    virtual void debugger_eventComplete();
    virtual MakiJITD *debugger_createJITD(int vcpuid, int bringitup=1);
    virtual scriptVar debugger_readStack(int n);
    virtual const char *debugger_getCodeBlock(int vcpuid);

  private:
    VCPUDebugger debugger;
};

#endif
