#include "sapplication.h"
#include <api/application/api_application.h>
#include <api.h>

// {B8E867B0-2715-4da7-A5BA-53DBA1FCFEAC}
static const GUID application_script_object_guid = 
{ 0xb8e867b0, 0x2715, 0x4da7, { 0xa5, 0xba, 0x53, 0xdb, 0xa1, 0xfc, 0xfe, 0xac } };

static ApplicationScriptObjectController _applicationController;
ScriptObjectController *applicationController=&_applicationController;

// -- Functions table -------------------------------------
function_descriptor_struct ApplicationScriptObjectController::exportedFunction[] =
{
	{L"GetApplicationName",			0, (void*)SApplication::GetApplicationName },
	{L"GetVersionString",		0, (void*)SApplication::GetVersionString },
	{L"GetVersionNumberString",			0, (void*)SApplication::GetVersionNumberString },
	{L"GetBuildNumber",0, (void*)SApplication::GetBuildNumber },
	{L"GetGUID",0, (void*)SApplication::GetGUID },
	{L"GetCommandLine",0, (void*)SApplication::GetCommandLine },
	{L"Shutdown",0, (void*)SApplication::Shutdown },
	{L"CancelShutdown",0, (void*)SApplication::CancelShutdown },
	{L"IsShuttingDown",0, (void*)SApplication::IsShuttingDown },
	{L"GetApplicationPath",0, (void*)SApplication::GetApplicationPath },
	{L"GetSettingsPath",0, (void*)SApplication::GetSettingsPath },
	{L"GetWorkingPath",0, (void*)SApplication::GetWorkingPath },
	{L"SetWorkingPath",1, (void*)SApplication::SetWorkingPath },
	{L"GetMachineGUID",0, (void*)SApplication::GetMachineGUID },
	{L"GetUserGUID",0, (void*)SApplication::GetUserGUID },
	{L"GetSessionGUID",0, (void*)SApplication::GetSessionGUID },
};
// --------------------------------------------------------

const wchar_t *ApplicationScriptObjectController::getClassName()
{
	return L"Application";
}

const wchar_t *ApplicationScriptObjectController::getAncestorClassName()
{
	return L"Object";
}

ScriptObjectController *ApplicationScriptObjectController::getAncestorController()
{
	return NULL;
}

ScriptObject *ApplicationScriptObjectController::instantiate()
{
	SApplication *c = new SApplication;
	if (!c) return NULL;
	return c->getScriptObject();
}

int ApplicationScriptObjectController::getInstantiable()
{
	return 0;
}

void ApplicationScriptObjectController::destroy(ScriptObject *o)
{
	SApplication *obj = static_cast<SApplication *>(o->vcpu_getInterface(application_script_object_guid));
	ASSERT(obj != NULL);
	delete obj;
}

void *ApplicationScriptObjectController::encapsulate(ScriptObject *o)
{
	return NULL;
}

void ApplicationScriptObjectController::deencapsulate(void *o)
{
}

int ApplicationScriptObjectController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *ApplicationScriptObjectController::getExportedFunctions()
{
	return exportedFunction;
}

GUID ApplicationScriptObjectController::getClassGuid()
{
	return application_script_object_guid;
}


SApplication::SApplication()
{
	getScriptObject()->vcpu_setInterface(application_script_object_guid, static_cast<SApplication *>(this));
	getScriptObject()->vcpu_setClassName(L"Application");
	getScriptObject()->vcpu_setController(applicationController);
}
SApplication::~SApplication()
{
}

static wchar_t guid_scratchpad[40];

scriptVar SApplication::GetApplicationName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING(WASABI_API_APP->main_getAppName());
}

scriptVar SApplication::GetVersionString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING(WASABI_API_APP->main_getVersionString());
}

scriptVar SApplication::GetVersionNumberString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING(WASABI_API_APP->main_getVersionNumString());
}

scriptVar SApplication::GetBuildNumber(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT(WASABI_API_APP->main_getBuildNumber());
}

scriptVar SApplication::GetGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	GUID g = WASABI_API_APP->main_getGUID();
	nsGUID::toCharW(g, guid_scratchpad);
	return MAKE_SCRIPT_STRING(guid_scratchpad);
}

scriptVar SApplication::GetCommandLine(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING(WASABI_API_APP->main_getCommandLine());
}

scriptVar SApplication::Shutdown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	WASABI_API_APP->main_shutdown();
	return MAKE_SCRIPT_VOID();
}

scriptVar SApplication::CancelShutdown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	WASABI_API_APP->main_cancelShutdown();
	return MAKE_SCRIPT_VOID();
}

scriptVar SApplication::IsShuttingDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_BOOLEAN(WASABI_API_APP->main_isShuttingDown());
}

scriptVar SApplication::GetApplicationPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING(WASABI_API_APP->path_getAppPath());
}

scriptVar SApplication::GetSettingsPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING(WASABI_API_APP->path_getUserSettingsPath());
}

scriptVar SApplication::GetWorkingPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING(WASABI_API_APP->path_getWorkingPath());
}

scriptVar SApplication::SetWorkingPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar string_path)
{
	SCRIPT_FUNCTION_INIT;
	WASABI_API_APP->path_setWorkingPath(GET_SCRIPT_STRING(string_path));
	return MAKE_SCRIPT_VOID();
}

scriptVar SApplication::GetMachineGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	GUID g;
	WASABI_API_APP->GetMachineID(&g);
	nsGUID::toCharW(g, guid_scratchpad);
	return MAKE_SCRIPT_STRING(guid_scratchpad);
}

scriptVar SApplication::GetUserGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	GUID g;
	WASABI_API_APP->GetUserID(&g);
	nsGUID::toCharW(g, guid_scratchpad);
	return MAKE_SCRIPT_STRING(guid_scratchpad);
}

scriptVar SApplication::GetSessionGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	GUID g;
	WASABI_API_APP->GetSessionID(&g);
	nsGUID::toCharW(g, guid_scratchpad);
	return MAKE_SCRIPT_STRING(guid_scratchpad);
}
