#ifndef NULLSOFT_WEBDEV_PLUGIN_SERVICE_HELPER_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_SERVICE_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ifc_omservice;
class ifc_omserviceeditor;
class ifc_omstorage;

#define WDSVCF_ROOT				0x00000001
#define WDSVCF_SPECIAL			0x00000002
#define WDSVCF_PREAUTHORIZED	0x00000004

HRESULT ServiceHelper_QueryStorage(ifc_omstorage **storage);
HRESULT ServiceHelper_Create(UINT serviceId, LPCWSTR pszName, LPCWSTR pszIcon, LPCWSTR pszUrl, UINT flags, BOOL fSave, ifc_omservice **serviceOut);
HRESULT ServiceHelper_Save(ifc_omservice *service);
HRESULT ServiceHelper_Delete(ifc_omservice *service);
HRESULT ServiceHelper_Reload(ifc_omservice *service);
HRESULT ServiceHelper_UpdateIcon(ifc_omserviceeditor *editor, LPCWSTR pszImage);
HRESULT ServiceHelper_IsSpecial(ifc_omservice *service);
HRESULT ServiceHelper_IsPreAuthorized(ifc_omservice *service);
HRESULT ServiceHelper_RegisterPreAuthorized(ifc_omservice *service);

#endif //NULLSOFT_WEBDEV_PLUGIN_SERVICE_HELPER_HEADER