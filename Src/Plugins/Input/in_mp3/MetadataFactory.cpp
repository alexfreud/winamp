#include "MetadataFactory.h"
#include "api__in_mp3.h"
#include "WasabiMetadata.h"

static const char serviceName[] = "MP3 Stream Metadata Provider";

FOURCC MetadataFactory::GetServiceType()
{
	return MP3StreamMetadata::getServiceType(); 
}

const char *MetadataFactory::GetServiceName()
{
	return serviceName;
}

GUID MetadataFactory::GetGUID()
{
	return MP3StreamMetadataGUID;
}

void *MetadataFactory::GetInterface(int global_lock)
{
	return new MP3StreamMetadata;
}

int MetadataFactory::SupportNonLockingInterface()
{
	return 1;
}

int MetadataFactory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	svc_metaTag *metadata = static_cast<svc_metaTag *>(ifc);
	MP3StreamMetadata *mp3metadata = static_cast<MP3StreamMetadata *>(metadata);
	delete mp3metadata;
	return 1;
}

const char *MetadataFactory::GetTestString()
{
	return 0;
}

int MetadataFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS MetadataFactory
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