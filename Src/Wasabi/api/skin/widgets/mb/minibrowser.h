#ifndef __MINIBROWSER_H
#define __MINIBROWSER_H

#include <bfc/dispatch.h>
#include <bfc/common.h>

class ifc_window;

class MiniBrowserCallback : public Dispatchable {
  public:
	int minibrowsercb_onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame);
	void minibrowsercb_onDocumentComplete(const wchar_t *url);
	void minibrowsercb_onDocumentReady(const wchar_t *url);
	void minibrowsercb_onMediaLink(const wchar_t *url);
	void minibrowsercb_onNavigateError(const wchar_t *url, int status);
	const wchar_t* minibrowsercb_messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3);

  enum {
	MINIBROWSER_ONBEFORENAVIGATE   = 10,
	MINIBROWSER_ONDOCUMENTCOMPLETE = 20,
	MINIBROWSER_ONMEDIALINK = 30,
	MINIBROWSER_ONNAVIGATEERROR = 40,
	MINIBROWSER_ONDOCUMENTREADY = 50,
	MINIBROWSER_MESSAGETOMAKI = 60,
  };
};

inline int MiniBrowserCallback ::minibrowsercb_onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame) {
  return _call(MINIBROWSER_ONBEFORENAVIGATE, 0, url, flags, frame);
}

inline void MiniBrowserCallback ::minibrowsercb_onDocumentComplete(const wchar_t *url) {
  _voidcall(MINIBROWSER_ONDOCUMENTCOMPLETE, url);
}

inline void MiniBrowserCallback ::minibrowsercb_onDocumentReady(const wchar_t *url) {
  _voidcall(MINIBROWSER_ONDOCUMENTREADY, url);
}

inline void MiniBrowserCallback ::minibrowsercb_onNavigateError(const wchar_t *url, int status) {
  _voidcall(MINIBROWSER_ONNAVIGATEERROR, url, status);
}

inline void MiniBrowserCallback ::minibrowsercb_onMediaLink(const wchar_t *url) {
  _voidcall(MINIBROWSER_ONMEDIALINK, url);
}

inline const wchar_t* MiniBrowserCallback ::minibrowsercb_messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3) {
  return _call(MINIBROWSER_MESSAGETOMAKI, (const wchar_t*)0, str1, str2, i1, i2, i3);
}

class MiniBrowserCallbackI : public MiniBrowserCallback {
  public:
    virtual int minibrowsercb_onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame)=0;
    virtual void minibrowsercb_onDocumentComplete(const wchar_t *url)=0;
	virtual void minibrowsercb_onDocumentReady(const wchar_t *url)=0;
		virtual void minibrowsercb_onMediaLink(const wchar_t *url)=0;
		virtual void minibrowsercb_onNavigateError(const wchar_t *url, int status)=0;
		virtual const wchar_t* minibrowsercb_messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3)=0;

  protected:
    RECVS_DISPATCH;
};

class MiniBrowser : public Dispatchable  {

  public:

    ifc_window *minibrowser_getRootWnd();
    int minibrowser_navigateUrl(const wchar_t *url);
    int minibrowser_back();
    int minibrowser_forward();
    int minibrowser_home();
    int minibrowser_refresh();
    int minibrowser_stop();
    void minibrowser_setTargetName(const wchar_t *name);
    const wchar_t *minibrowser_getTargetName();
    const wchar_t *minibrowser_getCurrentUrl();
		void minibrowser_getDocumentTitle(wchar_t *str, size_t len);
    void minibrowser_addCB(MiniBrowserCallback *cb);
    void minibrowser_setHome(const wchar_t *url);
    void minibrowser_setScrollbarsFlag(int a); // BROWSER_SCROLLBARS_ALWAYS, BROWSER_SCROLLBARS_AUTO, BROWSER_SCROLLBARS_NEVER
		void minibrowser_scrape();
		virtual void minibrowser_setCancelIEErrorPage(bool cancel);
		virtual const wchar_t* minibrowser_messageToJS(const wchar_t* str1, const wchar_t* str2, int i1, int i2, int i3);

  enum {
    MINIBROWSER_GETROOTWND      =  100,
    MINIBROWSER_NAVIGATEURL     =  200,
    MINIBROWSER_BACK            =  300,
    MINIBROWSER_FORWARD         =  400,
    MINIBROWSER_HOME            =  500,
    MINIBROWSER_REFRESH         =  600,
    MINIBROWSER_STOP            =  700,
    MINIBROWSER_SETTARGETNAME   =  800,
    MINIBROWSER_GETTARGETNAME   =  900,
    MINIBROWSER_GETCURRENTURL   = 1000,
    MINIBROWSER_ADDCB           = 1100,
    MINIBROWSER_SETHOME         = 1200,
    MINIBROWSER_SETSCROLLFLAG   = 1300,
		MINIBROWSER_SCRAPE = 2000,
		MINIBROWSER_GETDOCUMENTTITLE = 2100,
		MINIBROWSER_SETCANCELIEERRORPAGE = 2200,
		MINIBROWSER_MESSAGETOJS = 2300,
  };

  enum {
    BROWSER_SCROLLBARS_DEFAULT  = -1,
    BROWSER_SCROLLBARS_ALWAYS   = 0, 
    BROWSER_SCROLLBARS_AUTO     = 1, 
    BROWSER_SCROLLBARS_NEVER    = 2,
  };

};

inline ifc_window *MiniBrowser::minibrowser_getRootWnd() {
  return _call(MINIBROWSER_GETROOTWND, (ifc_window *)NULL);
}

inline int MiniBrowser::minibrowser_navigateUrl(const wchar_t *url) {
  return _call(MINIBROWSER_NAVIGATEURL, 0, url);
}

inline int MiniBrowser::minibrowser_back() {
  return _call(MINIBROWSER_BACK, 0);
}

inline int MiniBrowser::minibrowser_forward() {
  return _call(MINIBROWSER_FORWARD, 0);
}

inline int MiniBrowser::minibrowser_home() {
  return _call(MINIBROWSER_HOME, 0);
}

inline int MiniBrowser::minibrowser_refresh() {
  return _call(MINIBROWSER_REFRESH, 0);
}

inline int MiniBrowser::minibrowser_stop() {
  return _call(MINIBROWSER_STOP, 0);
}

inline void MiniBrowser::minibrowser_setHome(const wchar_t *url) {
  _voidcall(MINIBROWSER_SETHOME, url);
}

inline void MiniBrowser::minibrowser_setTargetName(const wchar_t *name) {
  _voidcall(MINIBROWSER_SETTARGETNAME, name);
}

inline const wchar_t *MiniBrowser::minibrowser_getTargetName() {
  return _call(MINIBROWSER_GETTARGETNAME, (const wchar_t *)NULL);
}

inline void MiniBrowser::minibrowser_getDocumentTitle(wchar_t *str, size_t len) {
  _voidcall(MINIBROWSER_GETDOCUMENTTITLE, str, len);
}

inline const wchar_t *MiniBrowser::minibrowser_getCurrentUrl() {
  return _call(MINIBROWSER_GETCURRENTURL, (const wchar_t *)NULL);
}

inline void MiniBrowser::minibrowser_addCB(MiniBrowserCallback *cb) {
  _voidcall(MINIBROWSER_ADDCB, cb);
}

inline void MiniBrowser::minibrowser_setScrollbarsFlag(int a) {
  _voidcall(MINIBROWSER_SETSCROLLFLAG, a);
}

inline void MiniBrowser::minibrowser_scrape()
{
	_voidcall(MINIBROWSER_SCRAPE);
}

inline void MiniBrowser::minibrowser_setCancelIEErrorPage(bool cancel)
{
	_voidcall(MINIBROWSER_SETCANCELIEERRORPAGE, cancel);
}

inline const wchar_t* MiniBrowser::minibrowser_messageToJS(const wchar_t* str1, const wchar_t* str2, int i1, int i2, int i3)
{
	return _call(MINIBROWSER_MESSAGETOJS, (const wchar_t *)NULL, str1, str2, i1, i2, i3);
}

class MiniBrowserI : public MiniBrowser {

  public:

    virtual ifc_window *minibrowser_getRootWnd()=0;
    virtual int minibrowser_navigateUrl(const wchar_t *url)=0;
    virtual int minibrowser_back()=0;
    virtual int minibrowser_forward()=0;
    virtual int minibrowser_home()=0;
    virtual int minibrowser_refresh()=0;
    virtual int minibrowser_stop()=0;
    virtual void minibrowser_setTargetName(const wchar_t *name)=0;
    virtual const wchar_t *minibrowser_getTargetName()=0;
    virtual const wchar_t *minibrowser_getCurrentUrl()=0;
		virtual void minibrowser_getDocumentTitle(wchar_t *str, size_t len)=0;
    virtual void minibrowser_addCB(MiniBrowserCallback *cb)=0;
    virtual void minibrowser_setHome(const wchar_t *url)=0;
    virtual void minibrowser_setScrollbarsFlag(int a)=0;
		virtual void minibrowser_scrape()=0;
		virtual void minibrowser_setCancelIEErrorPage(bool cancel)=0;
		virtual const wchar_t* minibrowser_messageToJS(const wchar_t* str1, const wchar_t* str2, int i1, int i2, int i3)=0;

  protected:
    RECVS_DISPATCH;
};


#endif

