#ifndef NULLSOFT_AUTH_LOGIN_TAB_HEADER
#define NULLSOFT_AUTH_LOGIN_TAB_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <commctrl.h>

#define NWC_LOGINTAB		L"NullsoftLoginTab"

BOOL LoginTab_RegisterClass(HINSTANCE hInstance);
HWND LoginTab_CreateWindow(UINT styleEx, LPCWSTR pszTitle, UINT style, INT x, INT y, INT cx, INT cy, HWND hParent, INT_PTR controlId);

typedef struct __NLTITEM
{
	UINT	mask;
	UINT	dwState;
	UINT	dwStateMask;
	LPWSTR	pszText;
	UINT	cchTextMax;
	UINT	iImage;
	UINT	iImageActive;
	UINT	iImageDisabled;
	LPARAM	param;
} NLTITEM;

// Item mask flags
#define NLTIF_STATE				0x00000001
#define NLTIF_TEXT				0x00000002
#define NLTIF_PARAM				0x00000004
#define NLTIF_IMAGE_MASK		(NLTIF_IMAGE | NLTIF_IMAGE_ACTIVE | NLTIF_IMAGE_DISABLED)
#define NLTIF_IMAGE				0x00000010
#define NLTIF_IMAGE_ACTIVE		0x00000020
#define NLTIF_IMAGE_DISABLED	0x00000040

// Item states
#define NLTIS_PRESSED		0x00000001
#define NLTIS_HIGHLIGHTED	0x00000002
#define NLTIS_SELECTED		0x00000004
#define NLTIS_DISABLED		0x00000008

// image index values
#define NLTM_IMAGE_NONE		((UINT)-1)
#define NLTM_IMAGE_CALLBACK	((UINT)-2)

// Messages
#define NLTM_FIRST		(WM_USER + 10)

#define NLTM_GETIDEALHEIGHT		(NLTM_FIRST + 0)	// wParam - not used, lParam - not used; Return ideal height.
#define LoginTab_GetIdealHeight(/*HWND*/ __hwnd)\
	((INT)SNDMSG((__hwnd), NLTM_GETIDEALHEIGHT, 0, 0L))

#define NLTM_INSERTITEM		(NLTM_FIRST + 1)	// wParam = (WPARAM)(INT)iItem, lParam = (LPARAM)(NLTITEM*)pItem; Return = index of new item or -1.
#define LoginTab_InsertItem(/*HWND*/ __hwnd, /*INT*/ __iItem, /*NLTITEM* */ __pItem)\
	((INT)SNDMSG((__hwnd), NLTM_INSERTITEM, (WPARAM)(__iItem), (LPARAM)(__pItem)))

#define NLTM_SETITEM		(NLTM_FIRST + 2)	// wParam = (WPARAM)(INT)iItem, lParam = (LPARAM)(NLTITEM*)pItem; Return = TRUE on success.
#define LoginTab_SetItem(/*HWND*/ __hwnd, /*INT*/ __iItem, /*NLTITEM* */ __pItem)\
	((BOOL)SNDMSG((__hwnd), NLTM_SETITEM, (WPARAM)(__iItem), (LPARAM)(__pItem)))

#define NLTM_GETITEM		(NLTM_FIRST + 3)	// wParam = (WPARAM)(INT)iItem, lParam = (LPARAM)(NLTITEM*)pItem; Return = TRUE on success.
#define LoginTab_GetItem(/*HWND*/ __hwnd, /*INT*/ __iItem, /*NLTITEM* */ __pItem)\
	((BOOL)SNDMSG((__hwnd), NLTM_GETITEM, (WPARAM)(__iItem), (LPARAM)(__pItem)))

#define NLTM_DELETEITEM		(NLTM_FIRST + 4)	// wParam = (WPARAM)(INT)iItem, lParam - not used; Return = TRUE on success.
#define LoginTab_DeleteItem(/*HWND*/ __hwnd, /*INT*/ __iItem)\
	((BOOL)SNDMSG((__hwnd), NLTM_DELETEITEM, (WPARAM)(__iItem), 0L))

#define NLTM_DELETEALLITEMS	(NLTM_FIRST + 5)	// wParam - not used, lParam - not used; Return = TRUE on success.
#define LoginTab_DeleteAllItems(/*HWND*/ __hwnd)\
	((BOOL)SNDMSG((__hwnd), NLTM_DELETEALLITEMS, 0, 0L))

#define NLTM_GETITEMCOUNT	(NLTM_FIRST + 6)	// wParam - not used, lParam - not used; Return item count.
#define LoginTab_GetItemCount(/*HWND*/ __hwnd)\
	((INT)SNDMSG((__hwnd), NLTM_GETITEMCOUNT, 0, 0L))

#define NLTM_GETCURSEL	(NLTM_FIRST + 7)	// wParam - not used, lParam - not used; Return item index or -1
#define LoginTab_GetCurSel(/*HWND*/ __hwnd)\
	((INT)SNDMSG((__hwnd), NLTM_GETCURSEL, 0, 0L))

#define NLTM_SETCURSEL	(NLTM_FIRST + 8)	// wParam = (WPARAM)(INT)iItem, lParam - not used; Return index of previously selected item if successful, or -1.
#define LoginTab_SetCurSel(/*HWND*/ __hwnd, /*INT*/ __iItem)\
	((INT)SNDMSG((__hwnd), NLTM_SETCURSEL, (WPARAM)(__iItem), 0L))

#define NLTM_SETIMAGELIST	(NLTM_FIRST + 9)	// wParam - not used, lParam - (LPARAM)(HIMAGELIST)himl; Returns the handle to the previous image list, or NULL if there is no previous image list.
#define LoginTab_SetImageList(/*HWND*/ __hwnd, /*HIMAGELIST*/ __himl)\
	((HIMAGELIST)SNDMSG((__hwnd), NLTM_SETIMAGELIST, 0, (LPARAM)(__himl)))

#define NLTM_GETIMAGELIST	(NLTM_FIRST + 10)	// wParam - not used, lParam - not used; Returns the handle to the image list if successful, or NULL otherwise.
#define LoginTab_GetImageList(/*HWND*/ __hwnd)\
	((HIMAGELIST)SNDMSG((__hwnd), NLTM_GETIMAGELIST, 0, 0L))

#define NLTM_RESETORDER		(NLTM_FIRST + 11)	// wParam - not used, lParam - not used; Return - ignored
#define LoginTab_ResetOrder(/*HWND*/ __hwnd)\
	(SNDMSG((__hwnd), NLTM_RESETORDER, 0, 0L))

#define NLTM_LOCKSELECTION	(NLTM_FIRST + 12)	// wParam - (BOOL)fLock, lParam - not used; Return - ignored.
#define LoginTab_LockSelection(/*HWND*/ __hwnd, /*BOOL*/ __fLock)\
	(SNDMSG((__hwnd), NLTM_LOCKSELECTION, (WPARAM)(__fLock), 0L))


#define NLTM_GETIDEALWIDTH	(NLTM_FIRST + 13)	// wParam = (WPARAM)(INT)itemCount, lParam - not used; Return ideal width.
#define LoginTab_GetIdealWidth(/*HWND*/ __hwnd, /*INT*/ __itemCount)\
	((INT)SNDMSG((__hwnd), NLTM_GETIDEALWIDTH, (WPARAM)(__itemCount), 0L))

// Notifications

typedef struct __NMLOGINTAB
{
	NMHDR hdr;
	INT	 iItem;
} NMLOGINTAB;

typedef struct __NMLOGINTABHELP
{
	NMHDR	hdr;
	INT		iItem;
	LPARAM	param;
	BSTR	bstrHelp;
} NMLOGINTABHELP;

typedef struct __NMLOGINTABCLICK
{
	NMHDR hdr;
	POINT pt;
} NMLOGINTABCLICK;

typedef struct __NMLOGINTABIMAGE
{
	NMHDR hdr;
	INT	 iItem;
	LPARAM	param;
	HIMAGELIST imageList;
	UINT maskRequest;
	UINT maskUpdate;
	UINT iImage;
	UINT iImageActive;
	UINT iImageDisabled;
} NMLOGINTABIMAGE;

#define NLTN_FIRST			(0 + 10)

#define NLTN_SELCHANGE		(NLTN_FIRST + 0)	// pnmh = (NMHDR*)lParam; 
#define NLTN_DELETEITEM		(NLTN_FIRST + 1)	// pnmh = (NMLOGINTAB*)lParam;
#define NLTN_DELETEALLITEMS	(NLTN_FIRST + 2)	// pnmh = (NMLOGINTAB*)lParam; iItem = -1, return TRUE if you don't want to receive NLTN_DELETEITEM
#define NLTN_GETITEMHELP	(NLTN_FIRST + 3)	// pnmh = (NMLOGINTABHELP*)lParam;
#define NLTN_GETITEMIMAGE	(NLTN_FIRST + 4)	// pnmh = (NMLOGINTABIMAGE*)lParam;

// common notifications
//NM_RCLICK	- pnmh = (NMLOGINTABCLICK*)lParam; 


//styles 
#define NLTS_LOCKED		0x00000001
#endif //NULLSOFT_AUTH_LOGIN_TAB_HEADER
