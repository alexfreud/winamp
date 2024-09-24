#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

// registered classes

#define TOOLCLS_STATIC				"static"
#define TOOLCLS_BUTTON				"button"
#define TOOLCLS_RATING				"rating"
#define TOOLCLS_PROGRESS			"progress"
#define TOOLCLS_ADDRESSBAR			"addressbar"

#define TOOLITEM_SEPARATOR			"separator"
#define TOOLITEM_SPACE				"space"
#define TOOLITEM_FLEXSPACE			"spaceFlexible"
#define TOOLITEM_CHEVRON			"chevron"

#define TOOLITEM_BUTTON_HOME			(TOOLCLS_BUTTON "Home")
#define TOOLITEM_BUTTON_BACK			(TOOLCLS_BUTTON "Back")
#define TOOLITEM_BUTTON_FORWARD			(TOOLCLS_BUTTON "Forward")
#define TOOLITEM_BUTTON_STOP			(TOOLCLS_BUTTON "Stop")
#define TOOLITEM_BUTTON_REFRESH			(TOOLCLS_BUTTON "Refresh")
#define TOOLITEM_BUTTON_HISTORY			(TOOLCLS_BUTTON "History")
#define TOOLITEM_CMDLINK_INFO			(TOOLCLS_BUTTON "Info")
#define TOOLITEM_CMDLINK_REPORT			(TOOLCLS_BUTTON "Report")
#define TOOLITEM_CMDLINK_UNSUBSCRIBE	(TOOLCLS_BUTTON "Unsubscribe")

#define TOOLITEM_USERRATING				(TOOLCLS_RATING "User")
#define TOOLITEM_DOWNLOADPROGRESS		(TOOLCLS_PROGRESS "Download")
#define TOOLITEM_ADDRESSBAR				(TOOLCLS_ADDRESSBAR "Main")

#define TOOLITEM_BUTTON_SCRIPTERROR			(TOOLCLS_BUTTON "ScriptError")
#define TOOLITEM_BUTTON_SECURECONNECTION	(TOOLCLS_BUTTON "SecureConnection")

#define NWC_ONLINEMEDIATOOLBAR		L"Nullsoft_omBrowserToolbar"


BOOL Toolbar_RegisterClass(HINSTANCE hInstance);

#define ITEM_ERR			((INT)-1)

// styles
#define TBS_LOCKUPDATE			0x00000001 // do not reset directly use Toolbar_LockUpdate().
#define TBS_AUTOHIDE			0x00000002
#define TBS_BOTTOMDOCK			0x00000004
#define TBS_TABSTOP				0x00000008
#define TBS_SHOWADDRESS			0x00000010
#define TBS_FORCEADDRESS		0x00000020
#define TBS_FANCYADDRESS		0x00000040

// item styles
#define TBIS_HIDDEN			0x0001
#define TBIS_DISABLED		0x0002
#define TBIS_CHEVRONONLY		0x0004	// show item only in chevron
#define TBIS_NOCHEVRON		0x0008	// show item in toolbar and ignore in chevron
#define TBIS_POPUP			0x0010	// item only take space when visible


// messages
#define TBM_FIRST			(WM_USER + 10)

#define TBM_UPDATESKIN			(TBM_FIRST + 0) //wParam = not used, lParam = (LPARAM)(BOOL)fRedraw.
#define Toolbar_UpdateSkin(/*HWND*/ __hToolbar, /*BOOL*/ __fRedraw)\
	(SENDMSG(__hToolbar, TBM_UPDATESKIN, 0, (LPARAM)(__fRedraw)))

#define TBM_GETIDEALHEIGHT	(TBM_FIRST + 1)
#define Toolbar_GetIdealHeight(/*HNWD*/__hToolbar)\
	((INT)SendMessage((__hToolbar), TBM_GETIDEALHEIGHT, 0, 0L))

#define TBM_GETICONSIZE		(TBM_FIRST + 3)
#define Toolbar_GetIconSize(/*HWND*/ __hToolbar, /*INT*/ __iconIndex, /*PSIZE*/ __sizeOut)\
	((BOOL)SendMessage((__hToolbar), TBM_GETICONSIZE, (WPARAM)(__iconIndex), (LPARAM)(__sizeOut)))

#define TBM_SENDCOMMAND		(TBM_FIRST + 4)
#define Toolbar_SendCommand(/*HWND*/ __hToolbar, /*INT*/ __commandId)\
	((BOOL)SendMessage((__hToolbar), TBM_SENDCOMMAND, (WPARAM)(__commandId), 0L))


typedef struct __TOOLBARDRAWICONPARAM
{
	HDC	 hdcDst;
	INT	 iconIndex;
	INT	 x;
	INT	 y;
	INT  cx;
	INT  cy;
	UINT itemState;
} TOOLBARDRAWICONPARAM;

#define TBM_DRAWICON				(TBM_FIRST + 5)
#define Toolbar_DrawIcon(/*HWND*/ __hToolbar, /*TOOLBARDRAWICONPARAM* */ __toolbarDrawIconParam)\
	((BOOL)SendMessage((__hToolbar), TBM_DRAWICON, 0, (LPARAM)(__toolbarDrawIconParam)))

#define Toolbar_LockUpdate(/*HWND*/ __hToolbar, /*BOOL*/__fLock)\
	(SendMessage((__hToolbar), WM_SETREDRAW, (WPARAM)(0 == (__fLock)), 0L))

#define TBM_GETITEMCOUNT			(TBM_FIRST + 6)
#define Toolbar_GetItemCount(/*HWND*/ __hToolbar)\
	((INT)SendMessage((__hToolbar), TBM_GETITEMCOUNT, 0, 0L))

#define TBM_CLEAR				(TBM_FIRST + 7)
#define Toolbar_Clear(/*HWND*/ __hToolbar)\
	((BOOL)SendMessage((__hToolbar), TBM_CLEAR, 0, 0L))

#define TBIP_FIRST		0x0000
#define TBIP_LAST		0xFFFFFF

typedef struct __TOOLBARINSERTITEM
{
	INT		cbSize;
	INT		insertBefore; // you can use TBIP_XXX here
	LPCSTR	pszName;
	UINT	style;
} TOOLBARINSERTITEM;

#define TBM_INSERTITEM			(TBM_FIRST + 8)  // wParam - not used, lParam = (LPARAM)(TOOLBARINSERTITEM*)pInsertItem; Return item index or ITEM_ERR
#define Toolbar_InsertItem(/*HWND*/ __hToolbar, /*TOOLBARINSERTITEM*/ __pInsertItem)\
	((INT)SendMessage((__hToolbar), TBM_INSERTITEM, 0, (LPARAM)__pInsertItem))

#define TBM_FINDITEM				(TBM_FIRST + 9)
#define Toolbar_FindItem(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName)\
	((INT)SendMessage((__hToolbar), TBM_FINDITEM, 0, (LPARAM)(__pszItemName)))

#define TBM_REMOVEITEM			(TBM_FIRST + 10) // itemName can be INT index
#define Toolbar_RemoveItem(/*HWND*/ __hToolbar,  /*LPCSTR*/__pszItemName)\
	((BOOL)SendMessage((__hToolbar), TBM_REMOVEITEM, 0, (LPARAM)(__pszItemName)))

#define TBM_SETITEMINT				(TBM_FIRST + 11)		// itemName can be INT index
#define Toolbar_SetItemInt(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName, /*INT*/ __intValue)\
	((BOOL)SendMessage((__hToolbar), TBM_SETITEMINT, (WPARAM)(__intValue), (LPARAM)(__pszItemName)))

#define TBM_SETITEMSTRING			(TBM_FIRST + 12) // itemName can be INT index
#define Toolbar_SetItemString(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName, /*LPCWSTR*/ __stringValue)\
	((BOOL)SendMessage((__hToolbar), TBM_SETITEMSTRING, (WPARAM)(__stringValue), (LPARAM)(__pszItemName)))

#define TBM_GETBKCOLOR				(TBM_FIRST + 13)
#define Toolbar_GetBkColor(/*HWND*/ __hToolbar)\
	((COLORREF)SendMessage((__hToolbar), TBM_GETBKCOLOR, 0, 0L))

#define TBM_GETFGCOLOR				(TBM_FIRST + 14)
#define Toolbar_GetFgColor(/*HWND*/ __hToolbar)\
	((COLORREF)SendMessage((__hToolbar), TBM_GETFGCOLOR, 0, 0L))

#define TBM_GETTEXTCOLOR				(TBM_FIRST + 15)
#define Toolbar_GetTextColor(/*HWND*/ __hToolbar)\
	((COLORREF)SendMessage((__hToolbar), TBM_GETTEXTCOLOR, 0, 0L))

#define TBM_GETHILITECOLOR				(TBM_FIRST + 16)
#define Toolbar_GetHiliteColor(/*HWND*/ __hToolbar)\
	((COLORREF)SendMessage((__hToolbar), TBM_GETHILITECOLOR, 0, 0L))

#define TBM_ENABLEITEM				(TBM_FIRST + 17) // itemName can be INT index
#define Toolbar_EnableItem(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName, /*BOOL*/ __fEnable)\
	((BOOL)SendMessage((__hToolbar), TBM_ENABLEITEM, (WPARAM)(__fEnable), (LPARAM)(__pszItemName)))

#define TBM_SHOWITEM				(TBM_FIRST + 18) // itemName can be INT index
#define Toolbar_ShowItem(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName, /*BOOL*/ __fShow)\
	((BOOL)SendMessage((__hToolbar), TBM_SHOWITEM, (WPARAM)(__fShow), (LPARAM)(__pszItemName)))

#define TBM_UPDATETIP				(TBM_FIRST + 19)
#define Toolbar_UpdateTip(/*HWND*/ __hToolbar)\
	((BOOL)SendMessage((__hToolbar), TBM_UPDATETIP, 0, 0L))

typedef struct __TOOLBARTEXTMETRIC
{
	INT height;
	INT baseY;
	INT origY;
	INT aveCharWidth;
	INT overhang;
} TOOLBARTEXTMETRIC;

#define TBM_GETTEXTMETRICS				(TBM_FIRST + 20)
#define Toolbar_GetTextMetrics(/*HWND*/ __hToolbar, /*TOOLBARTEXTMETRIC* */ __textMetric)\
	((BOOL)SendMessage((__hToolbar), TBM_GETTEXTMETRICS, 0, (LPARAM)__textMetric))

#define TBM_GETBKBRUSH					(TBM_FIRST + 21)
#define Toolbar_GetBkBrush(/*HWND*/ __hToolbar)\
	((HBRUSH)SendMessage((__hToolbar), TBM_GETBKBRUSH, 0, 0L))

typedef struct __TOOLBARLAYOUT
{
	const RECT *prcParent;		// [in] - parent window rect
	HWND	 insertAfter;		// [out] - toolar insert after
	RECT toolbarRect;		// [out] - toolbar rect
	RECT clientRect;		// [out] - new parent client rect
} TOOLBARLAYOUT;

#define TBM_LAYOUT				(TBM_FIRST + 22)
#define Toolbar_Layout(/*HWND*/ __hToolbar, /*TOOLBARLAYOUT* */ __pLayout)\
	((BOOL)SendMessage((__hToolbar), TBM_LAYOUT, 0, (LPARAM)(__pLayout)))

#define TBNS_NEXTITEM			MAKEINTRESOURCEA(0)
#define TBNS_PREVITEM			MAKEINTRESOURCEA(1)

#define TBM_NEXTITEM				(TBM_FIRST + 23)
#define Toolbar_NextItem(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName, /*BOOL*/ __fUseName)\
	((BOOL)SendMessage((__hToolbar), TBM_NEXTITEM, (WPARAM)(__fUseName), (LPARAM)(__pszItemName)))

#define TBM_GETITEMSTYLE			(TBM_FIRST + 24) // itemName can be INT index, 
#define Toolbar_GetItemStyle(/*HWND*/ __hToolbar,  /*LPCSTR*/ __pszItemName, /*UINT*/ __fMask)\
	((UINT)SendMessage((__hToolbar), TBM_GETITEMSTYLE, (WPARAM)(__fMask), (LPARAM)(__pszItemName)))

#define TBM_GETITEMCOMMAND		(TBM_FIRST + 25) // itemName can be INT index, 
#define Toolbar_GetItemCommand(/*HWND*/ __hToolbar,  /*LPCSTR*/ __pszItemName)\
	((INT)SendMessage((__hToolbar), TBM_GETITEMCOMMAND, 0, (LPARAM)(__pszItemName)))

#define TBM_SETITEMDESCRIPTION	(TBM_FIRST + 26) // itemName can be INT index
#define Toolbar_SetItemDescription(/*HWND*/ __hToolbar,  /*LPCSTR*/ __pszItemName, /*LPCWSTR*/__pszDescription)\
	((BOOL)SendMessage((__hToolbar), TBM_SETITEMDESCRIPTION, (WPARAM)(__pszDescription), (LPARAM)(__pszItemName)))

typedef struct __TBITEMINFO
{
	INT commandId;
	UINT style;
	LPWSTR pszText;
	INT cchText;
	LPWSTR pszDescription;
	INT cchDescription;
} TBITEMINFO;

#define TBM_GETITEMINFO				(TBM_FIRST + 27) // itemName can be INT index
#define Toolbar_GetItemInfo(/*HWND*/ __hToolbar,  /*LPCSTR*/ __pszItemName, /*TBITEM* */__itemInfo)\
	((BOOL)SendMessage((__hToolbar), TBM_GETITEMINFO, (WPARAM)(__itemInfo), (LPARAM)(__pszItemName)))

#define TBPF_NORMAL					0x00000000
#define TBPF_NOSERVICECOMMANDS		0x00000001
#define TBPF_READONLYADDRESS		0x00000004

#define TBM_AUTOPOPULATE				(TBM_FIRST + 28) //wParam - (WPARAM)(UINT)populateFlags; lParam = (LPARAM)(ifc_omservice*)service; Return: number of buttons added.
#define Toolbar_AutoPopulate(/*HWND*/ __hToolbar,  /*ifc_omservice* */ __service, /*UINT*/__populateFlags)\
	((UINT)SendMessage((__hToolbar), TBM_AUTOPOPULATE, (WPARAM)(__populateFlags), (LPARAM)(__service)))

#define TBM_ENABLEBOTTOMDOCK			(TBM_FIRST + 29) //wParam - not used; lParam = (LPARAM)(BOOL)fEnable; Return: previous state.
#define Toolbar_EnableBottomDock(/*HWND*/ __hToolbar,  /*BOOL*/ __fEnable)\
	((BOOL)SendMessage((__hToolbar), TBM_ENABLEBOTTOMDOCK, 0, (LPARAM)(__fEnable)))

#define TBM_ENABLEAUTOHIDE			(TBM_FIRST + 30) //wParam - not used; lParam = (LPARAM)(BOOL)fEnable; Return: previous state.
#define Toolbar_EnableAutoHide(/*HWND*/ __hToolbar,  /*BOOL*/ __fEnable)\
	((BOOL)SendMessage((__hToolbar), TBM_ENABLEAUTOHIDE, 0, (LPARAM)(__fEnable)))

#define TBM_ENABLETABSTOP			(TBM_FIRST + 31) //wParam - not used; lParam = (LPARAM)(BOOL)fEnable; Return: previous state.
#define Toolbar_EnableTabStop(/*HWND*/ __hToolbar,  /*BOOL*/ __fEnable)\
	((BOOL)SendMessage((__hToolbar), TBM_ENABLETABSTOP, 0, (LPARAM)(__fEnable)))

#define TBM_SETBROWSERHOST			(TBM_FIRST + 32) //wParam = 0, lParam = (LPARAM)(HWND)hwndBrowserHost.
#define Toolbar_SetBrowserHost(/*HWND*/ __hStatusbar, /*HWND*/ __hwndBrowserHost)\
	((BOOL)SENDMSG(__hStatusbar, TBM_SETBROWSERHOST, 0, (LPARAM)(__hwndBrowserHost)))

#define TBM_GETEDITCOLOR				(TBM_FIRST + 33)
#define Toolbar_GetEditColor(/*HWND*/ __hToolbar)\
	((COLORREF)SendMessage((__hToolbar), TBM_GETEDITCOLOR, 0, 0L))

#define TBM_GETEDITBKCOLOR				(TBM_FIRST + 34)
#define Toolbar_GetEditBkColor(/*HWND*/ __hToolbar)\
	((COLORREF)SendMessage((__hToolbar), TBM_GETEDITBKCOLOR, 0, 0L))

#define TBM_GETIMAGELISTHEIGHT			(TBM_FIRST + 35)
#define Toolbar_GetImageListHeight(/*HWND*/ __hToolbar)\
	((INT)SendMessage((__hToolbar), TBM_GETIMAGELISTHEIGHT, 0, 0L))


#define TBM_GETNEXTTABITEM				(TBM_FIRST + 36)
#define Toolbar_GetNextTabItem(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName, /*BOOL*/ __fPrevious)\
	((INT)SendMessage((__hToolbar), TBM_GETNEXTTABITEM, (BOOL)(__fPrevious), (LPARAM)(__pszItemName)))

#define TBM_CHECKHIDE					(TBM_FIRST + 37) // wParam - not used, lParam = (LPARAM)(BOOL)__fImmediate
#define Toolbar_CheckHide(/*HWND*/ __hToolbar, /*BOOL*/ __fImmediate)\
	(SendMessage((__hToolbar), TBM_CHECKHIDE, 0, (BOOL)(__fImmediate)))

#define TBM_ENABLEFORCEADDRESS			(TBM_FIRST + 38) //wParam - not used; lParam = (LPARAM)(BOOL)fEnable; Return: previous state.
#define Toolbar_EnableForceAddress(/*HWND*/ __hToolbar,  /*BOOL*/ __fEnable)\
	((BOOL)SendMessage((__hToolbar), TBM_ENABLEFORCEADDRESS, 0, (LPARAM)(__fEnable)))

#define TBM_ENABLEFANCYADDRESS			(TBM_FIRST + 39) //wParam - not used; lParam = (LPARAM)(BOOL)fEnable; Return: previous state.
#define Toolbar_EnableFancyAddress(/*HWND*/ __hToolbar,  /*BOOL*/ __fEnable)\
	((BOOL)SendMessage((__hToolbar), TBM_ENABLEFANCYADDRESS, 0, (LPARAM)(__fEnable)))

#define TBM_GETTEXTLENGTH				(TBM_FIRST + 40) // wParam - (WPARAM)(size_t*)__textLengthOut, lParam - (LPARAM)(LPCSTR)(__itemName); Return TRUE if supported
#define Toolbar_GetTextLength(/*HWND*/ __hToolbar, /*LPCSTR*/ __pszItemName, /*size_t* */ __textLengthOut)\
	((BOOL)SendMessage((__hToolbar), TBM_GETTEXTLENGTH, (WPARAM)(__textLengthOut), (LPARAM)(__pszItemName)))

// Nitifications (WM_COMMAND)
#define TBN_DOCKCHANGED			1
#define TBN_AUTOHIDECHANGED		2
#define TBN_TABSTOPCHANGED		3

#endif //NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_HEADER
