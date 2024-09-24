#ifndef NULLOSFT_ONLINEMEDIA_WASABI_API_HEADER
#define NULLOSFT_ONLINEMEDIA_WASABI_API_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#include <api/service/api_service.h>
extern api_service *wasabiManager;
#define WASABI_API_SVC wasabiManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Config/api_config.h"
#include "../Agave/Language/api_language.h"

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memManagerApi;
#define WASABI_API_MEMMNGR memManagerApi

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_securityApi;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_securityApi

#include <api/service/svcs/svc_imgload.h>
extern svc_imageLoader *pngLoaderApi;
#define WASABI_API_PNGLOADER pngLoaderApi
EXTERN_C const GUID pngLoaderGUID;

#include "../auth/api_auth.h"
extern api_auth *authApi;
#define AGAVE_API_AUTH authApi

#include <obj_ombrowser.h>
extern obj_ombrowser *browserManager;
#define OMBROWSERMNGR browserManager

#include <ifc_omservicemanager.h>
extern ifc_omservicemanager *serviceManager;
#define OMSERVICEMNGR serviceManager

#include <ifc_omutility.h>
extern ifc_omutility *omUtility;
#define OMUTILITY omUtility

HRESULT WasabiApi_Initialize(HINSTANCE hInstance, api_service *serviceApi);
HRESULT WasabiApi_LoadDefaults();
ULONG WasabiApi_AddRef(void);
ULONG WasabiApi_Release(void);

void *Wasabi_QueryInterface(REFGUID interfaceGuid);
void Wasabi_ReleaseInterface(REFGUID interfaceGuid, void *pInstance);

#define QueryWasabiInterface(__interfaceType, __interfaceGuid) ((##__interfaceType*)Wasabi_QueryInterface(__interfaceGuid))
#define ReleaseWasabiInterface(__interfaceGuid, __interfaceInstance) (Wasabi_ReleaseInterface((__interfaceGuid), (__interfaceInstance)))

#endif // NULLOSFT_ONLINEMEDIA_WASABI_API_HEADER