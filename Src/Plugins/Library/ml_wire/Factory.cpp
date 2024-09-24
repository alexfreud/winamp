#include "api__ml_wire.h"
#include "Factory.h"
#include "Wire.h"

static const char serviceName[] = "Podcasts";

FOURCC PodcastsFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *PodcastsFactory::GetServiceName()
{
	return serviceName;
}

GUID PodcastsFactory::GetGUID()
{
	return api_podcastsGUID;
}

void *PodcastsFactory::GetInterface( int global_lock )
{
//	if (global_lock)
//		plugin.service->service_lock(this, (void *)ifc);
	return &channels;
}

int PodcastsFactory::SupportNonLockingInterface()
{
	return 1;
}

int PodcastsFactory::ReleaseInterface( void *ifc )
{
	//plugin.service->service_unlock(ifc);
	return 1;
}

const char *PodcastsFactory::GetTestString()
{
	return 0;
}

int PodcastsFactory::ServiceNotify( int msg, int param1, int param2 )
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS PodcastsFactory
START_DISPATCH;
CB( WASERVICEFACTORY_GETSERVICETYPE,                GetServiceType )
CB( WASERVICEFACTORY_GETSERVICENAME,                GetServiceName )
CB( WASERVICEFACTORY_GETGUID,                       GetGUID )
CB( WASERVICEFACTORY_GETINTERFACE,                  GetInterface )
CB( WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface )
CB( WASERVICEFACTORY_RELEASEINTERFACE,              ReleaseInterface )
CB( WASERVICEFACTORY_GETTESTSTRING,                 GetTestString )
CB( WASERVICEFACTORY_SERVICENOTIFY,                 ServiceNotify )
END_DISPATCH;
