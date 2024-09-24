#include <precomp.h>
#include <wasabicfg.h>
#include "vcpudebug.h"
#include <api/script/debugger/jitd.h>

VCPUDebugger::VCPUDebugger() {
  filter.setFilterObject(&reentryfilter);
}

VCPUDebugger::~VCPUDebugger() {
}

// ------------------------------------------------------------------------

// instruction pointer
int VCPUDebugger::getVIP() {
  return WASABI_API_MAKIDEBUG->debugger_getVIP();
}

// script descriptor (vcpuid)
int VCPUDebugger::getVSD() {
  return WASABI_API_MAKIDEBUG->debugger_getVSD();
}

// variables stack pointer
int VCPUDebugger::getVSP() {
  return WASABI_API_MAKIDEBUG->debugger_getVSP();
}

// call stack pointer
int VCPUDebugger::getVCC() {
  return WASABI_API_MAKIDEBUG->debugger_getVCC();
}

// ------------------------------------------------------------------------
void VCPUDebugger::trace() {
  int i;
  for (i=0;i<jitds.getNumItems();i++) {
    MakiJITD *jitd = jitds.enumItem(i);
    if (jitd->getVCPUId() == getVSD()) {
      jitd->trace();
    }
  }
}

MakiJITD *VCPUDebugger::getJITD(int vcpuid) {
  int i;
  for (i=0;i<jitds.getNumItems();i++) {
    MakiJITD *jitd = jitds.enumItem(i);
    if (jitd->getVCPUId() == vcpuid)
      return jitd;
  }
  return NULL;
}

// if this returns 1, you should not call eventComplete!
int VCPUDebugger::filterEvent(int vcpuid, int eventid) {
  MakiJITD *jitd = getJITD(vcpuid);
  if (!jitd || !jitd->isOnHold()) {
    WASABI_API_WND->pushModalWnd();
    scopestack.push(0);
    return 0;
  }
  filter.enterScope((vcpuid<<16) + eventid); // (vcpuid<<16) + eventid
  if (filter.mustLeave()) {
    filter.leaveScope();
    return 1; 
  }
  WASABI_API_WND->pushModalWnd();
  scopestack.push(1);
  return 0;
}

void VCPUDebugger::eventComplete() {
  int n;
  scopestack.pop(&n);
  WASABI_API_WND->popModalWnd();
  if (n) {
    filter.leaveScope();
  }
}

int VCPUDebugger::isActive() {
  foreach(jitds)
    if (jitds.getfor()->isActive())
      return 1;
  endfor;
  return 0;
}

MakiJITD *VCPUDebugger::createJITD(int vcpuid) {
  MakiJITD *jitd = new MakiJITD(this, vcpuid);  
  jitds.addItem(jitd);
  return jitd;
}

const char *VCPUDebugger::getCodeBlock(int vcpuid) {
  return WASABI_API_MAKIDEBUG->debugger_getCodeBlock(vcpuid);
}
