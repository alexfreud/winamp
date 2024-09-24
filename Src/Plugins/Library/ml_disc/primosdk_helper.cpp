#include "./primosdk_helper.h"
//#include "../primo/obj_primo.h"
#include "api__ml_disc.h"
#include <api/service/waservicefactory.h>

typedef struct _PRIMOSDK_INSTANCE
{
//	obj_primo *primo;
	BOOL				bLoadFailed;
	LONG				uRef;
} PRIMOSDK_INSTANCE;

static PRIMOSDK_INSTANCE sdk = {/*NULL,*/ FALSE, 0, };

BOOL PrimoSDKHelper_IsInitialized(void)
{
	return (0 != sdk.uRef);
}

BOOL PrimoSDKHelper_IsLoaded(void)
{
	/*char t[64] = {0};
	wsprintfA(t,"%d %d %d\n%d",sdk.bLoadFailed, sdk.primo, sdk.uRef, !(!sdk.uRef || !sdk.primo));
	MessageBoxA(0,t,0,0);*/
	return !(!sdk.uRef /*|| !sdk.primo*/);//(sdk.bLoadFailed==FALSE);
}

LONG PrimoSDKHelper_Initialize(void)
{	
	return 0;
#if 0
	if (sdk.bLoadFailed) return 0; 

	if (!sdk.uRef)
	{
		BOOL bFailed = TRUE;
		waServiceFactory *sf = plugin.service->service_getServiceByGuid(obj_primo::getServiceGuid());
		if (sf) sdk.primo = reinterpret_cast<obj_primo *>(sf->getInterface());
		if (sdk.primo)
			bFailed = FALSE;

		sdk.bLoadFailed += bFailed;
	}
	InterlockedIncrement(&sdk.uRef);

	return sdk.uRef;
#endif
}

LONG PrimoSDKHelper_Uninitialize(void)
{
	return 0;
#if 0
	if (sdk.uRef && 0 == InterlockedDecrement(&sdk.uRef))
	{
		if (sdk.primo)
		{
			waServiceFactory *sf = plugin.service->service_getServiceByGuid(obj_primo::getServiceGuid());
			if (sf) sf->releaseInterface(sdk.primo);
			sdk.primo = 0;
		}
	}
	return sdk.uRef;
#endif
}

DWORD PrimoSDKHelper_UnitInfo(PDWORD pdwUnit, PDWORD pdwType, PBYTE szDescr, PDWORD pdwReady)
{
	return PRIMOSDK_CMDSEQUENCE;
#if 0
	if (!sdk.uRef) return PRIMOSDK_CMDSEQUENCE;
	if (!sdk.primo) return PRIMOSDK_CMDSEQUENCE;

	return sdk.primo->UnitInfo(pdwUnit, pdwType, szDescr, pdwReady);
#endif
}

DWORD PrimoSDKHelper_UnitInfo2(PDWORD pdwUnit, PDWORD pdwTypes, PDWORD pdwClass, PDWORD pdwBusType, PDWORD pdwRFU)
{
return PRIMOSDK_CMDSEQUENCE;
#if 0
	if (!sdk.uRef) return PRIMOSDK_CMDSEQUENCE;
	if (!sdk.primo) return PRIMOSDK_CMDSEQUENCE;

	return sdk.primo->UnitInfo2(pdwUnit, pdwTypes, pdwClass, pdwBusType, pdwRFU);
#endif
}

DWORD PrimoSDKHelper_DiscInfoEx(PDWORD pdwUnit, DWORD dwFlags, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree)
{
#if 0
	if (!sdk.uRef) return PRIMOSDK_CMDSEQUENCE;
	if (!sdk.primo) return PRIMOSDK_CMDSEQUENCE;

	return sdk.primo->DiscInfoEx(pdwUnit, dwFlags, pdwMediumType, pdwMediumFormat, pdwErasable, pdwTracks, pdwUsed, pdwFree);
#endif
	return PRIMOSDK_CMDSEQUENCE;
}

DWORD PrimoSDKHelper_DiscInfo2(PDWORD pdwUnit, PDWORD pdwMedium, PDWORD pdwProtectedDVD, PDWORD pdwFlags, PDWORD pdwMediumEx, PDWORD pdwRFU3)
{
	return PRIMOSDK_CMDSEQUENCE;
#if 0
	if (!sdk.uRef) return PRIMOSDK_CMDSEQUENCE;
	if (!sdk.primo) return PRIMOSDK_CMDSEQUENCE;

	return sdk.primo->DiscInfo2(pdwUnit, pdwMedium, pdwProtectedDVD, pdwFlags, pdwMediumEx, pdwRFU3);
#endif
}
