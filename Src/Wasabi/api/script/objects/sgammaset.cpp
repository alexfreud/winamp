#include <precomp.h>
#include "SGammaset.h"

#include <api.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objecttable.h>

// {0D024DB9-9574-42d0-B8C7-26B553F1F987}
static const GUID gammasetSoGuid = 
{ 0xd024db9, 0x9574, 0x42d0, { 0xb8, 0xc7, 0x26, 0xb5, 0x53, 0xf1, 0xf9, 0x87 } };

GammasetScriptController _gammasetController;
GammasetScriptController *gammasetController=&_gammasetController;

// -- Functions table -------------------------------------
function_descriptor_struct GammasetScriptController::exportedFunction[] = {
	{L"apply",	0, (void*)SGammaset::script_vcpu_apply },
	{L"getID",	0, (void*)SGammaset::script_vcpu_getID },
	{L"rename",	1, (void*)SGammaset::script_vcpu_rename },
	{L"remove",	0, (void*)SGammaset::script_vcpu_update },
	{L"delete",	0, (void*)SGammaset::script_vcpu_delete },
	{L"getGeneralGroup",	0, (void*)SGammaset::script_vcpu_getDefaultGammaGroup },
	{L"getGammaGroup",	1, (void*)SGammaset::script_vcpu_getGammaGroup },
	{L"getNumGammaGroups",	0, (void*)SGammaset::script_vcpu_getNumGammaGroups },
	{L"enumGammaGroup",	0, (void*)SGammaset::script_vcpu_enumGammaGroup },
};
// --------------------------------------------------------

const wchar_t *GammasetScriptController::getClassName() {
	return L"Gammaset";
}

const wchar_t *GammasetScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *GammasetScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *GammasetScriptController::instantiate() {
  SGammaset *xd = new SGammaset;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void GammasetScriptController::destroy(ScriptObject *o) {
  SGammaset *xd = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
  ASSERT(xd != NULL);
  delete xd;
}

void *GammasetScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for now
}

void GammasetScriptController::deencapsulate(void *o) {
}

int GammasetScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *GammasetScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID GammasetScriptController::getClassGuid() {
	return gammasetSoGuid;
}

void SGammaset::__construct()
{
  getScriptObject()->vcpu_setInterface(gammasetSoGuid, (void *)static_cast<SGammaset *>(this));
  getScriptObject()->vcpu_setClassName(L"Gammaset");
  getScriptObject()->vcpu_setController(gammasetController);
  this->gammasetID = NULL;
}

SGammaset::SGammaset()
{
	this->__construct();
}

SGammaset::SGammaset(const wchar_t * gammaSetID)
{
	this->__construct();
	this->gammasetID = gammaSetID;
}

SGammaset::SGammaset(int n)
{
	this->__construct();
	this->gammasetID = WASABI_API_COLORTHEMES->enumGammaSet(n);
}

SGammaset::~SGammaset()
{
}

// VCPU

scriptVar SGammaset::script_vcpu_apply(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{
		WASABI_API_SKIN->colortheme_setColorSet(m->gammasetID);
		//WASABI_API_COLORTHEMES->setGammaSet(m->gammasetID);
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammaset::script_vcpu_update(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{
		WASABI_API_COLORTHEMES->updateGammaSet(m->gammasetID);
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammaset::script_vcpu_delete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{
		WASABI_API_COLORTHEMES->deleteGammaSet(m->gammasetID);
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammaset::script_vcpu_rename(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name) {
	SCRIPT_FUNCTION_INIT;
	ASSERT(name.type == SCRIPT_STRING);
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{
		WASABI_API_COLORTHEMES->renameGammaSet(m->gammasetID, name.data.sdata);
	}
	RETURN_SCRIPT_VOID;
}

scriptVar SGammaset::script_vcpu_getID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_STRING(m->gammasetID);
	}
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar SGammaset::script_vcpu_getDefaultGammaGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	ScriptObject *s = NULL;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{	
		SGammagroup *gg = new SGammagroup(m->gammasetID, -1);
		if (gg != NULL && gg->grp != NULL) s = gg->getScriptObject();
	}
	return MAKE_SCRIPT_OBJECT(s);
}

scriptVar SGammaset::script_vcpu_getGammaGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name) {
	SCRIPT_FUNCTION_INIT;
	ASSERT(name.type == SCRIPT_STRING);
	ScriptObject *s = NULL;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{	
		SGammagroup *gg = new SGammagroup(m->gammasetID, GET_SCRIPT_STRING(name));
		if (gg != NULL) s = gg->getScriptObject();
	}
	return MAKE_SCRIPT_OBJECT(s);
}

scriptVar SGammaset::script_vcpu_enumGammaGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n) {
	SCRIPT_FUNCTION_INIT;
	ASSERT(n.type == SCRIPT_INT);
	ScriptObject *s = NULL;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{	
		SGammagroup *gg = new SGammagroup(m->gammasetID, GET_SCRIPT_INT(n));
		if (gg != NULL) s = gg->getScriptObject();
	}
	return MAKE_SCRIPT_OBJECT(s);
}

scriptVar SGammaset::script_vcpu_getNumGammaGroups(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	int n = 0;
	SGammaset *m = static_cast<SGammaset *>(o->vcpu_getInterface(gammasetSoGuid));
	if (m)
	{	
		n = WASABI_API_COLORTHEMES->getNumGammaGroups(m->gammasetID);
	}
	return MAKE_SCRIPT_INT(n);
}