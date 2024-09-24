#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include "api/service/api_service.h"
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "api/service/waservicefactory.h"

#include "Agave/Config/api_config.h"
extern api_config *configApi;
#define AGAVE_API_CONFIG configApi

#include "api/memmgr/api_memmgr.h"
extern api_memmgr *memmgr;
#define WASABI_API_MEMMGR memmgr

#include "Agave/Language/api_language.h"

#include "Agave/AlbumArt/svc_albumArtProvider.h"

template <class api_T>
static void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
static void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
}

class AlbumArtFactory : public waServiceFactory
{
public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);
	
protected:
	RECVS_DISPATCH;
};

extern AlbumArtFactory albumArtFactory;

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

#endif