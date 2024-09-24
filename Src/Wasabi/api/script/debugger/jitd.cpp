#include <precomp.h>
#include "jitd.h"
#include "jitdbreak.h"
#include <api/service/svcs/svc_debuggerui.h>
#include <api/service/svc_enum.h>
#include <api/script/debugger/vcpudebug.h>
#include <api/script/debugger/debuggerui.h>
#include <api/script/debugger/sdebuggerui.h>
#include <api/script/debugger/debugsymbols.h>

MakiJITD::MakiJITD(VCPUDebugger *_debugger, int _vcpuid) {
  vip = vsp = vsd = vcc = -1;
  globalbreakpoint = 1; // fucko!!
  onhold = 0;
  debugger = _debugger;
  vcpuid = _vcpuid;
  uisvc = NULL;
  ui = NULL;
  simpleui = NULL;
  codeblock = _debugger->getCodeBlock(vcpuid);
  sysbp = new JITDBreakpoint(this, 0);
  breakpoints.addItem(sysbp);
  sysbp->setEnabled(0);
  symbols = new DebugSymbols(_vcpuid);
}

MakiJITD::~MakiJITD() {
  breakpoints.deleteAll();
  if (uisvc != NULL) {
    uisvc->destroyUI(ui);
    WASABI_API_SVC->service_release(uisvc);
  }
  delete simpleui;
  delete symbols;
}

int MakiJITD::getVIP() {
  return vip;
}

int MakiJITD::getVCC() {
  return vcc;
}

int MakiJITD::getVSD() {
  return vsd;
}

int MakiJITD::getVSP() {
  return vsp;
}

int MakiJITD::isGlobalBreakpointSet() {
  return globalbreakpoint;
}

void MakiJITD::setGlobalBreakpoint(int s) {
  globalbreakpoint = s;
}
 
int MakiJITD::getNumBreakpoints() {
  return breakpoints.getNumItems();
}

JITDBreakpoint *MakiJITD::enumBreakpoint(int n) {
  return breakpoints.enumItem(n);
}

JITDBreakpoint *MakiJITD::findBreakpoint(int pointer) {
  int i;
  for (i=0;i<breakpoints.getNumItems();i++) {
    JITDBreakpoint *bp = breakpoints.enumItem(i);
    if (bp->isEnabled() && bp->getPointer() == pointer)
      return bp;
  }
  return NULL;
}

void MakiJITD::addBreakpoint(int pointer) {
  breakpoints.addItem(new JITDBreakpoint(this, pointer));
}

void MakiJITD::removeBreakpoint(JITDBreakpoint *breakpoint) {
  breakpoints.removeItem(breakpoint);
  delete breakpoint;
}

void MakiJITD::clearAllBreakpoints() {
  breakpoints.deleteAll();
}

int MakiJITD::getVCPUId() {
  return vcpuid;
}

void MakiJITD::trace() {
  vip = debugger->getVIP();
  vsp = debugger->getVSP();
  vsd = debugger->getVSD();
  vcc = debugger->getVCC();

  if (globalbreakpoint || findBreakpoint(vip)) {
    enterUi();
  }
}

int MakiJITD::isActive() {
  return 1;
}

void MakiJITD::enterUi() {
  createUi();
  sysbp->setEnabled(0);
  globalbreakpoint = 0;
  onhold = 1;
  int next_command = ui->messageLoop();
  onhold = 0;
  switch (next_command) {
    case JITD_RETURN_STEPINTO:
      globalbreakpoint = 1;
      break;
    case JITD_RETURN_STEPOUT:
      // for now, continue
      break;
    case JITD_RETURN_TERMINATE:
      // for now, continue
      break;
    case JITD_RETURN_CONTINUE:
      // do nothing
    default:
      break;
  }
}

void MakiJITD::createUi() {
  if (ui != NULL) return;
  waServiceFactory *f = WASABI_API_SVC->service_getServiceByGuid(SERVICE_DEBUGGERUI);
  if (f != NULL) {
    uisvc = castService<svc_debuggerUI>(f);
    if (uisvc != NULL) {
      ui = uisvc->createUI();
      ui->setJITD(this);
    }
  } else {
    simpleui = new SimpleDebuggerUI();
    ui = simpleui;
    ui->setJITD(this);
  }
}

int MakiJITD::isOnHold() {
  return onhold;
}

const char *MakiJITD::getCodeBlock() {
  return codeblock;
}

void MakiJITD::setSysBreakpoint(int pointer) {
  sysbp->setPointer(pointer);
  sysbp->setEnabled(1);
}

int MakiJITD::getNumLines() {
  return symbols->getNumLines();
}

SourceCodeLine *MakiJITD::enumLine(int n) {
  return symbols->enumLine(n);
}

int MakiJITD::findLine(int pointer) {
  return symbols->findLine(pointer);
}


