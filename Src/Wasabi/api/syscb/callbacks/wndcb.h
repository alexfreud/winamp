#ifndef _WNDCB_H
#define _WNDCB_H

#include <api/syscb/callbacks/syscbi.h>
#include <bfc/common.h>

class Container;
class ifc_window;

class WndInfo {
  public:
  GUID guid;
  const wchar_t *groupid;
  const wchar_t *wndtype;
  Container *c;
};

namespace WndCallback {
  enum {
    SHOWWINDOW=10,
    HIDEWINDOW=20,
    GROUPCHANGE=30,
    TYPECHANGE=40,
  };
};

#define WNDCALLBACKI_PARENT SysCallbackI
class WndCallbackI : public WNDCALLBACKI_PARENT {
public:
  virtual FOURCC syscb_getEventType() { return SysCallback::WINDOW; }

protected:
  virtual int onShowWindow(Container *c, GUID guid, const wchar_t *groupid) { return 0; }
  virtual int onHideWindow(Container *c, GUID guid, const wchar_t *groupid) { return 0; }
  virtual int onGroupChange(const wchar_t *groupid) { return 0; }
  virtual int onTypeChange(const wchar_t *type) { return 0; }

private:
  virtual int syscb_notify(int msg, intptr_t param1=0, intptr_t param2=0);

};

#endif
