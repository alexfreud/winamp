#ifndef __APIWNDMGR_H
#define __APIWNDMGR_H

#include <api/wndmgr/api_wndmgr.h>
#include <bfc/stack.h>
#include <bfc/string/StringW.h>

class WndMgrApi : public wndmgr_apiI {
  public:
    WndMgrApi();
    virtual ~WndMgrApi();

    virtual void wndTrackAdd(ifc_window *wnd);
    virtual void wndTrackRemove(ifc_window *wnd);
    virtual bool wndTrackDock(ifc_window *wnd, RECT *r, int mask);
    virtual bool wndTrackDock2(ifc_window *wnd, RECT *r, RECT *orig_r, int mask);
    virtual void wndTrackStartCooperative(ifc_window *wnd);
    virtual void wndTrackEndCooperative();
    virtual int wndTrackWasCooperative();
    virtual void wndTrackInvalidateAll();
    virtual int skinwnd_toggleByGuid(GUID g, const wchar_t *prefered_container = NULL, int container_flag = 0, RECT *sourceanimrect = NULL, int transcient = 0);
    virtual int skinwnd_toggleByGroupId(const wchar_t *groupid, const wchar_t *prefered_container = NULL, int container_flag = 0, RECT *sourceanimrect = NULL, int transcient = 0);
    virtual ifc_window *skinwnd_createByGuid(GUID g, const wchar_t *prefered_container = NULL, int container_flag = 0, RECT *sourceanimrect = NULL, int transcient = 0, int starthidden = 0, int *isnew=NULL);
    virtual ifc_window *skinwnd_createByGroupId(const wchar_t *groupid, const wchar_t *prefered_container = NULL, int container_flag = 0, RECT *sourceanimrect = NULL, int transcient = 0, int starthidden = 0, int *isnew=NULL);
    virtual void skinwnd_destroy(ifc_window *w, RECT *destanimrect = NULL);
    virtual int skinwnd_getNumByGuid(GUID g);
    virtual ifc_window *skinwnd_enumByGuid(GUID g, int n);
    virtual int skinwnd_getNumByGroupId(const wchar_t *groupid);
    virtual ifc_window *skinwnd_enumByGroupId(const wchar_t *groupid, int n);
    virtual void skinwnd_attachToSkin(ifc_window *w, int side, int size);
    virtual ScriptObject *skin_getContainer(const wchar_t *container_name);
    virtual ScriptObject *skin_getLayout(ScriptObject *container, const wchar_t *layout_name);
    virtual void wndholder_register(WindowHolder *wh);
    virtual void wndholder_unregister(WindowHolder *wh);
    virtual int messageBox(const wchar_t *txt, const wchar_t *title, int flags, const wchar_t *not_anymore_identifier, ifc_window *parenwnt);
    virtual ifc_window *getModalWnd();
    virtual void pushModalWnd(ifc_window *w = MODALWND_NOWND);
    virtual void popModalWnd(ifc_window *w = MODALWND_NOWND);
    virtual void drawAnimatedRects(const RECT *r1, const RECT *r2);
    virtual int autopopup_registerGuid(GUID g, const wchar_t *desc, const wchar_t *prefered_container = NULL, int container_flag = 0);
    virtual int autopopup_registerGroupId(const wchar_t *groupid, const wchar_t *desc, const wchar_t *prefered_container = NULL, int container_flag = 0);
    virtual void autopopup_unregister(int id);
    virtual int autopopup_getNumGuids();
    virtual GUID autopopup_enumGuid(int n);
    virtual int autopopup_getNumGroups();
    virtual const wchar_t *autopopup_enumGroup(int n);
    virtual const wchar_t *varmgr_translate(const wchar_t *str);
    virtual Container *newDynamicContainer(const wchar_t *name, int transcient);
    virtual const wchar_t *autopopup_enumGuidDescription(int n);
    virtual const wchar_t *autopopup_enumGroupDescription(int n);

  private:

    static Stack<ifc_window*> modal_wnd_stack;
    StringW ret;
};


#endif // __APIWNDMGR_H
