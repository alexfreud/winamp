#ifndef __XUIBROWSER_H
#define __XUIBROWSER_H

#include <api/skin/widgets/mb/minibrowserwnd.h>
#include <api/script/objcontroller.h>

#define MAIN_BROWSER_ATOM_NAME L"browser.main.object"
#define SCRIPTBROWSERWND_PARENT MiniBrowserWnd 

// -----------------------------------------------------------------------------------------------------
class ScriptBrowserWnd : public SCRIPTBROWSERWND_PARENT {
  
  public:

    ScriptBrowserWnd();
    virtual ~ScriptBrowserWnd();

    virtual void onSetVisible(int v);

    // XuiObject

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    // MiniBrowserWnd
    int onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame);
    void onDocumentComplete(const wchar_t *url);
	void onDocumentReady(const wchar_t *url);
	void onNavigateError(const wchar_t *url, int status);
		void onMediaLink(const wchar_t *url);
		void getDocumentTitle(wchar_t *str, size_t len);

    virtual int navigateUrl(const wchar_t *url);
	const wchar_t* messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3);
	const wchar_t* messageToJS(const wchar_t* str1, const wchar_t* str2, int i1, int i2, int i3);

    // --

    void setUrl(const wchar_t *url);
    void setMainMB(int tf);
		void Scrape(); // benski> added Aug 17 2007

		void setCancelIEErrorPage(bool cancel);

protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    enum {
      BROWSER_SETURL = 0,
      BROWSER_SETMAINMB,
      BROWSER_SETTARGETNAME,
      BROWSER_SETSCROLLBARS,
	  BROWSER_CANCELIEERRORPAGE,
    };
static XMLParamPair params[];
    int translateScrollbarFlag(const wchar_t *scrollbarflag);

    StringW defurl;
    int myxuihandle;
};

// -----------------------------------------------------------------------------------------------------
class BrowserScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"Browser"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return browserGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static scriptVar browser_navigateUrl(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url);
    static scriptVar browser_back(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar browser_forward(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar browser_home(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar browser_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar browser_refresh(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
		static scriptVar browser_scrape(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar browser_setTargetName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);
    static scriptVar browser_onDocumentComplete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url);
	static scriptVar browser_onDocumentReady(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url);
	static scriptVar browser_onNavigateError(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url, scriptVar status);
    static scriptVar browser_onBeforeNavigate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url, scriptVar flags, scriptVar framename);
	static scriptVar browser_messageToMaki(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str1, scriptVar str2, scriptVar i1, scriptVar i2, scriptVar i3);
	static scriptVar browser_messageToJS(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str1, scriptVar str2, scriptVar i1, scriptVar i2, scriptVar i3);
		static scriptVar browser_onMediaLink(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url);
		static /*string*/ scriptVar browser_getDocumentTitle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
		static scriptVar browser_setCancelIEErrorPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar cancel);
  private:

    static function_descriptor_struct exportedFunction[];
};

extern BrowserScriptController *browserController;

extern const wchar_t browserXuiObjectStr[];
extern char browserXuiSvcName[];
class BrowserXuiSvc : public XuiObjectSvc<ScriptBrowserWnd, browserXuiObjectStr, browserXuiSvcName> {};

#endif
