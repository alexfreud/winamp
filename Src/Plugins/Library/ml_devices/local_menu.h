#ifndef _NULLSOFT_WINAMP_ML_DEVICES_MENU_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

int 
Menu_TrackSkinnedPopup(HMENU menu, 
					   unsigned int flags, 
					   int x,
					   int y,
					   HWND hwnd,
					   LPTPMPARAMS lptpm);

unsigned int 
Menu_InsertDeviceItems(HMENU menu,
					   int position,
					   unsigned int baseId,
					   ifc_device *device,
					   DeviceCommandContext context);

unsigned int 
Menu_FreeItemData(HMENU menu, 
				  unsigned int start, 
				  int count);

ULONG_PTR
Menu_GetItemData(HMENU menu, 
				 unsigned int item,
				 BOOL byPosition);


typedef BOOL (*Menu_FindItemByDataCb)(ULONG_PTR param, void *user);

unsigned int
Menu_FindItemByData(HMENU menu, 
					Menu_FindItemByDataCb callback, 
					void *user);

unsigned int
Menu_FindItemByAnsiStringData(HMENU menu,
							  const char *string);


#endif //_NULLSOFT_WINAMP_ML_DEVICES_MENU_HEADER