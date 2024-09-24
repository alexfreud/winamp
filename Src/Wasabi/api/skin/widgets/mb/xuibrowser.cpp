#include <precomp.h>
#include "xuibrowser.h"

const wchar_t browserXuiObjectStr[] = L"Browser"; // This is the xml tag
char browserXuiSvcName[] = "Browser xui object"; // this is the name of the xuiservice

XMLParamPair ScriptBrowserWnd::params[] = {
            {BROWSER_SETMAINMB, L"MAINMB"},
            {BROWSER_SETSCROLLBARS, L"SCROLLBARS"},
            {BROWSER_SETTARGETNAME, L"TARGETNAME"},
            {BROWSER_SETURL, L"URL"},
        };

ScriptBrowserWnd::ScriptBrowserWnd()
{
	getScriptObject()->vcpu_setInterface(browserGuid, (void *)static_cast<ScriptBrowserWnd *>(this));
	getScriptObject()->vcpu_setClassName(L"Browser"); // this is the script class name
	getScriptObject()->vcpu_setController(browserController);

	myxuihandle = newXuiHandle();
CreateXMLParameters(myxuihandle);
}

void ScriptBrowserWnd::CreateXMLParameters(int master_handle)
{
	//SCRIPTBROWSERWND_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

ScriptBrowserWnd::~ScriptBrowserWnd()
{

	if (WASABI_API_MAKI->maki_getObjectAtom(MAIN_BROWSER_ATOM_NAME) == getScriptObject())
		WASABI_API_MAKI->maki_setObjectAtom(MAIN_BROWSER_ATOM_NAME, NULL);
}

int ScriptBrowserWnd::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value)
{
	if (xuihandle != myxuihandle)
		return SCRIPTBROWSERWND_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

	switch (xmlattributeid)
	{
	case BROWSER_SETURL:
		setUrl(value);
		break;
	case BROWSER_SETMAINMB:
		setMainMB(WTOI(value));
		break;
	case BROWSER_SETTARGETNAME:
		setTargetName(value);
		break;
	case BROWSER_SETSCROLLBARS:
		setScrollbarsFlag(translateScrollbarFlag(value));
		break;
	default:
		return 0;
	}
	return 1;
}

int ScriptBrowserWnd::translateScrollbarFlag(const wchar_t *scrollbarflag)
{
	if (WCSCASEEQLSAFE(scrollbarflag, L"auto")) return MiniBrowser::BROWSER_SCROLLBARS_AUTO;
	if (WCSCASEEQLSAFE(scrollbarflag, L"never")) return MiniBrowser::BROWSER_SCROLLBARS_NEVER;
	if (WCSCASEEQLSAFE(scrollbarflag, L"always")) return MiniBrowser::BROWSER_SCROLLBARS_ALWAYS;
	if (WCSCASEEQLSAFE(scrollbarflag, L"default")) return MiniBrowser::BROWSER_SCROLLBARS_DEFAULT; // as specified by HTML content
	return MiniBrowser::BROWSER_SCROLLBARS_AUTO;
}

void ScriptBrowserWnd::onSetVisible(int v)
{
	SCRIPTBROWSERWND_PARENT::onSetVisible(v);
	if (v && !defurl.isempty() && _wcsicmp(defurl, getCurrentUrl() ? getCurrentUrl() : L"")) 
		navigateUrl(defurl);
}

void ScriptBrowserWnd::setMainMB(int m)
{
	if (m)
		WASABI_API_MAKI->maki_setObjectAtom(MAIN_BROWSER_ATOM_NAME, getScriptObject());
	else
		if (WASABI_API_MAKI->maki_getObjectAtom(MAIN_BROWSER_ATOM_NAME) == getScriptObject())
			WASABI_API_MAKI->maki_setObjectAtom(MAIN_BROWSER_ATOM_NAME, NULL);
}

void ScriptBrowserWnd::setUrl(const wchar_t *url)
{
	defurl = url;
	if (isVisible())
	{
		if (!defurl.isempty())
			navigateUrl(defurl);
	}
}

void ScriptBrowserWnd::setCancelIEErrorPage(bool cancel)
{
	MiniBrowser *browser = getBrowser();
	if (browser) browser->minibrowser_setCancelIEErrorPage(cancel);
}

void ScriptBrowserWnd::Scrape()
{
	MiniBrowser *browser = getBrowser();
	if (browser) browser->minibrowser_scrape();
}

void ScriptBrowserWnd::getDocumentTitle(wchar_t *str, size_t len) 
{
	MiniBrowser *browser = getBrowser();
	if (browser) 
		browser->minibrowser_getDocumentTitle(str, len);
	else
		str[0]=0;
}

int ScriptBrowserWnd::navigateUrl(const wchar_t *url)
{
	defurl = url;
	return SCRIPTBROWSERWND_PARENT::navigateUrl(url);
}

int ScriptBrowserWnd::onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame)
{
	if (SCRIPTBROWSERWND_PARENT::onBeforeNavigate(url, flags, frame)) return 1;
	scriptVar v = BrowserScriptController::browser_onBeforeNavigate(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url), MAKE_SCRIPT_INT(flags), MAKE_SCRIPT_STRING(frame));
	if (v.type != SCRIPT_VOID)
		return GET_SCRIPT_BOOLEAN(v);
	return 0;
}

void ScriptBrowserWnd::onDocumentComplete(const wchar_t *url)
{
	SCRIPTBROWSERWND_PARENT::onDocumentComplete(url);
	if (url)
		BrowserScriptController::browser_onDocumentComplete(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url));
}

void ScriptBrowserWnd::onDocumentReady(const wchar_t *url)
{
	SCRIPTBROWSERWND_PARENT::onDocumentComplete(url);
	if (url)
		BrowserScriptController::browser_onDocumentReady(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url));
}

void ScriptBrowserWnd::onNavigateError(const wchar_t *url, int status)
{
	SCRIPTBROWSERWND_PARENT::onNavigateError(url, status);
	BrowserScriptController::browser_onNavigateError(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url), MAKE_SCRIPT_INT(status));
}

void ScriptBrowserWnd::onMediaLink(const wchar_t *url)
{
	SCRIPTBROWSERWND_PARENT::onMediaLink(url);
	if (url)
		BrowserScriptController::browser_onMediaLink(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url));
}

const wchar_t * ScriptBrowserWnd::messageToMaki(wchar_t* str1, wchar_t* str2, int intval1, int intval2, int intval3)
{
	const wchar_t* ret = SCRIPTBROWSERWND_PARENT::messageToMaki(str1, str2, intval1, intval2, intval3);
	if (ret) return ret;

	scriptVar v = BrowserScriptController::browser_messageToMaki(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(str1), MAKE_SCRIPT_STRING(str2), MAKE_SCRIPT_INT(intval1), MAKE_SCRIPT_INT(intval2), MAKE_SCRIPT_INT(intval3));
	if (v.type == SCRIPT_STRING)
		return v.data.sdata;
	return 0;
}

const wchar_t * ScriptBrowserWnd::messageToJS(const wchar_t* str1, const wchar_t* str2, int i1, int i2, int i3)
{
	MiniBrowser *browser = getBrowser();
	if (browser) return browser->minibrowser_messageToJS(str1, str2, i1, i2, i3);
	return 0;
}

// -----------------------------------------------------------------------
// Script Object

BrowserScriptController _browserController;
BrowserScriptController *browserController = &_browserController;

// -- Functions table -------------------------------------
function_descriptor_struct BrowserScriptController::exportedFunction[] = {
            {L"gotoUrl",			1, (void*)BrowserScriptController::browser_navigateUrl },
            {L"navigateUrl",		1, (void*)BrowserScriptController::browser_navigateUrl },
            {L"back",				0, (void*)BrowserScriptController::browser_back },
            {L"forward",			0, (void*)BrowserScriptController::browser_forward },
            {L"home",				0, (void*)BrowserScriptController::browser_home},
            {L"stop",				0, (void*)BrowserScriptController::browser_stop},
            {L"refresh",			0, (void*)BrowserScriptController::browser_refresh},
			{L"scrape",				0, (void*)BrowserScriptController::browser_scrape},						
            {L"setTargetName",		1, (void*)BrowserScriptController::browser_setTargetName},
            {L"onBeforeNavigate",	3, (void*)BrowserScriptController::browser_onBeforeNavigate},
            {L"onDocumentComplete", 1, (void*)BrowserScriptController::browser_onDocumentComplete},
            {L"onDocumentReady",	1, (void*)BrowserScriptController::browser_onDocumentReady},
			{L"onNavigateError",	2, (void*)BrowserScriptController::browser_onNavigateError},
			{L"onMediaLink",		1, (void*)BrowserScriptController::browser_onMediaLink},
			{L"getDocumentTitle",	0, (void*)BrowserScriptController::browser_getDocumentTitle},
			{L"setCancelIEErrorPage",1, (void*)BrowserScriptController::browser_setCancelIEErrorPage}, 
			{L"messageToMaki",		5, (void*)BrowserScriptController::browser_messageToMaki}, 
			{L"messageToJS",		5, (void*)BrowserScriptController::browser_messageToJS}, 
		};

ScriptObject *BrowserScriptController::instantiate()
{
	ScriptBrowserWnd *sb = new ScriptBrowserWnd;
	ASSERT(sb != NULL);
	return sb->getScriptObject();
}

void BrowserScriptController::destroy(ScriptObject *o)
{
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd *>(o->vcpu_getInterface(browserGuid));
	ASSERT(sb != NULL);
	delete sb;
}

void *BrowserScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for browsers yet
}

void BrowserScriptController::deencapsulate(void *o)
{}

int BrowserScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *BrowserScriptController::getExportedFunctions()
{
	return exportedFunction;
}


scriptVar BrowserScriptController::browser_navigateUrl(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(url.type == SCRIPT_STRING);
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->navigateUrl(url.data.sdata);
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_back(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->back();
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_forward(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->forward();
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_home(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->home();
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_refresh(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->refresh();
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->stop();
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_scrape(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->Scrape();
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_setCancelIEErrorPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar cancel)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb) sb->setCancelIEErrorPage(!!cancel.data.idata);
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_setTargetName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb)
		sb->setTargetName(GET_SCRIPT_STRING(name));
	RETURN_SCRIPT_VOID;
}

scriptVar BrowserScriptController::browser_onBeforeNavigate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url, scriptVar flags, scriptVar framename)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS3(o, browserController, url, flags, framename);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT3(o, url, flags, framename);
}

scriptVar BrowserScriptController::browser_messageToMaki(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str1, scriptVar str2, scriptVar i1, scriptVar i2, scriptVar i3)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS5(o, browserController, str1, str2, i1, i2, i3);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT5(o, str1, str2, i1, i2, i3);
}

scriptVar BrowserScriptController::browser_messageToJS(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str1, scriptVar str2, scriptVar i1, scriptVar i2, scriptVar i3)
{
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	if (sb)
	{
		const wchar_t * ret = sb->messageToJS(str1.data.sdata, str2.data.sdata, i1.data.idata, i2.data.idata, i3.data.idata);
		if (ret)
			return MAKE_SCRIPT_STRING(ret);
	}
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar BrowserScriptController::browser_onDocumentComplete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS1(o, browserController, url);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, url);
}

scriptVar BrowserScriptController::browser_onDocumentReady(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS1(o, browserController, url);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, url);
}

scriptVar BrowserScriptController::browser_onNavigateError(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url, scriptVar status)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS2(o, browserController, url, status);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT2(o, url, status);
}

scriptVar BrowserScriptController::browser_onMediaLink(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS1(o, browserController, url);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, url);
}

static wchar_t docTitle[1024];

/*string*/ scriptVar BrowserScriptController::browser_getDocumentTitle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT
	ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
	docTitle[0]=0;
	if (sb) sb->getDocumentTitle(docTitle, 1024);
	return MAKE_SCRIPT_STRING(docTitle);
}