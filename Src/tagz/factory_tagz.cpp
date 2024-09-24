//#define GUID_EQUALS_DEFINED
#include "api__tagz.h"
#include "factory_tagz.h"
#include "impl_tagz.h"

Tagz tagz;

static const char serviceName[] = "Advanced Title Formatter";

FOURCC TagzFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *TagzFactory::GetServiceName()
{
	return serviceName;
}

GUID TagzFactory::GetGUID()
{
	return tagzGUID;
}

void *TagzFactory::GetInterface(int global_lock)
{
	return &tagz;
}

int TagzFactory::SupportNonLockingInterface()
{
	return 1;
}

int TagzFactory::ReleaseInterface(void *ifc)
{
	return 1;
}

const char *TagzFactory::GetTestString()
{
	return NULL;
}

int TagzFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS TagzFactory
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
