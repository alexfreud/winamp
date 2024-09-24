#include "HistoryAPIFactory.h"
#include "api__ml_history.h"
#include "HistoryAPI.h"

HistoryAPI historyAPI;
static const char serviceName[] = "History API";

FOURCC HistoryAPIFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *HistoryAPIFactory::GetServiceName()
{
	return serviceName;
}

GUID HistoryAPIFactory::GetGUID()
{
	return HistoryApiGuid;
}

void *HistoryAPIFactory::GetInterface(int global_lock)
{
//	if (global_lock)
//		plugin.service->service_lock(this, (void *)ifc);
	return &historyAPI;
}

int HistoryAPIFactory::SupportNonLockingInterface()
{
	return 1;
}

int HistoryAPIFactory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	return 1;
}

const char *HistoryAPIFactory::GetTestString()
{
	return 0;
}

int HistoryAPIFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS HistoryAPIFactory
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