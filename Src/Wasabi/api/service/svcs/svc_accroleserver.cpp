#include <precomp.h>

#include "svc_accroleserver.h"
#include <api/script/objects/guiobject.h>
#include <api/wnd/api_window.h>

#define CBCLASS svc_accRoleServerI
START_DISPATCH;
  CB(RS_HANDLEROLE,      handleRole);
  CB(RS_CREATEOBJECT,    createObject);
  VCB(RS_DESTROYOBJECT,  destroyObject);
END_DISPATCH;
#undef CBCLASS


#define CBCLASS roleServerObjectI
START_DISPATCH;
  CB(RSO_WNDPROC,        wndProc);
  CB(RSO_GETHWND,        gethWnd);
  CB(RSO_FLATTENCONTENT, flattenContent);
END_DISPATCH;
#undef CBCLASS


roleServerObjectI::roleServerObjectI(HWND par, api_window *w) {
  wnd = w;
  hwnd = NULL;
  parent = par;
  triedyet = 0;
}

roleServerObjectI::~roleServerObjectI() {
  if (hwnd != NULL)
    DestroyWindow(hwnd);
}

api_window *roleServerObjectI::getWnd() {
  return wnd;
}

HWND roleServerObjectI::gethWnd() {
  if (!triedyet) {
    triedyet = 1;
    hwnd = createWindow(parent);
    if (hwnd !=NULL)
      oldproc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
    else
      oldproc = NULL;
  }
  return hwnd;
}

ScriptObject *roleServerObjectI::getScriptObject() {
  if (wnd == NULL) return NULL;
  GuiObject *go = wnd->getGuiObject();
  if (go == NULL) return NULL;
  return go->guiobject_getScriptObject();
}

WNDPROC roleServerObjectI::getOldProc() {
  return oldproc;
}

int roleServerObjectI::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return CallWindowProc(oldproc, hWnd, uMsg, wParam, lParam);
}

int roleServerObjectI::flattenContent(HWND *w) {
  return FLATTENFLAG_ASKPARENT;
}
