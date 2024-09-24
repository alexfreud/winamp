/* copyright 2006 Ben Allison */
#include "factory_alac.h"
#include "ALACMP4Decoder.h"

static const char serviceName[] = "ALAC Decoder";
static const char testString[] = "alac";

// {47ADFABA-BDB5-4e2e-A91F-BC184C123DE9}
static const GUID api_downloads_GUID= { 0x47adfaba, 0xbdb5, 0x4e2e, { 0xa9, 0x1f, 0xbc, 0x18, 0x4c, 0x12, 0x3d, 0xe9 } };

FOURCC ALACFactory::GetServiceType()
{
	return WaSvc::MP4AUDIODECODER; 
}

const char *ALACFactory::GetServiceName()
{
	return serviceName;
}

GUID ALACFactory::GetGUID()
{
	return api_downloads_GUID;
}

void *ALACFactory::GetInterface(int global_lock)
{
	MP4AudioDecoder *ifc=new ALACMP4Decoder;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int ALACFactory::SupportNonLockingInterface()
{
	return 1;
}

int ALACFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	MP4AudioDecoder *decoder = static_cast<MP4AudioDecoder *>(ifc);
	ALACMP4Decoder *aacPlusDecoder = static_cast<ALACMP4Decoder *>(decoder);
	delete aacPlusDecoder;
	return 1;
}

const char *ALACFactory::GetTestString()
{
	return testString;
}

int ALACFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ALACFactory
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
