#include "./browserWindow.h"
#include "./browserEvent.h"
#include "./common.h"

#include "../../ombrowser/browserHost.h"

#define NBWF_USERFLAGSMASK	0x00FFFFFF
#define NBWF_UNICODE		0x01000000

typedef struct __BROWSERWND
{
	WNDPROC originalProc;
	UINT	flags;
	BrowserEvent *eventHandler;
} BROWSERWND;

#define BROWSERWND_PROP		L"NullsoftLoginboxBrowserWindow"

#define GetBrowserWnd(__hwnd) ((BROWSERWND*)GetProp((__hwnd), BROWSERWND_PROP))

static UINT NBWM_QUEUEAPC = 0;

static LRESULT CALLBACK BrowserWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


BOOL BrowserWindow_Attach(HWND hBrowser, BrowserEvent *eventHandler)
{
	if (!IsWindow(hBrowser)) 
		return  FALSE;

	if (0 == NBWM_QUEUEAPC)
		NBWM_QUEUEAPC = RegisterWindowMessage(L"NullsoftBrowserExtMessage");

	BROWSERWND *browserWnd = (BROWSERWND*)GetProp(hBrowser, BROWSERWND_PROP);
	if (NULL != browserWnd) return TRUE;
	
	browserWnd = (BROWSERWND*)calloc(1, sizeof(BROWSERWND));
	if (NULL == browserWnd) return FALSE;


	ZeroMemory(browserWnd, sizeof(BROWSERWND));

	if (IsWindowUnicode(hBrowser))
		browserWnd->flags |= NBWF_UNICODE;

	browserWnd->originalProc = (WNDPROC)(LONG_PTR)((0 != (NBWF_UNICODE & browserWnd->flags)) ? 
						SetWindowLongPtrW(hBrowser, GWLP_WNDPROC, (LONGX86)(LONG_PTR)BrowserWindow_WindowProc) : 
						SetWindowLongPtrA(hBrowser, GWLP_WNDPROC, (LONGX86)(LONG_PTR)BrowserWindow_WindowProc));

	if (NULL == browserWnd->originalProc || !SetProp(hBrowser, BROWSERWND_PROP, browserWnd))
	{
		if (NULL != browserWnd->originalProc)
		{
			if (0 != (NBWF_UNICODE & browserWnd->flags))
				SetWindowLongPtrW(hBrowser, GWLP_WNDPROC, (LONGX86)(LONG_PTR)browserWnd->originalProc);
			else
				SetWindowLongPtrA(hBrowser, GWLP_WNDPROC, (LONGX86)(LONG_PTR)browserWnd->originalProc);
		}
			
		free(browserWnd);
		return FALSE;
	}

	if (NULL != eventHandler)
	{
		browserWnd->eventHandler = eventHandler;
		eventHandler->AddRef();
	}
	return TRUE;
}

BOOL BrowserWindow_Detach(HWND hBrowser)
{
	if (NULL == hBrowser || FALSE == IsWindow(hBrowser))
		return FALSE;

	BROWSERWND *browserWnd = GetBrowserWnd(hBrowser);
	RemoveProp(hBrowser, BROWSERWND_PROP);

	if (NULL == browserWnd) 
		return FALSE;

	if (NULL != browserWnd->originalProc)
	{
		if (0 != (NBWF_UNICODE & browserWnd->flags))
			SetWindowLongPtrW(hBrowser, GWLP_WNDPROC, (LONGX86)(LONG_PTR)browserWnd->originalProc);
		else
			SetWindowLongPtrA(hBrowser, GWLP_WNDPROC, (LONGX86)(LONG_PTR)browserWnd->originalProc);
	}

	if (NULL != browserWnd->eventHandler)
		browserWnd->eventHandler->Release();

	free(browserWnd);

	return TRUE;
}

BOOL BrowserWindow_QueueApc(HWND hBrowser, LPARAM param)
{
	if (0 == NBWM_QUEUEAPC || 
		NULL == hBrowser || FALSE == IsWindow(hBrowser))
	{
		return FALSE;
	}
	return PostMessage(hBrowser, NBWM_QUEUEAPC, 0, (LPARAM)param);
}


static LRESULT BrowserWindow_CallOrigWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BROWSERWND *browserWnd = GetBrowserWnd(hwnd);

	if (NULL == browserWnd || NULL == browserWnd->originalProc)
	{
		return (0 != (NBWF_UNICODE & browserWnd->flags)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	return (0 != (NBWF_UNICODE & browserWnd->flags)) ? 
			CallWindowProcW(browserWnd->originalProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(browserWnd->originalProc, hwnd, uMsg, wParam, lParam);
}

static void BrowserWindow_OnDestroy(HWND hwnd)
{
	BROWSERWND *browserWnd = GetBrowserWnd(hwnd);
	
	WNDPROC originalProc = browserWnd->originalProc;
	BOOL fUnicode = (0 != (NBWF_UNICODE & browserWnd->flags));

	BrowserWindow_Detach(hwnd);

	if (NULL != originalProc)
	{
		if (FALSE != fUnicode) 
			CallWindowProcW(originalProc, hwnd, WM_DESTROY, 0, 0L);
		else
			CallWindowProcA(originalProc, hwnd, WM_DESTROY, 0, 0L);
	}
}

static void BrowserWindow_OnQueueApc(HWND hwnd, LPARAM param)
{
	BROWSERWND *browserWnd = GetBrowserWnd(hwnd);
	if (NULL != browserWnd && NULL != browserWnd->eventHandler)
		browserWnd->eventHandler->Event_InvokeApc(hwnd, param);
}

static void BrowserWindow_OnBrowserNotify(HWND hwnd, NMHDR *pnmh)
{
	BROWSERWND *browserWnd = GetBrowserWnd(hwnd);
	if (NULL == browserWnd || NULL == browserWnd->eventHandler)
		return;

	switch(pnmh->code)
	{
		case NBHN_READY:
			browserWnd->eventHandler->Event_BrowserReady(hwnd);
			break;
		case NBHN_DOCUMENTREADY:
			browserWnd->eventHandler->Event_DocumentReady(hwnd);
			break;
		case NBHN_CLOSING:
			browserWnd->eventHandler->Event_BrowserClosing(hwnd);
			break;
	}
}

static LRESULT BrowserWindow_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{	
	LRESULT result =  BrowserWindow_CallOrigWindowProc(hwnd, WM_NOTIFY, (WPARAM)controlId, (LPARAM)pnmh);
	switch(controlId)
	{
		case 0x1000/*IDC_BROWSER*/:
			BrowserWindow_OnBrowserNotify(hwnd, pnmh);
			break;
	}

	return result;
}

static LRESULT CALLBACK BrowserWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:	BrowserWindow_OnDestroy(hwnd); return 0;
		case WM_NOTIFY:		return BrowserWindow_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam);
	}
	
	if (NULL != NBWM_QUEUEAPC && NBWM_QUEUEAPC == uMsg)
	{
		BrowserWindow_OnQueueApc(hwnd, lParam);
		return 0;
	}
	return BrowserWindow_CallOrigWindowProc(hwnd, uMsg, wParam, lParam);
}
