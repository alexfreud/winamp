#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_WASABI_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_WASABI_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <api/service/api_service.h>
extern api_service *wasabiManager;
#define WASABI_API_SVC wasabiManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"

#include "../../devices/api_devicemanager.h"
extern api_devicemanager *deviceManagerApi;
#define WASABI_API_DEVICES deviceManagerApi

BOOL 
Wasabi_Initialize(HINSTANCE instance,
				  api_service *serviceMngr);

BOOL 
Wasabi_InitializeFromWinamp(HINSTANCE instance, 
							HWND winampWindow);

BOOL
Wasabi_LoadDefaultServices(void);

unsigned long 
Wasabi_AddRef(void);

unsigned long
Wasabi_Release(void);

void *
Wasabi_QueryInterface0(const GUID &interfaceGuid);

#define Wasabi_QueryInterface(_interfaceType, _interfaceGuid)\
		((##_interfaceType*)Wasabi_QueryInterface0(_interfaceGuid))


void 
Wasabi_ReleaseInterface0(const GUID &interfaceGuid, 
						 void *interfaceInstance);

#define Wasabi_ReleaseInterface(_interfaceGuid, _interfaceInstance)\
		(Wasabi_ReleaseInterface0((_interfaceGuid), (_interfaceInstance)))


#define Wasabi_ReleaseObject(_object)\
		{if (NULL != (_object)) {((Dispatchable*)(_object))->Release();}} 


#endif // _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_WASABI_HEADER
