#ifndef _GUISTATUSCB_H
#define _GUISTATUSCB_H

#include <bfc/dispatch.h>

class AppCmds;
class ifc_dependent;

class GuiStatusCallback : public Dispatchable {
  public:
    void onSetStatusText(const wchar_t *text, int overlay);
    void onAddAppCmds(AppCmds *commands);
    void onRemoveAppCmds(AppCmds *commands);
    void pushCompleted(int max=100);
    void incCompleted(int add=1);
    void setCompleted(int pos);
    void popCompleted();

    api_dependent *status_getDependencyPtr();
  enum {
    STATUS_ONSETTEXT   = 101,
    STATUS_GETDEP      = 200,
    STATUS_ADDCTXTCMDS = 300,
    STATUS_REMCTXTCMDS = 400,
    STATUS_PUSHCOMPLETED = 500,
    STATUS_INCCOMPLETED = 600,
    STATUS_SETCOMPLETED = 700,
    STATUS_POPCOMPLETED = 800,
  };
};

inline void GuiStatusCallback ::onSetStatusText(const wchar_t *text, int overlay) {
  _voidcall(STATUS_ONSETTEXT, text, overlay);
}

inline api_dependent *GuiStatusCallback ::status_getDependencyPtr() {
  return  _call(STATUS_GETDEP, (api_dependent *)NULL);
}

inline void GuiStatusCallback ::onAddAppCmds(AppCmds *commands) {
  _voidcall(STATUS_ADDCTXTCMDS, commands);
}

inline void GuiStatusCallback ::onRemoveAppCmds(AppCmds *commands) {
  _voidcall(STATUS_REMCTXTCMDS, commands);
}

inline
void GuiStatusCallback::pushCompleted(int max) {
  _voidcall(STATUS_PUSHCOMPLETED, max);
}

inline
void GuiStatusCallback::incCompleted(int add) {
  _voidcall(STATUS_INCCOMPLETED, add);
}

inline
void GuiStatusCallback::setCompleted(int pos) {
  _voidcall(STATUS_SETCOMPLETED, pos);
}

inline
void GuiStatusCallback::popCompleted() {
  _voidcall(STATUS_POPCOMPLETED);
}

class GuiStatusCallbackI : public GuiStatusCallback {
  public:
    virtual void onSetStatusText(const wchar_t *text, int overlay)=0;
    virtual api_dependent *status_getDependencyPtr()=0;
    virtual void onAddAppCmds(AppCmds *commands)=0;
    virtual void onRemoveAppCmds(AppCmds *commands)=0;
    virtual void pushCompleted(int max=100)=0;
    virtual void incCompleted(int add=1)=0;
    virtual void setCompleted(int pos)=0;
    virtual void popCompleted()=0;

  protected:
    RECVS_DISPATCH;
};

#endif
