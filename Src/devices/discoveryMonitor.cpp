#include "main.h"
#include "./discoveryMonitor.h"

DiscoveryMonitor::DiscoveryMonitor()
{
	InitializeCriticalSection(&lock);
}

DiscoveryMonitor::~DiscoveryMonitor()
{
	DeleteCriticalSection(&lock);
}

void DiscoveryMonitor::Lock()
{
	EnterCriticalSection(&lock);
}

void DiscoveryMonitor::Unlock()
{
	LeaveCriticalSection(&lock);
}

BOOL DiscoveryMonitor::Register(ifc_deviceprovider *provider)
{
	Lock();

	size_t index = activityList.size();
	while(index--)
	{
		ActiveDiscovery *entry = &activityList[index];
		if ((intptr_t)provider == entry->providerId)
		{
			entry->ref++;
			Unlock();
			return FALSE;
		}
	}

	ActiveDiscovery record;
	record.providerId = (intptr_t)provider;
	record.ref = 1;

	activityList.push_back(record);
	index = activityList.size();

	Unlock();
	return (1 == index);
}

BOOL DiscoveryMonitor::Unregister(ifc_deviceprovider *provider)
{
	Lock();

	size_t index = activityList.size();
	while(index--)
	{
		ActiveDiscovery *entry = &activityList[index];
		if ((intptr_t)provider == entry->providerId)
		{
			if (1 == entry->ref)
			{
				activityList.erase(activityList.begin() + index);
				index = activityList.size();
				Unlock();
				return (0 == index);
			}

			entry->ref--;
			break;
		}
	}

	Unlock();
	return FALSE;
}

BOOL DiscoveryMonitor::IsActive()
{
	size_t count;

	Lock();

	count = activityList.size();

	Unlock();

	return (0 != count);
}

BOOL DiscoveryMonitor::Reset()
{
	size_t count;

	Lock();

	count = activityList.size();
	activityList.clear();

	Unlock();

	return (0 != count);
}


