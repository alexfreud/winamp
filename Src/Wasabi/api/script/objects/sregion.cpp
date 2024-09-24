#include <precomp.h>
#include "sregion.h"

#include <api/script/script.h>
#include <api/script/scriptmgr.h>

RegionScriptController _regionController;
RegionScriptController *regionController=&_regionController;

// -- Functions table -------------------------------------
function_descriptor_struct RegionScriptController::exportedFunction[] = {
  {L"add",             1, (void*)SRegion::script_vcpu_add },
  {L"sub",             1, (void*)SRegion::script_vcpu_sub },
  {L"offset",          2, (void*)SRegion::script_vcpu_offset },
  {L"stretch",         1, (void*)SRegion::script_vcpu_stretch },
  {L"copy",            1, (void*)SRegion::script_vcpu_copy },
  {L"loadFromMap",     3, (void*)SRegion::script_vcpu_loadFromMap },
  {L"loadFromBitmap",  1, (void*)SRegion::script_vcpu_loadFromBitmap },
  {L"getBoundingBoxX", 0, (void*)SRegion::script_vcpu_getBoundX },
  {L"getBoundingBoxY", 0, (void*)SRegion::script_vcpu_getBoundY },
  {L"getBoundingBoxW", 0, (void*)SRegion::script_vcpu_getBoundW },
  {L"getBoundingBoxH", 0, (void*)SRegion::script_vcpu_getBoundH },
};
// --------------------------------------------------------

const wchar_t *RegionScriptController::getClassName() {
  return L"Region";
}

const wchar_t *RegionScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *RegionScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *RegionScriptController::instantiate() {
  SRegion *r = new SRegion;
  ASSERT(r != NULL);
  return r->getScriptObject();
}

void RegionScriptController::destroy(ScriptObject *o) {
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  ASSERT(r != NULL);
  delete r;
}

void *RegionScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for regions yet
}

void RegionScriptController::deencapsulate(void *o) {
}

int RegionScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *RegionScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID RegionScriptController::getClassGuid() {
  return regionGuid;
}

SRegion::SRegion() {
  getScriptObject()->vcpu_setInterface(regionGuid, (void *)static_cast<SRegion*>(this));
	getScriptObject()->vcpu_setClassName(L"Region");
  getScriptObject()->vcpu_setController(regionController);
  reg = new RegionI;
}

SRegion::~SRegion() {
  delete reg;
}

int SRegion::inRegion(int x, int y) {
  if (!reg) return 0;
  POINT pt={x,y};
  return reg->ptInRegion(&pt);
}

int SRegion::getBoundX() {
  if (!reg) return 0;
  RECT r;
  reg->getBox(&r);
  return r.left;
}

int SRegion::getBoundY() {
  if (!reg) return 0;
  RECT r;
  reg->getBox(&r);
  return r.top;
}

int SRegion::getBoundW() {
  if (!reg) return 0;
  RECT r;
  reg->getBox(&r);
  return r.right-r.left;
}

int SRegion::getBoundH() {
  if (!reg) return 0;
  RECT r;
  reg->getBox(&r);
  return r.bottom-r.top;
}

api_region *SRegion::getRegion() {
  return reg;
}

void SRegion::addRegion(SRegion *s) {
  if (!reg) reg = new RegionI;
  reg->addRegion(s->getRegion());
}

void SRegion::subRegion(SRegion *s) {
  if (!reg) return;
  reg->subtractRgn(s->getRegion());
}

void SRegion::offset(int x, int y) {
  if (!reg) return;
  reg->offset(x, y);
}

void SRegion::stretch(double s) {
  if (!reg) return;
  reg->scale(s, s);
}

void SRegion::copy(SRegion *s) {
  if (!reg) reg = new RegionI;
  else reg->empty();
  reg->addRegion(s->getRegion());
}

void SRegion::loadFromMap(SMap *m, int byte, int inverted) {
  delete reg;
  RECT r={m->getBitmap()->getX(), m->getBitmap()->getY(), m->getBitmap()->getWidth(), m->getBitmap()->getHeight()};
  reg = new RegionI(m->getBitmap(), &r, 0, 0, FALSE, 1, byte, inverted);
}

void SRegion::loadFromBitmap(const wchar_t *p)
{
  delete reg;
  SkinBitmap *b = new SkinBitmap(p);
  ASSERT(b); // TODO: should be guru
  reg = new RegionI(b);
  delete b;
}

// -----------------------------------------------------------------------

scriptVar SRegion::script_vcpu_loadFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&byte));
  ASSERT(SOM::isNumeric(&inv));
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  SMap *m = static_cast<SMap *>(GET_SCRIPT_OBJECT_AS(map, mapGuid));
  if (r) r->loadFromMap(m, GET_SCRIPT_INT(byte), GET_SCRIPT_BOOLEAN(inv));
  RETURN_SCRIPT_VOID;  
}

scriptVar SRegion::script_vcpu_loadFromBitmap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar b) 
{
  SCRIPT_FUNCTION_INIT; 
  ASSERT(b.type == SCRIPT_STRING);
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) r->loadFromBitmap(GET_SCRIPT_STRING(b));
  RETURN_SCRIPT_VOID;  
}

scriptVar SRegion::script_vcpu_inRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  ASSERT(SOM::isNumeric(&y));
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) return MAKE_SCRIPT_INT(r->inRegion(GET_SCRIPT_INT(x), GET_SCRIPT_INT(y)));
  RETURN_SCRIPT_ZERO;
}

scriptVar SRegion::script_vcpu_add(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r) {
  SCRIPT_FUNCTION_INIT; 
  SRegion *r1 = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  SRegion *r2 = static_cast<SRegion *>(GET_SCRIPT_OBJECT_AS(r, regionGuid));
  if (r1) r1->addRegion(r2);
  RETURN_SCRIPT_VOID;
}

scriptVar SRegion::script_vcpu_sub(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r) {
  SCRIPT_FUNCTION_INIT; 
  SRegion *r1 = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  SRegion *r2 = static_cast<SRegion *>(GET_SCRIPT_OBJECT_AS(r, regionGuid));
  if (r1) r1->subRegion(r2);
  RETURN_SCRIPT_VOID;
}

scriptVar SRegion::script_vcpu_offset(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  ASSERT(SOM::isNumeric(&y));
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) r->offset(GET_SCRIPT_INT(x), GET_SCRIPT_INT(y));
  RETURN_SCRIPT_VOID;
}

scriptVar SRegion::script_vcpu_stretch(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&s));
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) r->stretch(SOM::makeDouble(&s));
  RETURN_SCRIPT_VOID;
}

scriptVar SRegion::script_vcpu_getBoundX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) return MAKE_SCRIPT_INT(r->getBoundX());
  RETURN_SCRIPT_ZERO;
}

scriptVar SRegion::script_vcpu_getBoundY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) return MAKE_SCRIPT_INT(r->getBoundY());
  RETURN_SCRIPT_ZERO;
}

scriptVar SRegion::script_vcpu_getBoundW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) return MAKE_SCRIPT_INT(r->getBoundW());
  RETURN_SCRIPT_ZERO;
}

scriptVar SRegion::script_vcpu_getBoundH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SRegion *r = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  if (r) return MAKE_SCRIPT_INT(r->getBoundH());
  RETURN_SCRIPT_ZERO;
}

scriptVar SRegion::script_vcpu_copy(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r) {
  SCRIPT_FUNCTION_INIT; 
  SRegion *r1 = static_cast<SRegion *>(o->vcpu_getInterface(regionGuid));
  SRegion *r2 = static_cast<SRegion *>(GET_SCRIPT_OBJECT_AS(r, regionGuid));
  if (r1) r1->copy(r2);
  RETURN_SCRIPT_VOID;
}


