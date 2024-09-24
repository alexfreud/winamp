#include <precomp.h>
#include "SGammagroup.h"

#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objecttable.h>

// {B81F004D-ACBA-453d-A06B-30192A1DA17D}
static const GUID gammagroupSoGuid = 
{ 0xb81f004d, 0xacba, 0x453d, { 0xa0, 0x6b, 0x30, 0x19, 0x2a, 0x1d, 0xa1, 0x7d } };


GammagroupScriptController _gammagroupController;
GammagroupScriptController *gammagroupController=&_gammagroupController;

// -- Functions table -------------------------------------
function_descriptor_struct GammagroupScriptController::exportedFunction[] = {
	{L"getRed",			0, (void*)SGammagroup::script_vcpu_getRed },
	{L"getGreen",		0, (void*)SGammagroup::script_vcpu_getGreen },
	{L"getBlue",		0, (void*)SGammagroup::script_vcpu_getBlue },
	{L"getBoost",		0, (void*)SGammagroup::script_vcpu_getBoost },
	{L"getGray",		0, (void*)SGammagroup::script_vcpu_getGray },
	{L"setRed",			1, (void*)SGammagroup::script_vcpu_setRed },
	{L"setGreen",		1, (void*)SGammagroup::script_vcpu_setGreen },
	{L"setBlue",		1, (void*)SGammagroup::script_vcpu_setBlue },
	{L"setBoost",		1, (void*)SGammagroup::script_vcpu_setBoost },
	{L"setGray",		1, (void*)SGammagroup::script_vcpu_setGray },
	{L"getID",			0, (void*)SGammagroup::script_vcpu_getID },
	{L"setID",			1, (void*)SGammagroup::script_vcpu_setID },
};
// --------------------------------------------------------

const wchar_t *GammagroupScriptController::getClassName() {
	return L"Gammagroup";
}

const wchar_t *GammagroupScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *GammagroupScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *GammagroupScriptController::instantiate() {
  SGammagroup *xd = new SGammagroup;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void GammagroupScriptController::destroy(ScriptObject *o) {
  SGammagroup *xd = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
  ASSERT(xd != NULL);
  delete xd;
}

void *GammagroupScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for now
}

void GammagroupScriptController::deencapsulate(void *o) {
}

int GammagroupScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *GammagroupScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID GammagroupScriptController::getClassGuid() {
	return gammagroupSoGuid;
}

void SGammagroup::__construct()
{
  getScriptObject()->vcpu_setInterface(gammagroupSoGuid, (void *)static_cast<SGammagroup *>(this));
  getScriptObject()->vcpu_setClassName(L"Gammagroup");
  getScriptObject()->vcpu_setController(gammagroupController);
  this->grp = NULL;
}

SGammagroup::SGammagroup()
{
	this->__construct();
}

SGammagroup::SGammagroup(const wchar_t * parentSet, const wchar_t * grp)
{
	this->__construct();
	this->parentSet = parentSet;
	this->grp = WASABI_API_COLORTHEMES->getColorThemeGroup(parentSet, grp);
}

SGammagroup::SGammagroup(const wchar_t * parentSet, int n)
{
	this->__construct();
	this->parentSet = parentSet;
	for (int i = 0; i < (int)WASABI_API_COLORTHEMES->getNumGammaSets(); i++)
	{
		if(WCSICMPSAFE(parentSet, WASABI_API_COLORTHEMES->enumGammaSet(i)))
		{
			continue;
		}

		this->grp = WASABI_API_COLORTHEMES->enumColorThemeGroup(i, n);
		break;
	}
}

SGammagroup::~SGammagroup()
{
}

// VCPU


scriptVar SGammagroup::script_vcpu_getID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_STRING(m->grp->getName());
	}
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar SGammagroup::script_vcpu_getBlue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->grp->getBlue());
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SGammagroup::script_vcpu_getGreen(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->grp->getGreen());
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SGammagroup::script_vcpu_getRed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->grp->getRed());
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SGammagroup::script_vcpu_getBoost(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->grp->getBoost());
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SGammagroup::script_vcpu_getGray(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->grp->getGray());
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SGammagroup::script_vcpu_setID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		(m->grp->setName(color.data.sdata));
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammagroup::script_vcpu_setBlue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		(m->grp->setBlue(color.data.idata));
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammagroup::script_vcpu_setGreen(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		(m->grp->setGreen(color.data.idata));
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammagroup::script_vcpu_setRed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		(m->grp->setRed(color.data.idata));
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammagroup::script_vcpu_setBoost(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		(m->grp->setBoost(color.data.idata));
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammagroup::script_vcpu_setGray(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color) {
	SCRIPT_FUNCTION_INIT;
	SGammagroup *m = static_cast<SGammagroup *>(o->vcpu_getInterface(gammagroupSoGuid));
	if (m)
	{
		(m->grp->setGray(color.data.idata));
	}
	RETURN_SCRIPT_VOID;
}