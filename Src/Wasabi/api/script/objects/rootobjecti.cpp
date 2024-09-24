#include <precomp.h>

//<?#include "<class data="implementationheader"/>"
#include "rootobjecti.h"
//?>

#include <bfc/wasabi_std.h>
#include <api/script/scriptmgr.h>
#include <api/script/scriptobj.h>
#include <bfc/foreach.h>


// --------------------------------------------------------------------------------------------

RootObjectI::RootObjectI(ScriptObject *o) {
  my_script_object = o;
}

RootObjectI::~RootObjectI() {
}

const wchar_t *RootObjectI::rootobject_getClassName() 
{
  if (!my_script_object) return NULL;
  return my_script_object->vcpu_getClassName();
}

void RootObjectI::rootobject_notify(const wchar_t *s, const wchar_t *t, int u, int v) {
  foreach(cbs) 
    cbs.getfor()->rootobjectcb_onNotify(s, t, u, v);
  endfor;
  RootObject_ScriptMethods::onNotify(SCRIPT_CALL, rootobject_getScriptObject(), MAKE_SCRIPT_STRING(s), MAKE_SCRIPT_STRING(t), MAKE_SCRIPT_INT(u), MAKE_SCRIPT_INT(v));
}

ScriptObject *RootObjectI::rootobject_getScriptObject() {
  return my_script_object;
}

void RootObjectI::rootobject_setScriptObject(ScriptObject *obj) {
  my_script_object = obj;
}

void RootObjectI::rootobject_addCB(RootObjectCallback *cb) {
  cbs.addItem(cb);
}

