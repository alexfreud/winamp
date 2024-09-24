#ifndef _RUNLEVELCB_H
#define _RUNLEVELCB_H

#include <api/syscb/callbacks/syscbi.h>
#include <api/service/service.h>

#define RUNLEVELCALLBACKI_PARENT SysCallbackI
class RunlevelCallbackI : public RUNLEVELCALLBACKI_PARENT {
public:
  virtual FOURCC syscb_getEventType() { return SysCallback::RUNLEVEL; }

  // override these
  virtual void runlevelcb_onStartup() {}
  virtual void runlevelcb_onAppRunning() {}
  virtual void runlevelcb_onShutdown() {}
  virtual void runlevelcb_onBeforeShutdown() {}

private:
  virtual int syscb_notify(int msg, intptr_t param1=0, intptr_t param2=0) {
    switch (msg) {
      case SvcNotify::ONSTARTUP:
        runlevelcb_onStartup();
      break;
      case SvcNotify::ONAPPRUNNING:
        runlevelcb_onAppRunning();
      break;
      case SvcNotify::ONSHUTDOWN:
        runlevelcb_onShutdown();
      break;
      case SvcNotify::ONBEFORESHUTDOWN:
        runlevelcb_onBeforeShutdown();
        break;
      default: return 0;
    }
    return 1;
  }
};

#endif
