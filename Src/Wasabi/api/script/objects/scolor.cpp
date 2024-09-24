#include <precomp.h>
#include "SColor.h"
#include <api.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objecttable.h>
#include <api/imgldr/imgldr.h>


// {95DDB221-00E3-4e2b-8EA5-833548C13C10}
static const GUID colorSoGuid = 
{ 0x95ddb221, 0xe3, 0x4e2b, { 0x8e, 0xa5, 0x83, 0x35, 0x48, 0xc1, 0x3c, 0x10 } };


colorScriptController _colorController;
colorScriptController *colorController=&_colorController;

// -- Functions table -------------------------------------
function_descriptor_struct colorScriptController::exportedFunction[] = {
	{L"getID",		0, (void*)SColor::script_vcpu_getColorID },
	{L"getRed",			0, (void*)SColor::script_vcpu_getRed },
	{L"getGreen",		0, (void*)SColor::script_vcpu_getGreen },
	{L"getBlue",		0, (void*)SColor::script_vcpu_getBlue },
	{L"getAlpha",		0, (void*)SColor::script_vcpu_getAlpha },
	{L"getGreenWithGamma",		0, (void*)SColor::script_vcpu_getGreenWithGamma },
	{L"getBlueWithGamma",		0, (void*)SColor::script_vcpu_getBlueWithGamma },
	{L"getRedWithGamma",		0, (void*)SColor::script_vcpu_getRedWithGamma },
	{L"getGammagroup",	0, (void*)SColor::script_vcpu_getGammagroup },
};
// --------------------------------------------------------

const wchar_t *colorScriptController::getClassName() {
	return L"Color";
}

const wchar_t *colorScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *colorScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *colorScriptController::instantiate() {
  SColor *xd = new SColor;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void colorScriptController::destroy(ScriptObject *o) {
  SColor *xd = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
  ASSERT(xd != NULL);
  delete xd;
}

void *colorScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for now
}

void colorScriptController::deencapsulate(void *o) {
}

int colorScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *colorScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID colorScriptController::getClassGuid() {
	return colorSoGuid;
}

void SColor::__construct()
{
  getScriptObject()->vcpu_setInterface(colorSoGuid, (void *)static_cast<SColor *>(this));
  getScriptObject()->vcpu_setClassName(L"Color");
  getScriptObject()->vcpu_setController(colorController);
  gammagroup = NULL;
  colorID = NULL;
  color = 0;
}

SColor::SColor()
{
	SColor::__construct();
}

SColor::SColor(const wchar_t* colorID)
{
	__construct();
	this->colorID = colorID;
	color = WASABI_API_PALETTE->getColorElement(colorID, &gammagroup);
}

SColor::SColor(int n)
{
	__construct();
	colorID = WASABI_API_PALETTE->enumColorElement(n);
	color = WASABI_API_PALETTE->getColorElement(colorID, &gammagroup);
}

SColor::~SColor()
{
}

int SColor::getARGBValue(int whichCol)
{

  whichCol %= 4;

  /**
	whichCol
	0: red
	1: green
	2: blue
	3: alpha
  */ 

  ARGB32 c = this->color;

  c = c >> (whichCol * 8);
  c &= 0x000000FF;

  return (int)c;
}

int SColor::getARGBValueWithGamma(int whichCol)
{
	ARGB32 c = imageLoader::filterSkinColor(this->color, this->colorID, NULL);
	whichCol %= 4;

	/**
	whichCol
	0: red
	1: green
	2: blue
	3: alpha
	*/ 

	c = c >> (whichCol * 8);
	c &= 0x000000FF;

	return (int)c;
}

// VCPU

scriptVar SColor::script_vcpu_getBlue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->getARGBValue(2));
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SColor::script_vcpu_getGreen(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->getARGBValue(1));
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SColor::script_vcpu_getRed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->getARGBValue(0));
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SColor::script_vcpu_getBlueWithGamma(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->getARGBValueWithGamma(2));
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SColor::script_vcpu_getGreenWithGamma(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->getARGBValueWithGamma(1));
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SColor::script_vcpu_getRedWithGamma(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->getARGBValueWithGamma(0));
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SColor::script_vcpu_getAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_INT(m->getARGBValue(3));
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SColor::script_vcpu_getColorID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_STRING(m->colorID);
	}
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar SColor::script_vcpu_getGammagroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SColor *m = static_cast<SColor *>(o->vcpu_getInterface(colorSoGuid));
	if (m)
	{
		return MAKE_SCRIPT_STRING(m->gammagroup?m->gammagroup:L"");
	}
	return MAKE_SCRIPT_STRING(L"");
}