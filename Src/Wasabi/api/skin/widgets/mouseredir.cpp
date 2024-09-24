#include <precomp.h>
#include "mouseredir.h"

#include <api/skin/widgets/group.h>
#include <api/script/scriptmgr.h>
#include <api/script/objects/smap.h>
#include <api/script/objects/sregion.h>
const wchar_t mouseRedirXuiObjectStr[] = L"MouseRedir"; // This is the xml tag
char mouseRedirXuiSvcName[] = "MouseRedir xui object"; // this is the name of the xuiservice

XMLParamPair MouseRedir::params[] = {
                                        {MOUSEREDIR_TARGET, L"target"},
                                    };
MouseRedir::MouseRedir()
{
	getScriptObject()->vcpu_setInterface(mouseredirGuid, (void *)static_cast<MouseRedir *>(this));
	getScriptObject()->vcpu_setClassName(L"MouseRedir");
	getScriptObject()->vcpu_setController(mouseredirController);
	rgn = NULL;
	redirobject = NULL;
	getGuiObject()->guiobject_setClickThrough(0);
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
}

void MouseRedir::CreateXMLParameters(int master_handle)
{
	//MOUSEREDIR_PARENT::CreateXMLParameters(master_handle);
		int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_REQUIRED);
}

MouseRedir::~MouseRedir()
{
	delete rgn;
}

int MouseRedir::setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value)
{
	if (_xuihandle == xuihandle)
	{
		switch (xmlattributeid)
		{
		case MOUSEREDIR_TARGET:
			setTarget(value);
			return 1;
		}
	}
	return MOUSEREDIR_PARENT::setXuiParam(_xuihandle, xmlattributeid, xmlattributename, value);
}

int MouseRedir::onInit()
{
	MOUSEREDIR_PARENT::onInit();
	if (!deferedredirobjectid.isempty())
		doSetTarget(deferedredirobjectid);
	deferedredirobjectid = L"";
	return 1;
}

int MouseRedir::mouseInRegion(int x, int y)
{
	if (!rgn) return 1;
	RECT cr;
	getClientRect(&cr);
	POINT pt = {x - cr.left, y - cr.top};
	return rgn->ptInRegion(&pt);
}

void MouseRedir::setRedirection(GuiObject *o)
{
	redirobject = o;
}

GuiObject *MouseRedir::getRedirection()
{
	return redirobject;
}

void MouseRedir::setTarget(const wchar_t *id)
{
	if (!isInited())
		deferedredirobjectid = id;
	else
		doSetTarget(id);
}

void MouseRedir::doSetTarget(const wchar_t *id)
{
	Group *g = getGuiObject()->guiobject_getParentGroup();
	if (!g) return ;
	GuiObject *o = g->getObject(id);
	if (!o) return ;
	redirobject = o;
}

MouseRedirScriptController _mouseredirController;
MouseRedirScriptController *mouseredirController = &_mouseredirController;

// -- Functions table -------------------------------------
function_descriptor_struct MouseRedirScriptController::exportedFunction[] = {
            {L"setRegionFromMap", 1, (void*)MouseRedir::script_vcpu_setRegionFromMap},
            {L"setRegion", 1, (void*)MouseRedir::script_vcpu_setRegion },
            {L"setRedirection", 1, (void*)MouseRedir::script_vcpu_setRedirection},
            {L"getRedirection", 0, (void*)MouseRedir::script_vcpu_getRedirection},
        };
// --------------------------------------------------------


const wchar_t *MouseRedirScriptController::getClassName()
{
	return L"MouseRedir";
}

const wchar_t *MouseRedirScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *MouseRedirScriptController::instantiate()
{
	MouseRedir *m = new MouseRedir;
	if (!m) return NULL;
	return m->getScriptObject();
}

void MouseRedirScriptController::destroy(ScriptObject *o)
{
	MouseRedir *obj = static_cast<MouseRedir*>(o->vcpu_getInterface(mouseredirGuid));
	ASSERT(obj != NULL);
	delete obj;
}

void *MouseRedirScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for mouseredir yet
}

void MouseRedirScriptController::deencapsulate(void *)
{}


int MouseRedirScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *MouseRedirScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID MouseRedirScriptController::getClassGuid()
{
	return mouseredirGuid;
}

//---------------------------------------------------------------------------


void MouseRedir::setRegionFromMap(SMap *map, int byte, int inversed)
{
	if (rgn)
	{
		delete rgn;
		rgn = NULL;
	}
	rgn = new RegionI(map->getBitmap(), NULL, 0, 0, FALSE, 1, (unsigned char)byte, inversed);
}

void MouseRedir::setRegion(SRegion *reg)
{
	if (rgn) { delete rgn; rgn = NULL; }
	if (!reg) { invalidate(); return ; }
	rgn = new RegionI();
	rgn->addRegion(reg->getRegion());
}

ifc_window *MouseRedir::getForwardWnd()
{
	if (redirobject) return redirobject->guiobject_getRootWnd();
	return this;
}

scriptVar MouseRedir::script_vcpu_setRedirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj)
{
	SCRIPT_FUNCTION_INIT;
	MouseRedir *mr = static_cast<MouseRedir *>(o->vcpu_getInterface(mouseredirGuid));
	if (mr) mr->setRedirection(static_cast<GuiObject *>(GET_SCRIPT_OBJECT_AS(obj, guiObjectGuid)));
	RETURN_SCRIPT_VOID;
}

scriptVar MouseRedir::script_vcpu_getRedirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	MouseRedir *mr = static_cast<MouseRedir *>(o->vcpu_getInterface(mouseredirGuid));
	return MAKE_SCRIPT_OBJECT(mr ? mr ->getRedirection()->guiobject_getScriptObject() : NULL);
}

scriptVar MouseRedir::script_vcpu_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv)
{
	SCRIPT_FUNCTION_INIT;
	ASSERT(SOM::isNumeric(&byte));
	ASSERT(SOM::isNumeric(&inv));
	MouseRedir *mr = static_cast<MouseRedir *>(o->vcpu_getInterface(mouseredirGuid));
	SMap *sm = static_cast<SMap*>(GET_SCRIPT_OBJECT_AS(map, mapGuid));
	if (mr) mr->setRegionFromMap(sm, SOM::makeInt(&byte), SOM::makeInt(&inv));
	RETURN_SCRIPT_VOID;
}

scriptVar MouseRedir::script_vcpu_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar reg)
{
	SCRIPT_FUNCTION_INIT;
	MouseRedir *mr = static_cast<MouseRedir *>(o->vcpu_getInterface(mouseredirGuid));
	SRegion *r = static_cast<SRegion *>(GET_SCRIPT_OBJECT_AS(reg, regionGuid));
	if (mr) mr->setRegion(r);
	RETURN_SCRIPT_VOID;
}

