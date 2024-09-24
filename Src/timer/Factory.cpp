#include "Factory.h"
#include "api.h"
#include "timerapi.h"

TimerApi *timer_svc = 0;
static const char serviceName[] = "Timer Service";

void Factory::Stop()
{
	delete timer_svc;
	timer_svc=0;
}
FOURCC Factory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *Factory::GetServiceName()
{
	return serviceName;
}

GUID Factory::GetGUID()
{
	return timerApiServiceGuid;
}

void *Factory::GetInterface(int global_lock)
{
	if (!timer_svc)
		timer_svc = new TimerApi;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return timer_svc;
}

int Factory::SupportNonLockingInterface()
{
	return 1;
}

int Factory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *Factory::GetTestString()
{
	return 0;
}

int Factory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS Factory
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