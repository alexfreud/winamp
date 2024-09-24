#pragma once

#include <wtypes.h>
#include <windowsx.h>

#include "./graphics.h"
#include "DeviceView.h"
#include "../ml_pmp/pmp.h"
#include "..\..\General\gen_ml/itemlist.h"
#include <map>
#include "./syncDialog.h"
#include "./syncCloudDialog.h"

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDMLIPC(__hwndML, __ipcMsgId, __param) SENDMSG((__hwndML), WM_ML_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define SENDWAIPC(__hwndWA, __ipcMsgId, __param) SENDMSG((__hwndWA), WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define SENDCMD(__hwnd, __ctrlId, __eventId, __hctrl) (SENDMSG((__hwnd), WM_COMMAND, MAKEWPARAM(__ctrlId, __eventId), (LPARAM)(__hctrl)))

#define DIALOG_RESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#ifndef RECTWIDTH
  #define RECTWIDTH(__r) ((__r).right - (__r).left)
#endif

#ifndef RECTHEIGHT
  #define RECTHEIGHT(__r) ((__r).bottom - (__r).top)
#endif

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifndef ARRAYSIZE
  #define ARRAYSIZE(_a) (sizeof(_a)/sizeof((_a)[0]))
#endif

#define DEFAULT_PMP_SEND_TO 1
extern DeviceView *configDevice;
extern winampMediaLibraryPlugin plugin;
extern C_ItemList devices;
extern HWND mainMessageWindow;
extern std::map<DeviceView *, bool> device_update_map;
extern C_Config * global_config;
extern void Devices_Init();
extern HMENU m_context_menus, m_context_menus2;

/* indirectplaybackserver.cpp - for localhost HTTP-based playback (for devices that don't support direct playback */
void startServer();
void stopServer();

ATOM GetViewAtom();

void *GetViewData(HWND hwnd);

BOOL SetViewData(HWND hwnd, void *data);

void *RemoveViewData(HWND hwnd);

BOOL FormatResProtocol(const wchar_t *resourceName, 
					   const wchar_t *resourceType, 
					   wchar_t *buffer, 
					   size_t bufferMax);

#define CENTER_OVER_WINAMP		((HWND)0)
#define CENTER_OVER_ML			((HWND)1)
#define CENTER_OVER_ML_VIEW		((HWND)2)

BOOL CenterWindow(HWND window, HWND centerWindow);
HWND OnSelChanged(HWND hwndDlg, HWND external, DeviceView *dev);

LinkedQueue *getTransferQueue(DeviceView *deviceView = NULL);
LinkedQueue *getFinishedTransferQueue(DeviceView *deviceView = NULL);
int getTransferProgress(DeviceView *deviceView = NULL);

extern int groupBtn, customAllowed, enqueuedef;