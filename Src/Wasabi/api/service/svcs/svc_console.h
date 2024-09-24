#ifndef _SVC_CONSOLE_H
#define _SVC_CONSOLE_H

#include <bfc/dispatch.h>

class NOVTABLE svc_console : public Dispatchable 
{
public:
  int activated();
  int outputString(int severity, const char *string);

  enum {
    ACTIVATED=10,
    OUTPUTSTRING=20,
  };
};

inline int svc_console::activated() {
  return _call(ACTIVATED, 0);
}

inline int svc_console::outputString(int severity, const char *string) {
  return _call(OUTPUTSTRING, 0, severity, string);
}

// derive from this one
class svc_consoleI : public svc_console {
public:
  virtual int activated()=0;
  virtual int outputString(int severity, const char *string)=0;

protected:
  RECVS_DISPATCH;
};

#endif
