#include "main.h"
#include "./httpgrab.h"


static HHOOK g_hook = NULL;
static HWND g_hook_host = NULL;
static HWND g_hook_fwd = NULL;
static DWORD g_hook_flags = 0;
static HWND *g_hook_phwnd =NULL;

static LRESULT CALLBACK TargetWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK LabelWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOL IsDialog(HWND hwnd)
{
	wchar_t szName[256] = {0};
	return  (GetClassNameW(hwnd, szName, sizeof(szName)/sizeof(wchar_t)) &&
				CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
									NORM_IGNORECASE, szName, -1, L"#32770", -1));
}

static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam) 
{
	if (HCBT_CREATEWND == code && IsDialog((HWND)wParam) && ((CBT_CREATEWND*)lParam)->lpcs->hwndParent == g_hook_host)
	{
		LRESULT result;
		
		result = CallNextHookEx(g_hook, code, wParam, lParam); 
		UnhookWindowsHookEx(g_hook);
		g_hook = NULL;
	
		if (0 == result)
		{
			((CBT_CREATEWND*)lParam)->lpcs->style &= ~WS_VISIBLE;
			WNDPROC oldProc = (WNDPROC)SetWindowLongPtrW((HWND)wParam, GWLP_WNDPROC, (LONG_PTR)TargetWndProc);
			if (oldProc)
			{
				SetPropW((HWND)wParam, L"WNDPROC", oldProc);
				if (g_hook_fwd) SetPropW((HWND)wParam, L"FWDHWND", g_hook_fwd);
				if (g_hook_flags) SetPropW((HWND)wParam, L"FLAGS", (HANDLE)(INT_PTR)g_hook_flags);
				if (g_hook_phwnd) *g_hook_phwnd = (HWND)wParam;
			}
		}
		else if (g_hook_host && IsWindow(g_hook_host)) DestroyWindow(g_hook_host);

		g_hook_fwd = NULL;
		g_hook_flags = 0;
		g_hook_host = NULL;
		return result;
	}
	return CallNextHookEx(g_hook, code, wParam, lParam); 
}

HWND BeginGrabHTTPText(HWND hwndFwd, UINT flags, HWND *phwndTarget)
{	

	g_hook_host = CreateWindowExW(0x08000000/*WS_EX_NOACTIVATE*/ | WS_EX_NOPARENTNOTIFY, L"STATIC", L"", WS_DISABLED | WS_POPUP, -10000, -10000, 0, 0, NULL, NULL, NULL, NULL); 
	if (!g_hook_host) return NULL;

	g_hook = SetWindowsHookEx(WH_CBT, HookProc, NULL, GetCurrentThreadId());
	if (!g_hook)
	{
		if (g_hook_host) DestroyWindow(g_hook_host);
		g_hook_host = NULL;
		g_hook_fwd = NULL;
		g_hook_flags = 0;
		g_hook_phwnd = NULL;
	}
	else
	{
		g_hook_fwd = hwndFwd;
		g_hook_flags = flags;
		g_hook_phwnd = phwndTarget;
	}
	return g_hook_host;
}

void EndGrabHTTPText(HWND hwndHost)
{
	if (g_hook_host == hwndHost)
	{
		if (g_hook) UnhookWindowsHookEx(g_hook);
		g_hook = NULL;
		g_hook_host = NULL;
		g_hook_fwd = NULL;
		g_hook_flags = 0;
	}
	if (IsWindow(hwndHost)) DestroyWindow(hwndHost);
}

static LRESULT CALLBACK TargetWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC fnOldProc = (WNDPROC)GetPropW(hwnd, L"WNDPROC");
	
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				HWND hwndFwd;

				hwndFwd = (HWND)GetPropW(hwnd, L"FWDHWND");
				DWORD flags = (DWORD)(INT_PTR)GetPropW(hwnd, L"FLAGS");
				RemovePropW(hwnd, L"FLAGS");
				RemovePropW(hwnd, L"FWDHWND");

				if (hwndFwd)
				{
					HWND hwndText;
					if (HTTPGRAB_USESTATUSTEXT & flags)
					{
						hwndText = GetDlgItem(hwnd, 1180/*IDC_STATUS*/);
						WNDPROC oldProc = (WNDPROC)SetWindowLongPtrW(hwndText, GWLP_WNDPROC, (LONG_PTR)LabelWndProc);
						if (oldProc) SetPropW(hwndText, L"WNDPROC", oldProc);
						else hwndText = hwnd;
					}
					else hwndText = hwnd;
					SetPropW(hwndText, L"FWDTEXT", hwndFwd);

					WCHAR szText[256] = {0};
					GetWindowTextW(hwndText, szText, sizeof(szText)/sizeof(wchar_t)); 
					SetWindowTextW(hwndFwd, szText);
				}
			}
			break;
		case WM_DESTROY:
			{
				RemovePropW(hwnd, L"WNDPROC");
				RemovePropW(hwnd, L"FWDTEXT");
				if (fnOldProc) SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)fnOldProc);
				HWND hwndOwner;
				hwndOwner = GetWindow(hwnd, GW_OWNER);
				if (hwndOwner) DestroyWindow(hwndOwner);
			}
			break;		
		case WM_STYLECHANGING:
			if (GWL_STYLE & wParam) ((STYLESTRUCT*)lParam)->styleNew &= ~WS_VISIBLE;
			return 0;
		case WM_STYLECHANGED:
			if ((GWL_STYLE & wParam) && (WS_VISIBLE & ((STYLESTRUCT*)lParam)->styleNew)) 
					SetWindowLongPtrW(hwnd, GWL_STYLE, (((STYLESTRUCT*)lParam)->styleNew & ~WS_VISIBLE));
			return 0;
		case WM_WINDOWPOSCHANGING:
			((WINDOWPOS*)lParam)->flags = (((WINDOWPOS*)lParam)->flags & ~(SWP_SHOWWINDOW | SWP_FRAMECHANGED)) | SWP_NOACTIVATE | SWP_NOZORDER;
			return 0;
		case WM_SETTEXT: 
			{
				HWND hwndFwd = (HWND)GetPropW(hwnd, L"FWDTEXT");
				if (hwndFwd) 
				{
					(IsWindowUnicode(hwndFwd)) ? SendMessageW(hwndFwd, WM_SETTEXT, wParam, lParam) :
										SendMessageA(hwndFwd, WM_SETTEXT, wParam, lParam);
				}
			}
			break;
	}
	if (!fnOldProc) return DefWindowProc(hwnd, uMsg, wParam, lParam);
	return (IsWindowUnicode(hwnd)) ? CallWindowProcW(fnOldProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(fnOldProc, hwnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK LabelWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC fnOldProc = (WNDPROC)GetPropW(hwnd, L"WNDPROC");
	
	switch(uMsg)
	{
		case WM_SETTEXT: 
			{
				HWND hwndFwd = (HWND)GetPropW(hwnd, L"FWDTEXT");
				if (hwndFwd) 
				{
					(IsWindowUnicode(hwndFwd)) ? SendMessageW(hwndFwd, WM_SETTEXT, wParam, lParam) :
										SendMessageA(hwndFwd, WM_SETTEXT, wParam, lParam);
				}
			}
		case WM_DESTROY:
			RemovePropW(hwnd, L"WNDPROC");
			RemovePropW(hwnd, L"FWDTEXT");
			if (fnOldProc) SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)fnOldProc);
			break;						
	}
	if (!fnOldProc) return DefWindowProc(hwnd, uMsg, wParam, lParam);
	return (IsWindowUnicode(hwnd)) ? CallWindowProcW(fnOldProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(fnOldProc, hwnd, uMsg, wParam, lParam);
}
