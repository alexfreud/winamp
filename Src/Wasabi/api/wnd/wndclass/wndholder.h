#ifndef __WINDOWHOLDER_H
#define __WINDOWHOLDER_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <bfc/ptrlist.h>
#include <api/syscb/callbacks/wndcb.h>
#include <api/timer/timerclient.h>

class svc_windowCreate;

#define WINDOWHOLDER_PARENT GuiObjectWnd 


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class WindowHolder : public Dispatchable 
{
  public:
    ifc_window *onInsertWindow(GUID g, const wchar_t *groupid);
    void onRemoveWindow(int deferred=0);
    int wantGuid(GUID g);
    int wantGroup(const wchar_t *groupid);
    GUID getCurGuid();
    const wchar_t *getCurGroupId();
    ifc_window *getCurRootWnd();
    const wchar_t *getCurId();
    ifc_window *getRootWndPtr();
    int acceptsGenericGuid();
    int acceptsGenericGroup();
    int wndholder_getPreferences(int what);
    void wndholder_onNeedReloadGroup(const wchar_t *id);
    void cancelDeferredRemove();
    int wndholder_wantAutoFocus();
    int wndholder_isAutoAvailable();

  enum {
    WNDHOLDER_ONINSERTWINDOW=50,
    WNDHOLDER_ONREMOVEWINDOW=100,
    WNDHOLDER_WANTGUID=150,
    WNDHOLDER_WANTGROUP=200,
    WNDHOLDER_GETROOTWNDPTR=250,
    WNDHOLDER_GETCURGUID=300,
    WNDHOLDER_GETCURGROUPID=350,
    WNDHOLDER_GETCURROOTWND=400,
    WNDHOLDER_GETCURID=450,
    WNDHOLDER_ISGENERICGUID=500,
    WNDHOLDER_ISGENERICGROUP=550,
    WNDHOLDER_GETPREFERENCES=600,
    WNDHOLDER_ONNEEDRELOADGRP=650,
    WNDHOLDER_CANCELDEFERREDREMOVE=660,
    WNDHOLDER_WANTAUTOFOCUS=670,
    WNDHOLDER_ISAUTOAVAILABLE=680,
  };
};

inline ifc_window *WindowHolder::onInsertWindow(GUID g, const wchar_t *groupid) {  
  return _call(WNDHOLDER_ONINSERTWINDOW, (ifc_window *)NULL, g, groupid);
}

inline void WindowHolder::onRemoveWindow(int def) {  
  _voidcall(WNDHOLDER_ONREMOVEWINDOW, def);
}

inline int WindowHolder::wantGuid(GUID g) {  
  return _call(WNDHOLDER_WANTGUID, 0, g);
}

inline int WindowHolder::wantGroup(const wchar_t *groupid) {  
  return _call(WNDHOLDER_WANTGROUP, 0, groupid);
}                                             

inline ifc_window *WindowHolder::getRootWndPtr() {
  return _call(WNDHOLDER_GETROOTWNDPTR, (ifc_window *)NULL);
}

inline GUID WindowHolder::getCurGuid() {
  return _call(WNDHOLDER_GETCURGUID, INVALID_GUID);
}

inline const wchar_t *WindowHolder::getCurGroupId() {
  return _call(WNDHOLDER_GETCURGROUPID, (const wchar_t *)NULL);
}

inline ifc_window *WindowHolder::getCurRootWnd() {
  return _call(WNDHOLDER_GETCURROOTWND, (ifc_window *)NULL);
}

inline const wchar_t *WindowHolder::getCurId() {
  return _call(WNDHOLDER_GETCURID, (const wchar_t *)NULL);
}

inline int WindowHolder::acceptsGenericGuid() {
  return _call(WNDHOLDER_ISGENERICGUID, 0);
}

inline int WindowHolder::acceptsGenericGroup() {
  return _call(WNDHOLDER_ISGENERICGROUP, 0);
}

inline int WindowHolder::wndholder_getPreferences(int what) {
  return _call(WNDHOLDER_GETPREFERENCES, 0, what);
}

inline void WindowHolder::wndholder_onNeedReloadGroup(const wchar_t *id) {
  _voidcall(WNDHOLDER_ONNEEDRELOADGRP, id);
}

inline void WindowHolder::cancelDeferredRemove() {
  _voidcall(WNDHOLDER_CANCELDEFERREDREMOVE);
}

inline int WindowHolder::wndholder_wantAutoFocus() {
  return _call(WNDHOLDER_WANTAUTOFOCUS, 1);
}

inline int WindowHolder::wndholder_isAutoAvailable() {
  return _call(WNDHOLDER_ISAUTOAVAILABLE, 1);
}

class DeferredRemove;

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class WindowHolderI : public WindowHolder {
  public:

    WindowHolderI();
    virtual ~WindowHolderI();

    virtual ifc_window *onInsertWindow(GUID g, const wchar_t *groupid);
    

    virtual void onRemoveWindow(int deferred);

 
    void addAcceptGuid(GUID g);
    void addAcceptGroup(const wchar_t *groupid);
    void setAcceptAllGuids(int tf);
    

    void setAcceptAllGroups(int tf);

    virtual int acceptsGenericGroup() { return generic_group; }

    virtual int acceptsGenericGuid() { return generic_guid; }

    virtual int wantGuid(GUID g);
    virtual int wantGroup(const wchar_t *groupid);

    virtual void onInsert(ifc_window *w, const wchar_t *id) {};
    
    virtual void onRemove(ifc_window *w, const wchar_t *id) {};

    virtual ifc_window *getRootWndPtr()=0; 
    virtual GUID getCurGuid() { return cur_guid; }
    virtual const wchar_t *getCurGroupId() { return cur_groupid; }

    virtual ifc_window *getCurRootWnd() { return wnd; }

    virtual GUID *getFirstAcceptedGuid();
    virtual const wchar_t *getFirstAcceptedGroup();
    virtual const wchar_t *getCurId() { return cur_id; }

    virtual void cancelDeferredRemove();
    virtual int wndholder_isAutoAvailable() { return 1; }
    
    GUID getDeferedGuid() { return defered_guid; }

    virtual int wndholder_getPreferences(int what)=0;
 
    virtual void wndholder_onNeedReloadGroup(const wchar_t *id);

  private:

  
    ifc_window *createWindow(const GUID *g, const wchar_t *groupid);
    virtual int wndholder_wantAutoFocus();
    

    void destroyWindow();

    ifc_window *wnd;
    GUID cur_guid;
    StringW cur_groupid;
    StringW cur_id;
    PtrList<GUID> accepted_guids;
    PtrList<StringW> accepted_groups;
    int generic_guid;
    int generic_group;

    svc_windowCreate *wc_svc;
    GUID defered_guid;

    DeferredRemove *dr;

  protected:

    RECVS_DISPATCH;
};


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class WindowHolderWnd : public WINDOWHOLDER_PARENT, public WindowHolderI 
{

  public:

    WindowHolderWnd();
    virtual ~WindowHolderWnd();
    virtual int onInit();

    virtual ifc_window *getRootWndPtr() { return this; }
    virtual void onInsert(ifc_window *w, const wchar_t *id);
    virtual void onRemove(ifc_window *w, const wchar_t *id);
    virtual int onResize();
    virtual int handleRatio();
    virtual int handleDesktopAlpha();
    virtual int handleTransparency();
    virtual int wndholder_getPreferences(int what) { return getPreferences(what); }
    virtual int getPreferences(int what);
    void setAutoOpen(int tf) { autoopen = tf; }
    void setAutoClose(int tf) { autoclose = tf; }
    void setNoCmdBar(int tf) { nocmdbar = tf; if (isInited()) invalidate(); }
    void setNoAnim(int tf) { noanim = tf; }
    virtual int onGroupChange(const wchar_t *grpid);
    virtual int wndholder_wantAutoFocus();
    void setAutoFocus(int autof) { autofocus = autof; }
    void setAutoAvailable(int autoa) { autoavail = autoa; }
    virtual int wndholder_isAutoAvailable() { return autoavail; }

  private:
    void notifyOnRemove(); // no virtual please
    void notifyOnInsert(); // no virtual please
    virtual void onSetVisible(int show);
    virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

    int autoopen;
    int autoclose;
    int nocmdbar;
    int noanim;
    int has_wnd;
    int autofocus;
    int autoavail;
};

class DeferredRemove : public TimerClientDI 
{
  public:
    DeferredRemove(WindowHolderI *parent) : whi(parent) {}
    virtual ~DeferredRemove() {}

    void post() {
      timerclient_postDeferredCallback(1, 0);
    }

    virtual int timerclient_onDeferredCallback(intptr_t p1, intptr_t p2) {
      if (p1 == 1 && whi != NULL) whi->onRemoveWindow(0);
      else return TimerClientDI::timerclient_onDeferredCallback(p1, p2);
      return 0;
    }

  private:
    WindowHolderI *whi;
};

#endif
