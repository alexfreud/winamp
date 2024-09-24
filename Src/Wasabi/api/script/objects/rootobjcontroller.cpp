#include "precomp.h"
//<?#include "<class data="implementationheader"/>"
#include "rootobjcontroller.h"
//?>

#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobjecti.h>

RootScriptObjectController _rootScriptObjectController;
RootScriptObjectController *rootScriptObjectController = &_rootScriptObjectController;

// -- Functions table -------------------------------------
function_descriptor_struct RootScriptObjectController::exportedFunction[] = {
  {L"getClassName",  0, (void*)RootObject_ScriptMethods::getClassName },
  {L"notify",        4, (void*)RootObject_ScriptMethods::notify },
  {L"onNotify",      4, (void*)RootObject_ScriptMethods::onNotify },
};

const wchar_t *RootScriptObjectController::getClassName() {
  return L"Object";
}

const wchar_t *RootScriptObjectController::getAncestorClassName() {
  return NULL;
}

int RootScriptObjectController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *RootScriptObjectController::getExportedFunctions() {
  return exportedFunction;
}

GUID RootScriptObjectController::getClassGuid() {
  return rootObjectGuid;
}

ScriptObject *RootScriptObjectController::instantiate() {
  RootObjectInstance *o = new RootObjectInstance;
  ASSERT(o != NULL);
  return o->getScriptObject();
}

void RootScriptObjectController::destroy(ScriptObject *o) {
  RootObjectInstance *obj = static_cast<RootObjectInstance *>(o->vcpu_getInterface(rootObjectGuid)); 
  ASSERT(obj != NULL);
  delete obj;
}

void RootScriptObjectController::deencapsulate(void *o) {
  RootObjectI *obj = static_cast<RootObjectI *>(o); 
  delete obj;
}

void *RootScriptObjectController::encapsulate(ScriptObject *o) {
  return new RootObjectI(o);
}

// -------------------------------------------------------------------------------------------------------------------------------------
// Script Methods and Events for RootObject (Object)
// -------------------------------------------------------------------------------------------------------------------------------------

scriptVar RootObject_ScriptMethods::getClassName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  return MAKE_SCRIPT_STRING(o->vcpu_getClassName());
} 

scriptVar RootObject_ScriptMethods::onNotify(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s, scriptVar s2, scriptVar i1, scriptVar i2) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS4(o, rootScriptObjectController, s, s2, i1, i2);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT4(o, s, s2, i1, i2);
}

scriptVar RootObject_ScriptMethods::notify(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s, scriptVar s2, scriptVar i1, scriptVar i2) {
  SCRIPT_FUNCTION_INIT
  RootObject *ro = static_cast<RootObject*>(o->vcpu_getInterface(rootObjectGuid));
  if (ro) ro->rootobject_notify(GET_SCRIPT_STRING(s), GET_SCRIPT_STRING(s2), GET_SCRIPT_INT(i1), GET_SCRIPT_INT(i2));
  RETURN_SCRIPT_VOID;
}
