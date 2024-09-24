#include <precomp.h>
#include <api/script/objects/spopup.h>
#include <api/script/scriptmgr.h>

SPopup::SPopup() {
	getScriptObject()->vcpu_setInterface(popupGuid, (void *)static_cast<SPopup*>(this));
	getScriptObject()->vcpu_setClassName(L"Popup");
	getScriptObject()->vcpu_setController(popupController);
}

SPopup::~SPopup() {
}

PopupScriptController _popupController;
PopupScriptController *popupController = &_popupController;

// -- Functions table -------------------------------------
function_descriptor_struct PopupScriptController::exportedFunction[] = {
  {L"addSubMenu",      2, (void*)SPopup::script_vcpu_addSubMenu },
  {L"addCommand",      4, (void*)SPopup::script_vcpu_addCommand },
  {L"addSeparator",    0, (void*)SPopup::script_vcpu_addSeparator },
  {L"popAtXY",         2, (void*)SPopup::script_vcpu_popAtXY },
  {L"popAtMouse",      0, (void*)SPopup::script_vcpu_popAtMouse },
  {L"getNumCommands",  0, (void*)SPopup::script_vcpu_getNumCommands },
  {L"checkCommand",    2, (void*)SPopup::script_vcpu_checkCommand },
  {L"disableCommand",  2, (void*)SPopup::script_vcpu_disableCommand },

//todo: events
};

// --------------------------------------------------------
const wchar_t *PopupScriptController::getClassName() {
	return L"Popup";
}

const wchar_t *PopupScriptController::getAncestorClassName() {
	return L"Object";
}

ScriptObject *PopupScriptController::instantiate() {
	SPopup *p = new SPopup;
	ASSERT(p != NULL);
	return p->getScriptObject();
}

void PopupScriptController::destroy(ScriptObject *o) { 
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	ASSERT(p != NULL);
	delete p;
}

void *PopupScriptController::encapsulate(ScriptObject *o) {
	return NULL; // no encapsulation for popups yet
}

void PopupScriptController::deencapsulate(void *o) {
}

int PopupScriptController::getNumFunctions() {
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *PopupScriptController::getExportedFunctions() {
	return exportedFunction;                                                        
}

GUID PopupScriptController::getClassGuid() {
	return popupGuid;
}

// -----------------------------------------------------------------------
scriptVar SPopup::script_vcpu_addSubMenu(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar popup, scriptVar str) 
{
	SCRIPT_FUNCTION_INIT; 
	ASSERT(str.type == SCRIPT_STRING); // compiler discarded
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	SPopup *p2 = static_cast<SPopup *>(GET_SCRIPT_OBJECT_AS(popup, popupGuid));
	if (p) p->addSubMenu(p2, GET_SCRIPT_STRING(str));
	RETURN_SCRIPT_VOID;
}

scriptVar SPopup::script_vcpu_addCommand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str, scriptVar cmd, scriptVar checked, scriptVar disabled) {
	SCRIPT_FUNCTION_INIT; 
	ASSERT(str.type == SCRIPT_STRING); // compiler discarded
	ASSERT(SOM::isNumeric(&cmd)); // compiler discarded
	ASSERT(SOM::isNumeric(&checked)); // compiler discarded
	ASSERT(SOM::isNumeric(&disabled)); // compiler discarded
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	if (p)
		p->addCommand(str.data.sdata, SOM::makeInt(&cmd), SOM::makeBoolean(&checked), SOM::makeBoolean(&disabled));
	RETURN_SCRIPT_VOID;
}

scriptVar SPopup::script_vcpu_addSeparator(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT; 
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	if (p) p->addSeparator();
	RETURN_SCRIPT_VOID;
}

scriptVar SPopup::script_vcpu_checkCommand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i, scriptVar check) {
	SCRIPT_FUNCTION_INIT; 
	ASSERT(SOM::isNumeric(&i));
	ASSERT(SOM::isNumeric(&check));
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	if (p) p->checkCommand(SOM::makeInt(&i), SOM::makeBoolean(&check));
	RETURN_SCRIPT_VOID;
}

scriptVar SPopup::script_vcpu_disableCommand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i, scriptVar disable) {
	SCRIPT_FUNCTION_INIT; 
	ASSERT(SOM::isNumeric(&i));
	ASSERT(SOM::isNumeric(&disable));
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	if (p) p->disableCommand(SOM::makeInt(&i), SOM::makeBoolean(&disable));
	RETURN_SCRIPT_VOID;
}

scriptVar SPopup::script_vcpu_popAtXY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
	SCRIPT_FUNCTION_INIT; 
	ASSERT(SOM::isNumeric(&x));
	ASSERT(SOM::isNumeric(&y));
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	if (p) return MAKE_SCRIPT_INT(p->popAtXY(SOM::makeInt(&x), SOM::makeInt(&y)));
	RETURN_SCRIPT_ZERO;
}

scriptVar SPopup::script_vcpu_popAtMouse(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT; 
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	if (p) return MAKE_SCRIPT_INT(p->popAtMouse());
	RETURN_SCRIPT_ZERO;
}

scriptVar SPopup::script_vcpu_getNumCommands(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT; 
	SPopup *p = static_cast<SPopup *>(o->vcpu_getInterface(popupGuid));
	if (p) return MAKE_SCRIPT_INT(p->getNumCommands());
	RETURN_SCRIPT_ZERO;
}
