#include "Main.h"
#include "Browser.h"
#include "./api.h"

#include "../nu/ns_wc.h"
#include "menuv5.h"
#include "ExternalCOM.h"
#include "wa_dlg.h"

#include "./updateService.h"

UpdateBrowser *updateBrowser=0;
static WNDPROC oldUpdateProc = 0;
static BOOL fUnicode = FALSE;

static void CALLBACK UpdateBrowser_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD elapsed)
{
	KillTimer(hwnd, eventId);
	
	if (0 == (WS_VISIBLE & GetWindowLongPtrW(hwnd, GWL_STYLE)))
	{
		ShowWindow(hwnd, SW_SHOW);
	}

	embedWindowState *state = (embedWindowState*)(ULONG_PTR)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if(NULL != state)
	{
		if (NULL != state->wasabi_window)
		{
			state->wasabi_window->activate();
			return;
		}
		else if (NULL != g_dialog_box_parent)
		{
			SetTimer(hwnd, eventId, 200,  UpdateBrowser_TimerProc);
			return;
		}
	}
	
	HWND hFrame = hwnd;
	while (NULL != hFrame && 0 != (WS_CHILD & GetWindowLongPtrW(hFrame, GWL_STYLE)))
		hFrame = GetAncestor(hFrame, GA_PARENT);

	if (NULL != hFrame && (g_dialog_box_parent == hFrame || hMainWindow == hFrame))
		hFrame = NULL;
			
	if (NULL == hFrame) hFrame = hwnd;

	SetWindowPos(hFrame, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(hFrame);
	
}

HRESULT UpdateWindow_Show(LPCSTR pszUrl)
{
	if (0 != _strnicmp(pszUrl, "http://client.winamp.com", 21))
		return E_UNEXPECTED;

	
	UpdateService *service;
	if (SUCCEEDED(UpdateService::CreateInstance(pszUrl, &service)))
	{
		HRESULT hr = service->Show();
		service->Release();
		if (SUCCEEDED(hr))
			return hr;
	}

	updateBrowser = new UpdateBrowser;
	updateBrowser->CreateHWND();
	updateBrowser->setVisible(TRUE);
	updateBrowser->NavigateToName((LPCTSTR)pszUrl);
	SetTimer(updateBrowser->m_hwnd, 1980, 1000, UpdateBrowser_TimerProc);
	
	return S_OK;
}

static LRESULT WINAPI BrowserSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
		{
			HDC out = GetDC(hwnd);
			RECT r;
			GetClientRect(hwnd, &r);
			r.left=11;
			r.top=20;
			r.right-=8;
			r.bottom-=14;

			HBRUSH b = CreateSolidBrush(WADlg_getColor(WADLG_WNDBG));
			FillRect(out, &r, b);
			DeleteObject(b);
			ValidateRect(hwnd, &r);
		}
		break;

	case WM_USER+0x102:
		{
			if (wParam == 1)
			{
				ShowWindow(updateBrowser->m_hwnd, SW_SHOW);
			}
			else
			{
				ShowWindow(updateBrowser->m_hwnd, SW_HIDE);
			}
		}
		break;
	case WM_USER + 101:
		{
			//updateBrowser->setVisible(FALSE);
			ShowWindow(hwnd, SW_HIDE);
			ShowWindow(updateBrowser->m_hwnd, SW_HIDE);
			return 0;
		}
		break;

	case WM_NCDESTROY:
	case WM_DESTROY:
		updateBrowser->m_hwnd = 0;
		return 0;
		break;

	}


	if (oldUpdateProc)
		return (fUnicode) ? CallWindowProcW(oldUpdateProc, hwnd, msg, wParam, lParam) : CallWindowProcA(oldUpdateProc, hwnd, msg, wParam, lParam);
	else
		return (fUnicode) ? DefWindowProcW(hwnd, msg, wParam, lParam) : DefWindowProcA(hwnd, msg, wParam, lParam);

}

HWND UpdateBrowser::CreateHWND()
{
	if (!m_hwnd)
	{
		state.flags = EMBED_FLAGS_NOWINDOWMENU;
		state.me = 0;
		state.r.left = config_wx;
		state.r.right = config_wx+300;
		state.r.top = config_wy;
		state.r.bottom = config_wy+200;

		HWND owner = (HWND) SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM) & state, IPC_GET_EMBEDIF);

		m_hwnd = owner;
		setLocation(11,20, 300-19, 200-34);
		fUnicode = IsWindowUnicode(owner);
		oldUpdateProc = (WNDPROC) ((fUnicode) ? SetWindowLongPtrW(owner, GWLP_WNDPROC, (LONG_PTR)BrowserSubclassProc) : 
												SetWindowLongPtrA(owner, GWLP_WNDPROC, (LONG_PTR)BrowserSubclassProc));
		SetWindowTextW(owner, getStringW(IDS_WINAMP_UPDATE, NULL, 0));
	}
	
	return m_hwnd;
}

// ---------------------------------------------------------------
void UpdateBrowser::NavigateToName(LPCTSTR pszUrl)
{
	if (!m_pweb) return ;
	DWORD dwChars = lstrlen (pszUrl) + 1;
	LPWSTR pwszUrl = (LPWSTR)LocalAlloc (LPTR, dwChars * sizeof (WCHAR));
	long moptions = navNoReadFromCache | navNoWriteToCache | navNoHistory;
	VARIANT options;
	memset( (void*)&options, 0, sizeof(VARIANT));
	V_VT(&options) = VT_I4;
	V_I4(&options) = moptions;
	if (pwszUrl)
	{
		MultiByteToWideCharSZ(CP_ACP, 0, (LPCSTR)pszUrl, -1, pwszUrl, dwChars);
		m_pweb->Navigate (pwszUrl, &options , 0, 0, 0);
		LocalFree (pwszUrl);
	}
}


#define VIDEO_GENFF_SIZEREQUEST (WM_USER+2048)
void UpdateBrowser::Resized(unsigned long width, unsigned long height)
{
	updateBrowser->setLocation(11,20, width, height);

	if (GetParent(m_hwnd))
		SendMessageW(GetParent(m_hwnd), VIDEO_GENFF_SIZEREQUEST, width, height);
	else
		SetWindowPos(m_hwnd, 0, 0, 0, width + 19, height + 34, SWP_NOMOVE|SWP_ASYNCWINDOWPOS);

	InvalidateRect(m_hwnd, NULL, TRUE);

}

HRESULT UpdateBrowser::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;

	ExternalCOM *external;
	HRESULT hr = JSAPI1_GetExternal(&external);
	if (FAILED(hr)) external = NULL;
	
	*ppDispatch = (IDispatch*) external;
	return S_OK;
}


void UpdateBrowser::OnNavigateComplete()
{
	setVisible(TRUE);
	RECT r;
	GetClientRect(m_hwnd, &r);
	setLocation(11,20, r.right-19, r.bottom-34);
}