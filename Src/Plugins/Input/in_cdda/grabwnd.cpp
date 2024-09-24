#include ".\grabwnd.h"
#include <strsafe.h>


#define LCID_INVARIANT	MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)
typedef struct _GRABDATA
{
	HHOOK	hook;
	HWND		hwndParent;
	WCHAR	szClassName[256];
	WCHAR	szTitle[256];
	GRABCB	callback;
	ULONG_PTR user;
} GRABDATA;

static GRABDATA g_grab = {0, 0, };

static BOOL IsTargetClass(HWND hwnd, LPCWSTR plzClassName)
{
	wchar_t szName[256] = {0};
	return  (0x00 == plzClassName[0] || 
				(GetClassNameW(hwnd, szName, sizeof(szName)/sizeof(wchar_t)) &&
								CSTR_EQUAL == CompareStringW(LCID_INVARIANT, NORM_IGNORECASE, szName, -1, plzClassName, -1)));
}

static BOOL IsTargetTitle(HWND hwnd, LPCWSTR pszTitle)
{
	wchar_t szName[256] = {0};
	return  (0x00 == pszTitle[0] || 
				(GetWindowTextW(hwnd, szName, sizeof(szName)/sizeof(wchar_t)) &&
								CSTR_EQUAL == CompareStringW(LCID_INVARIANT, NORM_IGNORECASE, szName, -1, pszTitle, -1)));
}
static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam) 
{
	if (HCBT_CREATEWND == code && IsTargetClass((HWND)wParam, g_grab.szClassName) && 
			IsTargetTitle((HWND)wParam, g_grab.szTitle) && 
			(!g_grab.hwndParent || ((CBT_CREATEWND*)lParam)->lpcs->hwndParent == g_grab.hwndParent))
	{
		LRESULT result;
		if (g_grab.callback)
		{
			g_grab.callback((HWND)wParam, ((CBT_CREATEWND*)lParam)->lpcs, &((CBT_CREATEWND*)lParam)->hwndInsertAfter, g_grab.user);
		}
		result = CallNextHookEx(g_grab.hook, code, wParam, lParam); 
		UnhookWindowsHookEx(g_grab.hook);
	
		
		ZeroMemory(&g_grab, sizeof(GRABDATA)); 
		return result;
	}
	return CallNextHookEx(g_grab.hook, code, wParam, lParam); 
}

BOOL BeginGrabCreateWindow(LPCWSTR pszClassName, LPCWSTR pszTitle, HWND hwndParent, GRABCB callback, ULONG_PTR user)
{
	if (g_grab.hook || !callback) return FALSE;
	if (pszClassName) StringCchCopyW(g_grab.szClassName, sizeof(g_grab.szClassName)/sizeof(wchar_t), pszClassName);
	else g_grab.szClassName[0] = 0x00;
	if (pszTitle) StringCchCopyW(g_grab.szTitle, sizeof(g_grab.szTitle)/sizeof(wchar_t), pszTitle);
	else g_grab.szTitle[0] = 0x00;
	g_grab.hwndParent = hwndParent;
	g_grab.callback = callback;
	g_grab.user = user;

	g_grab.hook =  SetWindowsHookEx(WH_CBT, HookProc, NULL, GetCurrentThreadId());
	if (!g_grab.hook) ZeroMemory(&g_grab, sizeof(GRABDATA));
	
	return (NULL != g_grab.hook);
}
void EndGrabCreateWindow(void)
{
	if (g_grab.hook) UnhookWindowsHookEx(g_grab.hook);
	ZeroMemory(&g_grab, sizeof(GRABDATA));
}