#include "main.h"
#include "api__playlist.h"
#include "factory_playlists.h"
#include "Playlists.h"

Playlists playlists;
static const char serviceName[] = "Playlists";

FOURCC PlaylistsFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *PlaylistsFactory::GetServiceName()
{
	return serviceName;
}

GUID PlaylistsFactory::GetGUID()
{
	return api_playlistsGUID;
}

void *PlaylistsFactory::GetInterface(int global_lock)
{
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &playlists;
}

int PlaylistsFactory::SupportNonLockingInterface()
{
	return 1;
}

int PlaylistsFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *PlaylistsFactory::GetTestString()
{
	return 0;
}

int PlaylistsFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS PlaylistsFactory
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