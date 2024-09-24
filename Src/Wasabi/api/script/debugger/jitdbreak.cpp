#include "precomp.h"
#include "jitdbreak.h"

JITDBreakpoint::JITDBreakpoint(MakiJITD *_jitd, int _pointer) {
  jitd = _jitd;
  pointer = _pointer;
  enabled = 1;
}

JITDBreakpoint::~JITDBreakpoint() {
}

int JITDBreakpoint::getPointer() {
  return pointer;
}

void JITDBreakpoint::setEnabled(int e) {
  enabled = e;
}

int JITDBreakpoint::isEnabled() {
  return enabled;
}

void JITDBreakpoint::setPointer(int _pointer) {
  pointer = _pointer;
}