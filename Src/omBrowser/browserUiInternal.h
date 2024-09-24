#ifndef NULLSOFT_WINAMP_OMBROWSER_UI_INTERNAL_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_UI_INTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

//styles ex
#define NBCS_EX_BROWSERREADY		0x00000001
#define NBCS_EX_NAVCOMPLETED		0x00000002
#define NBCS_EX_BLOCKNAVIGATION		0x00000004

// popup only
#define NBCS_EX_NAVIGATEDONCE		0x00000100
#define NBCS_EX_FULLSCREEN			0x00000200
#define NBCS_EX_SCRIPTMODE			0x00000400	

#define ACCELTABLE_VIEW			0
#define ACCELTABLE_POPUP		1

HACCEL BrowserControl_GetAccelTable(UINT tableType);

BOOL BrowserControl_ProcessCommonCommand(HWND hControl, INT commandId);
BOOL BrowserControl_ProcessToolbarCommand(HWND hControl, INT commandId);
BOOL BrowserControl_ProcessStatusbarCommand(HWND hControl, INT commandId);
BOOL BrowserControl_ProcessAppCommand(HWND hControl, INT commandId);

typedef struct __OPERATIONINFO OPERATIONINFO;
BOOL BrowserControl_OnShowOperation(HWND hControl, OPERATIONINFO *poi);
HWND BrowserControl_GetOperationWidget(HWND hControl);

BOOL BrowserControl_UpdateLayout(HWND hControl, BOOL fRedraw, BOOL fFrame, HRGN updateRegion,const POINT *updateOffset);
BOOL BrowserControl_EnableChildren(HWND hControl, BOOL fEnable);
BOOL BrowserControl_SetBlockedState(HWND hControl, BOOL fBlocked);
#endif // NULLSOFT_WINAMP_OMBROWSER_UI_INTERNAL_HEADER