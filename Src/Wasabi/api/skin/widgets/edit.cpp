#include <precomp.h>
#include <bfc/wasabi_std.h>
#include <api/skin/skinparse.h>
#include "edit.h"
#include <api/script/scriptmgr.h>
#include <api/skin/widgets/mb/xuibrowser.h>
#include <api/skin/widgets/mb/mainminibrowser.h>

#define BUFSIZE 0x7ffe

const wchar_t editXuiObjectStr[] = L"Edit"; // This is the xml tag
char editXuiSvcName[] = "Edit xui object"; // this is the name of the xuiservice


EditScriptController _editController;
EditScriptController *editController = &_editController;

// -- Functions table -------------------------------------
function_descriptor_struct EditScriptController::exportedFunction[] = {
            {L"setText", 1, (void*)Edit::script_vcpu_setText },
            {L"setAutoEnter", 1, (void*)Edit::script_vcpu_setAutoEnter },
            {L"getAutoEnter", 0, (void*)Edit::script_vcpu_getAutoEnter },
            {L"getText", 0, (void*)Edit::script_vcpu_getText },
            {L"onEnter", 0, (void*)Edit::script_vcpu_onEnter },
            {L"onAbort", 0, (void*)Edit::script_vcpu_onAbort },
            {L"onIdleEditUpdate", 0, (void*)Edit::script_vcpu_onIdleEditUpdate },
            {L"onEditUpdate", 0, (void*)Edit::script_vcpu_onEditUpdate },
            {L"selectAll", 0, (void*)Edit::script_vcpu_selectAll },
            {L"enter", 0, (void*)Edit::script_vcpu_enter },
            {L"setIdleEnabled", 1, (void*)Edit::script_vcpu_setIdleEnabled},
            {L"getIdleEnabled", 0, (void*)Edit::script_vcpu_getIdleEnabled},
        };
// --------------------------------------------------------

const wchar_t *EditScriptController::getClassName()
{
	return L"Edit";
}

const wchar_t *EditScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *EditScriptController::instantiate()
{
	Edit *e = new Edit;
	ASSERT(e != NULL);
	return e->getScriptObject();
}

void EditScriptController::destroy(ScriptObject *o)
{
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	ASSERT(e != NULL);
	delete e;
}

void *EditScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for edit yet
}

void EditScriptController::deencapsulate(void *o)
{}

int EditScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *EditScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID EditScriptController::getClassGuid()
{
	return editGuid;
}

// -----------------------------------------------------------------------------

XMLParamPair Edit::params[] = {
                                  {EDIT_ACTION, L"ACTION"},
                                  {EDIT_AUTOENTER, L"AUTOENTER"},
                                  {EDIT_AUTOHSCROLL, L"AUTOHSCROLL"},
                                  {EDIT_AUTOSELECT, L"AUTOSELECT"},
                                  {EDIT_MULTILINE, L"MULTILINE"},
                                  {EDIT_PASSWORD, L"PASSWORD"},
                                  {EDIT_TEXT, L"TEXT"},
                                  {EDIT_VSCROLL, L"VSCROLL"},
                              };
Edit::Edit()
{
	getScriptObject()->vcpu_setInterface(editGuid, (void *)static_cast<Edit *>(this));
	getScriptObject()->vcpu_setClassName(L"Edit");
	getScriptObject()->vcpu_setController(editController);
	my_buffer = WMALLOC(BUFSIZE);
	*my_buffer = 0;
	autourl = 0;
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	
}

void Edit::CreateXMLParameters(int master_handle)
{
	//EDIT_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

Edit::~Edit()
{
	FREE(my_buffer);
}

int Edit::setXuiParam(int _xuihandle, int xmlattrid, const wchar_t *name, const wchar_t *value)
{
	if (xuihandle == _xuihandle)
	{
		switch (xmlattrid)
		{
		case EDIT_TEXT: setText(value); return 1;
		case EDIT_ACTION: if (SkinParser::getAction(value) == ACTION_MB_URL) setAutoUrl(1); return 1;
		case EDIT_MULTILINE: setMultiline(WTOI(value)); return 1;
		case EDIT_VSCROLL: setVScroll(WTOI(value)); return 1;
		case EDIT_AUTOHSCROLL: setAutoHScroll(WTOI(value)); return 1;
		case EDIT_AUTOENTER: setAutoEnter(WTOI(value)); return 1;
		case EDIT_PASSWORD: setPassword(WTOI(value)); return 1;
		case EDIT_AUTOSELECT: setAutoSelect(WTOI(value)); return 1;
		}
	}
	return EDIT_PARENT::setXuiParam(_xuihandle, xmlattrid, name, value);
}

int Edit::onInit()
{
	int r = EDIT_PARENT::onInit();
	setBuffer(my_buffer, BUFSIZE - 1); 
	my_buffer[BUFSIZE - 1] = 0;
	return r;
}

void Edit::onEditUpdate()
{
	EDIT_PARENT::onEditUpdate();
	script_vcpu_onEditUpdate(SCRIPT_CALL, getScriptObject());
}

void Edit::onIdleEditUpdate()
{
	EDIT_PARENT::onIdleEditUpdate();
	script_vcpu_onIdleEditUpdate(SCRIPT_CALL, getScriptObject());
}

int Edit::onEnter()
{
	if (autourl)
	{
		MainMiniBrowser::navigateUrl(my_buffer);
	}
#ifdef WASABI_COMPILE_CONFIG
	getGuiObject()->guiobject_setCfgString(my_buffer);
#endif
	int r = EDIT_PARENT::onEnter();
#ifdef WASABI_COMPILE_CONFIG
	script_vcpu_onEnter(SCRIPT_CALL, getScriptObject());
#endif
	return r;
}

#ifdef WASABI_COMPILE_CONFIG
int Edit::onReloadConfig()
{
	EDIT_PARENT::onReloadConfig();
	setText(getGuiObject()->guiobject_getCfgString());
	return 1;
}
#endif

int Edit::onAbort()
{
	if (autourl)
	{
		ScriptObject *so = MainMiniBrowser::getScriptObject();
		if (so)
		{
			ScriptBrowserWnd *sbw = static_cast<ScriptBrowserWnd *>(so->vcpu_getInterface(browserGuid));
			if (sbw)
				setText(sbw->getCurrentUrl());
		}
	}
	int r = EDIT_PARENT::onAbort();
	script_vcpu_onAbort(SCRIPT_CALL, getScriptObject());
	return r;
}

void Edit::setText(const wchar_t *t)
{
	wcsncpy(my_buffer, t, BUFSIZE - 1);
	setBuffer(my_buffer, BUFSIZE - 1);
	my_buffer[BUFSIZE - 1] = 0;
}

void Edit::setAutoUrl(int a)
{
	autourl = a;
}

// -----------------------------------------------------------------------------

scriptVar Edit::script_vcpu_setText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(t.type == SCRIPT_STRING);
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) e->setText(t.data.sdata);
	RETURN_SCRIPT_VOID;
}

scriptVar Edit::script_vcpu_setAutoEnter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t)
{
	SCRIPT_FUNCTION_INIT
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) e->setAutoEnter(t.data.idata);
	RETURN_SCRIPT_VOID;
}

scriptVar Edit::script_vcpu_getAutoEnter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	int a = 0;
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) a = e->getAutoEnter();
	return MAKE_SCRIPT_INT(a);
}

scriptVar Edit::script_vcpu_setIdleEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t)
{
	SCRIPT_FUNCTION_INIT
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) e->setIdleEnabled(t.data.idata);
	RETURN_SCRIPT_VOID;
}

scriptVar Edit::script_vcpu_getIdleEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	int a = 0;
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) a = e->getIdleEnabled();
	return MAKE_SCRIPT_INT(a);
}

scriptVar Edit::script_vcpu_getText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) return MAKE_SCRIPT_STRING(e->getBufferPtr());
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar Edit::script_vcpu_selectAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) e->selectAll();
	RETURN_SCRIPT_VOID;
}

scriptVar Edit::script_vcpu_enter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Edit *e = static_cast<Edit *>(o->vcpu_getInterface(editGuid));
	if (e) e->enter();
	RETURN_SCRIPT_VOID;
}

scriptVar Edit::script_vcpu_onEnter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, editController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Edit::script_vcpu_onIdleEditUpdate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, editController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Edit::script_vcpu_onEditUpdate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, editController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Edit::script_vcpu_onAbort(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, editController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}