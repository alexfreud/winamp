#ifndef __SKINWND_H
#define __SKINWND_H

#include <bfc/common.h>
#include "api.h"
#include <api/script/scriptobj.h>

#define WASABISTDCONTAINER_RESIZABLE_STATUS      "resizable_status"
#define WASABISTDCONTAINER_RESIZABLE_NOSTATUS    "resizable_nostatus"
#define WASABISTDCONTAINER_STATIC                "static"
#define WASABISTDCONTAINER_MODAL                 "modal"

#define SKINWND_ATTACH_LEFT     1
#define SKINWND_ATTACH_TOP      2
#define SKINWND_ATTACH_RIGHT    3
#define SKINWND_ATTACH_BOTTOM   4

class ifc_window;
class GuiObject;

class SkinWnd 
{
  public:
    SkinWnd(GUID svc_or_group_guid, const wchar_t *prefered_container=NULL, int container_flag=0, RECT *animated_rect_source=NULL, int transcient=0, int starthidden=0);
    SkinWnd(const wchar_t *group_id, const wchar_t *prefered_container=NULL, int container_flag=0, RECT *animated_rect_source=NULL, int transcient=0, int starthidden=0);
    virtual ~SkinWnd();
    void destroy(RECT *animated_rect_dest=NULL);
    ifc_window *getWindow() { return wnd; }
    ScriptObject *getContainer();
    ScriptObject *getLayout();
    int runModal(int center=0);
    void endModal(int retcode);
    GuiObject *findObject(const wchar_t *object_id);
    void notifyMinMaxChanged();
    int isNewContainer() { return isnew; }

  private:
    ifc_window *wnd;
    int isnew;
};

#endif
