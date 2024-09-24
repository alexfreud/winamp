#include "ServiceManager.h"
#include "api__wasabi-replicant.h"
#include "service/ifc_servicefactory.h"
#include "service/svccb.h"

using namespace nu;

ServiceManager::ServiceManager()
{
#ifdef _WIN32
	component_wait = CreateSemaphoreW(NULL, 0, LONG_MAX, NULL);
#else
	sem_init(&component_wait, 0, 0);
#endif

}

ServiceManager::~ServiceManager()
{
	#ifdef _WIN32
	if (component_wait)
		CloseHandle(component_wait);
#else
	sem_destroy(&component_wait);
#endif
}

int ServiceManager::Dispatchable_QueryInterface(GUID interface_guid, void **object) 
{
	if (interface_guid == ifc_component_sync::GetInterfaceGUID())
	{
		*object = (ifc_component_sync *)this;
	}
	return NErr_Unknown;
}
//-------------------------------------------
int ServiceManager::GetServiceIndex(GUID key)
{
	for(int idx = 0; idx < services_indexer.size(); idx++)
	{
		if (memcmp(&key, &services_indexer[idx], sizeof(GUID)) == 0)
		{
			return idx;
		}
	}
	
	return -1;
}

int ServiceManager::Service_Register(ifc_serviceFactory *svc)
{
	AutoLock lock(serviceGuard LOCKNAME("ServiceManager::service_register"));
	GUID service_type = svc->GetServiceType();
	GUID service_id = svc->GetGUID();

	// add the service to the master list
	ifc_serviceFactory* new_factory = services[service_id];
	if (new_factory) // if someone already has this GUID, we need to replace
	{
		// replace factory in services_by_type map
		ServiceList* type_list = services_by_type[service_type];

		if (type_list)
		{
			for (ServiceList::iterator itr=type_list->begin();itr!=type_list->end();itr++)
			{
				ifc_serviceFactory *f = *itr;
				if (f->GetGUID() == service_id)
				{
					*itr = svc;
				}
			}
		}
		// tell the old factory we're kicking its ass to the curb.
		new_factory->ServiceNotify(ifc_serviceFactory::ONUNREGISTERED);
		// HAKAN:
		// Should we delete old factory?
		// new_factory = svc;
	}
	else // not used yet, just assign
	{
		//new_factory = svc;
		services_indexer.push_back(service_id);

		// add it to the by-type lookup
		ServiceList *&type_list = services_by_type[service_type];
		if (!type_list)
			type_list = new ServiceList;

		type_list->push_back(svc);
	}

	services[service_id]=svc;

	// send notifications
	svc->ServiceNotify(ifc_serviceFactory::ONREGISTERED);

	WASABI2_API_SYSCB->IssueCallback(Service::event_type,
		Service::on_register,
		(intptr_t)&service_type, reinterpret_cast<intptr_t>(svc));
	return NErr_Success;
}

int ServiceManager::Service_Unregister(ifc_serviceFactory *svc)
{
	AutoLock lock(serviceGuard LOCKNAME("ServiceManager::Service_Unregister"));
	GUID service_type = svc->GetServiceType();
	GUID service_id = svc->GetGUID();

	// remove it from the master list
	ServiceMap::iterator itr = services.find(service_id);
	if (itr != services.end())
		services.erase(itr);

	// and from the type lookup map
	ServiceList *type_list = services_by_type[service_type];
	if (type_list)
	{
		//type_list->eraseObject(svc);
		for (auto it = type_list->begin(); it != type_list->end(); it++)
		{
			if (*it == svc)
			{
				it = type_list->erase(it);
				break;
			}
		}
	}
	WASABI2_API_SYSCB->IssueCallback(Service::event_type,	Service::on_deregister, (intptr_t)&service_type, reinterpret_cast<intptr_t>(svc));
	svc->ServiceNotify(ifc_serviceFactory::ONUNREGISTERED);

	return NErr_Success;
}

size_t ServiceManager::Service_GetServiceCount(GUID svc_type)
{
	AutoLock lock(serviceGuard LOCKNAME("ServiceManager::Service_GetServiceCount"));
	ServiceList *type_list = services_by_type[svc_type];
	if (type_list)
		return type_list->size();
	else
		return 0;
}

ifc_serviceFactory *ServiceManager::Service_EnumService(GUID svc_type, size_t n)
{
	AutoLock lock(serviceGuard LOCKNAME("ServiceManager::Service_EnumService"));
	ServiceList *type_list = services_by_type[svc_type];
	if (type_list && (size_t)n < type_list->size())
		return type_list->at(n);
	else
		return 0;
}

ifc_serviceFactory *ServiceManager::Service_EnumService(size_t n)
{
	AutoLock lock(serviceGuard LOCKNAME("ServiceManager::Service_EnumService"));
	if ((size_t)n < services.size())
	{
		//return services.at(n).second;
		if (n < services_indexer.size())
		{
			GUID g = services_indexer[n];
			return services[g];
		}
	}
	
	return 0;
}

ifc_serviceFactory *ServiceManager::Service_GetServiceByGUID(GUID guid)
{
	AutoLock lock(serviceGuard LOCKNAME("ServiceManager::service_getServiceByGuid"));
	ServiceMap::iterator itr = services.find(guid);
	if (itr != services.end())
		return itr->second;
	else
		return 0;
}

void ServiceManager::Service_ComponentDone()
{
#ifdef _WIN32
		ReleaseSemaphore(component_wait, 1, NULL);
#else
		sem_post(&component_wait);
#endif
}

int ServiceManager::ComponentSync_Wait(size_t count)
{
	while (count--)
	{
#ifdef _WIN32
		WaitForSingleObject(component_wait, INFINITE);
#else
		sem_wait(&component_wait);
#endif
	}
	return NErr_Success;
}
