#ifndef __MAKIJITD_H
#define __MAKIJITD_H

#include <bfc/ptrlist.h>

class JITDBreakpoint;
class VCPUDebugger;
class svc_debuggerUI;
class SimpleDebuggerUI;
class DebuggerUI;
class DebugSymbols;
class SourceCodeLine;

enum {
  JITD_RETURN_STEPINTO  = 0,
  JITD_RETURN_STEPOVER  = 1,
  JITD_RETURN_STEPOUT   = 2,
  JITD_RETURN_TERMINATE = 3,
  JITD_RETURN_CONTINUE  = 4,
};

class MakiJITD { // dispatch me!
  public:
    MakiJITD(VCPUDebugger *debugger, int vcpuid);
    virtual ~MakiJITD();

    int getNumBreakpoints();
    JITDBreakpoint *enumBreakpoint(int n);
    JITDBreakpoint *findBreakpoint(int pointer);
    void addBreakpoint(int pointer);
    void removeBreakpoint(JITDBreakpoint *breakpoint);
    void clearAllBreakpoints();
    int isGlobalBreakpointSet();
    void setGlobalBreakpoint(int s);
    void setSysBreakpoint(int pointer); // deactivates automatically next time the debugger is activated
    int getVCPUId();
    void trace();
    int isActive();
    int isOnHold(); // 1 if running in the jitd's message loop

    int getVIP();
    int getVSD();
    int getVCC();
    int getVSP();

    const char *getCodeBlock();

    virtual int findLine(int pointer);
    virtual int getNumLines();
    virtual SourceCodeLine *enumLine(int n);

  private:
    void createUi();
    void enterUi();

    int vcpuid;
    int vip, vsd, vcc, vsp;
    int globalbreakpoint;

    PtrList<JITDBreakpoint> breakpoints;
    VCPUDebugger *debugger;
    svc_debuggerUI *uisvc;
    DebuggerUI *ui;
    SimpleDebuggerUI *simpleui;
    int onhold;
    const char *codeblock;
    JITDBreakpoint *sysbp;
    DebugSymbols *symbols;
};

#endif