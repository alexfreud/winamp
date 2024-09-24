#include "factory_nsvVlb.h"
#include "NSVFactory.h"

static const char serviceName[] = "Dolby VLB NSV Decoder";

// {69D8A07C-ECE4-44f8-9C40-12506422A882}
static const GUID nsv_vlb_guid =
{ 0x69d8a07c, 0xece4, 0x44f8, { 0x9c, 0x40, 0x12, 0x50, 0x64, 0x22, 0xa8, 0x82 } };


static NSVFactory nsvFactory;
FOURCC NSVVLBFactory::GetServiceType()
{
	return WaSvc::NSVFACTORY; 
}

const char *NSVVLBFactory::GetServiceName()
{
	return serviceName;
}

GUID NSVVLBFactory::GetGUID()
{
	return nsv_vlb_guid;
}

void *NSVVLBFactory::GetInterface(int global_lock)
{
	svc_nsvFactory *ifc = &nsvFactory;
	return ifc;
}

int NSVVLBFactory::SupportNonLockingInterface()
{
	return 1;
}

int NSVVLBFactory::ReleaseInterface(void *ifc)
{
	return 1;
}

const char *NSVVLBFactory::GetTestString()
{
	return 0;
}

int NSVVLBFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}


#define CBCLASS NSVVLBFactory
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