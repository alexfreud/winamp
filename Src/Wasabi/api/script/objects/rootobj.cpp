#include <api/script/objcontroller.h>
#include "rootobj.h"
#include <api/script/objects/rootobject.h>
#include "api.h"
#include <api/script/scriptguid.h>

RootObjectInstance::RootObjectInstance() 
{
	ASSERT(WASABI_API_MAKI != 0);
  rootobject_init();
}

RootObjectInstance::~RootObjectInstance() 
{
  WASABI_API_MAKI->maki_deencapsulate(rootObjectGuid, my_root_object);
}

RootObject *RootObjectInstance::getRootObject() {
  return my_root_object;
}

ScriptObject *RootObjectInstance::getScriptObject() {
  return &my_script_object;
}

const wchar_t *RootObjectInstance::getClassName() 
{
  return my_root_object->rootobject_getClassName();
}

void RootObjectInstance::notify(const wchar_t *s, const wchar_t *t, int u, int v) {
  my_root_object->rootobject_notify(s, t, u, v);
}

void RootObjectInstance::rootobject_init() 
{
  my_root_object = static_cast<RootObject *>(WASABI_API_MAKI->maki_encapsulate(rootObjectGuid, &my_script_object)); // creates an encapsulated RootObject
  my_script_object.vcpu_setInterface(rootObjectInstanceGuid, static_cast<void *>(this));
  my_script_object.vcpu_setClassName(L"Object");
  my_script_object.vcpu_setController(WASABI_API_MAKI->maki_getController(rootObjectGuid));
}

