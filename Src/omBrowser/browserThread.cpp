#include "main.h"
#include "./browserThread.h"
#include "../nu/threadname.h"

#include <exdisp.h>

typedef struct __BROWSERTHREADCRAEATEPARAM
{
	BTCREATEWNDPROC fnCreateWnd;
	BTKEYFILTERPROC fnKeyFilter;
	ULONG_PTR		user;
	HANDLE			readyEvent;
	HWND				hHost;
	HWND				hWinamp;
} BROWSERTHREADCREATEPARAM;

typedef struct __BROWSERTHREAD
{
	HHOOK	messageHook;
	HANDLE	wakeupEvent;
	UINT	flags;
} BROWSERTHREAD;

#define NAVIGATE_WAITTIMEOUT		30

static size_t tlsIndex = TLS_OUT_OF_INDEXES;
static UINT BHTM_DESTROY = 0xFEFE;

static DWORD CALLBACK BrowserThread_MainLoop(LPVOID param);

#define GetThreadInstance() ((TLS_OUT_OF_INDEXES != tlsIndex) ? (BROWSERTHREAD*)Plugin_TlsGetValue(tlsIndex) : NULL)

BOOL BrowserThread_IsQuiting()
{
	BROWSERTHREAD *thread = GetThreadInstance();
	return (NULL == thread || 0 != ((BHTF_BEGINDESTROY | BHTF_QUITLOOP) & thread->flags));
}

BOOL BrowserThread_SetFlags(UINT flags, UINT flagsMask, BOOL fAlarm)
{
	BROWSERTHREAD *thread = GetThreadInstance();
	if (NULL == thread) return FALSE;
	
	thread->flags = ((thread->flags & flagsMask) | flags);
	if (FALSE == fAlarm)
		return TRUE;

	return (NULL != thread->wakeupEvent && SetEvent(thread->wakeupEvent));
}


HANDLE BrowserThread_Create(HWND hWinamp, BTCREATEWNDPROC fnCreateWnd, ULONG_PTR user, BTKEYFILTERPROC fnKeyFilter, HWND *pWnd, DWORD *pThreadId)
{
	if (NULL == fnCreateWnd)
		return NULL;

	if (TLS_OUT_OF_INDEXES == tlsIndex)
	{
		tlsIndex = Plugin_TlsAlloc();
		if (TLS_OUT_OF_INDEXES == tlsIndex)
			return NULL;
	}

	DWORD threadId;
	
	BROWSERTHREADCREATEPARAM param;
	ZeroMemory(&param, sizeof(BROWSERTHREADCREATEPARAM));

	param.fnCreateWnd = fnCreateWnd;
	param.fnKeyFilter = fnKeyFilter;
	param.user = user;
	param.readyEvent = CreateEvent(0, TRUE, FALSE, 0); 
	param.hWinamp = hWinamp;

	HANDLE hThread = CreateThread(NULL, 0, BrowserThread_MainLoop, (LPVOID)&param, 0, &threadId);
	
	if (NULL != hThread)
	{
		if (NULL != param.readyEvent)
			WaitForSingleObject(param.readyEvent, INFINITE);
	}
	else
	{
		if (NULL != param.hHost)
		{
			DestroyWindow(param.hHost);
			param.hHost = NULL;
		}
		threadId = 0;
	}

	if (NULL != param.readyEvent)
		CloseHandle(param.readyEvent);


	if (NULL != pThreadId)
		*pThreadId = threadId;

	if (NULL != pWnd)
		*pWnd = param.hHost;

	return hThread;
}



BOOL BrowserThread_PostDestroyEx(DWORD threadId, HWND hHost)
{
	if (0 == BHTM_DESTROY)
		BHTM_DESTROY = RegisterWindowMessage(L"omBrowserDestroyMsg");

	if (0 == BHTM_DESTROY || 
		FALSE == PostThreadMessage(threadId, BHTM_DESTROY, 0, (LPARAM)hHost))
	{
		return FALSE;
	}

	BrowserThread_SetFlags(BHTF_BEGINDESTROY, BHTF_BEGINDESTROY, FALSE);
	return TRUE;
}

BOOL BrowserThread_PostDestroy(HWND hHost)
{
	return BrowserThread_PostDestroyEx(GetCurrentThreadId(), hHost);
}

BOOL BrowserThread_WaitNavigateComplete(IWebBrowser2 *pWeb2, UINT waitMax)
{
	MSG msg;
	READYSTATE state;
	if (NULL == pWeb2)
		return FALSE;

	BOOL resultOk = FALSE;
	DWORD tickStart = GetTickCount();

	for(;;)
	{
		if (FAILED(pWeb2->get_ReadyState(&state)))
			break;

		if (READYSTATE_INTERACTIVE <= state)
		{
			resultOk = TRUE;
			break;
		}
		else
		{
			DWORD tickNow = GetTickCount();
			if (tickNow < tickStart || (tickNow - tickStart) >= waitMax)
			{
				break; // time out
			}
		}

		DWORD status = MsgWaitForMultipleObjectsEx(0, NULL, NAVIGATE_WAITTIMEOUT, QS_POSTMESSAGE | QS_TIMER | QS_SENDMESSAGE, MWMO_ALERTABLE);
		switch(status)
		{
			case (WAIT_OBJECT_0 + 0):
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
				{
					if (!CallMsgFilter(&msg, MSGF_BROWSERLOOP))
					{
						DispatchMessageW(&msg);
					}
				}
				break;
		}
	}
	return resultOk;
}

static BOOL BrowserThread_HandleMessage(MSG *pMsg)
{
	switch(pMsg->message)
	{
		case WM_QUIT:
			BrowserThread_SetFlags(BHTF_QUITLOOP, BHTF_QUITLOOP, TRUE);
			return TRUE;
	}

	if (0 != BHTM_DESTROY && BHTM_DESTROY == pMsg->message)
	{
		HWND hHost = (HWND)pMsg->lParam;
		if (NULL != hHost)
		{
			BrowserThread_SetFlags(BHTF_BEGINDESTROY, BHTF_BEGINDESTROY, FALSE);
			SendMessage(hHost, BTM_RELEASECONTAINER, 0, 0L);
			DestroyWindow(hHost);
		}
        return TRUE;
	}

	return FALSE;
}

static LRESULT CALLBACK BrowserThread_MessageFilterProc(INT code, WPARAM wParam, LPARAM lParam)
{
	BROWSERTHREAD *thread = GetThreadInstance();
	if (code >= 0)
	{
		if (BrowserThread_HandleMessage((MSG*)lParam)) 
		{
			return TRUE;
		}
	}

	return (NULL != thread && NULL != thread->messageHook) ? 
				CallNextHookEx(thread->messageHook, code, wParam, lParam) : 
				FALSE;
}

static BOOL CALLBACK BrowserThread_DefaultKeyFilter(HWND hwnd, MSG *pMsg)
{
	return FALSE;
}


inline static BOOL BrowserThread_ProcessMessage(HWND hHost, HWND hWinamp, MSG *pMsg, BTKEYFILTERPROC IsHostMessage)
{
	if (hHost != pMsg->hwnd && FALSE == IsChild(hHost, pMsg->hwnd))
		return FALSE;

	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		if (FALSE != IsHostMessage(hHost, pMsg))
			return TRUE;

		switch(pMsg->wParam)
		{
			case VK_TAB:
				{
					HWND hOwner = (HWND)(LONG_PTR)GetWindowLongPtr(hHost, GWLP_HWNDPARENT);
					if (NULL == hOwner || hWinamp == hOwner) 
						hOwner = hHost;
					return IsDialogMessageW(hOwner, pMsg);
				}
				break;
		}
	}

	if (pMsg->message == WM_MOUSEWHEEL)
	{
		POINT cursor;
		HWND targetWindow;

		POINTSTOPOINT(cursor, pMsg->lParam);
		targetWindow = WindowFromPoint(cursor);

		if (NULL != targetWindow && 
			FALSE == IsChild(hHost, targetWindow ) &&
			GetWindowThreadProcessId(targetWindow, NULL) != GetWindowThreadProcessId(hHost, NULL))
		{
			PostMessage(hWinamp, pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}

	}

	return FALSE;
}

static void BrowserThread_FinishThread(BROWSERTHREAD *thread)
{
	if (NULL != thread)
	{
		if (NULL != thread->messageHook)
		{
			UnhookWindowsHookEx(thread->messageHook);
			thread->messageHook = NULL;
		}

		if (NULL != thread->wakeupEvent)
		{
			CloseHandle(thread->wakeupEvent);
			thread->wakeupEvent = NULL;
		}
	}

	if (TLS_OUT_OF_INDEXES != tlsIndex)
		Plugin_TlsSetValue(tlsIndex, NULL);

	OleUninitialize();

  #ifdef _DEBUG
	aTRACE_FMT("[%d] %S: thread exit\r\n", GetCurrentThreadId(), OMBROWSER_NAME);
  #endif // _DEBUG
}

static DWORD CALLBACK BrowserThread_MainLoop(LPVOID param)
{

#ifdef _DEBUG
	SetThreadName(GetCurrentThreadId(), "omBrowserThread");
	aTRACE_FMT("[%d] %S: thread created\r\n", GetCurrentThreadId(), OMBROWSER_NAME);
#endif //_DEBUG

	BROWSERTHREADCREATEPARAM *createParam = (BROWSERTHREADCREATEPARAM*)param;

	HWND hWinamp = createParam->hWinamp;

	BROWSERTHREAD thread;
	ZeroMemory(&thread, sizeof(BROWSERTHREAD));

	if (TLS_OUT_OF_INDEXES != tlsIndex)
		Plugin_TlsSetValue(tlsIndex, &thread);

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	BTKEYFILTERPROC IsHostMessage = (NULL != createParam->fnKeyFilter) ? createParam->fnKeyFilter : BrowserThread_DefaultKeyFilter;

	thread.messageHook = SetWindowsHookEx(WH_MSGFILTER, BrowserThread_MessageFilterProc, NULL, GetCurrentThreadId()); 
	thread.wakeupEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	HWND hHost = createParam->fnCreateWnd(createParam->user);

	createParam->hHost = hHost;

#ifdef _DEBUG
	if (NULL != hHost)
		aTRACE_FMT("[%d] %S: host created\r\n", GetCurrentThreadId(), OMBROWSER_NAME);
	else
		aTRACE_FMT("[%d] %S: host creation fialed\r\n", GetCurrentThreadId(), OMBROWSER_NAME);
#endif //_DEBUG

	if (NULL != createParam->readyEvent)
		SetEvent(createParam->readyEvent);


	if (NULL != hHost && FAILED(OleInitialize(0)))
	{
		DestroyWindow(hHost);
		hHost = NULL;
	}

	if (NULL == hHost)
	{
		BrowserThread_FinishThread(&thread);
		return -1;
	}

	SendMessage(hHost, BTM_INITCONTAINER, (WPARAM)hWinamp, 0L);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	while (0 == (BHTF_QUITLOOP & thread.flags))
	{
		DWORD status = MsgWaitForMultipleObjectsEx(1, &thread.wakeupEvent, INFINITE, 
							QS_ALLPOSTMESSAGE | QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);

		switch(status)
		{
			case (WAIT_OBJECT_0 + 0):
				// wake up!!!
				break;

			case (WAIT_OBJECT_0 + 1):
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
				{
					if (!CallMsgFilter(&msg, MSGF_BROWSERLOOP) && NULL != msg.hwnd)
					{
						if (0 == (BHTF_BEGINDESTROY & thread.flags))
						{
							if (FALSE == BrowserThread_ProcessMessage(hHost, hWinamp, &msg, IsHostMessage))
							{
								TranslateMessage(&msg);	
								DispatchMessageW(&msg);
							}
						}
						else
						{
							DispatchMessageW(&msg);
						}
					}
				}
				break;
		}
	}

	BrowserThread_FinishThread(&thread);
	return 0;
}

INT BrowserThread_ModalLoop(HWND hwnd, HANDLE hCancel, DWORD timeout)
{
	MSG msg;
	for (;;)
	{
		DWORD status = MsgWaitForMultipleObjectsEx(1, &hCancel, timeout, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		if (WAIT_OBJECT_0 == status)
		{
			return 0;
		}
		else if ((WAIT_OBJECT_0  + 1) == status)
		{
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					PostQuitMessage((INT)msg.wParam);
					return (INT)msg.wParam;
				}

				if (!IsDialogMessageW(hwnd, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
		}
	}

	return 0;
}