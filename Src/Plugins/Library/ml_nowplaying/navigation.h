#ifndef NULLSOFT_NOWPLAYING_PLUGIN_NAVIGATION_HEADER
#define NULLSOFT_NOWPLAYING_PLUGIN_NAVIGATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef LPVOID HNAVITEM;
class OmService;


BOOL Navigation_Initialize(void);
BOOL Navigation_ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result);

#define NAVFLAG_NORMAL				0x0000
#define NAVFLAG_ENSUREITEMVISIBLE	0x0001
#define NAVFLAG_ENSUREMLVISIBLE		0x0002
#define NAVFLAG_FORCEACTIVE			0x0004

HRESULT Navigation_ShowService(UINT serviceId, LPCWSTR pszUrl, UINT navFlags);
HNAVITEM Navigation_FindService(UINT serviceId, OmService **serviceOut);
HNAVITEM Navigation_GetActive(OmService **serviceOut);
HWND Navigation_GetActiveView(OmService **serviceOut);

#endif //NULLSOFT_NOWPLAYING_PLUGIN_NAVIGATION_HEADER