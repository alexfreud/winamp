#include "ScriptObjectFactory.h"
#include "api__playlist.h"
#include "ScriptObjectService.h"

ScriptObjectService svc;
static const char serviceName[] = "Playlist Maki Objects";

// {2406A46C-9740-4af7-A0EB-A544AAB8F00C}
static const GUID playlist_script_object_guid = 
{ 0x2406a46c, 0x9740, 0x4af7, { 0xa0, 0xeb, 0xa5, 0x44, 0xaa, 0xb8, 0xf0, 0xc } };


FOURCC ScriptObjectFactory::GetServiceType()
{
	return svc.getServiceType();
}

const char *ScriptObjectFactory::GetServiceName()
{
	return serviceName;
}

GUID ScriptObjectFactory::GetGUID()
{
	return playlist_script_object_guid;
}

void *ScriptObjectFactory::GetInterface(int global_lock)
{
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &svc;
}

int ScriptObjectFactory::SupportNonLockingInterface()
{
	return 1;
}

int ScriptObjectFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *ScriptObjectFactory::GetTestString()
{
	return 0;
}

int ScriptObjectFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS ScriptObjectFactory
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