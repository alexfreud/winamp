#include <bfc/wasabi_std.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/c_script/scripthook.h>
#include "api.h"

#define CBCLASS ScriptObjectControllerI
START_DISPATCH;
  CB(GETCLASSGUID,          getClassGuid);
  CB(GETCLASSNAME,          getClassName);
  CB(GETANCESTORCLASSNAME,  getAncestorClassName);
  CB(GETNUMFUNCTIONS,       getNumFunctions);
  CB(GETEXPORTEDFUNCTIONS,  getExportedFunctions);
  CB(INSTANTIATE,		        instantiate);
  VCB(DESTROY,		          destroy);
  CB(ENCAPSULATE,		        encapsulate);
  VCB(DEENCAPSULATE,		    deencapsulate);
  VCB(SETCLASSID,           setClassId);
  CB(GETCLASSID,            getClassId);
  VCB(SETANCESTORCLASSID,   setAncestorClassId);
  CB(GETANCESTORCLASSID,    getAncestorClassId);
  CB(GETINSTANTIABLE,       getInstantiable);
  CB(GETREFERENCEABLE,      getReferenceable);
  CB(GETANCESTORCONTROLLER, getAncestorController);
  VCB(ADDCLASSHOOK,         addClassHook);
  VCB(ADDOBJHOOK,           addObjectHook);
  VCB(REMHOOKS,             removeHooks);
  VCB(ONREGISTERCLASS,      onRegisterClass);
  CB(CAST,                  cast);
END_DISPATCH;

ScriptObjectControllerI::ScriptObjectControllerI() {
  my_class_id = -1;
  my_ancestor_class_id = -1; 
  rootController = NULL;
  incast = 0;
}

ScriptObjectControllerI::~ScriptObjectControllerI() {
}

int ScriptObjectControllerI::processHooks(ScriptObject *o, int dlfid, scriptVar **table, int nparams) {
  //CUT: int n=-1;
  for (int j=0;j<classhooks.getNumItems();j++) {
    classhooks.enumItem(j)->eventCallback(o, dlfid, table, nparams);
  }
  for (int i=0;i<objhooks.getNumItems();i++) {
    object_hook_struct *s = objhooks.enumItem(i);
    if (s->object == o) {
      s->hook->eventCallback(o, dlfid, table, nparams); 
    }
  }
  return 0;
}

ScriptObject *ScriptObjectControllerI::cast(ScriptObject *o, GUID g) {
  if (incast) return NULL;
  incast = 1;
  ASSERT(o != NULL);
  ScriptObject *obj = NULL;
  void *i = o->vcpu_getInterfaceObject(g, &obj);
  incast = 0;
  if (i != NULL) {
    if (obj != NULL) return obj;
    return o;
  }
  return NULL;
}

void ScriptObjectControllerI::addClassHook(ScriptHook *h) {
  classhooks.addItem(h);
}

void ScriptObjectControllerI::addObjectHook(ScriptHook *h, ScriptObject *o) {
  object_hook_struct *s = new object_hook_struct;
  s->hook = h;
  s->object = o;
  objhooks.addItem(s);
/*  if (getAncestorController())
    getAncestorController()->addObjectHook(h, o);
  else
    if (rootController && this != rootController) rootController->addObjectHook(h, o);*/
}

void ScriptObjectControllerI::removeHooks(ScriptHook *h) {
  classhooks.removeItem(h);
  for (int i=0;i<objhooks.getNumItems();i++) {
    object_hook_struct *o = objhooks.enumItem(i);
    if (o->hook == h) {
      delete o;
      objhooks.removeByPos(i);
      i--;
    }
  }
/*  if (getAncestorController())
    getAncestorController()->removeHooks(h);
  else
    if (rootController && this != rootController) rootController->removeHooks(h);*/
}

void ScriptObjectControllerI::onRegisterClass(ScriptObjectController *root) {
  rootController = root;
}


scriptVar MAKE_SCRIPT_INT(int i) { 
  scriptVar r; 
  r.type = SCRIPT_INT; 
  r.data.idata = i; 
  return r; 
}

scriptVar MAKE_SCRIPT_VOID() { 
  scriptVar r; r.type = SCRIPT_VOID; 
  r.data.idata = 0; 
  return r; 
}

scriptVar MAKE_SCRIPT_FLOAT(float f) { 
  scriptVar r; r.type = SCRIPT_FLOAT; 
  r.data.fdata = f; 
  return r; 
}

scriptVar MAKE_SCRIPT_DOUBLE(double d) { 
  scriptVar r; r.type = SCRIPT_DOUBLE; 
  r.data.ddata = d; 
  return r; 
}

scriptVar MAKE_SCRIPT_BOOLEAN(int b) { 
  scriptVar r; r.type = SCRIPT_BOOLEAN; 
  r.data.idata = b; 
  return r; 
}

scriptVar MAKE_SCRIPT_OBJECT(ScriptObject *o) { 
  scriptVar r; 
  r.type = SCRIPT_OBJECT; 
  r.data.odata = o; 
  return r; 
}

scriptVar MAKE_SCRIPT_STRING(const wchar_t *s) 
{ 
  scriptVar r; 
  r.type = SCRIPT_STRING; 
  r.data.sdata = s; 
  return r; 
}

void *GET_SCRIPT_OBJECT_AS(scriptVar v, GUID g) {
  ScriptObject *o = GET_SCRIPT_OBJECT(v);
  if (!o) return NULL;
  return o->vcpu_getInterface(g);
}

