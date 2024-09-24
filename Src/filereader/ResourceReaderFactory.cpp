#include "api__filereader.h"
#include "ResourceReaderFactory.h"
#include "ResourceReader.h"

static const char serviceName[] = "Resource reader";

// {C975969A-5DFD-4f2b-B767-4EDC6C7D6484}
static const GUID ResourceReaderGUID = 
{ 0xc975969a, 0x5dfd, 0x4f2b, { 0xb7, 0x67, 0x4e, 0xdc, 0x6c, 0x7d, 0x64, 0x84 } };


FOURCC ResourceReaderFactory::GetServiceType()
{
	return WaSvc::FILEREADER; 
}

const char *ResourceReaderFactory::GetServiceName()
{
	return serviceName;
}

GUID ResourceReaderFactory::GetGUID()
{
	return ResourceReaderGUID;
}

void *ResourceReaderFactory::GetInterface( int global_lock )
{
	ResourceReader *ifc = new ResourceReader;

	if ( global_lock )
		WASABI_API_SVC->service_lock( this, (void *)ifc );

	return ifc;
}

int ResourceReaderFactory::SupportNonLockingInterface()
{
	return 1;
}

int ResourceReaderFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	svc_fileReader *reader = static_cast<svc_fileReader *>(ifc);
	ResourceReader *resourceReader = static_cast<ResourceReader *>(reader);
	delete resourceReader;
	return 1;
}

const char *ResourceReaderFactory::GetTestString()
{
	return 0;
}

int ResourceReaderFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ResourceReaderFactory
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
