#include "Main.h"
#include "Browser.h"
#include "menuv5.h"
#include "ExternalCOM.h"
#include "wa_dlg.h"
#include "resource.h"
#include "../nu/ns_wc.h"

Browser *browser = 0;
static WNDPROC oldBrowserProc = 0;
static DWORD browserThreadId=0;
static HANDLE killBrowserEvent=0;
static HANDLE browserThreadHandle=0;
static BOOL fUnicode = FALSE;
static LRESULT WINAPI BrowserSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
		{
			HDC out = GetDC(hwnd);
			RECT r;
			GetClientRect(hwnd, &r);
			r.left = 11;
			r.top = 20;
			r.right -= 8;
			r.bottom -= 14;

			HBRUSH b = CreateSolidBrush(WADlg_getColor(WADLG_WNDBG));
			FillRect(out, &r, b);
			DeleteObject(b);
			ValidateRect(hwnd, &r);
		}
		break;

	case WM_USER + 0x100:
		if (wParam == 1 && lParam)
		{
			config_si_wx = ((POINT *)lParam)->x;
			config_si_wy = ((POINT *)lParam)->y;
		}
		break;

	case WM_USER + 0x101:
		if (wParam == 1 && lParam)
		{
			config_si_width = ((POINT *)lParam)->x;
			config_si_height = ((POINT *)lParam)->y;
		}
		break;

	case WM_USER + 0x102:
		{
			if (wParam == 1)
			{
				if (!config_minimized)
					ShowWindow(browser->m_hwnd, SW_SHOW);
				config_si_open = 1;
				const char *url = PlayList_getbrowser(PlayList_getPosition());
				if (url && *url)
					browser->NavigateToName(url);
			}
			else
			{
				ShowWindow(browser->m_hwnd, SW_HIDE);
				config_si_open = 0;
			}
			browser->SetMenuCheckMark();
		}
		break;

	case WM_USER + 101:
		{
			ShowWindow(hwnd, SW_HIDE);
			ShowWindow(browser->m_hwnd, SW_HIDE);
			config_si_open = 0;
			browser->SetMenuCheckMark();
			return 0;
		}
		break;

	case WM_DESTROY:
		browser->m_hwnd = 0;
		SetEvent(killBrowserEvent);
		return 0;
	}

	if (oldBrowserProc) return (fUnicode) ? CallWindowProcW(oldBrowserProc, hwnd, msg, wParam, lParam) : CallWindowProcA(oldBrowserProc, hwnd, msg, wParam, lParam);
	else return (fUnicode) ? DefWindowProcW(hwnd, msg, wParam, lParam) : DefWindowProcA(hwnd, msg, wParam, lParam);
}
// {25CE50EF-3EE2-4356-A638-6E495C44BFB8}
static const GUID StationInfoGUID  = 
{ 0x25ce50ef, 0x3ee2, 0x4356, { 0xa6, 0x38, 0x6e, 0x49, 0x5c, 0x44, 0xbf, 0xb8 } };

Browser::Browser() 
: minimised(false), threadId(0), state(0)
{
}

Browser::~Browser()
{
}

HWND Browser::CreateHWND()
{
	if (!m_hwnd)
	{
		threadId = GetCurrentThreadId();

		state.flags = EMBED_FLAGS_NOWINDOWMENU;
		state.me = 0;
		state.r.left = config_si_wx;
		state.r.right = config_si_wx + config_si_width;
		state.r.top = config_si_wy;
		state.r.bottom = config_si_wy + config_si_height;

		state.flags |= EMBED_FLAGS_GUID; 
		void *blah = state.extra_data+4;
		memcpy(blah, &StationInfoGUID, sizeof(GUID));

		HWND owner = (HWND)SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&state, IPC_GET_EMBEDIF);

		m_hwnd = owner;
		setLocation(11, 20, config_si_width - 19, config_si_height - 34);
		fUnicode = IsWindowUnicode(owner);
		oldBrowserProc = (WNDPROC) ((fUnicode) ? SetWindowLongPtrW(owner, GWLP_WNDPROC, (LONG_PTR)BrowserSubclassProc) : 
												SetWindowLongPtrA(owner, GWLP_WNDPROC, (LONG_PTR)BrowserSubclassProc));
												wchar_t langBuf[1024];
		SetWindowTextW(owner, getStringW(IDS_STATIONINFOCAPTION, langBuf, 1024));
	}
	EnableMenuItem(main_menu, WINAMP_BROWSER_ID, MF_BYCOMMAND | MF_ENABLED);

	if (config_si_autoshow || (config_si_open/* && !visible*/))
	{
		if(g_showcode!=SW_SHOWMINIMIZED && !config_minimized)
			ShowWindow(m_hwnd, SW_SHOW);
		else
			browser->minimised = 1;
		config_si_open = 1;
		browser->SetMenuCheckMark();
	}

	return m_hwnd;
}

// ---------------------------------------------------------------
void Browser::NavigateToName(LPCTSTR pszUrl)
{
	if (!config_si_open)
		return ;
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
		MultiByteToWideCharSZ(CP_ACP, 0, pszUrl, -1, pwszUrl, dwChars);
		m_pweb->Navigate (pwszUrl, &options , 0, 0, 0);
		LocalFree (pwszUrl);
	}
}

#define VIDEO_GENFF_SIZEREQUEST (WM_USER+2048)
void Browser::Resized(unsigned long width, unsigned long height)
{
	if (!config_si_autosize)
		return ;
	setLocation(11, 20, width, height);
	config_si_width = width + 19;
	config_si_height = height + 34;
	if (GetParent(m_hwnd))
		SendMessage(GetParent(m_hwnd), VIDEO_GENFF_SIZEREQUEST, width, height);
	else
		SetWindowPos(m_hwnd, 0, 0, 0, width + 19, height + 34, SWP_NOMOVE | SWP_ASYNCWINDOWPOS);

	InvalidateRect(m_hwnd, NULL, TRUE);
}

HRESULT Browser::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	*ppDispatch = (IDispatch *) & externalCOM;
	return S_OK;
}

void Browser::ToggleVisible(int showing)
{
	if ((!config_si_open && !showing) || (showing && minimised))
	{
		if(minimised) minimised = 0;
		CreateHWND();
		if (m_hwnd)
			PostMessage(m_hwnd, WM_USER + 0x102, 1, 0);
	}
	else if(!showing)
	{
		if(minimised) minimised = 0;
		if (m_hwnd)
			PostMessage(m_hwnd, WM_USER + 0x102, 0, 0);
	}
}

void Browser::OnNavigateComplete()
{
	setVisible(TRUE);
	RECT r;
	GetClientRect(m_hwnd, &r);
	setLocation(11, 20, r.right - 19, r.bottom - 34);
}

void Browser::SetMenuCheckMark()
{
	MENUITEMINFO i = {sizeof(i), MIIM_STATE , MFT_STRING, config_si_open ? MFS_CHECKED : MFS_UNCHECKED, WINAMP_BROWSER_ID};
	SetMenuItemInfo(main_menu, WINAMP_BROWSER_ID, FALSE, &i);
}

/* ---- APCs ---- */
VOID CALLBACK ToggleVisibleAPC(ULONG_PTR param)
{
	browser->ToggleVisible(param);
}

VOID CALLBACK SetVisibleAPC(ULONG_PTR param)
{
	BOOL visible = (BOOL)param;
	browser->setVisible(visible);
}

VOID CALLBACK NavigateAPC(ULONG_PTR param)
{
	char *url = (char *)param;
	browser->NavigateToName(url);
	free(url);
}

VOID CALLBACK CreateHWNDAPC(ULONG_PTR param)
{
	browser->CreateHWND();
}

HANDLE browserEvent=0;
/* ---- ---- */
static DWORD CALLBACK BrowserThread(LPVOID param)
{
	if (!browser)
		browser = new Browser;
	killBrowserEvent = CreateEvent(0, TRUE, FALSE, 0);
	SetEvent(browserEvent);

	while (1) 
	{
		DWORD dwStatus = MsgWaitForMultipleObjectsEx(1, &killBrowserEvent,
			INFINITE, QS_ALLINPUT,
			MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		if (dwStatus == WAIT_OBJECT_0)
		{
			browser->remove();
			browser->close();
			browser->Release();
			CloseHandle(killBrowserEvent);
			return 0;
		}
		else if (dwStatus == WAIT_OBJECT_0 + 1)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
			{
				if (msg.message == WM_QUIT) 
				{
					browser->remove();
					browser->close();
					browser->Release();
					CloseHandle(killBrowserEvent);
					return 0;
				}
				if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN ||
					msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP)
				{
					if (!browser->translateKey(&msg))
					{
						PostMessage(hMainWindow, msg.message, msg.wParam, msg.lParam);
					}
				}
				else
				{
					if (!browser->translateKey(&msg))
						TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
	}
	return 0;
}

static void CreateBrowser()
{
	if (!browser)
	{
		browserEvent = CreateEvent(0, TRUE, FALSE, 0);
		browserThreadHandle = CreateThread(0, 0, BrowserThread, 0, 0, &browserThreadId);
		WaitForSingleObject(browserEvent, INFINITE);
	}
}

static void CallAPC(PAPCFUNC func, ULONG_PTR param)
{
	if (browserThreadHandle)
	{
		DWORD curThreadId = GetCurrentThreadId();
		if (curThreadId == browserThreadId)
			func(param);
		else
		{
			QueueUserAPC(func, browserThreadHandle, param);
		}
	}
}

void Browser_toggleVisible(int showing)
{
	CreateBrowser();
	CallAPC(ToggleVisibleAPC, showing);
}

void CloseBrowser()
{
	if (!browser || !browser->m_hwnd)
		return ;
	if (config_si_autohide)
		PostMessage(browser->m_hwnd, WM_USER + 0x102, 0, 0);
	else
		CallAPC(SetVisibleAPC, FALSE);
}

void LaunchBrowser(const char *url)
{
	OpenBrowser();
	CallAPC(NavigateAPC, (ULONG_PTR)_strdup(url));
}

void OpenBrowser()
{
	CreateBrowser();
	CallAPC(CreateHWNDAPC, 0);
}

void Browser_init()
{
	MENUITEMINFO i = {sizeof(i), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING, MFS_UNCHECKED, WINAMP_BROWSER_ID};
	i.dwTypeData = getString(IDS_STATIONINFO_MENU,NULL,0);;
	InsertMenuItem(main_menu, 10 + g_mm_optionsbase_adj, TRUE, &i);
	g_mm_optionsbase_adj++;
}

void Browser_kill()
{
	if (browserThreadHandle)
	{
		SetEvent(killBrowserEvent); // just in case, so we don't hang here
		WaitForSingleObject(browserThreadHandle, INFINITE);
		CloseHandle(browserThreadHandle);
		browserThreadHandle=0;
	}
}