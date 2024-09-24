#include "./loginbox.h"
#include "./loginpage.h"
#include "../resource.h"

#include "../api.h"

#include <windows.h>

typedef struct __TOSREMINDERCREATEPARAM
{
	HANDLE hModal;
	BOOL fAnimate;
	INT_PTR *pResult;
} TOSREMINDERCREATEPARAM;

typedef struct __TOSREMINDER
{
	HANDLE hModal;
	BOOL fAnimate;
	SIZE idealSize;
	INT_PTR *pResult;
} TOSREMINDER;



#define TOSREMINDER_PROP		L"LoginboxTosReminderProp"
#define GetTosReminder(__hwnd) ((TOSREMINDER*)GetProp(__hwnd, TOSREMINDER_PROP))

static INT_PTR CALLBACK TosReminder_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR TosReminder_EnterModalLoop(HANDLE hExit);

HWND TosReminder_CreateWindow(HWND hParent)
{
	if (NULL == hParent)
		return NULL;

	return WASABI_API_CREATEDIALOGPARAMW(IDD_TOSREMINDER, hParent, TosReminder_DialogProc, 0L);
}

INT_PTR TosReminder_Show(HWND hParent, INT controlId, BOOL fAnimate)
{
	if (NULL == hParent)
		return -1;

	INT_PTR result;
	TOSREMINDERCREATEPARAM param;
	param.fAnimate = fAnimate;
	param.pResult = &result;
	param.hModal = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == param.hModal) return -1;

	HWND hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_TOSREMINDER, 
		hParent, TosReminder_DialogProc, (LPARAM)&param);
	
	if (NULL == hwnd) return -1;

	SetWindowLongPtr(hwnd, GWLP_ID, controlId);
	SetWindowPos(hParent, NULL, 0, 0, 0, 0, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

	if (FALSE == fAnimate || 
		0 == AnimateWindow(hwnd, 150, AW_ACTIVATE | AW_VER_POSITIVE | AW_SLIDE))
	{
		ShowWindow(hwnd, SW_SHOWNORMAL);
	}

	TosReminder_EnterModalLoop(param.hModal);
	return result;
}

static void TosReminder_EndDialog(HWND hwnd, INT_PTR code)
{
	TOSREMINDER *reminder = GetTosReminder(hwnd);
	if (NULL != reminder)
	{
		if (NULL != reminder->pResult)
		{
			(*reminder->pResult) = code;
			reminder->pResult = NULL;
		}
		
		if (NULL != reminder->hModal)
		{
			SetEvent(reminder->hModal);
			reminder->hModal = NULL;
		}

		if (FALSE == reminder->fAnimate || 
			0 == AnimateWindow(hwnd, 150, AW_HIDE | AW_VER_NEGATIVE | AW_SLIDE))
		{
			ShowWindow(hwnd, SW_HIDE);
		}
	}

	DestroyWindow(hwnd);
}

static void TosReminder_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	RECT clientRect;
	if (FALSE == GetClientRect(hwnd, &clientRect)) return;

	LONG centerX = clientRect.left + (clientRect.right - clientRect.left)/2;
	LONG buttonTop = clientRect.bottom;
	const INT szControls[] = {IDOK, IDCANCEL, IDC_TEXT, };

	RECT controlRect;

	UINT sharedFlags = SWP_NOZORDER | SWP_NOACTIVATE;
	if (FALSE == fRedraw) sharedFlags |= SWP_NOREDRAW;

	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szControls));
	if (NULL == hdwp) return;

	UINT controlFlags;
	LONG x, y, cx, cy;

	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		HWND hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl || FALSE == GetWindowRect(hControl, &controlRect))
			continue;

		controlFlags = sharedFlags;
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);
		x = controlRect.left;
		y = controlRect.top;
		cx = controlRect.right - controlRect.left;
		cy = controlRect.bottom - controlRect.top;

		switch(szControls[i])
		{
			case IDOK:
				x = centerX - cx - 8;
				if (x < clientRect.left) x = clientRect.left;
				y = (clientRect.bottom - 8) - cy;
				if	(y < clientRect.top) y = clientRect.top;
				if (y < buttonTop) buttonTop = y;
				break;
			
			case IDCANCEL:
				x = centerX + 8;
				if ((x + cx) > clientRect.right) x = clientRect.right - cx;
				y = (clientRect.bottom - 8) - cy;
				if	(y < clientRect.top) y = clientRect.top;
				if (y < buttonTop) buttonTop = y;
				break;

			case IDC_TEXT:
				cx = (clientRect.right - clientRect.left) - 2*x;
				cy = (buttonTop - y) - 16;
				break;
		}
		
		hdwp = DeferWindowPos(hdwp, hControl, NULL, x, y, cx, cy, controlFlags);
		if (NULL == hdwp) return;
	}

	EndDeferWindowPos(hdwp);

	if (FALSE != fRedraw)
	{
		HWND hControl = GetDlgItem(hwnd, IDC_TEXT);
		if (NULL != hControl) InvalidateRect(hControl, NULL, FALSE);
	}
}

static INT_PTR TosReminder_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{	
	TOSREMINDER *reminder = (TOSREMINDER*)malloc(sizeof(TOSREMINDER));
	if (NULL == reminder) 
	{
		DestroyWindow(hwnd);
		return 0;
	}
	
	ZeroMemory(reminder, sizeof(TOSREMINDER));
	SetProp(hwnd, TOSREMINDER_PROP, reminder);
	
	TOSREMINDERCREATEPARAM *createParam = (TOSREMINDERCREATEPARAM*)param;
	if (NULL != createParam)
	{
		reminder->fAnimate = createParam->fAnimate;
		reminder->hModal = createParam->hModal;
		reminder->pResult = createParam->pResult;
	}

	RECT windowRect;
	if (FALSE != GetWindowRect(hwnd, &windowRect))
	{
		reminder->idealSize.cx = windowRect.right - windowRect.left;
		reminder->idealSize.cy = windowRect.bottom - windowRect.top;
	}

	PostMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);
	return 0;
}

static void TosReminder_OnDestroy(HWND hwnd)
{	
	TOSREMINDER *reminder = GetTosReminder(hwnd);
	RemoveProp(hwnd, TOSREMINDER_PROP);
	if (NULL == reminder) 
		return;

	if (NULL != reminder->pResult)
	{
		(*reminder->pResult) = -1;
	}
	
	if (NULL != reminder->hModal)
	{
		SetEvent(reminder->hModal);
		reminder->hModal = NULL;
	}

	free(reminder);
}


static void TosReminder_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) 
	{
		TosReminder_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));
	}
}

static void TosReminder_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	
	switch(commandId)
	{
		case IDOK:
			TosReminder_EndDialog(hwnd, commandId);
			break;

		case IDCANCEL:
			TosReminder_EndDialog(hwnd, commandId);
			break;
	}

}

static BOOL TosReminder_OnGetIdealSize(HWND hwnd, SIZE *pSize)
{
	TOSREMINDER *reminder = GetTosReminder(hwnd);
	if (NULL == reminder || NULL == pSize) return FALSE;

	CopyMemory(pSize, &reminder->idealSize, sizeof(SIZE));
	return TRUE;

}

static BOOL TosReminder_OnClose(HWND hwnd, BOOL *pfAbort)
{
	TOSREMINDER *reminder = GetTosReminder(hwnd);
	if (NULL != reminder) reminder->fAnimate = FALSE;

	TosReminder_EndDialog(hwnd, IDCANCEL);
	return TRUE;
}

static INT_PTR CALLBACK TosReminder_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return TosReminder_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			TosReminder_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	TosReminder_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:			TosReminder_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;

		case NLPM_GETIDEALSIZE:		MSGRESULT(hwnd, TosReminder_OnGetIdealSize(hwnd, (SIZE*)lParam));
		case NLPM_CLOSE:			MSGRESULT(hwnd, TosReminder_OnClose(hwnd, (BOOL*)lParam));
	}

	return 0;
}

static BOOL TosReminder_PumpMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (WM_QUIT == msg.message)
		{
			PostQuitMessage((int)msg.wParam);
			return TRUE;
		}

		if (!CallMsgFilter(&msg, MSGF_DIALOGBOX)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return FALSE;
}

static INT_PTR TosReminder_EnterModalLoop(HANDLE hExit)
{
	if (NULL == hExit)
		return FALSE;
		
	for(;;)
	{
		DWORD code = MsgWaitForMultipleObjectsEx(1, &hExit, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		switch(code)
		{
			case WAIT_FAILED:
				return FALSE;

			case WAIT_OBJECT_0: 
				return TRUE;
			
			case (WAIT_OBJECT_0 + 1):
				if (FALSE != TosReminder_PumpMessages())
					return TRUE;
				break;
		}
	}
	return TRUE;
}