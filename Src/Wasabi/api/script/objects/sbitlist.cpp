#include <precomp.h>
#include "sbitlist.h"
#include <bfc/bitlist.h>

#include <api/script/script.h>
#include <api/script/scriptmgr.h>

// {87C65778-E743-49fe-85F9-09CC532AFD56}
static const GUID bitlistGuid = 
{ 0x87c65778, 0xe743, 0x49fe, { 0x85, 0xf9, 0x9, 0xcc, 0x53, 0x2a, 0xfd, 0x56 } };

BitlistScriptController _bitlistController;
BitlistScriptController *bitlistController = &_bitlistController;

// -- Functions table -------------------------------------
function_descriptor_struct BitlistScriptController::exportedFunction[] = {
  {L"getItem", 1, (void*)SBitlist::script_vcpu_getitem },
  {L"setItem", 2, (void*)SBitlist::script_vcpu_setitem },
  {L"setSize",  1, (void*)SBitlist::script_vcpu_setsize },
  {L"getSize", 0, (void*)SBitlist::script_vcpu_getsize },
};
// --------------------------------------------------------

const wchar_t *BitlistScriptController::getClassName() {
	return L"BitList";
}

const wchar_t *BitlistScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObject *BitlistScriptController::instantiate() {
  SBitlist *bl = new SBitlist;
  ASSERT(bl != NULL);
  return bl->getScriptObject();
}

void BitlistScriptController::destroy(ScriptObject *o) 
{
	// TODO?
  SBitlist *bl = static_cast<SBitlist *>(o->vcpu_getInterface(bitlistGuid));
}

void *BitlistScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for bitlist yet
}

void BitlistScriptController::deencapsulate(void *o) {
}

int BitlistScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *BitlistScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID BitlistScriptController::getClassGuid() {
  return bitlistGuid;
}

//-------------------------------------------------------------------------------------------------------------


SBitlist::SBitlist() {
  list = new BitList;
  getScriptObject()->vcpu_setInterface(bitlistGuid, (void *)static_cast<SBitlist *>(this));
  getScriptObject()->vcpu_setClassName(L"BitList");
  getScriptObject()->vcpu_setController(bitlistController);
}

SBitlist::~SBitlist() {
  delete list;
}

BitList *SBitlist::getBitList() {
  return list;
}

scriptVar SBitlist::script_vcpu_getitem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&n));  // Compiler shouldn't do this
  scriptVar r;
  r = SOM::makeVar(SCRIPT_INT);
  SBitlist *bl = static_cast<SBitlist *>(o->vcpu_getInterface(bitlistGuid));
  if (bl) SOM::assign(&r, bl->getBitList()->getitem(SOM::makeInt(&n)));
  return r;  
}

scriptVar SBitlist::script_vcpu_setitem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n, scriptVar v) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&n));  // Compiler shouldn't do this
  ASSERT(SOM::isNumeric(&v));  // Compiler shouldn't do this
  SBitlist *bl = static_cast<SBitlist *>(o->vcpu_getInterface(bitlistGuid));
  if (bl) bl->getBitList()->setitem(SOM::makeInt(&n), SOM::makeBoolean(&v));
  RETURN_SCRIPT_VOID;  
}

scriptVar SBitlist::script_vcpu_setsize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newsize) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&newsize));  // Compiler shouldn't do this
  SBitlist *bl = static_cast<SBitlist *>(o->vcpu_getInterface(bitlistGuid));
  if (bl) bl->getBitList()->setSize(SOM::makeInt(&newsize));
  RETURN_SCRIPT_VOID;  
}

scriptVar SBitlist::script_vcpu_getsize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  scriptVar r;
  r = SOM::makeVar(SCRIPT_INT);
  SBitlist *bl = static_cast<SBitlist *>(o->vcpu_getInterface(bitlistGuid));
  if (bl) SOM::assign(&r, bl->getBitList()->getsize());
  return r;  
}



