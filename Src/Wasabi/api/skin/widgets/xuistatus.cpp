#include <precomp.h>
#include "xuistatus.h"

const wchar_t StatusXuiObjectStr[] = L"LayoutStatus"; // This is the xml tag
char StatusXuiSvcName[] = "Status xui object";

LayoutStatusController _layoutStatusController;
LayoutStatusController *layoutStatusController = &_layoutStatusController;


XMLParamPair XuiStatus::params[] = {
  {EXCLUDE, L"EXCLUDE"},
  {INCLUDE, L"INCLUDEONLY"},
	};

XuiStatus::XuiStatus() {
  getScriptObject()->vcpu_setInterface(layoutStatusGuid, (void *)static_cast<XuiStatus *>(this));
  getScriptObject()->vcpu_setClassName(L"LayoutStatus"); // this is the script class name
  getScriptObject()->vcpu_setController(layoutStatusController);

  myxuihandle = newXuiHandle();
CreateXMLParameters(myxuihandle);	
}

void XuiStatus::CreateXMLParameters(int master_handle)
{
	//XUISTATUS_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

int XuiStatus::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return XUISTATUS_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);
  switch (xmlattributeid) {
    case EXCLUDE:
      setExclude(value);
    break;
    case INCLUDE:
      setIncludeOnly(value);
    break;
    default:
      return 0;
  }
  return 1;
}

// -----------------------------------------------------------------------
void XuiStatus::callme(const wchar_t *str) 
{
  fakeButtonPush(str);
}

// -----------------------------------------------------------------------
// Script Object

// -- Functions table -------------------------------------
function_descriptor_struct LayoutStatusController::exportedFunction[] = {
  {L"callme",             1, (void*)LayoutStatusController::layoutstatus_callme },
};

ScriptObject *LayoutStatusController::instantiate() {
  XuiStatus *xs = new XuiStatus;
  ASSERT(xs != NULL);
  return xs->getScriptObject();
}

void LayoutStatusController::destroy(ScriptObject *o) {
  XuiStatus *xs = static_cast<XuiStatus *>(o->vcpu_getInterface(layoutStatusGuid));
  ASSERT(xs!= NULL);
  delete xs;
}

void *LayoutStatusController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for layoutstatus yet
}

void LayoutStatusController::deencapsulate(void *o) {
}

int LayoutStatusController::getNumFunctions() { 
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *LayoutStatusController::getExportedFunctions() { 
  return exportedFunction; 
}


scriptVar LayoutStatusController::layoutstatus_callme(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str) {
  SCRIPT_FUNCTION_INIT
  XuiStatus *xs = static_cast<XuiStatus *>(o->vcpu_getInterface(layoutStatusGuid));
  if (xs) xs->callme(GET_SCRIPT_STRING(str));
  RETURN_SCRIPT_VOID;  
}

