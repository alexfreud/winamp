#ifndef _NULLSOFT_WINAMP_ML_PMP_SYNC_CLOUD_DIALOG_HEADER
#define _NULLSOFT_WINAMP_ML_PMP_SYNC_CLOUD_DIALOG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class DeviceView;
class C_ItemList;

INT_PTR SyncCloudDialog_Show(HWND centerWindow, DeviceView *device,
							 C_ItemList *libraryList/*, C_ItemList *deviceList,
							 BOOL autofillMode*/);

#endif //_NULLSOFT_WINAMP_ML_PMP_SYNC_CLOUD_DIALOG_HEADER