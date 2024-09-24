#ifndef __DEBUGGERUI_H
#define __DEBUGGERUI_H

#include <bfc/dispatch.h>

class MakiJITD;

/*-------------------------------------------
DebuggerUI 
  int messageLoop();
  void setJITD(MakiJITD *jitd);
-------------------------------------------*/

class DebuggerUI : public Dispatchable {
  public:
    int messageLoop();
    void setJITD(MakiJITD *jitd);

  enum {
    DEBUGGERUI_MESSAGELOOP = 0,
    DEBUGGERUI_SETJITD = 10,
  };
};

inline int DebuggerUI::messageLoop() {
  return _call(DEBUGGERUI_MESSAGELOOP, (int)0);
}

inline void DebuggerUI::setJITD(MakiJITD *jitd) {
  _voidcall(DEBUGGERUI_SETJITD, jitd);
}

class DebuggerUII : public DebuggerUI {
  public:
    virtual int messageLoop()=0;
    virtual void setJITD(MakiJITD *jitd)=0;

  protected:
    RECVS_DISPATCH;
};

#endif