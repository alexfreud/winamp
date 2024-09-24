#include <precomp.h>
#include "slist.h"

#include <api/script/script.h>
#include <api/script/scriptmgr.h>

ListScriptController _listController;
ListScriptController *listController=&_listController;

// -- Functions table -------------------------------------
function_descriptor_struct ListScriptController::exportedFunction[] = {
  {L"addItem", 1, (void*)SList::script_vcpu_addItem },
  {L"removeItem", 1, (void*)SList::script_vcpu_removeItem },
  {L"enumItem", 1, (void*)SList::script_vcpu_enumItem },
  {L"findItem2", 2, (void*)SList::script_vcpu_findItem2 },
  {L"findItem", 1, (void*)SList::script_vcpu_findItem },
  {L"getNumItems", 0, (void*)SList::script_vcpu_getNumItems },
  {L"removeAll", 0, (void*)SList::script_vcpu_removeAll },
};
// --------------------------------------------------------

const wchar_t *ListScriptController::getClassName() {
	return L"List";
}

const wchar_t *ListScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObject *ListScriptController::instantiate() {
  SList *l = new SList;
  ASSERT(l != NULL);
  return l->getScriptObject();
}

void ListScriptController::destroy(ScriptObject *o) {
  SList *obj = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  ASSERT(obj != NULL);
  delete obj;
}

void *ListScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for list yet
}

void ListScriptController::deencapsulate(void *o) {
}

int ListScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *ListScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID ListScriptController::getClassGuid() {
  return slistGuid;
}

//------------------------------------------------------------------------

SList::SList() {
  getScriptObject()->vcpu_setInterface(slistGuid, (void *)static_cast<SList *>(this));
  getScriptObject()->vcpu_setClassName(L"List");
  getScriptObject()->vcpu_setController(listController);
}

SList::~SList() {
}

TList<scriptVar> *SList::getTList() {  
  return &list;
}

scriptVar SList::script_vcpu_enumItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&i));
  SList *l = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  if (l) return l->getTList()->enumItem(SOM::makeInt(&i));
  RETURN_SCRIPT_ZERO;
}

scriptVar SList::script_vcpu_addItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj) {
  SCRIPT_FUNCTION_INIT; 
  scriptVar dup;
  dup = SOM::makeVar(obj.type);
  SOM::assignPersistent(&dup, &obj);
  SList *l = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  if (l) l->getTList()->addItem(dup);
  RETURN_SCRIPT_VOID;  
}

scriptVar SList::script_vcpu_removeItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&i));
  SList *l = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  if (l) {
    scriptVar dup = l->getTList()->enumItem(SOM::makeInt(&i));
    if (dup.type == SCRIPT_STRING) {
      if (dup.data.odata) FREE(dup.data.odata);
    }
    l->getTList()->delByPos(SOM::makeInt(&i));
  }
  RETURN_SCRIPT_VOID;  
}

scriptVar SList::script_vcpu_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SList *l = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  if (l) return MAKE_SCRIPT_INT(l->getTList()->getNumItems());
  return MAKE_SCRIPT_INT(0);
}

scriptVar SList::script_vcpu_findItem2(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj, scriptVar start) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&start));
  SList *l = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  if (l) {
    if (obj.type != SCRIPT_STRING) {
      for (int i=SOM::makeInt(&start);i<l->getTList()->getNumItems();i++) {
        if (!MEMCMP(&l->getTList()->enumItem(i), &obj, sizeof(scriptVar))) {
          return MAKE_SCRIPT_INT(i);
        }
      }
    } else {
      for (int i=SOM::makeInt(&start);i<l->getTList()->getNumItems();i++) {
        if (!wcscmp(l->getTList()->enumItem(i).data.sdata, obj.data.sdata)) {
          return MAKE_SCRIPT_INT(i);
        }
      }
    }
  }
  return MAKE_SCRIPT_INT(-1);  
}

scriptVar SList::script_vcpu_findItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj) {
	SCRIPT_FUNCTION_INIT; 
  SList *l = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  if (l) {
    if (obj.type != SCRIPT_STRING) {
      for (int i=0;i<l->getTList()->getNumItems();i++) {
        if (!MEMCMP(&l->getTList()->enumItem(i), &obj, sizeof(scriptVar))) {
          return MAKE_SCRIPT_INT(i);
        }
      }
    } else {
      for (int i=0;i<l->getTList()->getNumItems();i++) {
        if (!wcscmp(l->getTList()->enumItem(i).data.sdata, obj.data.sdata)) {
          return MAKE_SCRIPT_INT(i);
        }
      }
    }
  }
	return MAKE_SCRIPT_INT(-1);  
}

scriptVar SList::script_vcpu_removeAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SList *l = static_cast<SList *>(o->vcpu_getInterface(slistGuid));
  if (l) {
    for (int i=0;i<l->getTList()->getNumItems();i++) {
      scriptVar v = l->getTList()->enumItem(i);
      if (v.type == SCRIPT_STRING)
        FREE((wchar_t *)v.data.sdata);
    }
    l->getTList()->removeAll();
  }
  RETURN_SCRIPT_VOID;  
}


