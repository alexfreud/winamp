#ifndef _APPBARWND_H
#define _APPBARWND_H

#include <bfc/common.h>
#include <shellapi.h>
#include <api/wnd/wndclass/clickwnd.h>

#define APPBARWND_PARENT ClickWnd

#define APPBAR_TOP_ENABLED    1
#define APPBAR_LEFT_ENABLED   2
#define APPBAR_BOTTOM_ENABLED 4
#define APPBAR_RIGHT_ENABLED  8

#define APPABR_ALL_ENABLED (APPBAR_TOP_ENABLED|APPBAR_LEFT_ENABLED|APPBAR_BOTTOM_ENABLED|APPBAR_RIGHT_ENABLED)

#define APPBAR_CALLBACK 	WM_USER + 1010

#define IDT_AUTOHIDE 	  0x10000
#define IDT_AUTOUNHIDE 	0x10001

#ifdef WIN32
#define APPBAR_NOTDOCKED -1
#define APPBAR_LEFT		ABE_LEFT
#define APPBAR_TOP	  ABE_TOP
#define APPBAR_RIGHT  ABE_RIGHT
#define APPBAR_BOTTOM ABE_BOTTOM
#else
#error port me
#endif

// todo : dispatch
class AppBar {
public:
    virtual void appbar_dock(int side)=0;
    virtual int appbar_isDocked()=0;
    virtual int appbar_getSide()=0;
    virtual void appbar_setEnabledSides(int mask)=0;
    virtual int appbar_getEnabledSides()=0;
    virtual int appbar_isSideEnabled(int side)=0;
    virtual int appbar_testDock(int x, int y, RECT *dockrect=NULL)=0;
    virtual int appbar_updateAutoHide()=0;
    virtual int appbar_updateAlwaysOnTop()=0;
    virtual int appbar_isHiding()=0;
    virtual int appbar_wantAutoHide()=0;
    virtual int appbar_wantAlwaysOnTop()=0;
    virtual int appbar_isAutoHiding()=0;
    virtual void appbar_onDock(int side) {}
    virtual void appbar_onUnDock() {}
    virtual void appbar_onSlide() {}
    virtual void appbar_posChanged()=0;
    virtual int appbar_isSideAutoHideSafe(int side)=0;
    virtual int appbar_getAutoHideWidthHeight()=0;
    virtual void appbar_setNoRestore(int no)=0;
};

// {242CFAA4-31B3-4b01-97C8-2F0A9FFDEF79}
static const GUID appBarGuid = 
{ 0x242cfaa4, 0x31b3, 0x4b01, { 0x97, 0xc8, 0x2f, 0xa, 0x9f, 0xfd, 0xef, 0x79 } };

class api_region;

// TODO: benski> only class making active use of being derived from this seems to be Layout and GuiObjectWnd
// maybe just layout ...

class AppBarWnd : public APPBARWND_PARENT, public AppBar {
  public:
    AppBarWnd();
    virtual ~AppBarWnd();

    void appbar_dock(int side);
    int appbar_isDocked();
    int appbar_getSide();

    void appbar_setEnabledSides(int mask);
    int appbar_getEnabledSides();
    int appbar_isSideEnabled(int side);

    int appbar_testDock(int x, int y, RECT *dockrect=NULL);

    int appbar_updateAutoHide();
    int appbar_updateAlwaysOnTop();

    int appbar_isSideAutoHideSafe(int side);

    virtual int appbar_wantAutoHide() { return 1; }
    virtual int appbar_wantAlwaysOnTop() { return 1; }

    int appbar_isHiding();
    int appbar_isAutoHiding();

    void appbar_posChanged();
    void appbar_setNoRestore(int no);
    virtual int appbar_getAutoHideWidthHeight() { return 2; }

    virtual LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    virtual void onAfterReinit();
    virtual void onSetVisible(int show);

    virtual void onRatioChanged();
  
  private:

    void appBarCallback(UINT uMsg, WPARAM wParam, LPARAM lParam);

    int registerWinAppBar();
    void unregisterWinAppBar();
    void notifyWinAppBarPosition(int side, RECT rect);
    
    OSWINDOWHANDLE getCurAutoHide(int side);

    void getDockRect(int side, RECT *rc);
    void getEdge(int side, RECT *rc);
    void straightenRect(int side, RECT *r);
    void updateDocking();
    void updateSide();
    void updateTimers();
    void resetAutoHideSide(int side);
    void setAutoHideSide(int side);
    void setAutoHideTimer();
    void setAutoUnHideTimer();
    void resetAutoHideTimer();
    void resetAutoUnHideTimer();
    void onAutoHideTimer();
    void onAutoUnHideTimer();
    void autoHide();
    void autoUnHide();
    void slideWindow(RECT *prc);
    int screenCorner(POINT *pt);
    void snapAdjust(RECT *r, int way);

    void dock(int side);
    void unDock();

    void unOwn();
    void reOwn();

    int m_registered;
    int m_side;
    int m_enabled;
    int m_cur_autohide;
    int m_cur_side;
    int m_cur_hiding;
    OSWINDOWHANDLE m_oldZOrder;
    int m_destroying;
    int m_norestore;
    int m_autohide_timer_set;
    int m_autounhide_timer_set;
    int m_sliding;
    int m_suspended;
    int m_fs;
    int m_wahidden;
};

#endif //_APPBARWND_H