#include <precomp.h>

#include <api/script/scriptobj.h>
#include <api/script/objcontroller.h>
#include "c_rootobj.h"

C_RootObject::C_RootObject(ScriptObject *o) 
{
  inited = 0;
  obj = NULL;
  C_hook(o);
}

C_RootObject::C_RootObject() 
{
  inited = 0;
  obj = NULL;
}

void C_RootObject::C_hook(ScriptObject *o) 
{
  ASSERT(!inited);
  obj = o;
  if (count++ == 0) {
    getclassname_id = WASABI_API_MAKI->maki_addDlfRef(o->vcpu_getController(), L"getClassName", this);
    notify_id = WASABI_API_MAKI->maki_addDlfRef(o->vcpu_getController(), L"notify", this);
  }
  inited=1;
}

C_RootObject::~C_RootObject() 
{
  if (!inited) return;
  WASABI_API_MAKI->maki_remDlfRef(this);
  //count--;
}

ScriptObject *C_RootObject::getScriptObject() 
{
  ASSERT(inited);
  return obj;
}

const wchar_t *C_RootObject::getClassName() 
{
  ASSERT(inited);
  return GET_SCRIPT_STRING(WASABI_API_MAKI->maki_callFunction(obj, getclassname_id, NULL));
}

void C_RootObject::notify(const wchar_t *a, const wchar_t *b, int c, int d) 
{
  ASSERT(inited);
  scriptVar _a = MAKE_SCRIPT_STRING(a);
  scriptVar _b = MAKE_SCRIPT_STRING(b);
  scriptVar _c = MAKE_SCRIPT_INT(c);
  scriptVar _d = MAKE_SCRIPT_INT(d);
  scriptVar *params[4]={&_a, &_b, &_c, &_d};
  WASABI_API_MAKI->maki_callFunction(obj, notify_id, params);
}

int C_RootObject::getclassname_id=0;
int C_RootObject::notify_id=0;
int C_RootObject::inited=0;
int C_RootObject::count=0;
