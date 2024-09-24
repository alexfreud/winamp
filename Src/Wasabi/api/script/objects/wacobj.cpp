#include <precomp.h>
#include <bfc/wasabi_std.h>
#include <bfc/nsguid.h>
#include <api/script/script.h>
#include <api/script/scriptobj.h>
#include <api/script/scriptmgr.h>
#include <api/script/objects/wacobj.h>

#ifdef _WASABIRUNTIME
#include <api/api.h>
#include <api/wac/compon.h>
#endif

WacScriptController _wacController;
WacScriptController *wacController = &_wacController;

// -- Functions table -------------------------------------
function_descriptor_struct WacScriptController::exportedFunction[] = {
  {L"getGUID",        1, (void*)WACObject::script_getGUID },
  {L"getName",        1, (void*)WACObject::script_getName },
  {L"onNotify",       3, (void*)WACObject::script_vcpu_onNotify },

// DEPRECATED! DO NOTHING
  {L"sendCommand",    4, (void*)WACObject::script_vcpu_dummy4},
  {L"show",           0, (void*)WACObject::script_vcpu_dummy0},
  {L"hide",           0, (void*)WACObject::script_vcpu_dummy0},
  {L"isVisible",      0, (void*)WACObject::script_vcpu_dummy0},
  {L"onShow",         0, (void*)WACObject::script_vcpu_dummy0},
  {L"onHide",         0, (void*)WACObject::script_vcpu_dummy0},
  {L"setStatusBar",   1, (void*)WACObject::script_vcpu_dummy1},
  {L"getStatusBar",   0, (void*)WACObject::script_vcpu_dummy0},
};
// --------------------------------------------------------

const wchar_t *WacScriptController::getClassName() {
  return L"Wac";
}

const wchar_t *WacScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *WacScriptController::getAncestorController() { return rootScriptObjectController; }

int WacScriptController::getInstantiable() {
  return 0;
}

int WacScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *WacScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID WacScriptController::getClassGuid() {
  return wacGuid;
}

ScriptObject *WacScriptController::instantiate() {
  return NULL;
}

void WacScriptController::destroy(ScriptObject *o) {
}

void *WacScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for wacobject yet
}
 
void WacScriptController::deencapsulate(void *o ) {
}

WACObject::WACObject() {
  getScriptObject()->vcpu_setInterface(wacGuid, (void *)static_cast<WACObject *>(this));
  getScriptObject()->vcpu_setClassName(L"Wac");
  getScriptObject()->vcpu_setController(wacController);
  wacobjs.addItem(this);
}

WACObject::~WACObject() {
  wacobjs.delItem(this);
}

void WACObject::setGUID(GUID g) {
  myGUID = g;
}

GUID WACObject::getGUID(void) {
  return myGUID;
}

int WACObject::onScriptNotify(const wchar_t *s, int i1, int i2) 
{
  scriptVar _i1 = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_i1, i1);
  scriptVar _i2 = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_i2, i2);
  scriptVar _str = SOM::makeVar(SCRIPT_STRING);
  SOM::assign(&_str, (wchar_t *)s);
  script_vcpu_onNotify(SCRIPT_CALL, getScriptObject(), _str, _i1, _i2);
  return 1;
}

int WACObject::notifyScripts(WaComponent *comp, const wchar_t *s, int i1, int i2) 
{
  for (int i=0;i<wacobjs.getNumItems();i++) 
	{
    GUID g = comp->getGUID();
    if (!MEMCMP(&g, &wacobjs.enumItem(i)->myGUID, sizeof(GUID)))
      return wacobjs.enumItem(i)->onScriptNotify(s, i1, i2);
  }
  return 0;
}

scriptVar WACObject::script_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) 
{
  SCRIPT_FUNCTION_INIT; 
  static wchar_t guid[nsGUID::GUID_STRLEN+1];
  WACObject *wo = static_cast<WACObject *>(o->vcpu_getInterface(wacGuid));
  *guid=0;
  if (wo) 
		nsGUID::toCharW(wo->myGUID, guid);
  return MAKE_SCRIPT_STRING(guid);
}

scriptVar WACObject::script_getName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
#ifdef WASABI_COMPILE_COMPONENTS
  WACObject *wo = static_cast<WACObject *>(o->vcpu_getInterface(wacGuid));
  if (wo) return MAKE_SCRIPT_STRING(WASABI_API_COMPONENT->getComponentName(wo->myGUID));
#endif
  return MAKE_SCRIPT_STRING(L"");
}

scriptVar WACObject::script_vcpu_onNotify(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str, scriptVar i1, scriptVar i2) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS3(o, wacController, str, i1, i2);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT3(o, str, i1, i2);
}

scriptVar WACObject::script_vcpu_dummy4(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a, scriptVar b, scriptVar c, scriptVar d) {
  SCRIPT_FUNCTION_INIT; 
  RETURN_SCRIPT_ZERO;
}

scriptVar WACObject::script_vcpu_dummy1(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a) {
  SCRIPT_FUNCTION_INIT; 
  RETURN_SCRIPT_ZERO;
}

scriptVar WACObject::script_vcpu_dummy0(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  RETURN_SCRIPT_ZERO;
}


PtrList<WACObject> WACObject::wacobjs;

