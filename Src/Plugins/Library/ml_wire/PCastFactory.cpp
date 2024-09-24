#include "api__ml_wire.h"
#include ".\pcastfactory.h"
#include "Wire.h"

static PCastURIHandler PCastUH;

static const char serviceName[] = "PCast";

FOURCC PCastFactory::GetServiceType()
{
	return svc_urihandler::getServiceType(); 
}

const char *PCastFactory::GetServiceName()
{
	return serviceName;
}

void *PCastFactory::GetInterface(int global_lock)
{
//	if (global_lock)
//		plugin.service->service_lock(this, (void *)ifc);
	return &PCastUH;
}

int PCastFactory::SupportNonLockingInterface()
{
	return 1;
}

int PCastFactory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	return 1;
}

const char *PCastFactory::GetTestString()
{
	return NULL;
}

int PCastFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 0;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS PCastFactory
START_DISPATCH;
CB( WASERVICEFACTORY_GETSERVICETYPE, GetServiceType )
CB( WASERVICEFACTORY_GETSERVICENAME, GetServiceName )
CB( WASERVICEFACTORY_GETINTERFACE, GetInterface )
CB( WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface )
CB( WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface )
CB( WASERVICEFACTORY_GETTESTSTRING, GetTestString )
CB( WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify )
END_DISPATCH;
