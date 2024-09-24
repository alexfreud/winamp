/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"
#include <windows.h>
#include "strutil.h"
#include "../nu/ns_wc.h"
#include "plush/plush.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "WinampAttributes.h"

#undef GetSystemMetrics

int IsUrl(const char *url)
{
	return !!strstr(url, "://");
}


void link_startsubclass(HWND hwndDlg, UINT id){
HWND ctrl = GetDlgItem(hwndDlg, id);
	if(!GetPropW(ctrl, L"link_proc"))
	{
		SetPropW(ctrl, L"link_proc",
				(HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
	}
}

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if(uMsg == WM_SETCURSOR)
	{
		if((HWND)wParam == hwndDlg)
		{
			if(!link_hand_cursor)
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON)
		{
			wchar_t wt[123] = {0};
			int y;
			RECT r;
			HPEN hPen, hOldPen;
			GetDlgItemTextW(hwndDlg, wParam, wt, sizeof(wt)/sizeof(wt[0])); 

			// draw text
			SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			r = di->rcItem;
			// is used by the file types page to have a slimmer button so it doesn't override other
			// characters which are otherwise drawn over by the size of the button needed for a link
			if(GetPropW(di->hwndItem, L"slim"))
			{
				r.top -= 2;
			}
			r.left += 2;
			DrawTextW(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

			memset(&r, 0, sizeof(r));
			DrawTextW(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

			// draw underline
			y = di->rcItem.bottom - ((di->rcItem.bottom - di->rcItem.top) - (r.bottom - r.top)) / 2 - 1;
			hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			hOldPen = (HPEN) SelectObject(di->hDC, hPen);
			MoveToEx(di->hDC, di->rcItem.left + 2, y, NULL);
			LineTo(di->hDC, di->rcItem.right + 2 - ((di->rcItem.right - di->rcItem.left) - (r.right - r.left)), y);
			SelectObject(di->hDC, hOldPen);
			DeleteObject(hPen);
		}
	}
}

///////// if you update this, be sure to update the copy of it in $/winampa/winampicon.c
// thx.
int geticonid(int x)
{
	switch (x) {
		case 1: return IDI_FILEICON;
		case 2: return IDI_FILEICON2;
		case 3: return IDI_FILEICON3;
		case 4: return IDI_FILEICON10;
		case 5: return IDI_FILEICON5;
		case 6: return IDI_FILEICON6;
		case 7: return IDI_FILEICON7;
		case 8: return IDI_FILEICON8;
		case 9: return IDI_FILEICON9;
		case 10: return IDI_FILEICON4;
		case 11: return IDI_FILEICON11;
		case 12: return ICON_TB1;
		case 13: return -666;
		default: return ICON_XP;
	}
}
/*
void plSplineGetPoint(pl_Spline *s, float frame, float *out)
{
	int i, i_1, i0, i1, i2;
	float time1, time2, time3;
	float t1, t2, t3, t4, u1, u2, u3, u4, v1, v2, v3;
	float a, b, c, d;

	float *keys = s->keys;

	a = (1 - s->tens) * (1 + s->cont) * (1 + s->bias);
	b = (1 - s->tens) * (1 - s->cont) * (1 - s->bias);
	c = (1 - s->tens) * (1 - s->cont) * (1 + s->bias);
	d = (1 - s->tens) * (1 + s->cont) * (1 - s->bias);
	v1 = t1 = -a / 2.0f; u1 = a;
	u2 = ( -6 - 2 * a + 2 * b + c) / 2.0f; v2 = (a - b) / 2.0f; t2 = (4 + a - b - c) / 2.0f;
	t3 = ( -4 + b + c - d) / 2.0f;
	u3 = (6 - 2 * b - c + d) / 2.0f;
	v3 = b / 2.0f;
	t4 = d / 2.0f; u4 = -t4;

	i0 = (int) frame;
	i_1 = i0 - 1;
	while (i_1 < 0) i_1 += s->numKeys;
	i1 = i0 + 1;
	while (i1 >= s->numKeys) i1 -= s->numKeys;
	i2 = i0 + 2;
	while (i2 >= s->numKeys) i2 -= s->numKeys;
	time1 = frame - (float) ((int) frame);
	time2 = time1 * time1;
	time3 = time2 * time1;
	i0 *= s->keyWidth;
	i1 *= s->keyWidth;
	i2 *= s->keyWidth;
	i_1 *= s->keyWidth;
	for (i = 0; i < s->keyWidth; i ++)
	{
		a = t1 * keys[i + i_1] + t2 * keys[i + i0] + t3 * keys[i + i1] + t4 * keys[i + i2];
		b = u1 * keys[i + i_1] + u2 * keys[i + i0] + u3 * keys[i + i1] + u4 * keys[i + i2];
		c = v1 * keys[i + i_1] + v2 * keys[i + i0] + v3 * keys[i + i1];
		*out++ = a * time3 + b * time2 + c * time1 + keys[i + i0];
	}
}
*/
//int transAccel(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	HACCEL h;
//	MSG msg;
//	if (hwnd == hMainWindow) h = hAccel[0];
//	else if (hwnd == hEQWindow) h = hAccel[1];
//	else if (hwnd == hPLWindow) h = hAccel[2];
//	//	else if (hwnd==hMBWindow) h=hAccel[3];
//	else if (hwnd == hVideoWindow || GetParent(hwnd) == hVideoWindow)
//	{
//		h = hAccel[0];
//	}
//	else
//	{
//		return 0;
//	}
//	msg.hwnd = hwnd;
//	msg.lParam = lParam;
//	msg.message = uMsg;
//	msg.wParam = wParam;
//	return TranslateAccelerator(hwnd, h, &msg);
//
//}

#include <pshpack4.h>

typedef struct
{
	DWORD dwSize;
	HANDLE hrasconn;
	char szEntryName[256 + 1];
	char szDeviceType[ 16 + 1 ];
	char szDeviceName[ 128 + 1 ];
}
RASCONN ;
#include <poppack.h>

typedef DWORD (WINAPI *RASENUMCONNECTIONS)(RASCONN *lprasconn,
        LPDWORD lpcb,
        LPDWORD lpcConnections);

static int isRASActive()
{
	int r = 0;
	HINSTANCE h = LoadLibraryA("rasapi32.dll");
	RASENUMCONNECTIONS RasEnumConnections;
	RASCONN v = {sizeof(RASCONN), };

	DWORD i = sizeof(v), o = 0;

	if (!h) return 0;
	RasEnumConnections = (RASENUMCONNECTIONS)GetProcAddress(h, "RasEnumConnectionsA");
	if (RasEnumConnections && !RasEnumConnections(&v, &i, &o) && o) r = 1;
	FreeModule(h);
	return r;
}

int isInetAvailable(void)
{
	if (config_inet_mode == 3)
	{
		if (isRASActive()) config_inet_mode = 1;
		else config_inet_mode = 0;
		return (1 == config_inet_mode);
	}
	if (config_inet_mode == 0) return 1;
	if (config_inet_mode == 2) return 0;
	return isRASActive();
}

unsigned int getDay(void)
{
	unsigned int day = 0;
	SYSTEMTIME tm, st = {0, };
	FILETIME ft1, ft2;
	ULARGE_INTEGER l1, l2;
	GetSystemTime(&tm);
	st.wYear = 1978;
	st.wMonth = 10;
	st.wDay = 14;
	SystemTimeToFileTime(&tm, &ft1);
	SystemTimeToFileTime(&st, &ft2);
	memcpy(&l1, &ft1, sizeof(l1));
	memcpy(&l2, &ft2, sizeof(l2));
	day = (int) ((l1.QuadPart - l2.QuadPart) / (10 * 1000 * 1000) / (60 * 60 * 24));
	return day;
}

void recent_add(const wchar_t *loc)
{
	wchar_t ls[MAX_PATH] = {0};
	_r_sW("RecentURL1", ls, MAX_PATH);
	_w_sW("RecentURL1", loc);

	if (wcscmp(ls, loc))
	{
		int x = 2;
		while(1)
		{
			char s[123] = {0};
			wchar_t temp[MAX_PATH] = {0};
			StringCchPrintfA(s, 123, "RecentURL%d", x);

			_r_sW(s, temp, MAX_PATH);
			if (ls[0]) _w_sW(s, ls);
			if (!_wcsicmp(temp, loc) || !temp[0])
				break;
			lstrcpynW(ls, temp, MAX_PATH);
			x++;
		}
	}
}

#include <assert.h>
void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;
	if (p || sr || wnd)
	{
		HMONITOR hm = NULL;

		if (sr)
			hm = MonitorFromRect(sr, MONITOR_DEFAULTTONEAREST);
		else if (wnd)
			hm = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
		else if (p)
			hm = MonitorFromPoint(*p, MONITOR_DEFAULTTONEAREST);

		if (hm)
		{
			MONITORINFOEXW mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfoW(hm, &mi))
			{
				if (!full)
					*r = mi.rcWork;
				else
					*r = mi.rcMonitor;

				return ;
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top    = r->left = 0;
		r->right  = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfoW(SPI_GETWORKAREA, 0, r, 0);
	}
}

BOOL windowOffScreen(HWND hwnd, POINT pt)
{
	RECT r = {0}, wnd = {0}, sr = {0};
	GetWindowRect(hwnd, &wnd);
	sr.left = pt.x;
	sr.top = pt.y;
	sr.right = sr.left + (wnd.right - wnd.left);
	sr.bottom = sr.top + (wnd.bottom - wnd.top);
	getViewport(&r, hwnd, 0, &sr);
	return !PtInRect(&r, pt);
}

void readwrite_client_uid(int isWrite, wchar_t uid_str[64])
{
	HKEY hkey;
	wchar_t path[MAX_PATH] = {0};
	GetModuleFileNameW(0, path, MAX_PATH);

	if (isWrite)
	{
		IFileTypeRegistrar *registrar=0;
		if (GetRegistrar(&registrar, true) == 0 && registrar)
		{
			registrar->WriteClientUIDKey(path, uid_str);
			registrar->Release();
		}
		return;
	}

	if (!isWrite) *uid_str = 0;
	if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nullsoft\\Winamp", 0, 0, 0, KEY_READ, NULL, &hkey, NULL) == ERROR_SUCCESS)
	{
		DWORD s = 512, t = REG_SZ;
		if (RegQueryValueExW(hkey, path, 0, &t, (LPBYTE)uid_str, &s) != ERROR_SUCCESS || t != REG_SZ) uid_str[0] = 0;
		RegCloseKey(hkey);
	}
}


BOOL read_compatmode()
{
	HKEY hkey = NULL;
	wchar_t path[MAX_PATH] = {0};
	GetModuleFileNameW(0, path, MAX_PATH);
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 0, 0, 0, KEY_READ, NULL, &hkey, NULL) == ERROR_SUCCESS)
	{
		DWORD s = 512, t = REG_SZ;
		wchar_t buf[128] = {0};
		if (RegQueryValueExW(hkey, path, 0, &t, (LPBYTE)buf, &s) != ERROR_SUCCESS || t != REG_SZ) buf[0] = 0;
		RegCloseKey(hkey);

		wchar_t *p = buf, *fields[] = {L"256COLOR", L"640X480", L"DISABLETHEMES", L"$",
									   L"DISABLEDWM", L"DISABLEUSERCALLBACKEXCEPTION",
									   L"HIGHDPIAWARE", L"RUNASADMIN", L"IGNOREFREELIBRARY",
									   L"ELEVATECREATEPROCESS", L"#", L"~", L"PLACEHOLDERFILES"};
		int fields_len[] = {8, 7, 13, 1, 10, 28, 12, 10, 17, 20, 1, 1, 16};
		while (p && *p)
		{
			wchar_t *pp = p;
			while (pp && *pp && *pp != L' ')
			{
				pp = CharNextW(pp);
			}

			wchar_t pp_old = (pp ? *pp : 0);
			if (pp && *pp) *pp = 0;

			bool found = false;
			for (int i = 0; i < sizeof(fields)/sizeof(fields[0]); i++)
			{
				if (!wcsncmp(fields[i], p, fields_len[i]))
					found = true;
			}
			if (!found)
				return TRUE;

			if (pp) *pp = pp_old;
			if (!pp_old) break;
			p = CharNextW(pp);
		}
	}
	return FALSE;
}

BOOL IsVista(void)
{
	static INT fVista = -1; 

	if (-1 == fVista) 
	{
		OSVERSIONINFO osver = {0};
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		fVista = (::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && (osver.dwMajorVersion >= 6)) ? 1 : 0;
	}

	return (1 == fVista);
}

BOOL IsVistaOrLower(void)
{
	static INT fVistaOrLower = -1; 

	if (-1 == fVistaOrLower) 
	{
		OSVERSIONINFO osver = {0};
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		fVistaOrLower = (::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT &&
						 (osver.dwMajorVersion <= 5 || (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0))) ? 1 : 0;
	}

	return (1 == fVistaOrLower);
}

BOOL IsWin8(void)
{
	static INT fWin8 = -1; 

	if (-1 == fWin8)
	{
		OSVERSIONINFO osver = {0};
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		fWin8 = ( ::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && (osver.dwMajorVersion > 6 || (osver.dwMajorVersion == 6 && osver.dwMinorVersion >= 2))) ? 1 : 0;
	}

	return (1 == fWin8);
}

//XP Theme crap
typedef HRESULT (WINAPI * ENABLETHEMEDIALOGTEXTURE)(HWND, DWORD);

int IsWinXPTheme(void)
{
	static int previousRet = -1;
	if (previousRet == -1)
	{
		BOOL bEnabled(FALSE);
		OSVERSIONINFO vi;

		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (GetVersionEx(&vi) && (vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion > 0)))
		{
			HINSTANCE dll = LoadLibraryW(TEXT("uxtheme.dll"));
			if (dll)
			{
				BOOL (WINAPI *waIsAppThemed)(void);
				BOOL (WINAPI *waIsThemeActive)(void);
				waIsAppThemed = (BOOL (WINAPI *)())GetProcAddress(dll, "IsAppThemed");
				waIsThemeActive = (BOOL (WINAPI *)())GetProcAddress(dll, "IsThemeActive");

				if (waIsAppThemed && waIsThemeActive && waIsAppThemed() && waIsThemeActive())
				{
					HMODULE hComCtl = LoadLibraryW(TEXT("comctl32.dll"));
					if (hComCtl)
					{
						HRESULT (CALLBACK *waDllGetVersion)(DLLVERSIONINFO*) = (HRESULT (CALLBACK *)(DLLVERSIONINFO*))GetProcAddress(hComCtl, "DllGetVersion");
						if (waDllGetVersion)
						{
							DLLVERSIONINFO dllVer;
							dllVer.cbSize = sizeof(DLLVERSIONINFO);
							if (S_OK == waDllGetVersion(&dllVer) && dllVer.dwMajorVersion >= 6) bEnabled = TRUE;
						}
						FreeLibrary(hComCtl);
					}
				}
				FreeLibrary(dll);
			}
		}
		previousRet = bEnabled;
	}
	return previousRet;
}

void DoWinXPStyle(HWND tab) 
{
	WAEnableThemeDialogTexture(tab, ETDT_ENABLETAB); 
}

HRESULT WAEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags)
{
	static int uxThemeTried = 0;
	static ENABLETHEMEDIALOGTEXTURE pfnETDT = NULL;
	
	if(!uxThemeTried)
	{
		HINSTANCE ux_hDll;
		if ((ux_hDll = LoadLibraryA("uxtheme.dll")) != NULL)
			pfnETDT = (ENABLETHEMEDIALOGTEXTURE)GetProcAddress(ux_hDll, "EnableThemeDialogTexture");

		uxThemeTried = 1;
	}

	return (pfnETDT) ? pfnETDT(hwnd, dwFlags) : E_NOTIMPL;
}

typedef BOOL(WINAPI*ISCOMPOSITIONACTIVE)(VOID);

int IsAero(void)
{
	static int uxTried = 0;
	static ISCOMPOSITIONACTIVE IsAeroActive = 0;
	if (!uxTried)
	{
		static HMODULE UXTHEME = 0;
		if ((UXTHEME = LoadLibraryA("uxtheme.dll")) != NULL)
			IsAeroActive = (ISCOMPOSITIONACTIVE)GetProcAddress(UXTHEME, "IsCompositionActive");

		uxTried = 1;
	}
	if (IsAeroActive)
		return !!IsAeroActive();
	else
		return 0;
}

/*
int IsCharSpace(char digit)
{
	WORD type=0;
	GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_SPACE;
}

int IsCharSpaceW(wchar_t digit)
{
	WORD type=0;
	GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_SPACE;
}
*/
LPCWSTR BuildFullPath(LPCWSTR pszPathRoot, LPCWSTR pszPath, LPWSTR pszDest, INT cchDest)
{
	LPCWSTR pszFile;
	if (!pszPath || !*pszPath) 
	{
		pszDest[0] = 0x00;
		return pszDest;
	}
	pszFile = PathFindFileNameW(pszPath);
	if (pszFile != pszPath) 
	{		
		if (PathIsRelativeW(pszPath))
		{	
			wchar_t szTemp[MAX_PATH] = {0};
			PathCombineW(szTemp, pszPathRoot, pszPath);
			PathCanonicalizeW(pszDest, szTemp);
		}
		else StringCchCopyW(pszDest, cchDest, pszPath);
	}
	else {
		if (pszPathRoot && *pszPathRoot) PathCombineW(pszDest, pszPathRoot, pszPath);
		else StringCchCopyW(pszDest, cchDest, pszPath);
	}
	
	return pszDest;
}

INT ComparePath(LPCWSTR pszPath1, LPCWSTR pszPath2, LPCWSTR pszPathRoot)  // compares two pathes
{
	INT cr;
	DWORD lcid;
	LPCWSTR pszFile1, pszFile2;

	if (!pszPath1 || !pszPath2) return 0;

	lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	
	pszFile1 = PathFindFileNameW(pszPath1);
	pszFile2 = PathFindFileNameW(pszPath2);

	cr = CompareStringW(lcid, NORM_IGNORECASE, pszFile1, -1, pszFile2, -1);
	
	if (CSTR_EQUAL == cr && (pszFile1 != pszPath1 || pszFile2 != pszPath2))
	{	
		wchar_t path1[MAX_PATH*2] = {0}, path2[MAX_PATH*2] = {0};
		pszPath1 = BuildFullPath(pszPathRoot, pszPath1, path1, sizeof(path1)/sizeof(wchar_t));
		pszPath2 = BuildFullPath(pszPathRoot, pszPath2, path2, sizeof(path2)/sizeof(wchar_t));
		if (!pszPath1 || !pszPath2) return 0;
		cr = CompareStringW(lcid, NORM_IGNORECASE, pszPath1, -1, path2, -1);
	}
	return cr;
}

BOOL DisabledWindow_OnMouseClick(HWND hwnd)
{
	DWORD windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (WS_DISABLED != ((WS_CHILD | WS_DISABLED) & windowStyle))
		return FALSE;

	HWND hActive = GetActiveWindow();
	HWND hPopup = GetWindow(hwnd, GW_ENABLEDPOPUP);

	BOOL beepOk = (hPopup == hActive || hwnd == GetWindow(hActive, GW_OWNER));
	if (!beepOk && NULL == hPopup)
	{
		for (HWND hWalker = hwnd; ;)
		{											
			hWalker = GetWindow(hWalker, GW_OWNER);
			if (NULL == hWalker || (0 != (WS_CHILD & GetWindowLongPtrW(hWalker, GWL_STYLE))))
				break;
			if (hActive == GetWindow(hWalker, GW_ENABLEDPOPUP))
			{
				beepOk = TRUE;
				break;
			}
		}
	}

	if (beepOk)
	{
		if (config_accessibility_modalflash.GetBool())
		{
			FLASHWINFO flashInfo;
			flashInfo.cbSize     = sizeof(FLASHWINFO);
			flashInfo.hwnd      = hActive;
			flashInfo.dwFlags   = FLASHW_CAPTION;
			flashInfo.uCount    = 2;
			flashInfo.dwTimeout = 100;

			FlashWindowEx(&flashInfo);
		}

		if (config_accessibility_modalbeep.GetBool())
			MessageBeep(MB_OK);
	}
	else
	{		
		for (HWND hWalker = hwnd; NULL == hPopup;)
		{											
			hWalker = GetWindow(hWalker, GW_OWNER);
			if (NULL == hWalker || (0 != (WS_CHILD & GetWindowLongPtrW(hWalker, GWL_STYLE))))
				break;
			hPopup = GetWindow(hWalker, GW_ENABLEDPOPUP);
		}

		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		if (NULL != hPopup && hPopup != hwnd)
		{
			BringWindowToTop(hPopup);
			SetActiveWindow(hPopup);
		}
	}

	return TRUE;
}