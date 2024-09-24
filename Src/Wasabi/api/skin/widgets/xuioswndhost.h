#ifndef __OSWndHost_H
#define __OSWndHost_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/wnd/wndclass/oswndhost.h>

#define  XUIOSWNDHOST_PARENT GuiObjectWnd

#define DCB_OSWNDHOST_REQUEST_IDEAL_SIZE 2048

#define OSWNDHOST_REQUEST_IDEAL_SIZE WM_USER + DCB_OSWNDHOST_REQUEST_IDEAL_SIZE

class DCBIdealSize {
  public:
  DCBIdealSize(int idealwidth, int idealheight) : m_idealwidth(idealwidth), m_idealheight(idealheight) {}
  int m_idealwidth;
  int m_idealheight;
};

// -----------------------------------------------------------------------
class XuiOSWndHost : public  XUIOSWNDHOST_PARENT, public OSWndHostI 
{
  
  public:
                             
    XuiOSWndHost();
    virtual ~XuiOSWndHost();
    virtual int onPaint(Canvas *c);
    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
		virtual int wantRedrawOnResize() { return 0; }
		virtual int onAfterResize();
    void setHWND(const char *hwnd);
    HWND getHWND() { return wnd; }
    void setHWND(HWND hwnd) { wnd = hwnd; }
    virtual void oswndhost_host(HWND oswnd);
    virtual void oswndhost_unhost();
    virtual void oswndhost_setRegionOffsets(RECT *r);

    virtual int onUserMessage(int msg, int w, int l, int *r);
    virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

    virtual void onBeforeReparent(int host);
    virtual void onAfterReparent(int host);
    virtual int handleRatio() { return 0; }
    virtual int handleDesktopAlpha() { return 0; }
	void onSetVisible(int show);
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:
		void doOnResize();
		void doHost();

    enum {
       XUIOSWNDHOST_SETHWND        = 0,
       XUIOSWNDHOST_SETOFFSETS     = 1,
    };
		static XMLParamPair params[];
    int myxuihandle;
    RECT regionrect;
    int hasregionrect;
    HWND wnd;
    HWND oldparent;
	DWORD savedStyle;
	DWORD savedExStyle;
    
    RECT oldrect;
		bool hosted;
		int visible_start_state;
};


// -----------------------------------------------------------------------

extern const wchar_t OSWndHostXuiObjectStr[];
extern char OSWndHostXuiSvcName[];
class OSWndHostXuiSvc : public XuiObjectSvc<XuiOSWndHost, OSWndHostXuiObjectStr, OSWndHostXuiSvcName> {};

#endif
