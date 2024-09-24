#ifndef NULLSOFT_WINAMP_OMBROWSER_BROWSERTHREAD_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_BROWSERTHREAD_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

interface IWebBrowser2;

#define MSGF_BROWSERLOOP  (MSGF_USER + 1)

#define BHTF_QUITLOOP		0x00000001
#define BHTF_BEGINDESTROY	0x00000002

typedef HWND (CALLBACK *BTCREATEWNDPROC)(ULONG_PTR /*user*/);
typedef BOOL (CALLBACK *BTKEYFILTERPROC)(HWND /*hwnd*/, MSG* /*pMsg*/);

HANDLE BrowserThread_Create(HWND hWinamp, BTCREATEWNDPROC fnCreateWnd, ULONG_PTR user, BTKEYFILTERPROC fnKeyFilter, HWND *pWnd, DWORD *pThreadId);

BOOL BrowserThread_IsQuiting();
BOOL BrowserThread_SetFlags(UINT flags, UINT flagsMask, BOOL fAlarm);
BOOL BrowserThread_WaitNavigateComplete(IWebBrowser2 *pWeb2, UINT waitMax);
BOOL BrowserThread_PostDestroy(HWND hHost);
BOOL BrowserThread_PostDestroyEx(DWORD threadId, HWND hHost);
INT BrowserThread_ModalLoop(HWND hwnd, HANDLE hCancel, DWORD timeout);


#define BTM_FIRST			(WM_APP - 10)
#define BTM_INITCONTAINER	(BTM_FIRST + 0)		// wParam - not used, lParam - not used, return - ignored
#define BTM_RELEASECONTAINER (BTM_FIRST + 1)		// wParam - not used, lParam - not used; return - ignored;

#endif //NULLSOFT_WINAMP_OMBROWSER_BROWSERTHREAD_HEADER
