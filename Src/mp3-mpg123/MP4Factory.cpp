#include "api__mp3-mpg123.h"
#include "MP4Factory.h"
#include "mp3_in_mp4.h"

static const char serviceName[] = "MP4 MP3 Decoder";
static const char testString[] = "mp4a";

// {ADBFEC60-41B4-4903-980B-0FA7E33D3567}
static const GUID MPEG4_MP3_GUID = 
{ 0xadbfec60, 0x41b4, 0x4903, { 0x98, 0xb, 0xf, 0xa7, 0xe3, 0x3d, 0x35, 0x67 } };

FOURCC MPEG4Factory::GetServiceType()
{
	return WaSvc::MP4AUDIODECODER; 
}

const char *MPEG4Factory::GetServiceName()
{
	return serviceName;
}

GUID MPEG4Factory::GetGUID()
{
	return MPEG4_MP3_GUID;
}

void *MPEG4Factory::GetInterface(int global_lock)
{
	return new MPEG4_MP3;
}

int MPEG4Factory::SupportNonLockingInterface()
{
	return 1;
}

int MPEG4Factory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	MP4AudioDecoder *decoder = static_cast<MP4AudioDecoder *>(ifc);
	MPEG4_MP3 *mp3Decoder = static_cast<MPEG4_MP3 *>(decoder);
	delete mp3Decoder;
	return 1;
}

const char *MPEG4Factory::GetTestString()
{
	return testString;
}

int MPEG4Factory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MPEG4Factory
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
