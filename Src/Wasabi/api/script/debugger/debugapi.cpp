#include <precomp.h>
#include <api/script/debugger/debugapi.h>
#include <api/script/debugger/jitd.h>
#include <api/script/vcpu.h>

api_makiDebugger *debugApi = NULL;

MakiDebuggerApi::MakiDebuggerApi() 
{
}

MakiDebuggerApi::~MakiDebuggerApi() {

}

void MakiDebuggerApi::debugger_trace() {
  debugger.trace();
}

int MakiDebuggerApi::debugger_isActive() {
  return debugger.isActive();
}

int MakiDebuggerApi::debugger_getVIP() {
  return VCPU::VIP;
}

int MakiDebuggerApi::debugger_getVSD() {
  return VCPU::VSD;
}

int MakiDebuggerApi::debugger_getVCC() {
  return VCPU::VCC;
}

int MakiDebuggerApi::debugger_getVSP() {
  return VCPU::VSP;
}

int MakiDebuggerApi::debugger_filterEvent(int vcpuid, int eventid) {
  return debugger.filterEvent(vcpuid, eventid);
}

void MakiDebuggerApi::debugger_eventComplete() {
  debugger.eventComplete();
}

MakiJITD *MakiDebuggerApi::debugger_createJITD(int vcpuid, int bringitup) {
  MakiJITD *jitd = debugger.createJITD(vcpuid);
  if (bringitup)
    jitd->setGlobalBreakpoint(1);
  return jitd;
}

scriptVar MakiDebuggerApi::debugger_readStack(int n) {
  VCPUscriptVar v = VCPU::peekAt(n);
  return v.v;
}

const char *MakiDebuggerApi::debugger_getCodeBlock(int vcpuid) {
  return VCPU::getCodeBlock(vcpuid);
}
