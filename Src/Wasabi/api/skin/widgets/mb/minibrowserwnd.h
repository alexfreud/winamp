#ifndef __MINIBROWSERWND_H
#define __MINIBROWSERWND_H 

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/service/svcs/svc_minibrowser.h>
#include <api/skin/widgets/mb/minibrowser.h>

#define MBWND_PARENT GuiObjectWnd

class MiniBrowserWnd : public MBWND_PARENT, public MiniBrowserCallbackI {

  public:
  
    MiniBrowserWnd(GUID mb_provider=GUID_MINIBROWSER_ANY);
    virtual ~MiniBrowserWnd();

    virtual int handleDesktopAlpha();
    virtual int handleRatio();
    virtual void onSetVisible(int i);
    virtual int onResize();
    virtual int onInit();
  
    virtual int navigateUrl(const wchar_t *url);
    virtual int back();
    virtual int forward();
    virtual int home();
    virtual int refresh();
    virtual int stop();
    virtual void setTargetName(const wchar_t *name);
    virtual const wchar_t *getTargetName();
    virtual const wchar_t *getCurrentUrl();
    virtual int onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame); // return 1 to cancel navigation
    virtual void onDocumentComplete(const wchar_t *url);
	virtual void onDocumentReady(const wchar_t *url);
	virtual void onNavigateError(const wchar_t *url, int status);
	virtual void onMediaLink(const wchar_t *url);
	virtual const wchar_t* messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3);

    virtual void setScrollbarsFlag(int a); // BROWSER_SCROLLBARS_ALWAYS, BROWSER_SCROLLBARS_AUTO, BROWSER_SCROLLBARS_NEVER

    virtual int minibrowsercb_onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame);
    virtual void minibrowsercb_onDocumentComplete(const wchar_t *url);
	virtual void minibrowsercb_onDocumentReady(const wchar_t *url);
	virtual void minibrowsercb_onMediaLink(const wchar_t *url);
	virtual void minibrowsercb_onNavigateError(const wchar_t *url, int status);
	virtual const wchar_t* minibrowsercb_messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3);

    virtual MiniBrowser *getBrowser();

  private:

    MiniBrowser *mb;
    svc_miniBrowser *mbsvc;
};

#endif

