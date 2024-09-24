#include "api__ml_local.h"
#include "mldbApiFactory.h"
#include "mldbApi.h"
MLDBAPI mldbApi;
static const char serviceName[] = "Media Library Database API";

FOURCC MLDBAPIFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *MLDBAPIFactory::GetServiceName()
{
	return serviceName;
}

GUID MLDBAPIFactory::GetGUID()
{
	return mldbApiGuid;
}

void *MLDBAPIFactory::GetInterface(int global_lock)
{
	return &mldbApi;
//	if (global_lock)
//		plugin.service->service_lock(this, (void *)ifc);
}

int MLDBAPIFactory::SupportNonLockingInterface()
{
	return 1;
}

int MLDBAPIFactory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	return 1;
}

const char *MLDBAPIFactory::GetTestString()
{
	return NULL;
}

int MLDBAPIFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS MLDBAPIFactory
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
