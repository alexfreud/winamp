#include "api__wasabi2.h"
#include "wasabi1_factory.h"


static const char serviceName[] = "Wasabi 2 Service API";

FOURCC Wasabi2ServiceFactory::GetServiceType()
{
	return Wasabi::WaSvc::UNIQUE; 
}

const char *Wasabi2ServiceFactory::GetServiceName()
{
	return serviceName;
}

GUID Wasabi2ServiceFactory::GetGUID()
{
	return api_service::GetServiceGUID();
}

void *Wasabi2ServiceFactory::GetInterface(int global_lock)
{
	return (api_service *)WASABI2_API_SVC;
	
}

int Wasabi2ServiceFactory::SupportNonLockingInterface()
{
	return 1;
}

int Wasabi2ServiceFactory::ReleaseInterface(void *ifc)
{
	return 1;
}

const char *Wasabi2ServiceFactory::GetTestString()
{
	return NULL;
}

int Wasabi2ServiceFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS Wasabi2ServiceFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE,                GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME,                GetServiceName)
CB(WASERVICEFACTORY_GETGUID,                       GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE,                  GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE,              ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING,                 GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY,                 ServiceNotify)
END_DISPATCH;
