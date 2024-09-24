#include "api.h"
#include "GracenoteFactory.h"
#include "GracenoteApi.h"
GracenoteApi gracenoteApi;
static const char serviceName[] = "Gracenote API";

FOURCC GracenoteFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *GracenoteFactory::GetServiceName()
{
	return serviceName;
}

GUID GracenoteFactory::GetGUID()
{
	return gracenoteApiGUID;
}

void *GracenoteFactory::GetInterface(int global_lock)
{
	return &gracenoteApi;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
}

int GracenoteFactory::SupportNonLockingInterface()
{
	return 1;
}

int GracenoteFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *GracenoteFactory::GetTestString()
{
	return NULL;
}

int GracenoteFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS GracenoteFactory
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
