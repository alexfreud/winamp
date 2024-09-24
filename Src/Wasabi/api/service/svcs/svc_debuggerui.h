#ifndef _SVC_DEBUGGERUI_H
#define _SVC_DEBUGGERUI_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class DebuggerUI;

// {8B055A0D-9A57-428c-BCFC-88F75AEF2CAD}
static const GUID SERVICE_DEBUGGERUI = 
{ 0x8b055a0d, 0x9a57, 0x428c, { 0xbc, 0xfc, 0x88, 0xf7, 0x5a, 0xef, 0x2c, 0xad } };

class svc_debuggerUI : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }

  DebuggerUI *createUI();
  void destroyUI(DebuggerUI *ui);

protected:
  enum {
    CREATEUI=10,
    DESTROYUI=20,
  };
};

inline DebuggerUI *svc_debuggerUI::createUI() {
  return _call(CREATEUI, (DebuggerUI *)NULL);
}

inline void svc_debuggerUI::destroyUI(DebuggerUI *ui) {
  _voidcall(DESTROYUI, ui);
}

class svc_debuggerUII : public svc_debuggerUI {
public:
  virtual DebuggerUI *createUI()=0;
  virtual void destroyUI(DebuggerUI *ui)=0;

protected:
  RECVS_DISPATCH;
};

#endif
