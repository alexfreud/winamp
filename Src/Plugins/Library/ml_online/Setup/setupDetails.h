#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPDETAILS_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPDETAILS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ifc_omservice;
class SetupGroup;
class SetupListboxItem;

#define NSDM_FIRST				(WM_APP + 7)
#define NSDM_GETUNIQUENAME		(NSDM_FIRST + 0)

EXTERN_C ATOM DETAILS_PROP;

BOOL SetupDetails_Initialize();
void SetupDetails_Uninitialize();


HWND SetupDetails_CreateServiceView(HWND hParent, LPCWSTR pszName, ifc_omservice *service);
HWND SetupDetails_CreateGroupView(HWND hParent, LPCWSTR pszName, SetupGroup *group);
BOOL SetupDetails_GetUniqueName(HWND hwnd, LPWSTR pszBuffer, UINT cchBufferMax);

// internal
void SetupDetails_SetDescription(HWND hEdit, LPCWSTR pszText);

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPDETAILS_HEADER
