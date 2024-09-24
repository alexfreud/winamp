#include <precomp.h>

#include "typesheet.h"
#include <api/wnd/wndclass/svcwndhold.h>
#include <api/wnd/wndclass/buttbar.h>
#include <api/service/svcs/svc_wndcreate.h>

TypeSheet::TypeSheet(const wchar_t *_windowtype) :
  TabSheet(ButtBar::STACK), windowtype(_windowtype)
{ }

int TypeSheet::onInit() {
  TYPESHEET_PARENT::onInit();
  load();
  return 1;
}

void TypeSheet::setWindowType(const wchar_t *wtype) {
  windowtype = wtype;
}

void TypeSheet::load() {
  if (windowtype == NULL || !*windowtype) return;
  WindowCreateByTypeEnum se(windowtype);
  svc_windowCreate *svc;
  while (svc = se.getNext()) {
    for (int i = 0; ; i++) {
      ServiceWndHolder *svcwnd = new ServiceWndHolder;
      ifc_window *wnd = svc->createWindowOfType(windowtype, svcwnd, i);
      if (wnd == NULL) {
        delete svcwnd;
        break;
      }
      svcwnd->setChild(wnd, svc);
      addChild(svcwnd);
    }
  }
}
