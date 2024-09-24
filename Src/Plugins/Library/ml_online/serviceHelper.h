#ifndef NULLSOFT_ONLINEMEDIA_PLUGIN_SERVICE_HELPER_HEADER
#define NULLSOFT_ONLINEMEDIA_PLUGIN_SERVICE_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ifc_omservice;
class ifc_omserviceenum;
class ifc_omserviceeditor;
class ifc_omstorage;

#include <ifc_omstorageasync.h>

#define SHF_VERBAL			0x00000001
#define SHF_NOTIFY			0x00000002
#define SHF_SAVE			0x00000004

HRESULT ServiceHelper_Initialize();

HRESULT ServiceHelper_QueryStorage(ifc_omstorage **storage);
HRESULT ServiceHelper_QueryWebStorage(ifc_omstorage **storage);
HRESULT ServiceHelper_Load(ifc_omserviceenum **enumerator);

HRESULT ServiceHelper_Create(UINT serviceId, LPCWSTR pszName, LPCWSTR pszIcon, LPCWSTR pszUrl, UINT flags, UINT generation, BOOL fSave, ifc_omservice **serviceOut);
HRESULT ServiceHelper_Save(ifc_omservice *service);
HRESULT ServiceHelper_Delete(ifc_omservice *service, UINT flags);
HRESULT ServiceHelper_UpdateIcon(ifc_omserviceeditor *editor, LPCWSTR pszImage);
HRESULT ServiceHelper_Find(UINT serviceId, ifc_omservice **serviceOut);

HRESULT ServiceHelper_SetFlags(ifc_omservice *service, UINT flags, UINT flagsMask);
HRESULT ServiceHelper_IsSpecial(ifc_omservice *service);
HRESULT ServiceHelper_IsPreAuthorized(ifc_omservice *service);
HRESULT ServiceHelper_IsSubscribed(ifc_omservice *service);
HRESULT ServiceHelper_IsModified(ifc_omservice *service);
HRESULT ServiceHelper_MarkModified(ifc_omservice *service, UINT modifiedFlag, UINT modifiedMask);

HRESULT ServiceHelper_SetRating(ifc_omservice *service, UINT rating, UINT flags);
HRESULT ServiceHelper_Subscribe(ifc_omservice *service, BOOL subscribe, UINT flags);
HRESULT ServiceHelper_ResetPermissions(ifc_omservice *service, UINT flags);
HRESULT ServiceHelper_ResetSubscription(UINT flags);
HRESULT ServiceHelper_BeginDiscover(LPCWSTR address);
HRESULT ServiceHelper_IsDiscovering();
HRESULT ServiceHelper_BeginVersionCheck(ifc_omservice *service);

HRESULT ServiceHelper_GetDetailsUrl(LPWSTR pszBuffer, UINT cchBufferMax, ifc_omservice *service, BOOL fLite);
HRESULT ServiceHelper_PostNotificationUrl(LPCWSTR pszUrl);
HRESULT ServiceHelper_UpdateOperationInfo(HWND hBrowser);

#define SHOWMODE_NORMAL			0
#define SHOWMODE_ENSUREVISIBLE	1
#define SHOWMODE_POPUP			2
HRESULT ServiceHelper_ShowService(UINT serviceId, UINT showMode);

#endif //NULLSOFT_ONLINEMEDIA_PLUGIN_SERVICE_HELPER_HEADER