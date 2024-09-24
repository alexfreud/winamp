#include <precomp.h>
#include "cfgscriptobj.h"

#include <api/config/items/cfgitemi.h>

#ifndef _WASABIRUNTIME

BEGIN_SERVICES(ConfigObject_Svc);
DECLARE_SERVICE(ScriptObjectCreator<ConfigScriptObjectSvc>);
END_SERVICES(ConfigObject_Svc, _ConfigObject_Svc);

#ifdef _X86_
extern "C" { int _link_ConfigObjectSvc; }
#else
extern "C" { int __link_ConfigObjectSvc; }
#endif

#endif

static ConfigScriptObjectController _configController;
ScriptObjectController *configController = &_configController;

static ConfigItemScriptObjectController _configItemController;
ScriptObjectController *configItemController = &_configItemController;

static ConfigAttributeScriptObjectController _configAttributeController;
ScriptObjectController *configAttributeController = &_configAttributeController;


// -----------------------------------------------------------------------------------------------------
// Service

ScriptObjectController *ConfigScriptObjectSvc::getController(int n) 
{
  switch (n) {
    case 0:
      return configController;
    case 1:
      return configItemController;
    case 2:
      return configAttributeController;
  }
  return NULL;
}

// -----------------------------------------------------------------------------------------------------
// ConfigObject

ConfigObject::ConfigObject() {
  numobjects++;
  getScriptObject()->vcpu_setInterface(CONFIG_SCRIPTOBJECT_GUID, (void *)static_cast<ConfigObject *>(this));
  getScriptObject()->vcpu_setClassName(L"Config");
  getScriptObject()->vcpu_setController(configController);
}

ConfigObject::~ConfigObject() {
  numobjects--;
  mylist.deleteAll();
  if (numobjects == 0) {
    foreach(ouraddeditems)
      WASABI_API_CONFIG->config_deregisterCfgItem(ouraddeditems.getfor());
    endfor
    ouraddeditems.deleteAll();
  }
}

PtrList<CfgItemI> ConfigObject::ouraddeditems;
int ConfigObject::numobjects=0;


function_descriptor_struct ConfigScriptObjectController::exportedFunction[] = {
  {L"getItem",            1, (void*)ConfigObject::config_getItem},
  {L"getItemByGuid",      1, (void*)ConfigObject::config_getItemByGuid},
  {L"newItem",            2, (void*)ConfigObject::config_newItem},
};

int ConfigScriptObjectController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

scriptVar ConfigObject::config_newItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar configitem_name, scriptVar configitem_guid) 
{
  SCRIPT_FUNCTION_INIT  
  ConfigObject *co = static_cast<ConfigObject*>(o->vcpu_getInterface(CONFIG_SCRIPTOBJECT_GUID));
  ConfigItemObject *ret=getItem(GET_SCRIPT_STRING(configitem_name), co);
  if (!ret)
		ret = getItem(GET_SCRIPT_STRING(configitem_guid), co);
  if (ret)
		return MAKE_SCRIPT_OBJECT(ret->getScriptObject());
  CfgItemI *item = new CfgItemI(GET_SCRIPT_STRING(configitem_name), nsGUID::fromCharW(GET_SCRIPT_STRING(configitem_guid)));
  ConfigObject::ouraddeditems.addItem(item);
  WASABI_API_CONFIG->config_registerCfgItem(item);
  ret = new ConfigItemObject(item);
  co->mylist.addItem(ret);
  return MAKE_SCRIPT_OBJECT(ret->getScriptObject());
}

ConfigItemObject *ConfigObject::getItem(const wchar_t *nameorguid, ConfigObject *co) {
  int i=0;
  ConfigItemObject *ret=NULL;
  GUID g = nsGUID::fromCharW(nameorguid);
  for (i=0;;i++) {
    CfgItem *item = WASABI_API_CONFIG->config_enumCfgItem(i);
    if (!item) break;
    GUID ig = item->getGuid();
    if (g == ig || WCSCASEEQLSAFE(nameorguid, item->getName())) 
		{
      ret = new ConfigItemObject(item);
      co->mylist.addItem(ret);
      break;
    }
#if 0//CUT
    for (int j=0;j<item->getNumChildren();j++) {
      if (STRCASEEQL(nameorguid, item->enumChild(j)->getName())) {
        ret = new ConfigItemObject(item->enumChild(j));
        co->mylist.addItem(ret);
        break;
      }
    }
#endif
  }
  return ret;
}

scriptVar ConfigObject::config_getItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar cfgitem_name) 
{
  SCRIPT_FUNCTION_INIT  
  ConfigObject *co = static_cast<ConfigObject*>(o->vcpu_getInterface(CONFIG_SCRIPTOBJECT_GUID));
  ConfigItemObject *ret=getItem(GET_SCRIPT_STRING(cfgitem_name), co);
  return MAKE_SCRIPT_OBJECT(ret ? ret->getScriptObject() : NULL);
}

scriptVar ConfigObject::config_getItemByGuid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar cfgitem_guid) 
{
  SCRIPT_FUNCTION_INIT  
//  GUID g = nsGUID::fromChar(GET_SCRIPT_STRING(guid));
//  api->
  int i=0;
  ConfigItemObject *ret=NULL;
  ConfigObject *co = static_cast<ConfigObject*>(o->vcpu_getInterface(CONFIG_SCRIPTOBJECT_GUID));

  const wchar_t *p = GET_SCRIPT_STRING(cfgitem_guid);
  if (p == NULL) {
    RETURN_SCRIPT_ZERO;
  }

  GUID g = nsGUID::fromCharW(p);

  for (i=0;;i++) {
    CfgItem *item = WASABI_API_CONFIG->config_enumCfgItem(i);
    if (!item) break;
    if (g == item->getGuid()) {
      ret = new ConfigItemObject(item);
      co->mylist.addItem(ret);
      break;
    }
#if 0//CUT
    for (int j=0;j<item->getNumChildren();j++) {
      if (g == item->enumChild(j)->getName()) {
        ret = new ConfigItemObject(item->enumChild(j));
        co->mylist.addItem(ret);
        break;
      }
    }
#endif
  }
  return MAKE_SCRIPT_OBJECT(ret ? ret->getScriptObject() : NULL);
}

ScriptObject *ConfigScriptObjectController::instantiate() {
  ConfigObject *c = new ConfigObject;
  if (!c) return NULL;
  return c->getScriptObject();
}

void ConfigScriptObjectController::destroy(ScriptObject *o) {
  ConfigObject *obj = static_cast<ConfigObject *>(o->vcpu_getInterface(CONFIG_SCRIPTOBJECT_GUID));
  ASSERT(obj != NULL);
  delete obj;
}

void *ConfigScriptObjectController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for config
}

void ConfigScriptObjectController::deencapsulate(void *) {
}

// -----------------------------------------------------------------------------------------------------
// ConfigItem

function_descriptor_struct ConfigItemScriptObjectController::exportedFunction[] = {
  {L"getAttribute",            1, (void*)ConfigItemObject::configItem_getAttribute},
  {L"getGuid",                 0, (void*)ConfigItemObject::configItem_getGuid},
  {L"newAttribute",            2, (void*)ConfigItemObject::configItem_newAttribute},
};

int ConfigItemScriptObjectController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

ConfigItemObject::ConfigItemObject(CfgItem *_item) 
{
  getScriptObject()->vcpu_setInterface(CONFIGITEM_SCRIPTOBJECT_GUID, (void *)static_cast<ConfigItemObject *>(this));
  getScriptObject()->vcpu_setClassName(L"ConfigItem");
  getScriptObject()->vcpu_setController(configItemController);
  wchar_t strguid[256];
  nsGUID::toCharW(_item->getGuid(), strguid);
  guid = strguid;
  item = _item;
}

ConfigAttributeObject *ConfigItemObject::getAttribute(const wchar_t *name) 
{
  if (!item) return NULL;
  for (int i=0;i<item->getNumAttributes();i++) 
	{
    if (!WCSICMP(item->enumAttribute(i), name)) 
		{
      ConfigAttributeObject *o = new ConfigAttributeObject(item, name, this);
      mylist.addItem(o);
      return o;
    }
  }
  return NULL;
}

ConfigAttributeObject *ConfigItemObject::newAttribute(const wchar_t *name, const wchar_t *defaultvalue) 
{
  if (!item) return NULL;
  ConfigAttributeObject *o = getAttribute(name);
  if (o != NULL) return o;
  
  item->addAttribute(name, defaultvalue);

  ConfigAttributeObject *cao = getAttribute(name);
  cao->setAutoDelete();
  return cao;
}

ConfigItemObject::~ConfigItemObject() 
{
  mylist.deleteAll();
}

scriptVar ConfigItemObject::configItem_getAttribute(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name) 
{
  SCRIPT_FUNCTION_INIT  
  ConfigItemObject *cio = static_cast<ConfigItemObject *>(o->vcpu_getInterface(CONFIGITEM_SCRIPTOBJECT_GUID));
  ConfigAttributeObject *cao = NULL;
  if (cio) 
		cao = cio->getAttribute(GET_SCRIPT_STRING(name));
  return MAKE_SCRIPT_OBJECT(cao ? cao->getScriptObject() : NULL);
}

scriptVar ConfigItemObject::configItem_getGuid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) 
{
  SCRIPT_FUNCTION_INIT
  ConfigItemObject *cio = static_cast<ConfigItemObject *>(o->vcpu_getInterface(CONFIGITEM_SCRIPTOBJECT_GUID));
  if (cio) return MAKE_SCRIPT_STRING(cio->getGuid());
  return MAKE_SCRIPT_STRING(L"");
}

scriptVar ConfigItemObject::configItem_newAttribute(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name, scriptVar defval) {
  SCRIPT_FUNCTION_INIT  
  ConfigItemObject *cio = static_cast<ConfigItemObject *>(o->vcpu_getInterface(CONFIGITEM_SCRIPTOBJECT_GUID));
  ConfigAttributeObject *cao = NULL;
  if (cio)
		cao = cio->newAttribute(GET_SCRIPT_STRING(name), GET_SCRIPT_STRING(defval));
  return MAKE_SCRIPT_OBJECT(cao ? cao->getScriptObject() : NULL);
}

// -----------------------------------------------------------------------------------------------------
// ConfigAttribute

function_descriptor_struct ConfigAttributeScriptObjectController::exportedFunction[] = {
  {L"getData",            0, (void*)ConfigAttributeObject::configAttr_getData},
  {L"setData",            1, (void*)ConfigAttributeObject::configAttr_setData},
  {L"onDataChanged",      0, (void*)ConfigAttributeObject::configAttr_onDataChanged},
  {L"getParentItem",      0, (void*)ConfigAttributeObject::configAttr_getParentItem},
  {L"getAttributeName",   0, (void*)ConfigAttributeObject::configAttr_getAttributeName},
};

int ConfigAttributeScriptObjectController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

void ConfigAttributeObject::setData(const wchar_t *data) 
{
  if (!item || !attr) return;
  item->setData(attr, data);
}

const wchar_t *ConfigAttributeObject::getData() 
{
  if (!item || !attr) return NULL;
  static wchar_t t[WA_MAX_PATH];
  item->getData(attr, t, WA_MAX_PATH);
  return t;
}

ConfigAttributeObject::ConfigAttributeObject(CfgItem *_item, const wchar_t *_attr, ConfigItemObject *_parent) 
{
  getScriptObject()->vcpu_setInterface(CONFIGATTRIBUTE_SCRIPTOBJECT_GUID, (void *)static_cast<ConfigAttributeObject *>(this));
  getScriptObject()->vcpu_setClassName(L"ConfigAttribute");
  getScriptObject()->vcpu_setController(configAttributeController);
  attr = _attr;
  item = _item;
  parent = _parent;
  viewer_addViewItem(item);
  autodelete = 0;
}

ConfigAttributeObject::~ConfigAttributeObject() {
  if (autodelete) getParentItem()->getCfgItem()->delAttribute(attr);
}

int ConfigAttributeObject::viewer_onEvent(CfgItem *item, int event, intptr_t param, void *ptr, size_t ptrlen) 
{
  if (event == CfgItem::Event_ATTRIBUTE_CHANGED) 
	{
    const wchar_t *_attr = reinterpret_cast<const wchar_t *>(ptr);
    if (!WCSICMP(attr, _attr))
      configAttr_onDataChanged(SCRIPT_CALL, getScriptObject());
  }
  return 1;
}

scriptVar ConfigAttributeObject::configAttr_setData(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val) 
{
  SCRIPT_FUNCTION_INIT  
  ConfigAttributeObject *cao = static_cast<ConfigAttributeObject *>(o->vcpu_getInterface(CONFIGATTRIBUTE_SCRIPTOBJECT_GUID));
  if (cao) 
		cao->setData(GET_SCRIPT_STRING(val));
  RETURN_SCRIPT_VOID;
}

scriptVar ConfigAttributeObject::configAttr_getData(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT  
  ConfigAttributeObject *cao = static_cast<ConfigAttributeObject *>(o->vcpu_getInterface(CONFIGATTRIBUTE_SCRIPTOBJECT_GUID));
  if (cao) 
		return MAKE_SCRIPT_STRING(cao->getData());
  return MAKE_SCRIPT_STRING(L"");
}

scriptVar ConfigAttributeObject::configAttr_onDataChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT  
  PROCESS_HOOKS0(o, configAttributeController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar ConfigAttributeObject::configAttr_getParentItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT  
  ConfigAttributeObject *cao = static_cast<ConfigAttributeObject *>(o->vcpu_getInterface(CONFIGATTRIBUTE_SCRIPTOBJECT_GUID));
  if (cao) return MAKE_SCRIPT_OBJECT(cao->getParentItem()->getScriptObject());
  return MAKE_SCRIPT_OBJECT(NULL);
}

scriptVar ConfigAttributeObject::configAttr_getAttributeName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT  
  ConfigAttributeObject *cao = static_cast<ConfigAttributeObject *>(o->vcpu_getInterface(CONFIGATTRIBUTE_SCRIPTOBJECT_GUID));

  if (cao) 
		return MAKE_SCRIPT_STRING(cao->getAttributeName());

  return MAKE_SCRIPT_STRING(L"");
}



