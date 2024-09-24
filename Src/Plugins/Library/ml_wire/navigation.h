#ifndef NULLSOFT_PODCAST_PLUGIN_NAVIGATION_HEADER
#define NULLSOFT_PODCAST_PLUGIN_NAVIGATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef LPVOID HNAVITEM;
class OmService;


BOOL Navigation_Initialize(void);
BOOL Navigation_ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result);

#define SHOWMODE_HIDE	((INT)0)
#define SHOWMODE_SHOW	((INT)1)
#define SHOWMODE_AUTO	((INT)-1)

HRESULT Navigation_ShowService(UINT serviceId, INT showMode);
HNAVITEM Navigation_FindService(UINT serviceId, HNAVITEM hStart, OmService **serviceOut);

#endif //NULLSOFT_PODCAST_PLUGIN_NAVIGATION_HEADER
