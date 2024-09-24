#include <precomp.h>
#include "smap.h"

#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objects/sregion.h>
#include <api/script/objecttable.h>

MapScriptController _mapController;
MapScriptController *mapController=&_mapController;

// -- Functions table -------------------------------------
function_descriptor_struct MapScriptController::exportedFunction[] = {
  {L"getValue", 2, (void*)SMap::script_vcpu_getValue },
  {L"getARGBValue", 3, (void*)SMap::script_vcpu_getARGBValue },
  {L"inRegion", 2, (void*)SMap::script_vcpu_inRegion },
  {L"loadMap", 1, (void*)SMap::script_vcpu_loadMap },
  {L"getWidth", 0, (void*)SMap::script_vcpu_getWidth },
  {L"getHeight", 0, (void*)SMap::script_vcpu_getHeight },
  {L"getRegion", 0, (void*)SMap::script_vcpu_getRegion },
  // todo: stretch  
};
// --------------------------------------------------------

const wchar_t *MapScriptController::getClassName() {
	return L"Map";
}

const wchar_t *MapScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *MapScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *MapScriptController::instantiate() {
  SMap *m = new SMap;
  ASSERT(m != NULL);
  return m->getScriptObject();
}

void MapScriptController::destroy(ScriptObject *o) {
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  ASSERT(m != NULL);
  delete m;
}

void *MapScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for maps for now
}

void MapScriptController::deencapsulate(void *o) {
}

int MapScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *MapScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID MapScriptController::getClassGuid() {
  return mapGuid;
}



SMap::SMap() {
  getScriptObject()->vcpu_setInterface(mapGuid, (void *)static_cast<SMap *>(this));
  getScriptObject()->vcpu_setClassName(L"Map");
  getScriptObject()->vcpu_setController(mapController);
  bmp = NULL;
  region_so = ObjectTable::instantiate(ObjectTable::getClassFromName(L"Region"));
  reg = static_cast<SRegion *>(region_so->vcpu_getInterface(regionGuid));
}

SMap::~SMap() {
  if (bmp) delete bmp;
  ObjectTable::destroy(region_so);
}

int SMap::getValue(int x, int y) {
  if (!bmp) return 0;
  ARGB32 c = bmp->getPixel(x, y);
//  if ((c & 0xFF000000) >> 24 != 0xFF) return -1;
  int v = MAX(MAX((c & 0xFF0000) >> 16, (c & 0xFF00) >> 8), c & 0xFF);
  return v;
}

int SMap::getARGBValue(int x, int y, int whichCol) {
  if (!bmp) return 0;
  ARGB32 c = bmp->getPixel(x, y);

  whichCol %= 4;

  /**
	whichCol
	0: blue
	1: green
	2: red
	3: alpha
  */ 

  ARGB32 a = c >> 24;
  a &= 0x000000FF; //just to be sure

  if (whichCol == 3) return a;

  c = c >> (whichCol * 8);
  c &= 0x000000FF;
	
  if (0 == a || 255 == a) return c;

  double d = c*255/a; // Correction for bitmaps w/ alpha channel, otherwise a lesser rgb value is returned
  d = ceil(d);

  return (int)d;
}

void SMap::loadMap(const wchar_t *b)
{
  bmp = new SkinBitmap(b, 0);
  reg->loadFromBitmap(b);
}

int SMap::getWidth() {
  if (!bmp) return 0;
  return bmp->getWidth();
}

int SMap::getHeight() {
  if (!bmp) return 0;
  return bmp->getHeight();
}

int SMap::inRegion(int x, int y) {
  POINT pt={x,y};
  return reg->getRegion()->ptInRegion(&pt);
}

SRegion *SMap::getSRegion() {
  return reg;
}

// VCPU

scriptVar SMap::script_vcpu_loadMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar bmp) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(bmp.type == SCRIPT_STRING);
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  if (m) m->loadMap(bmp.data.sdata);
  RETURN_SCRIPT_VOID;  
}

scriptVar SMap::script_vcpu_getValue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  ASSERT(SOM::isNumeric(&y));
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  if (m) return MAKE_SCRIPT_INT(m->getValue(SOM::makeInt(&x), SOM::makeInt(&y)));
  RETURN_SCRIPT_ZERO;
}

scriptVar SMap::script_vcpu_getARGBValue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y, scriptVar wichCol) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  ASSERT(SOM::isNumeric(&y));
  ASSERT(SOM::isNumeric(&wichCol));
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  if (m) return MAKE_SCRIPT_INT(m->getARGBValue(SOM::makeInt(&x), SOM::makeInt(&y), SOM::makeInt(&wichCol)));
  RETURN_SCRIPT_ZERO;
}

scriptVar SMap::script_vcpu_getWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  if (m) return MAKE_SCRIPT_INT(m->getWidth());  
  RETURN_SCRIPT_ZERO;
}

scriptVar SMap::script_vcpu_getHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  if (m) return MAKE_SCRIPT_INT(m->getHeight());  
  RETURN_SCRIPT_ZERO;
}

scriptVar SMap::script_vcpu_inRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  ASSERT(SOM::isNumeric(&y));
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  if (m) return MAKE_SCRIPT_BOOLEAN(m->inRegion(SOM::makeInt(&x), SOM::makeInt(&y)));
  RETURN_SCRIPT_ZERO;
}

scriptVar SMap::script_vcpu_getRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SMap *m = static_cast<SMap *>(o->vcpu_getInterface(mapGuid));
  if (m) {
    SRegion *s = m->getSRegion();
    if (s) return MAKE_SCRIPT_OBJECT(s->getScriptObject());
  }
  RETURN_SCRIPT_ZERO;
}