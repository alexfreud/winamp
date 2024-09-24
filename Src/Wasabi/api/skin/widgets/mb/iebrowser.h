#ifndef __BROWSER_H
#define __BROWSER_H

class BrowserWnd;

#define BROWSER_PARENT OSWnd
#define IDC_SINKOBJ 0x9871 // arbitrary unique id
#define MB_TIMERID1 0x1927
#define MB_TIMERID2 0x1928

class String;

#include <nu/HTMLContainer2.h>
#include <api/wnd/wndclass/oswnd.h>
#include <api/wnd/minibrowser.h>

class BrowserWnd : public BROWSER_PARENT, public HTMLContainer2, public MiniBrowserI {
public:
  BrowserWnd();
  virtual ~BrowserWnd();

public:
	static bool InitializeLibrary();
	static void UninitializeLibrary();

public:
  // ifc_window
  virtual int onInit();
  virtual void onSetVisible(int show);
  virtual int handleDesktopAlpha() { return 0; }
	DWORD OnGetDownlodFlags(void);

	virtual int onMouseWheelUp(int click, int lines){return 1;}
    virtual int onMouseWheelDown(int click, int lines){return 1;}

  // OSWnd
  virtual HWND getOSHandle();

  // MiniBrowser
  virtual int minibrowser_navigateUrl(const wchar_t *url);
  virtual void minibrowser_setHome(const wchar_t *url) { homepage = url; }
  virtual int minibrowser_back();
  virtual int minibrowser_forward();
  virtual int minibrowser_home();
  virtual int minibrowser_refresh();
  virtual int minibrowser_stop();
  virtual void minibrowser_setTargetName(const wchar_t *name);
  const wchar_t *minibrowser_getTargetName();
  const wchar_t *minibrowser_getCurrentUrl();
  virtual void minibrowser_addCB(MiniBrowserCallback *cb) { callbacks.addItem(cb); }
  virtual ifc_window *minibrowser_getRootWnd() { return this; }

  virtual void minibrowser_setScrollbarsFlag(int a); //BROWSER_SCROLLBARS_ALWAYS, BROWSER_SCROLLBARS_AUTO, BROWSER_SCROLLBARS_NEVER
	virtual void minibrowser_scrape();
	virtual void minibrowser_setCancelIEErrorPage(bool cancel);
	void minibrowser_getDocumentTitle(wchar_t *str, size_t len);
	virtual const wchar_t* minibrowser_messageToJS(const wchar_t* str1, const wchar_t* str2, int i1, int i2, int i3);
  //
  virtual void timerCallback(int id);
  void onTargetNameTimer();

  bool ProcessMessage(MSG *msg); // return true to 'eat' the message

  bool cancelIEErrorPage;
	const wchar_t* messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3);

protected:
  virtual void OnBeforeNavigate(IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel);
  virtual void OnDocumentComplete(IDispatch *pDispatch, VARIANT *URL);
  virtual void OnDocumentReady(IDispatch *pDispatch, VARIANT *URL); // So we can get rid of all iFrame completes 
  virtual void OnNavigateError(IDispatch *pDispatch, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel);
  virtual STDMETHODIMP GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
  virtual int initBrowserStuff();
  virtual void freeBrowserStuff();
  virtual void onScrollbarsFlagTimer();
  virtual int wantFocus() { return 1; }


private:
  virtual int doSetTargetName(const wchar_t *name);
  virtual int doSetScrollbars();

  virtual void updateTargetName();
  virtual void updateScrollbars();



    
  BOOL oleOk;
  StringW homepage;
  StringW deferednavigate;
  StringW targetname;
  StringW curpage;
  int timerset1;
  int timerset2;
  PtrList<MiniBrowserCallback> callbacks;
  int scrollbarsflag;
  ifc_messageprocessor *processor;
};

#endif

