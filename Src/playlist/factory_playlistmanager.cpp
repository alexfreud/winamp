#include "main.h"
#include "api__playlist.h"
#include "factory_playlistmanager.h"
#include "PlaylistManager.h"

static const char serviceName[] = "Playlist Manager";

FOURCC PlaylistManagerFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *PlaylistManagerFactory::GetServiceName()
{
	return serviceName;
}

GUID PlaylistManagerFactory::GetGUID()
{
	return api_playlistmanagerGUID;
}

void *PlaylistManagerFactory::GetInterface(int global_lock)
{
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &playlistManager;
}

int PlaylistManagerFactory::SupportNonLockingInterface()
{
	return 1;
}

int PlaylistManagerFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *PlaylistManagerFactory::GetTestString()
{
	return 0;
}

int PlaylistManagerFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS PlaylistManagerFactory
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
