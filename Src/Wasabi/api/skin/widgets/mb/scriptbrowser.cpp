#include "precomp.h"

#include "scriptbrowser.h"

#include "main.h"

#include "script/script.h"
#include "script/scriptmgr.h"
#include "script/vcpu.h"

#include "mbmgr.h"

BrowserScriptController _browserController;
BrowserScriptController *browserController = &_browserController;

// -- Functions table -------------------------------------
function_descriptor_struct BrowserScriptController::exportedFunction[] = {
  {"gotoUrl",            1, (void*)ScriptBrowserWnd::script_vcpu_gotoUrl },
  {"back",               0, (void*)ScriptBrowserWnd::script_vcpu_back },
  {"forward",            0, (void*)ScriptBrowserWnd::script_vcpu_forward },
  {"home",               0, (void*)ScriptBrowserWnd::script_vcpu_home},
  {"refresh",            0, (void*)ScriptBrowserWnd::script_vcpu_refresh},
  {"setTargetName",      1, (void*)ScriptBrowserWnd::script_vcpu_setTargetName},
  {"onBeforeNavigate",   3, (void*)ScriptBrowserWnd::script_vcpu_onBeforeNavigate},
  {"onDocumentComplete", 1, (void*)ScriptBrowserWnd::script_vcpu_onDocumentComplete},
  
};
// --------------------------------------------------------

const wchar_t *BrowserScriptController::getClassName() {
  return L"Browser";
}

const wchar_t *BrowserScriptController::getAncestorClassName() {
  return L"GuiObject";
}

ScriptObject *BrowserScriptController::instantiate() {
  ScriptBrowserWnd *sb = new ScriptBrowserWnd;
  ASSERT(sb != NULL);
  return sb->getScriptObject();
}

void BrowserScriptController::destroy(ScriptObject *o) {
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd *>(o->vcpu_getInterface(browserGuid));
  ASSERT(sb != NULL);
  delete sb;
}

void *BrowserScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for browsers yet
}

void BrowserScriptController::deencapsulate(void *o) {
}

int BrowserScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *BrowserScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID BrowserScriptController::getClassGuid() {
  return browserGuid;
}

ScriptBrowserWnd::ScriptBrowserWnd() {
  getScriptObject()->vcpu_setInterface(browserGuid, (void *)static_cast<ScriptBrowserWnd *>(this));
  getScriptObject()->vcpu_setClassName("Browser");
  getScriptObject()->vcpu_setController(browserController);
}

ScriptBrowserWnd::~ScriptBrowserWnd() {
}

int ScriptBrowserWnd::setXmlParam(const char *name, const char *value) {
  if (SCRIPTBROWSERWND_PARENT::setParam(name, value)) return 1;
  else if (STRCASEEQL(name,"url")) defurl = value;
	else if (STRCASEEQL(name,"mainmb")) setMainMB(WTOI(value));
	else if (STRCASEEQL(name,"targetname")) setTargetName(value);
	else return 0;
	return 1;
}

int ScriptBrowserWnd::onInit() {
  SCRIPTBROWSERWND_PARENT::onInit();
  if (!defurl.isempty()) navigateUrl(defurl);
  return 1;
}

void ScriptBrowserWnd::setMainMB(int m) {
  if (m)  
    MBManager::setMainMB(this);
  else
    if (MBManager::getMainMB() == this)
      MBManager::setMainMB(NULL);
}

int ScriptBrowserWnd::onBeforeNavigate(const char *url, int flags, const char *frame) {
  if (SCRIPTBROWSERWND_PARENT::onBeforeNavigate(url, flags, frame)) return 1;
  scriptVar v = script_vcpu_onBeforeNavigate(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url), MAKE_SCRIPT_INT(flags), MAKE_SCRIPT_STRING(frame));
  if (SOM::isNumeric(&v)) return GET_SCRIPT_BOOLEAN(v);
  return 0;
}

void ScriptBrowserWnd::onDocumentComplete(const char *url) {
  SCRIPTBROWSERWND_PARENT::onDocumentComplete(url);
  script_vcpu_onDocumentComplete(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url));
}


// VCPU

scriptVar ScriptBrowserWnd::script_vcpu_gotoUrl(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url) {
  SCRIPT_FUNCTION_INIT
  ASSERT(url.type == SCRIPT_STRING);
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
  if (sb) sb->navigateUrl(url.data.sdata);
  RETURN_SCRIPT_VOID;  
}

scriptVar ScriptBrowserWnd::script_vcpu_back(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
  if (sb) sb->back();
  RETURN_SCRIPT_VOID;  
}

scriptVar ScriptBrowserWnd::script_vcpu_forward(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
  if (sb) sb->forward();
  RETURN_SCRIPT_VOID;  
}

scriptVar ScriptBrowserWnd::script_vcpu_home(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
  if (sb) sb->home();
  RETURN_SCRIPT_VOID;  
}

scriptVar ScriptBrowserWnd::script_vcpu_refresh(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
  if (sb) sb->refresh();
  RETURN_SCRIPT_VOID;  
}

scriptVar ScriptBrowserWnd::script_vcpu_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
  if (sb) sb->stop();
  RETURN_SCRIPT_VOID;  
}

scriptVar ScriptBrowserWnd::script_vcpu_setTargetName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name) {
  SCRIPT_FUNCTION_INIT
  ScriptBrowserWnd *sb = static_cast<ScriptBrowserWnd*>(o->vcpu_getInterface(browserGuid));
  if (sb) sb->setTargetName(GET_SCRIPT_STRING(name));
  RETURN_SCRIPT_VOID;  
}

scriptVar ScriptBrowserWnd::script_vcpu_onBeforeNavigate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url, scriptVar flags, scriptVar framename) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS3(o, browserController, url, flags, framename);
  SCRIPT_FUNCTION_CHECKABORTEVENT;    
  SCRIPT_EXEC_EVENT3(o, url, flags, framename);
}

scriptVar ScriptBrowserWnd::script_vcpu_onDocumentComplete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, browserController, url);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, url);
}


