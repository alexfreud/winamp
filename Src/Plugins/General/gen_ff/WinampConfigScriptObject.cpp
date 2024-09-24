#include <precomp.h>
#include "WinampConfigScriptObject.h"

// {B2AD3F2B-31ED-4e31-BC6D-E9951CD555BB}
static const GUID winampConfigScriptGuid = 
{ 0xb2ad3f2b, 0x31ed, 0x4e31, { 0xbc, 0x6d, 0xe9, 0x95, 0x1c, 0xd5, 0x55, 0xbb } };

// {FC17844E-C72B-4518-A068-A8F930A5BA80}
static const GUID winampConfigGroupScriptGuid = 
{ 0xfc17844e, 0xc72b, 0x4518, { 0xa0, 0x68, 0xa8, 0xf9, 0x30, 0xa5, 0xba, 0x80 } };

static WinampConfigScriptController _winampConfigController;
ScriptObjectController *winampConfigController = &_winampConfigController;

static WinampConfigGroupScriptController _winampConfigGroupController;
ScriptObjectController *winampConfigGroupController = &_winampConfigGroupController;

BEGIN_SERVICES(WinampConfig_svcs);
DECLARE_SERVICETSINGLE(svc_scriptObject, WinampConfigScriptObjectSvc);
END_SERVICES(WinampConfig_svcs, _WinampConfig_svcs);

#ifdef _X86_
extern "C"
{
	int _link_WinampConfig_svcs;
}
#else
extern "C"
{
	int __link_WinampConfig_svcs;
}
#endif
// -----------------------------------------------------------------------------------------------------
// Service

ScriptObjectController *WinampConfigScriptObjectSvc::getController(int n)
{
	switch (n)
	{
		case 0:
			return winampConfigController;
		case 1:
			return winampConfigGroupController;
	}
	return NULL;
}

// -- Functions table -------------------------------------
function_descriptor_struct WinampConfigScriptController::exportedFunction[] = 
{
  {L"getGroup", 1, (void*)WinampConfig::script_vcpu_getGroup },
};

// --------------------------------------------------------

const wchar_t *WinampConfigScriptController::getClassName() 
{
	return L"WinampConfig";
}

const wchar_t *WinampConfigScriptController::getAncestorClassName() 
{
  return L"Object";
}

ScriptObject *WinampConfigScriptController::instantiate() 
{
  WinampConfig *wc = new WinampConfig;
  ASSERT(wc != NULL);
  return wc->getScriptObject();
}

void WinampConfigScriptController::destroy(ScriptObject *o) 
{
  WinampConfig *wc = static_cast<WinampConfig *>(o->vcpu_getInterface(winampConfigScriptGuid));
	if (wc)
		delete wc;
}

void *WinampConfigScriptController::encapsulate(ScriptObject *o) 
{
  return NULL; // no encapsulation yet
}

void WinampConfigScriptController::deencapsulate(void *o) 
{
}

int WinampConfigScriptController::getNumFunctions() 
{
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *WinampConfigScriptController::getExportedFunctions() 
{
  return exportedFunction;                                                        
}

GUID WinampConfigScriptController::getClassGuid() 
{
  return winampConfigScriptGuid;
}

/* ------------- */

WinampConfig::WinampConfig()
{
		getScriptObject()->vcpu_setInterface(winampConfigScriptGuid, (void *)static_cast<WinampConfig *>(this));
	getScriptObject()->vcpu_setClassName(L"WinampConfig");
	getScriptObject()->vcpu_setController(winampConfigController);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(Agave::AgaveConfigGUID);
	if (sf)
		config = (Agave::api_config *)sf->getInterface();
}

WinampConfig::~WinampConfig()
{
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(Agave::AgaveConfigGUID);
	if (sf)
		sf->releaseInterface(config);
}

scriptVar WinampConfig::script_vcpu_getGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar groupguid)
{
	SCRIPT_FUNCTION_INIT

	const wchar_t *g = GET_SCRIPT_STRING(groupguid);
  GUID _g = nsGUID::fromCharW(g);

	WinampConfig *wc = static_cast<WinampConfig *>(o->vcpu_getInterface(winampConfigScriptGuid));
	if (wc)
	{
		Agave::ifc_configgroup *group = wc->config->GetGroup(_g);
		if (group)
		{
			WinampConfigGroup *winampConfigGroup = new WinampConfigGroup(group);
			return MAKE_SCRIPT_OBJECT(winampConfigGroup->getScriptObject());
		}
	}
	RETURN_SCRIPT_NULL;
}

/* ------------- */

// -- Functions table -------------------------------------
function_descriptor_struct WinampConfigGroupScriptController::exportedFunction[] = 
{
  {L"getBool", 1, (void*)WinampConfigGroup::script_vcpu_getBool },
	{L"getString", 1, (void*)WinampConfigGroup::script_vcpu_getString },
	{L"getInt", 1, (void*)WinampConfigGroup::script_vcpu_getInt },
  {L"setBool", 2, (void*)WinampConfigGroup::script_vcpu_setBool },
};
// --------------------------------------------------------

const wchar_t *WinampConfigGroupScriptController::getClassName() 
{
	return L"WinampConfigGroup";
}

const wchar_t *WinampConfigGroupScriptController::getAncestorClassName() 
{
  return L"Object";
}

ScriptObject *WinampConfigGroupScriptController::instantiate() 
{
  WinampConfigGroup *wc = new WinampConfigGroup;
  ASSERT(wc != NULL);
  return wc->getScriptObject();
}

void WinampConfigGroupScriptController::destroy(ScriptObject *o) 
{
  WinampConfigGroup *wc = static_cast<WinampConfigGroup *>(o->vcpu_getInterface(winampConfigGroupScriptGuid));
	if (wc)
		delete wc;
}

void *WinampConfigGroupScriptController::encapsulate(ScriptObject *o) 
{
  return NULL; // no encapsulation yet
}

void WinampConfigGroupScriptController::deencapsulate(void *o) 
{
}

int WinampConfigGroupScriptController::getNumFunctions() 
{
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *WinampConfigGroupScriptController::getExportedFunctions() 
{
  return exportedFunction;                                                        
}

GUID WinampConfigGroupScriptController::getClassGuid() 
{
  return winampConfigGroupScriptGuid;
}

/* ------------- */

WinampConfigGroup::WinampConfigGroup()
{
getScriptObject()->vcpu_setInterface(winampConfigGroupScriptGuid, (void *)static_cast<WinampConfigGroup *>(this));
	getScriptObject()->vcpu_setClassName(L"WinampConfigGroup");
	getScriptObject()->vcpu_setController(winampConfigGroupController);
	configGroup = 0;
}

WinampConfigGroup::WinampConfigGroup(Agave::ifc_configgroup *_configGroup)
{
	getScriptObject()->vcpu_setInterface(winampConfigGroupScriptGuid, (void *)static_cast<WinampConfigGroup *>(this));
	getScriptObject()->vcpu_setClassName(L"WinampConfigGroup");
	getScriptObject()->vcpu_setController(winampConfigGroupController);
	configGroup = _configGroup;
}

Agave::ifc_configitem *WinampConfigGroup::GetItem(ScriptObject *o, scriptVar itemname)
{
	const wchar_t *item = GET_SCRIPT_STRING(itemname);
	WinampConfigGroup *group = static_cast<WinampConfigGroup *>(o->vcpu_getInterface(winampConfigGroupScriptGuid));
	if (group)
	{
		Agave::ifc_configitem *configitem = group->configGroup->GetItem(item);
		return configitem;
	}
	return 0;
}

scriptVar WinampConfigGroup::script_vcpu_getBool(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemname)
{
	SCRIPT_FUNCTION_INIT
	Agave::ifc_configitem *configitem = GetItem(o, itemname);
		if (configitem)
			return MAKE_SCRIPT_BOOLEAN(configitem->GetBool());
	
	RETURN_SCRIPT_ZERO;
}

scriptVar WinampConfigGroup::script_vcpu_getString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemname)
{
	SCRIPT_FUNCTION_INIT
	Agave::ifc_configitem *configitem = GetItem(o, itemname);
		if (configitem)
			return MAKE_SCRIPT_STRING(configitem->GetString());
	
	RETURN_SCRIPT_ZERO;
}

scriptVar WinampConfigGroup::script_vcpu_getInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemname)
{
	SCRIPT_FUNCTION_INIT
	Agave::ifc_configitem *configitem = GetItem(o, itemname);
		if (configitem)
			return MAKE_SCRIPT_INT(configitem->GetInt());
	
	RETURN_SCRIPT_ZERO;
}

scriptVar WinampConfigGroup::script_vcpu_setBool(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemname, scriptVar value)
{
	SCRIPT_FUNCTION_INIT
	const wchar_t *item = GET_SCRIPT_STRING(itemname);
	WinampConfigGroup *group = static_cast<WinampConfigGroup *>(o->vcpu_getInterface(winampConfigGroupScriptGuid));
	if (group)
	{
		Agave::ifc_configitem *configitem = group->configGroup->GetItem(item);
		if (configitem)
			configitem->SetBool(!!GET_SCRIPT_BOOLEAN(value));
	}
	RETURN_SCRIPT_VOID;
}