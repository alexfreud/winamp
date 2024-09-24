#include "ScriptFactory.h"
#include "api.h"
#include "ScriptObjectService.h"

static ScriptObjectService svc;
static const char serviceName[] = "Timer Maki Object";

// GUID of our service factory, not the GUID of the maki object
// {538A1D71-74B0-4fbf-877A-241D10A937F3}
static const GUID timer_maki_guid = 
{ 0x538a1d71, 0x74b0, 0x4fbf, { 0x87, 0x7a, 0x24, 0x1d, 0x10, 0xa9, 0x37, 0xf3 } };


FOURCC ScriptFactory::GetServiceType()
{
	return svc.getServiceType();
}

const char *ScriptFactory::GetServiceName()
{
	return serviceName;
}

GUID ScriptFactory::GetGUID()
{
	return timer_maki_guid;
}

void *ScriptFactory::GetInterface(int global_lock)
{
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &svc;
}

int ScriptFactory::SupportNonLockingInterface()
{
	return 1;
}

int ScriptFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *ScriptFactory::GetTestString()
{
	return 0;
}

int ScriptFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS ScriptFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
#undef CBCLASS