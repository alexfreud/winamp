#include <precomp.h>
#include "SColorMgr.h"
#include <api.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objecttable.h>


// {AEE235FF-EBD1-498f-96AF-D7E0DAD4541A}
static const GUID colorMgrSoGuid = 
{ 0xaee235ff, 0xebd1, 0x498f, { 0x96, 0xaf, 0xd7, 0xe0, 0xda, 0xd4, 0x54, 0x1a } };

ColorMgrScriptController _colorMgrController;
ColorMgrScriptController *colorMgrController=&_colorMgrController;

// -- Functions table -------------------------------------
function_descriptor_struct ColorMgrScriptController::exportedFunction[] = {
	{L"getColor",			1, (void*)SColorMgr::script_vcpu_getColor },
	{L"enumColor",			1, (void*)SColorMgr::script_vcpu_enumColor },
	{L"getNumColors",		0, (void*)SColorMgr::script_vcpu_getNumColors },
	{L"getGammaSet",		1, (void*)SColorMgr::script_vcpu_getGammaSet },
	{L"getCurrentGammaSet",	0, (void*)SColorMgr::script_vcpu_getCurrentGammaSet },	
	{L"enumGammaSet",		1, (void*)SColorMgr::script_vcpu_enumGammaSet },
	{L"getNumGammaSets",	0, (void*)SColorMgr::script_vcpu_getNumGammaSets },
	{L"onGuiLoaded",		0, (void*)SColorMgr::script_vcpu_onGuiLoaded },
	{L"onLoaded",			0, (void*)SColorMgr::script_vcpu_onLoaded },
	{L"newGammaSet",		1, (void*)SColorMgr::script_vcpu_newGammaSet },
	{L"onBeforeLoadingElements",	0, (void*)SColorMgr::script_vcpu_onBeforeLoadingElements },
	{L"onColorThemeChanged",		1, (void*)SColorMgr::script_vcpu_onColorThemeChanged },
	{L"onColorThemesListChanged",	0, (void*)SColorMgr::script_vcpu_onColorThemesListChanged },
};
// --------------------------------------------------------

const wchar_t *ColorMgrScriptController::getClassName() {
	return L"ColorMgr";
}

const wchar_t *ColorMgrScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *ColorMgrScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *ColorMgrScriptController::instantiate() {
  SColorMgr *xd = new SColorMgr;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void ColorMgrScriptController::destroy(ScriptObject *o) {
  SColorMgr *xd = static_cast<SColorMgr *>(o->vcpu_getInterface(colorMgrSoGuid));
  ASSERT(xd != NULL);
  delete xd;
}

void *ColorMgrScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for now
}

void ColorMgrScriptController::deencapsulate(void *o) {
}

int ColorMgrScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *ColorMgrScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID ColorMgrScriptController::getClassGuid() {
	return colorMgrSoGuid;
}

SColorMgr::SColorMgr()
{
	getScriptObject()->vcpu_setInterface(colorMgrSoGuid, (void *)static_cast<SColorMgr *>(this));
	getScriptObject()->vcpu_setClassName(L"ColorMgr");
	getScriptObject()->vcpu_setController(colorMgrController);
	WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI*>(this));
}

SColorMgr::~SColorMgr()
{
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI*>(this));
}

int SColorMgr::skincb_onGuiLoaded ()
{
	script_vcpu_onGuiLoaded(SCRIPT_CALL, this->getScriptObject());
	return 0;
}

int SColorMgr::skincb_onLoaded ()
{
	script_vcpu_onLoaded(SCRIPT_CALL, this->getScriptObject());
	return 0;
}

int SColorMgr::skincb_onBeforeLoadingElements ()
{
	script_vcpu_onBeforeLoadingElements(SCRIPT_CALL, this->getScriptObject());
	return 0;
}

int SColorMgr::skincb_onColorThemeChanged (const wchar_t* newcolortheme)
{
	script_vcpu_onColorThemeChanged(SCRIPT_CALL, this->getScriptObject(), MAKE_SCRIPT_STRING(newcolortheme));
	return 0;
}


int SColorMgr::skincb_onColorThemesListChanged()
{
	script_vcpu_onColorThemesListChanged(SCRIPT_CALL, this->getScriptObject());
	return 0;
}


// VCPU

scriptVar SColorMgr::script_vcpu_getColor(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar colorID)
{
	SCRIPT_FUNCTION_INIT; 
	ASSERT(colorID.type == SCRIPT_STRING);
	ScriptObject *s = NULL;
	SColor *sc = new SColor(colorID.data.sdata);
	if (sc != NULL) s = sc->getScriptObject();
	return MAKE_SCRIPT_OBJECT(s); 
}

scriptVar SColorMgr::script_vcpu_enumColor(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n)
{
	SCRIPT_FUNCTION_INIT; 
	ASSERT(n.type == SCRIPT_INT);
	ScriptObject *s = NULL;
	SColor *sc = new SColor(n.data.idata);
	if (sc != NULL) s = sc->getScriptObject();
	return MAKE_SCRIPT_OBJECT(s);
}

scriptVar SColorMgr::script_vcpu_getNumColors(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT(WASABI_API_PALETTE->getNumColorElements());
}

scriptVar SColorMgr::script_vcpu_getGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar colorID)
{
	SCRIPT_FUNCTION_INIT; 
	ASSERT(colorID.type == SCRIPT_STRING);
	ScriptObject *s = NULL;
	SGammaset *gs = new SGammaset(colorID.data.sdata);
	if (gs != NULL) s = gs->getScriptObject();
	return MAKE_SCRIPT_OBJECT(s); 
}

scriptVar SColorMgr::script_vcpu_newGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar colorID)
{
	SCRIPT_FUNCTION_INIT; 
	ASSERT(colorID.type == SCRIPT_STRING);
	WASABI_API_COLORTHEMES->newGammaSet(colorID.data.sdata);
	ScriptObject *s = NULL;
	SGammaset *gs = new SGammaset(colorID.data.sdata);
	if (gs != NULL) s = gs->getScriptObject();
	return MAKE_SCRIPT_OBJECT(s); 
}

scriptVar SColorMgr::script_vcpu_enumGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n)
{
	SCRIPT_FUNCTION_INIT; 
	ASSERT(n.type == SCRIPT_INT);
	ScriptObject *s = NULL;
	SGammaset *gs = new SGammaset(n.data.idata);
	if (gs != NULL) s = gs->getScriptObject();
	return MAKE_SCRIPT_OBJECT(s);
}

scriptVar SColorMgr::script_vcpu_getCurrentGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT; 
	ScriptObject *s = NULL;
	const wchar_t * cur = WASABI_API_COLORTHEMES->getGammaSet();
	SGammaset *gs = NULL;
	if (cur != NULL) new SGammaset(cur);
	if (gs != NULL) s = gs->getScriptObject();
	return MAKE_SCRIPT_OBJECT(s);
}

scriptVar SColorMgr::script_vcpu_getNumGammaSets(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT(WASABI_API_COLORTHEMES->getNumGammaSets());
}

scriptVar SColorMgr::script_vcpu_onGuiLoaded(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, colorMgrController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar SColorMgr::script_vcpu_onLoaded(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, colorMgrController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}


scriptVar SColorMgr::script_vcpu_onBeforeLoadingElements(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, colorMgrController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar SColorMgr::script_vcpu_onColorThemesListChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, colorMgrController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar SColorMgr::script_vcpu_onColorThemeChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newTheme)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, colorMgrController, newTheme);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, newTheme);
}