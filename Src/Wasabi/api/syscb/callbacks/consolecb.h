#ifndef _CONSOLECB_H
#define _CONSOLECB_H

#include <api/syscb/callbacks/syscbi.h>

namespace ConsoleCallback {
  enum {
    DEBUGMESSAGE=10,
  };
};
/*
#define CONSOLECALLBACKI_PARENT SysCallbackI

class ConsoleCallbackI : public CONSOLECALLBACKI_PARENT {
public:
  virtual FOURCC syscb_getEventType() { return SysCallback::CONSOLE; }

protected:
  virtual int consolecb_outputString(int severity, const char *string)=0;
private:
  virtual int syscb_notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0);
};
*/
#endif
