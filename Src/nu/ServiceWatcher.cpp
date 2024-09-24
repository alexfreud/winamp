#include "ServiceWatcher.h"
#include <api/service/waservicefactory.h>


static void *GetService(api_service *p_serviceManager, GUID p_serviceGUID)
{
	waServiceFactory *sf = p_serviceManager->service_getServiceByGuid( p_serviceGUID);
	if (sf)
		return sf->getInterface();
	else
		return 0;
}

static void ReleaseService(api_service *p_serviceManager, GUID p_serviceGUID, void *p_service)
{
	waServiceFactory *sf = p_serviceManager->service_getServiceByGuid( p_serviceGUID);
	if (sf)
		sf->releaseInterface( p_service);
}

void ServiceWatcher::WatchWith(api_service *_serviceApi) 
{ 
	serviceManager =_serviceApi;
	systemCallbacks=(api_syscb*)GetService( serviceManager, syscbApiServiceGuid);
}

void ServiceWatcher::StopWatching()
{
	if (systemCallbacks)
	{
		systemCallbacks->syscb_deregisterCallback(this);
		ReleaseService( serviceManager, syscbApiServiceGuid, systemCallbacks);
	}
	systemCallbacks=0;
}
void ServiceWatcher::Clear() 
{
	//watchList.Reset();
	watchList.clear();
}

ServiceWatcher::~ServiceWatcher()
{
	//StopWatching();
}

void ServiceWatcher::WatchForT(void **ptr, GUID watchGUID)
{
	watchList[watchGUID]=ptr;
	if (!*ptr) // try to get it if we need it
	{
		*ptr = GetService( serviceManager, watchGUID);
	}
}

int ServiceWatcher::Notify(int msg, intptr_t param1, intptr_t param2)
{
	switch (msg)
	{
		case SvcCallback::ONREGISTER:
		{
			waServiceFactory *sf = reinterpret_cast<waServiceFactory*>(param2);
			GUID serviceGUID = sf->getGuid();
			if (serviceGUID != INVALID_GUID)
			{
				WatchList::iterator itr = watchList.find(serviceGUID);
				if (itr!=watchList.end())
				{
					void **ptr = itr->second;
					if (ptr && !*ptr) // don't re-retrieve service if we already have it
					{
						*ptr = sf->getInterface();
					}
				}
			}
		}
		break;
		case SvcCallback::ONDEREGISTER:
		{
			waServiceFactory *sf = reinterpret_cast<waServiceFactory*>(param2);
			GUID serviceGUID = sf->getGuid();
			if (serviceGUID != INVALID_GUID)
			{
				WatchList::iterator itr = watchList.find(serviceGUID);
				if (itr!=watchList.end())
				{
					void **ptr = itr->second;
					if (ptr && *ptr)
					{
						// benski> probably not safe to do, so i'll leave it commented out: sf->releaseInterface(*ptr);
						*ptr = 0;
					}
				}
			}
		}
		break;
		default: return 0;
	}
	return 1;
}

#define CBCLASS ServiceWatcher
START_DISPATCH;
CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
CB(SYSCALLBACK_NOTIFY, Notify);
END_DISPATCH;
#undef CBCLASS

ServiceWatcherSingle::~ServiceWatcherSingle()
{
	//StopWatching();
}

void ServiceWatcherSingle::StopWatching()
{
	if (systemCallbacks)
	{
		systemCallbacks->syscb_deregisterCallback(this);
		ReleaseService( serviceManager, syscbApiServiceGuid, systemCallbacks);
	}
	systemCallbacks=0;
}

void ServiceWatcherSingle::WatchWith(api_service *_serviceApi) 
{
	serviceManager =_serviceApi;
	systemCallbacks=(api_syscb*)GetService( serviceManager, syscbApiServiceGuid);
}

void ServiceWatcherSingle::WatchForT(void **ptr, GUID watchGUID)
{
	service=ptr;
	serviceGUID=watchGUID;
	if (ptr && !*ptr) // try to get it if we need it
	{
		*ptr = GetService( serviceManager, watchGUID);
		if (*ptr)
			OnRegister();
	}
}

int ServiceWatcherSingle::Notify(int msg, intptr_t param1, intptr_t param2)
{
	switch (msg)
	{
		case SvcCallback::ONREGISTER:
		{
			if (service && !*service) // don't re-retrieve service if we already have it
			{
				waServiceFactory *sf = reinterpret_cast<waServiceFactory*>(param2);
				if (sf && sf->getGuid() == serviceGUID)
				{
					*service = sf->getInterface();
					if (*service)
					OnRegister();
				}
			}
		}
		break;
		case SvcCallback::ONDEREGISTER:
		{
			if (service && *service)
			{
				waServiceFactory *sf = reinterpret_cast<waServiceFactory*>(param2);
				if (serviceGUID == sf->getGuid())
				{
					OnDeregister();
					*service=0;
				}
			}
		}
		break;
		default: return 0;
	}
	return 1;
}

#define CBCLASS ServiceWatcherSingle
START_DISPATCH;
CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
CB(SYSCALLBACK_NOTIFY, Notify);
END_DISPATCH;
#undef CBCLASS