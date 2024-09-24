#ifndef NULLSOFT_WINAMP_OMBROWSER_BROWSERHOST_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_BROWSERHOST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class obj_ombrowser;

typedef void (CALLBACK *DISPATCHAPC)(IDispatch *pDisp, ULONG_PTR /*param*/);

HWND BrowserHost_CreateWindow(obj_ombrowser *browserManager, HWND hParent, UINT fStyle, INT x, INT y, INT cx, INT cy, INT controlId, HACCEL hAccel);

#define NBHS_POPUP				0x00000001
#define NBHS_SCRIPTMODE			0x00000002
#define NBHS_DISABLECONTEXTMENU	0x00000004
#define NBHS_DIALOGMODE			0x00000008
#define NBHS_DISABLEHOSTCSS		0x00000010

#define NBHS_BROWSERMASK		0x0000F000
#define NBHS_BROWSERREADY		0x00001000
#define NBHS_BROWSERACTIVE		0x00002000


#define NBHM_FIRST				(WM_USER + 200)
#define NBHM_DESTROY			(NBHM_FIRST + 10)	// wParam  = (WPARAM)(BOOL)fImmediate, lParam - not used. Preffered way to close it. Returns TRUE if processed

// Post this messages
#define NBHM_CONTAINERCOMMAND	(NBHM_FIRST + 11) // wParam = (WPARAM)(INT)browserCommandId, lParam - not used
#define NBHM_UPDATESKIN			(NBHM_FIRST + 12)	// wParam - not used, lParam - not used.
#define NBHM_NAVIGATE			(NBHM_FIRST + 13)	// wParam - (WPARAM)(UINT)navigateFlags, lParam = (LPARAM)(BSTR)navigateUrl. 
#define NBHM_ENABLECONTAINERUPDATE	(NBHM_FIRST + 14) // wParam - (WPARAM)(BOOL)fEnable, lParam = (WPARAM)(BOOL)fRedraw
#define NBHM_QUEUEAPC			(NBHM_FIRST + 15) // wParam = (WPARAM)userData, lParam = (LPARAM)(PAPCFUNC)pfnAPC


#define NBHM_SHOWHISTORYPOPUP	(NBHM_FIRST + 16) //wParam = (WPARAM)popupFlags, lParam =(LPARAM)MAKELPARAM(popupPos).
#define NBHM_GETDISPATCHAPC		(NBHM_FIRST + 17) // wParam = (WPARAM)param, lParam = (LPARAM)(DISPATCHAPC)callback
#define NBHM_ACTIVATE			(NBHM_FIRST + 18) // wParam - not used, lParam - not used.  Activates browser (OLEIVERB_UIACTIVATE)
#define NBHM_QUERYTITLE			(NBHM_FIRST + 19) // wParam - not used, lParam - not used.  Will query for current title and send NBHN_TITLECHANGE back
#define NBHM_TOGGLEFULLSCREEN	(NBHM_FIRST + 20) // wParam - not used, lParam - not used.

#define NBHM_WRITEDOCUMENT		(NBHM_FIRST + 21) // wParam - not used, lParam = (LPARAM)(BSTR)bstrDocument; 
#define NBHM_ENABLEWINDOW		(NBHM_FIRST + 22) // wParam - not used, lParam = (LPARAM)(BOOL)fEnabled;
#define NBHM_UPDATEEXTERNAL		(NBHM_FIRST + 23) // wParam - not used, lParam - not used.

#define NBHN_FIRST				(200)
#define NBHN_READY				(NBHN_FIRST + 0)

typedef struct __BHNNAVCOMPLETE
{
	NMHDR hdr;
	IDispatch	*pDispatch;
	VARIANT		*URL;
	BOOL		fTopFrame;
} BHNNAVCOMPLETE;

#define NBHN_DOCUMENTREADY		(NBHN_FIRST + 1)
#define NBHN_NAVIGATECOMPLETE	(NBHN_FIRST + 2)


typedef struct __BHNACTIVE
{
	NMHDR hdr;
	BOOL fActive;
} BHNACTIVE;
#define NBHN_BROWSERACTIVE		(NBHN_FIRST + 3)

typedef struct __BHNCMDSTATE
{
	NMHDR hdr;
	UINT commandId;
	BOOL fEnabled;
} BHNCMDSTATE;
#define NBHN_COMMANDSTATECHANGE	(NBHN_FIRST + 4)

typedef struct __BHNTEXTCHANGE
{
	NMHDR hdr;
	LPCWSTR pszText;
} BHNTEXTCHANGE;

#define NBHN_STATUSCHANGE			(NBHN_FIRST + 5)
#define NBHN_TITLECHANGE			(NBHN_FIRST + 6)

typedef struct __BHNSECUREICON
{
	NMHDR hdr;
	UINT iconId;
} BHNSECUREICON;
#define NBHN_SECUREICONCHANGE	(NBHN_FIRST + 7)


typedef struct __BHNSERVICE
{
	NMHDR hdr;
	void *instance;
} BHNSERVICE;
#define NBHN_GETOMSERVICE		(NBHN_FIRST + 8)  // call service->AddRef() and return TRUE if you support this

typedef struct __BHNCREATEPOPUP
{
	NMHDR		hdr;
	DISPATCHAPC	callback;
	ULONG_PTR	param;
} BHNCREATEPOPUP;

#define NBHN_CREATEPOPUP		(NBHN_FIRST + 9)

typedef struct __BHNVISIBLE
{
	NMHDR	hdr;
	BOOL	fVisible;
} BHNVISIBLE;
#define NBHN_VISIBLECHANGE		(NBHN_FIRST + 10) // send to popup

typedef struct __BHNRESIZABLE
{
	NMHDR	hdr;
	BOOL	fEnabled;
} BHNRESIZABLE;
#define NBHN_RESIZABLE		(NBHN_FIRST + 11)

typedef struct __BHNCLOSING
{
	NMHDR	hdr;
	BOOL	isChild;
	BOOL	cancel;
} BHNCLOSING;
#define NBHN_CLOSING		(NBHN_FIRST + 12)

typedef struct __BHNSHOWUI
{
	NMHDR	hdr;
	UINT	elementId;
	BOOL	fShow;
} BHNSHOWUI;
#define NBHN_SHOWUI		(NBHN_FIRST + 13)

typedef struct __BHNCLIENTTOHOST
{
	NMHDR	hdr;
	LONG	cx;
	LONG	cy;
} BHNCLIENTTOHOST;
#define NBHN_CLIENTTOHOST		(NBHN_FIRST + 14)

typedef struct __BHNSETWINDOWPOS
{
	NMHDR	hdr;
	UINT	flags;
	LONG	x;
	LONG	y;
	LONG	cx;
	LONG	cy;
} BHNSETWINDOWPOS;
#define NBHN_SETWINDOWPOS		(NBHN_FIRST + 15)

typedef struct __BHNFOCUSCHANGE
{
	NMHDR hdr;
	BOOL fAllow;
} BHNFOCUSCHANGE;
#define NBHN_FOCUSCHANGE		(NBHN_FIRST + 16)

typedef struct __BHNFULLSCREEN
{
	NMHDR	hdr;
	BOOL	fEnable;
} BHNFULLSCREEN;
#define NBHN_FULLSCREEN		(NBHN_FIRST + 17)


#define NBHN_CLOSEPOPUP		(NBHN_FIRST + 18) // param = NMHDR 




#endif // NULLSOFT_WINAMP_OMBROWSER_BROWSERHOST_HEADER
