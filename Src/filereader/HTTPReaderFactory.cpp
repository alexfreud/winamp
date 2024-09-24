#include "api__filereader.h"
#include "HTTPReaderFactory.h"
#include "HTTPReader.h"

static const char serviceName[] = "HTTP reader";

// {bc10fa00-53F5-4032-BD29-032B87EC3404}
static const GUID HTTPReaderGUID =
  { 0xbc10fa00, 0x53f5, 0x4032, { 0xa0, 0x09, 0x2, 0x2b, 0x87, 0xec, 0x34, 0x04 } };


FOURCC HTTPReaderFactory::GetServiceType()
{
	return WaSvc::FILEREADER;
}

const char *HTTPReaderFactory::GetServiceName()
{
	return serviceName;
}

GUID HTTPReaderFactory::GetGUID()
{
	return HTTPReaderGUID;
}

void *HTTPReaderFactory::GetInterface( int global_lock )
{
	HTTPReader *ifc = new HTTPReader;

	if ( global_lock )
		WASABI_API_SVC->service_lock( this, (void *)ifc );

	return ifc;
}

int HTTPReaderFactory::SupportNonLockingInterface()
{
	return 1;
}

int HTTPReaderFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	svc_fileReader *reader = static_cast<svc_fileReader *>(ifc);
	HTTPReader *resourceReader = static_cast<HTTPReader *>(reader);
	delete resourceReader;
	return 1;
}

const char *HTTPReaderFactory::GetTestString()
{
	return 0;
}

int HTTPReaderFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS HTTPReaderFactory
START_DISPATCH;
CB( WASERVICEFACTORY_GETSERVICETYPE,                GetServiceType )
CB( WASERVICEFACTORY_GETSERVICENAME,                GetServiceName )
CB( WASERVICEFACTORY_GETGUID,                       GetGUID )
CB( WASERVICEFACTORY_GETINTERFACE,                  GetInterface )
CB( WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface )
CB( WASERVICEFACTORY_RELEASEINTERFACE,              ReleaseInterface )
CB( WASERVICEFACTORY_GETTESTSTRING,                 GetTestString )
CB( WASERVICEFACTORY_SERVICENOTIFY,                 ServiceNotify )
END_DISPATCH;
