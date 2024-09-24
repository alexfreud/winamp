#ifndef NULLSOFT_MULTITHREADED_BROWSER_HEADER
#define NULLSOFT_MULTITHREADED_BROWSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../nu/HTMLContainer2.h"

typedef struct _MTBROWSER
{
	HTMLContainer2	*pContainer;			// HTMLContainer2 object
	HANDLE			hThread;				// Browser object thread
	DWORD			dwThreadId;				// Browser object thread Id
	BOOL			bQuiting;				// Browser quiting  (do not schedule any othe APC)
	HANDLE			hMemMngr;				// handle to the browser memory meanger. 
	HWND				hwndNotify;				// set from pContainer->GetParentHWND();
	UINT			uMsgNotify;				// if  0 != uMsgNotify you will be notified that browser finished operation with wParam set to cmd code, and lParam result.
} MTBROWSER;

typedef void* HAPC;
typedef void (CALLBACK *FREEPROC)(void* pMemFree);
typedef void (CALLBACK *APCPROC)(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult);

//  messages
#define MTBC_FIRST				(0x0000)
#define MTBC_LAST				(0x00FF)

#define MTBC_READY					(MTBC_FIRST)	// Posted when browser thread and browser object initialization completed //lParam = (HTMLContainer2*)
#define MTBC_DESTROYED				(MTBC_LAST)	// Posted when browser object destroyed itself and browser thread about to exit. //lParam = (HTMLContainer2*)- destroyed!!!

#define MTBC_APC_QUIT				(MTBC_FIRST + 20)	// Posted when browser executed quit APC
#define MTBC_APC_NAVIGATE			(MTBC_FIRST + 21)	// Posted when browser executed navigate APC. lParam = (HRESULT)
#define MTBC_APC_SETLOCATION			(MTBC_FIRST + 22)	// Posted when browser executed  SetLocation APC. lParam = (HRESULT)
#define MTBC_APC_REFRESH2			(MTBC_FIRST + 23)	// Posted when browser executed  Refresh2 APC. lParam = (HRESULT)

#define MTBC_DOCUMENTCOMPLETE		(MTBC_FIRST + 41)	// lParam = (BOOL)bDocumentReady



BOOL MTBrowser_Init(MTBROWSER *pmtb);
BOOL MTBrowser_Clear(MTBROWSER *pmtb);

BOOL MTBrowser_Start(MTBROWSER *pmtb, HTMLContainer2 *pContainer, UINT uMsgNotify);
BOOL MTBrowser_Kill(MTBROWSER *pmtb, UINT nTerminateDelay);			// use in WM_DESTROY

// Async calls
HAPC MTBrowser_InitializeAPC(MTBROWSER *pmtb, INT nCount, UINT nCmdCode, APCPROC fnAPC, VARIANTARG **pArgs);
BOOL MTBrowser_CallAPC(HAPC hAPC);

BOOL MTBrowser_QuitAPC(MTBROWSER *pmtb);
BOOL MTBrowser_NavigateToNameAPC(MTBROWSER *pmtb, LPCWSTR pszURL, UINT fFlags);
BOOL MTBrowser_SetLocationAPC(MTBROWSER *pmtb, RECT *pRect);
BOOL MTBrowser_SetVisibleAPC(MTBROWSER *pmtb, BOOL bVisible); 

BOOL MTBrowser_Refresh2APC(MTBROWSER *pmtb, INT nRefreshMode); 


/// Thread Memory manager 
BOOL MTBrowser_AddMemRec(MTBROWSER *pmtb, void *pMem, FREEPROC fnFreeProc); // if fnFreeProc == NULL - standart free() will be used
BOOL MTBrowser_FreeMemRec(MTBROWSER *pmtb, void *pMem);

#endif //NULLSOFT_MULTITHREADED_BROWSER_HEADER