#ifndef _NULLSOFT_WINAMP_DEVICES_DISCOVERY_MONITOR_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DISCOVERY_MONITOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "ifc_deviceprovider.h"
#include <vector>

class DiscoveryMonitor
{
public:
	DiscoveryMonitor();
	~DiscoveryMonitor();

public:
	BOOL Register(ifc_deviceprovider *provider);
	BOOL Unregister(ifc_deviceprovider *provider);
	BOOL IsActive();
	BOOL Reset();

protected:
	void Lock();
	void Unlock();

private:
	typedef struct ActiveDiscovery
	{
		intptr_t providerId;
		size_t ref;
	} ActiveDiscovery;

	typedef std::vector<ActiveDiscovery> ActivityList;

private:
	CRITICAL_SECTION lock;
	ActivityList activityList;
};



#endif // _NULLSOFT_WINAMP_DEVICES_DISCOVERY_MONITOR_HEADER