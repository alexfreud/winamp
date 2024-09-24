#ifndef __WNDMGR_API_H
#define __WNDMGR_API_H

#include <bfc/dispatch.h>
#include <bfc/nsguid.h>

class ifc_window;
class WindowHolder;
class Container;
class ScriptObject;

#ifndef MODALWND_DEF
#define MODALWND_DEF
ifc_window *const MODALWND_NOWND = reinterpret_cast<ifc_window*>(-1);
#endif

class wndmgr_api : public Dispatchable {
  public:
    void wndTrackAdd(ifc_window *wnd);
    void wndTrackRemove(ifc_window *wnd);
    bool wndTrackDock(ifc_window *wnd, RECT *r, int mask);
    bool wndTrackDock(ifc_window *wnd, RECT *r, RECT *orig_r, int mask);
    void wndTrackStartCooperative(ifc_window *wnd);
    void wndTrackEndCooperative();
    int wndTrackWasCooperative();
    void wndTrackInvalidateAll();
    int skinwnd_toggleByGuid(GUID g, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0);
    int skinwnd_toggleByGroupId(const wchar_t *groupid, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0);
    ifc_window *skinwnd_createByGuid(GUID g, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0, int starthidden =  0, int *isnew = NULL);
    ifc_window *skinwnd_createByGroupId(const wchar_t *groupid, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0, int starthidden =  0, int *isnew = NULL);
    void skinwnd_destroy(ifc_window *w, RECT *destanimrect =  NULL);
    int skinwnd_getNumByGuid(GUID g);
    ifc_window *skinwnd_enumByGuid(GUID g, int n);
    int skinwnd_getNumByGroupId(const wchar_t *groupid);
    ifc_window *skinwnd_enumByGroupId(const wchar_t *groupid, int n);
    void skinwnd_attachToSkin(ifc_window *w, int side, int size);
    //ScriptObject *skin_getContainer(const wchar_t *container_name);
    //ScriptObject *skin_getLayout(ScriptObject *container, const wchar_t *layout_name);
    void wndholder_register(WindowHolder *wh);
    void wndholder_unregister(WindowHolder *wh);
    int messageBox(const wchar_t *txt, const wchar_t *title, int flags, const wchar_t *not_anymore_identifier, ifc_window *parenwnt);
    ifc_window *getModalWnd();
    void pushModalWnd(ifc_window *w =  MODALWND_NOWND);
    void popModalWnd(ifc_window *w =  MODALWND_NOWND);
    void drawAnimatedRects(const RECT *r1, const RECT *r2);
    int autopopup_registerGuid(GUID g, const wchar_t *desc, const wchar_t *prefered_container =  NULL, int container_flag =  0);
    int autopopup_registerGroupId(const wchar_t *groupid, const wchar_t *desc, const wchar_t *prefered_container =  NULL, int container_flag =  0);
    void autopopup_unregister(int id);
    int autopopup_getNumGuids();
    GUID autopopup_enumGuid(int n);
    int autopopup_getNumGroups();
    const wchar_t *autopopup_enumGroup(int n);
    const wchar_t *autopopup_enumGuidDescription(int n);
    const wchar_t *autopopup_enumGroupDescription(int n);
    const wchar_t *varmgr_translate(const wchar_t *str);
    //Container *newDynamicContainer(const wchar_t *containername, int transcient = 1);

  enum {
    WNDMGR_API_WNDTRACKADD = 0,
    WNDMGR_API_WNDTRACKREMOVE = 10,
    WNDMGR_API_WNDTRACKDOCK = 20,
    WNDMGR_API_WNDTRACKDOCK2 = 30,
    WNDMGR_API_WNDTRACKSTARTCOOPERATIVE = 40,
    WNDMGR_API_WNDTRACKENDCOOPERATIVE = 50,
    WNDMGR_API_WNDTRACKWASCOOPERATIVE = 60,
    WNDMGR_API_WNDTRACKINVALIDATEALL = 70,
    WNDMGR_API_SKINWND_TOGGLEBYGUID = 80,
    WNDMGR_API_SKINWND_TOGGLEBYGROUPID = 90,
    WNDMGR_API_SKINWND_CREATEBYGUID = 100,
    WNDMGR_API_SKINWND_CREATEBYGROUPID = 110,
    WNDMGR_API_SKINWND_DESTROY = 120,
    WNDMGR_API_SKINWND_GETNUMBYGUID = 130,
    WNDMGR_API_SKINWND_ENUMBYGUID = 140,
    WNDMGR_API_SKINWND_GETNUMBYGROUPID = 150,
    WNDMGR_API_SKINWND_ENUMBYGROUPID = 160,
    WNDMGR_API_SKINWND_ATTACHTOSKIN = 170,
    //WNDMGR_API_SKIN_GETCONTAINER = 180,
    WNDMGR_API_SKIN_GETLAYOUT = 190,
    WNDMGR_API_WNDHOLDER_REGISTER = 200,
    WNDMGR_API_WNDHOLDER_UNREGISTER = 210,
    WNDMGR_API_MESSAGEBOX = 220,
    WNDMGR_API_GETMODALWND = 230,
    WNDMGR_API_PUSHMODALWND = 240,
    WNDMGR_API_POPMODALWND = 250,
    WNDMGR_API_DRAWANIMATEDRECTS = 260,
    WNDMGR_API_AUTOPOPUP_REGISTERGUID = 270,
    WNDMGR_API_AUTOPOPUP_REGISTERGROUPID = 280,
    WNDMGR_API_AUTOPOPUP_UNREGISTER = 290,
    WNDMGR_API_VARMGR_TRANSLATE = 300,
    WNDMGR_API_NEWDYNAMICCONTAINER = 310,
    WNDMGR_API_AUTOPOPUP_GETNUMGUIDS = 320,
    WNDMGR_API_AUTOPOPUP_ENUMGUID = 330,
    WNDMGR_API_AUTOPOPUP_GETNUMGROUPS = 340,
    WNDMGR_API_AUTOPOPUP_ENUMGROUPS = 350,
    WNDMGR_API_AUTOPOPUP_ENUMGUIDDESC = 360,
    WNDMGR_API_AUTOPOPUP_ENUMGROUPDESC = 370,
  };
};

inline void wndmgr_api::wndTrackAdd(ifc_window *wnd) {
  _voidcall(WNDMGR_API_WNDTRACKADD, wnd);
}

inline void wndmgr_api::wndTrackRemove(ifc_window *wnd) {
  _voidcall(WNDMGR_API_WNDTRACKREMOVE, wnd);
}

inline bool wndmgr_api::wndTrackDock(ifc_window *wnd, RECT *r, int mask) {
  return _call(WNDMGR_API_WNDTRACKDOCK, (bool)FALSE, wnd, r, mask);
}

inline bool wndmgr_api::wndTrackDock(ifc_window *wnd, RECT *r, RECT *orig_r, int mask) {
  return _call(WNDMGR_API_WNDTRACKDOCK2, (bool)FALSE, wnd, r, orig_r, mask);
}

inline void wndmgr_api::wndTrackStartCooperative(ifc_window *wnd) {
  _voidcall(WNDMGR_API_WNDTRACKSTARTCOOPERATIVE, wnd);
}

inline void wndmgr_api::wndTrackEndCooperative() {
  _voidcall(WNDMGR_API_WNDTRACKENDCOOPERATIVE);
}

inline int wndmgr_api::wndTrackWasCooperative() {
  return _call(WNDMGR_API_WNDTRACKWASCOOPERATIVE, (int)0);
}

inline void wndmgr_api::wndTrackInvalidateAll() {
  _voidcall(WNDMGR_API_WNDTRACKINVALIDATEALL);
}

inline int wndmgr_api::skinwnd_toggleByGuid(GUID g, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient) {
  return _call(WNDMGR_API_SKINWND_TOGGLEBYGUID, (int)0, g, prefered_container, container_flag, sourceanimrect, transcient);
}

inline int wndmgr_api::skinwnd_toggleByGroupId(const wchar_t *groupid, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient) {
  return _call(WNDMGR_API_SKINWND_TOGGLEBYGROUPID, (int)0, groupid, prefered_container, container_flag, sourceanimrect, transcient);
}

inline ifc_window *wndmgr_api::skinwnd_createByGuid(GUID g, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient, int starthidden, int *isnew) {
  return _call(WNDMGR_API_SKINWND_CREATEBYGUID, (ifc_window *)NULL, g, prefered_container, container_flag, sourceanimrect, transcient, starthidden, isnew);
}

inline ifc_window *wndmgr_api::skinwnd_createByGroupId(const wchar_t *groupid, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient, int starthidden, int *isnew) {
  return _call(WNDMGR_API_SKINWND_CREATEBYGROUPID, (ifc_window *)NULL, groupid, prefered_container, container_flag, sourceanimrect, transcient, starthidden, isnew);
}

inline void wndmgr_api::skinwnd_destroy(ifc_window *w, RECT *destanimrect) {
  _voidcall(WNDMGR_API_SKINWND_DESTROY, w, destanimrect);
}

inline int wndmgr_api::skinwnd_getNumByGuid(GUID g) {
  return _call(WNDMGR_API_SKINWND_GETNUMBYGUID, (int)0, g);
}

inline ifc_window *wndmgr_api::skinwnd_enumByGuid(GUID g, int n) {
  return _call(WNDMGR_API_SKINWND_ENUMBYGUID, (ifc_window *)NULL, g, n);
}

inline int wndmgr_api::skinwnd_getNumByGroupId(const wchar_t *groupid) {
  return _call(WNDMGR_API_SKINWND_GETNUMBYGROUPID, (int)0, groupid);
}

inline ifc_window *wndmgr_api::skinwnd_enumByGroupId(const wchar_t *groupid, int n) {
  return _call(WNDMGR_API_SKINWND_ENUMBYGROUPID, (ifc_window *)NULL, groupid, n);
}

inline void wndmgr_api::skinwnd_attachToSkin(ifc_window *w, int side, int size) {
  _voidcall(WNDMGR_API_SKINWND_ATTACHTOSKIN, w, side, size);
}
/*
inline ScriptObject *wndmgr_api::skin_getContainer(const wchar_t *container_name) {
  return _call(WNDMGR_API_SKIN_GETCONTAINER, (ScriptObject *)NULL, container_name);
}
*/
/*
inline ScriptObject *wndmgr_api::skin_getLayout(ScriptObject *container, const wchar_t *layout_name) {
  return _call(WNDMGR_API_SKIN_GETLAYOUT, (ScriptObject *)NULL, container, layout_name);
}
*/
inline void wndmgr_api::wndholder_register(WindowHolder *wh) {
  _voidcall(WNDMGR_API_WNDHOLDER_REGISTER, wh);
}

inline void wndmgr_api::wndholder_unregister(WindowHolder *wh) {
  _voidcall(WNDMGR_API_WNDHOLDER_UNREGISTER, wh);
}

inline int wndmgr_api::messageBox(const wchar_t *txt, const wchar_t *title, int flags, const wchar_t *not_anymore_identifier, ifc_window *parenwnt) {
  return _call(WNDMGR_API_MESSAGEBOX, (int)0, txt, title, flags, not_anymore_identifier, parenwnt);
}

inline ifc_window *wndmgr_api::getModalWnd() {
  return _call(WNDMGR_API_GETMODALWND, (ifc_window *)NULL);
}

inline void wndmgr_api::pushModalWnd(ifc_window *w) {
  _voidcall(WNDMGR_API_PUSHMODALWND, w);
}

inline void wndmgr_api::popModalWnd(ifc_window *w) {
  _voidcall(WNDMGR_API_POPMODALWND, w);
}

inline void wndmgr_api::drawAnimatedRects(const RECT *r1, const RECT *r2) {
  _voidcall(WNDMGR_API_DRAWANIMATEDRECTS, r1, r2);
}

inline int wndmgr_api::autopopup_registerGuid(GUID g, const wchar_t *desc, const wchar_t *prefered_container, int container_flag) {
  return _call(WNDMGR_API_AUTOPOPUP_REGISTERGUID, (int)0, g, desc, prefered_container, container_flag);
}

inline int wndmgr_api::autopopup_registerGroupId(const wchar_t *groupid, const wchar_t *desc, const wchar_t *prefered_container, int container_flag) {
  return _call(WNDMGR_API_AUTOPOPUP_REGISTERGROUPID, (int)0, groupid, desc, prefered_container, container_flag);
}

inline void wndmgr_api::autopopup_unregister(int id) {
  _voidcall(WNDMGR_API_AUTOPOPUP_UNREGISTER, id);
}

inline const wchar_t *wndmgr_api::varmgr_translate(const wchar_t *str) {
  return _call(WNDMGR_API_VARMGR_TRANSLATE, (const wchar_t *)0, str);
}
/*
inline Container *wndmgr_api::newDynamicContainer(const wchar_t *containername, int transcient) {
  return _call(WNDMGR_API_NEWDYNAMICCONTAINER, (Container *)NULL, containername, transcient);
}
*/
inline int wndmgr_api::autopopup_getNumGuids() {
  return _call(WNDMGR_API_AUTOPOPUP_GETNUMGUIDS, 0);
}

inline GUID wndmgr_api::autopopup_enumGuid(int n) {
  return _call(WNDMGR_API_AUTOPOPUP_ENUMGUID, INVALID_GUID, n);
}


inline int wndmgr_api::autopopup_getNumGroups() {
  return _call(WNDMGR_API_AUTOPOPUP_GETNUMGROUPS, 0);
}

inline const wchar_t *wndmgr_api::autopopup_enumGroup(int n) {
  return _call(WNDMGR_API_AUTOPOPUP_ENUMGROUPS, (const wchar_t*)NULL, n);
}

inline const wchar_t *wndmgr_api::autopopup_enumGuidDescription(int n) {
  return _call(WNDMGR_API_AUTOPOPUP_ENUMGUIDDESC, (const wchar_t *)NULL, n);
}

inline const wchar_t *wndmgr_api::autopopup_enumGroupDescription(int n) {
  return _call(WNDMGR_API_AUTOPOPUP_ENUMGROUPDESC, (const wchar_t *)NULL, n);
}

class wndmgr_apiI : public wndmgr_api {
  public:
    virtual void wndTrackAdd(ifc_window *wnd)=0;
    virtual void wndTrackRemove(ifc_window *wnd)=0;
    virtual bool wndTrackDock(ifc_window *wnd, RECT *r, int mask)=0;
    virtual bool wndTrackDock2(ifc_window *wnd, RECT *r, RECT *orig_r, int mask)=0;
    virtual void wndTrackStartCooperative(ifc_window *wnd)=0;
    virtual void wndTrackEndCooperative()=0;
    virtual int wndTrackWasCooperative()=0;
    virtual void wndTrackInvalidateAll()=0;
    virtual int skinwnd_toggleByGuid(GUID g, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0)=0;
    virtual int skinwnd_toggleByGroupId(const wchar_t *groupid, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0)=0;
    virtual ifc_window *skinwnd_createByGuid(GUID g, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0, int starthidden =  0, int *isnew=NULL)=0;
    virtual ifc_window *skinwnd_createByGroupId(const wchar_t *groupid, const wchar_t *prefered_container =  NULL, int container_flag =  0, RECT *sourceanimrect =  NULL, int transcient =  0, int starthidden =  0, int *isnew=NULL)=0;
    virtual void skinwnd_destroy(ifc_window *w, RECT *destanimrect =  NULL)=0;
    virtual int skinwnd_getNumByGuid(GUID g)=0;
    virtual ifc_window *skinwnd_enumByGuid(GUID g, int n)=0;
    virtual int skinwnd_getNumByGroupId(const wchar_t *groupid)=0;
    virtual ifc_window *skinwnd_enumByGroupId(const wchar_t *groupid, int n)=0;
    virtual void skinwnd_attachToSkin(ifc_window *w, int side, int size)=0;
    virtual ScriptObject *skin_getContainer(const wchar_t *container_name)=0;
    virtual ScriptObject *skin_getLayout(ScriptObject *container, const wchar_t *layout_name)=0;
    virtual void wndholder_register(WindowHolder *wh)=0;
    virtual void wndholder_unregister(WindowHolder *wh)=0;
    virtual int messageBox(const wchar_t *txt, const wchar_t *title, int flags, const wchar_t *not_anymore_identifier, ifc_window *parenwnt)=0;
    virtual ifc_window *getModalWnd()=0;
    virtual void pushModalWnd(ifc_window *w =  MODALWND_NOWND)=0;
    virtual void popModalWnd(ifc_window *w =  MODALWND_NOWND)=0;
    virtual void drawAnimatedRects(const RECT *r1, const RECT *r2)=0;
    virtual int autopopup_registerGuid(GUID g, const wchar_t *desc, const wchar_t *prefered_container =  NULL, int container_flag =  0)=0;
    virtual int autopopup_registerGroupId(const wchar_t *groupid, const wchar_t *desc, const wchar_t *prefered_container =  NULL, int container_flag =  0)=0;
    virtual void autopopup_unregister(int id)=0;
    virtual int autopopup_getNumGuids()=0;
    virtual GUID autopopup_enumGuid(int n)=0;
    virtual int autopopup_getNumGroups()=0;
    virtual const wchar_t *autopopup_enumGroup(int n)=0;
    virtual const wchar_t *varmgr_translate(const wchar_t *str)=0;
    virtual Container *newDynamicContainer(const wchar_t *containername, int transcient = 1)=0;
    virtual const wchar_t *autopopup_enumGuidDescription(int n)=0;
    virtual const wchar_t *autopopup_enumGroupDescription(int n)=0;

  protected:
    RECVS_DISPATCH;
};

// {038A3567-1530-4062-BA87-CCB4F01DA3E9}
static const GUID wndMgrApiServiceGuid = 
{ 0x38a3567, 0x1530, 0x4062, { 0xba, 0x87, 0xcc, 0xb4, 0xf0, 0x1d, 0xa3, 0xe9 } };

extern wndmgr_api *wndManagerApi;

#endif //wndmgr_api_h
