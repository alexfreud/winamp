#ifndef NULLSOFT_AUTH_LOGINBOX_HEADER
#define NULLSOFT_AUTH_LOGINBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
class api_auth;
class LoginDownloadResult;
class LoginProviderEnumerator;
class ifc_omcacherecord;
class LoginImageCache;

#define LBS_NOCENTEROWNER		0x00000001

HWND LoginBox_CreateWindow(api_auth *auth, const GUID *pRealm, HWND hOwner, UINT fStyle);
INT_PTR LoginBox_Show(api_auth *auth, const GUID *pRealm, HWND hOwner, UINT fStyle);
HWND LoginBox_FindActive(const GUID *pRealm);


// messages
#define NLBM_FIRST			(WM_APP + 10)

#define NLBM_GETAUTHAPI			(NLBM_FIRST + 1)	// wParam  - not used, lParam = (LPARAM)(api_auth**)__authApi; Returns TRUE on success
#define LoginBox_GetAuthApi(/*HNWD*/ __hwnd, /*api_auth** */ __authApi)\
	((BOOL)SendMessage((__hwnd), NLBM_GETAUTHAPI, 0, (LPARAM)(__authApi)))

#define NLBM_GETREALM			(NLBM_FIRST + 2)	// wParam  - not used, lParam = (LPARAM)(GUID*)__pGuidRealm; Returns TRUE on success
#define LoginBox_GetRealm(/*HNWD*/ __hwnd, /*GUID* */ __pGuidRealm)\
	((BOOL)SendMessage((__hwnd), NLBM_GETREALM, 0, (LPARAM)(__pGuidRealm)))

#define NLBM_GETACTIVEPROVIDER	(NLBM_FIRST + 3)	// wParam  - not used, lParam = (LPARAM)(LoginProvider**)__ppProvider; Returns TRUE on success
#define LoginBox_GetActiveProvider(/*HNWD*/ __hwnd, /*LoginProvider** */ __ppProvider)\
	((BOOL)SendMessage((__hwnd), NLBM_GETACTIVEPROVIDER, 0, (LPARAM)(__ppProvider)))

#define NLBM_GETSTATUS			(NLBM_FIRST + 4)	// wParam - not used, lParam = (LPARAM)(LoginStatus**)ppStatus; Returns TRUE on success
#define LoginBox_GetStatus(/*HNWD*/ __hwnd, /*LoginStatus** */ __ppLoginStatus)\
	((BOOL)SendMessage((__hwnd), NLBM_GETSTATUS, 0, (LPARAM)(__ppLoginStatus)))

#define NLBM_ADDSTATUS			(NLBM_FIRST + 5)	// wParam - not used, lParam = (LPARAM)(BSTR)bstrStatusText; Return - (UINT)statusCookie or -1
#define LoginBox_AddStatus(/*HNWD*/ __hwnd, /*BSTR*/ __bstrStatusText)\
	((UINT)SendMessage((__hwnd), NLBM_ADDSTATUS, 0, (LPARAM)(__bstrStatusText)))

#define NLBM_SETSTATUS			(NLBM_FIRST + 6)	// wParam = (WPARAM)(UINT)statusCookie, lParam = (LPARAM)(BSTR)bstrStatusText; Return TRUE on success.
#define LoginBox_SetStatus(/*HNWD*/ __hwnd, /*UINT*/__statusCookie, /*BSTR*/ __bstrStatusText)\
	((BOOL)SendMessage((__hwnd), NLBM_SETSTATUS, (WPARAM)(__statusCookie), (LPARAM)(__bstrStatusText)))

#define NLBM_REMOVESTATUS		(NLBM_FIRST + 7)	// wParam = (WPARAM)(UINT)statusCookie, lParam - not used. Return ingored.
#define LoginBox_RemoveStatus(/*HNWD*/ __hwnd, /*UINT*/__statusCookie)\
	(SendMessage((__hwnd), NLBM_REMOVESTATUS, (WPARAM)(__statusCookie), 0L))

#define NLBM_UPDATEPROVIDERS	(NLBM_FIRST + 8)	// wParam - (WPARAM)(BOOL)fForce, lParam - not used.
#define LoginBox_UpdateProviders(/*HNWD*/ __hwnd, /*BOOL*/ __forceUpdate)\
	((BOOL)SendMessage((__hwnd), NLBM_UPDATEPROVIDERS, (WPARAM)(__forceUpdate), 0L))

#define NLBM_GETUPDATESTATE		(NLBM_FIRST + 9)	// wParam - not used, lParam - not used. Return TRUE if update active.
#define LoginBox_GetUpdateState(/*HNWD*/ __hwnd)\
	((BOOL)SendMessage((__hwnd), NLBM_GETUPDATESTATE, 0, 0L))

#define NLBM_LOGINCOMPLETED		(NLBM_FIRST + 20)	// wParam  - not used, lParam = (LPARAM)(LoginResult*)__pResult;

typedef struct __PROVIDERUPDATERESULT
{
	LoginDownloadResult *downloader;
	HRESULT errorCode;
	BOOL dataIdentical;
	LoginProviderEnumerator *enumerator;
} PROVIDERUPDATERESULT;

#define NLBM_PROVIDERSUPDATED	(NLBM_FIRST + 21)	// wParam  - not used, lParam = (LPARAM)(PROVIDERUPDATERESULT*)updateResult;
#define LoginBox_ProvidersUpdated(/*HNWD*/ __hwnd, /*LoginDownloadResult*/ __downloader, /*HRESULT*/ __errorCode, /*BOOL*/__dataIdentical, /*LoginProviderEnumerator**/ __enumerator)\
	{PROVIDERUPDATERESULT __updateResult;\
		__updateResult.downloader = (__downloader);\
		__updateResult.errorCode = (__errorCode);\
		__updateResult.dataIdentical = (__dataIdentical);\
		__updateResult.enumerator = (__enumerator);\
		(SendMessage((__hwnd), NLBM_PROVIDERSUPDATED, 0, (LPARAM)&(__updateResult)));}


typedef struct __IMAGECACHERESULT
{
	LoginImageCache *imageCache;
	ifc_omcacherecord *cacheRecord;
} IMAGECACHERESULT;

#define NLBM_IMAGECACHED				(NLBM_FIRST + 22) // wParam  - not used, lParam = (LPARAM)(IMAGECACHERESULT*)cacheResult;


#endif //NULLSOFT_AUTH_LOGINBOX_HEADER