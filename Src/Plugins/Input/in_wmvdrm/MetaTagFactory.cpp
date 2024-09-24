#include "main.h"
#include "MetaTag.h"
#include "api.h"
#include "MetaTagFactory.h"

FOURCC MetaTagFactory::GetServiceType()
{
	return WaSvc::METATAG; 
}

const char *MetaTagFactory::GetServiceName()
{
	return ASFMetaTag::getServiceName();
}

GUID MetaTagFactory::GetGUID()
{
	return ASFMetaTag::getServiceGuid();
}

void *MetaTagFactory::GetInterface(int global_lock)
{
	svc_metaTag *ifc= new ASFMetaTag;
//	if (global_lock)
//		plugin.service->service_lock(this, (void *)ifc);
	return ifc;
}

int MetaTagFactory::SupportNonLockingInterface()
{
	return 1;
}

int MetaTagFactory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	svc_metaTag *metaTag = static_cast<svc_metaTag *>(ifc);
	ASFMetaTag *asfMetaTag = static_cast<ASFMetaTag *>(metaTag);
	delete asfMetaTag;
	return 1;
}

const char *MetaTagFactory::GetTestString()
{
	return NULL;
}

int MetaTagFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MetaTagFactory
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
