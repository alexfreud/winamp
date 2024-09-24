#include <precomp.h>
#include "xuipathpicker.h"

// -----------------------------------------------------------------------
const wchar_t PathPickerXuiObjectStr[] = L"Wasabi:PathPicker"; 
char PathPickerXuiSvcName[] = "Wasabi:PathPicker xui object"; 

// -----------------------------------------------------------------------
ScriptPathPicker::ScriptPathPicker() {
  getScriptObject()->vcpu_setInterface(pathPickerGuid, (void *)static_cast<PathPicker *>(this));
  getScriptObject()->vcpu_setClassName(L"PathPicker"); // this is the script class name
  getScriptObject()->vcpu_setController(pathPickerController);
}

// -----------------------------------------------------------------------
ScriptPathPicker::~ScriptPathPicker() {
}

// -----------------------------------------------------------------------
// Script Object

PathPickerScriptController _pathPickerController;
PathPickerScriptController *pathPickerController = &_pathPickerController;

// -- Functions table -------------------------------------
function_descriptor_struct PathPickerScriptController::exportedFunction[] = {
  {L"getPath",            0, (void*)PathPickerScriptController::PathPicker_getPath},
  {L"onPathChanged",      1, (void*)PathPickerScriptController::PathPicker_onPathChanged},
};
                                      
ScriptObject *PathPickerScriptController::instantiate() {
  ScriptPathPicker *sddl = new ScriptPathPicker;
  ASSERT(sddl != NULL);
  return sddl->getScriptObject();
}

void PathPickerScriptController::destroy(ScriptObject *o) {
  ScriptPathPicker *sddl= static_cast<ScriptPathPicker *>(o->vcpu_getInterface(pathPickerGuid));
  ASSERT(sddl != NULL);
  delete sddl;
}

void *PathPickerScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation 
}

void PathPickerScriptController::deencapsulate(void *o) {
}

int PathPickerScriptController::getNumFunctions() { 
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *PathPickerScriptController::getExportedFunctions() { 
  return exportedFunction; 
}


scriptVar PathPickerScriptController::PathPicker_getPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptPathPicker *sddl = static_cast<ScriptPathPicker*>(o->vcpu_getInterface(pathPickerGuid));
  const wchar_t *p=L"";
  if (sddl) p = sddl->getPath();
  return MAKE_SCRIPT_STRING(p);
}

//   PathPickerScriptController::PathPicker_onNewPath(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(newpath));
scriptVar PathPickerScriptController::PathPicker_onPathChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newpath) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, pathPickerController, newpath);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newpath);
}
