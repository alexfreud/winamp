#include <windows.h>
#include <commctrl.h>
#include <ddraw.h>
#include <math.h>
#include "../Winamp/wa_ipc.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"
#include <api/service/waServiceFactory.h>
#include "dd.h"
#include "../winamp/vis.h"

int (*warand)(void)=0;
extern wchar_t szAppName[];

extern void init_inifile(struct winampVisModule *this_mod);
extern wchar_t *ini_file;

static int g_width=640, g_height=480;
static int g_divx=16, g_divy=12, g_fadeout=2;
static C_DD g_dd;
extern HWND g_hwnd,hwndParent;
static int running;
int g_minflag;
extern api_service *WASABI_API_SVC;

void do_min(HWND hwnd)
{
	g_minflag=!IsIconic(hwnd);
	if (g_minflag)
	{
		ShowWindow(hwnd,SW_MINIMIZE);
		Sleep(100);
	}
}

void do_unmin(HWND hwnd)
{
	if (g_minflag)
	{
		g_minflag=0;
		ShowWindow(hwnd,SW_RESTORE);
		SetForegroundWindow(hwnd);
	}
}

static void readconfig(struct winampVisModule *this_mod)
{
	init_inifile(this_mod);
	g_width=GetPrivateProfileIntW(szAppName, L"sdwidth", 640, ini_file);
	g_height=GetPrivateProfileIntW(szAppName, L"sdheight", 480, ini_file);
	g_divx=GetPrivateProfileIntW(szAppName, L"sdx", 16, ini_file);
	g_divy=GetPrivateProfileIntW(szAppName, L"sdy", 12, ini_file);
	g_fadeout=GetPrivateProfileIntW(szAppName, L"sdfadeout", 2, ini_file);
}

extern void drawscope(unsigned char *out, int w, int h, unsigned char *visdata);
extern unsigned char *getnewpalette();
extern void moveframe_init(int w, int h, int divx, int divy, int fadeval);
extern void moveframe(unsigned char *in, unsigned char *out, unsigned char *visdata);
extern void moveframe_quit();

static void writeInt(wchar_t *name, int value)
{
	wchar_t buf[32] = {0};
	wsprintfW(buf, L"%d", value);
	WritePrivateProfileStringW(szAppName, name, buf, ini_file);
}

static HRESULT WINAPI _cb(LPDDSURFACEDESC lpDDSurfaceDesc,  LPVOID lpContext)
{
	if (lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==8)
	{
		wchar_t s[32] = {0};
		wsprintfW(s, L"%dx%d", lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight);
		LRESULT idx=SendMessageW((HWND)lpContext,CB_ADDSTRING,0,(LPARAM)s);
		SendMessage((HWND)lpContext,CB_SETITEMDATA,idx,MAKELONG(lpDDSurfaceDesc->dwWidth,lpDDSurfaceDesc->dwHeight));

		if ((int)lpDDSurfaceDesc->dwWidth == g_width && (int)lpDDSurfaceDesc->dwHeight == g_height)
			SendMessage((HWND)lpContext,CB_SETCURSEL,idx,0);
	}
	return DDENUMRET_OK;
}


BOOL IsDirectMouseWheelMessage(const UINT uMsg)
{
	static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");
		if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
			return FALSE;
	}

	return (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg);
}

HWND ActiveChildWindowFromPoint(HWND hwnd, POINTS cursor_s, const int *controls, size_t controlsCount)
{
	POINT pt;
	POINTSTOPOINT(pt, cursor_s);

	while(controlsCount--)
	{
		RECT controlRect;
		HWND controlWindow = GetDlgItem(hwnd, controls[controlsCount]);
		if (NULL != controlWindow &&
			FALSE != GetClientRect(controlWindow, &controlRect))
		{
			MapWindowPoints(controlWindow, HWND_DESKTOP, (POINT*)&controlRect, 2);
			if (FALSE != PtInRect(&controlRect, pt))
			{
				unsigned long windowStyle;
				windowStyle = (unsigned long)GetWindowLongPtrW(controlWindow, GWL_STYLE);
				if(WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & windowStyle))
					return controlWindow;
				break;
			}
		}

	}
	return NULL;
}

BOOL DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		const int controls[] = 
		{
			IDC_BANDS,
			IDC_VUSE,
			IDC_FALLOFF,
			IDC_FALLOFF2,
			IDC_SCSCALE,
			IDC_SLIDER1,
		};
		HWND targetWindow = ActiveChildWindowFromPoint(hwnd, MAKEPOINTS(lParam), controls, ARRAYSIZE(controls));
		if (NULL != targetWindow)
		{
			SendMessage(targetWindow, WM_MOUSEWHEEL, wParam, lParam);
			SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (long)TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK dlgProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		LPDIRECTDRAW dd;
		if (DirectDrawCreate(NULL,&dd,NULL) == DD_OK)	
		{
			IDirectDraw_EnumDisplayModes(dd,0,NULL,GetDlgItem(hwndDlg,IDC_MODELIST),_cb);
			IDirectDraw_Release(dd);
		}
		SetDlgItemInt(hwndDlg,IDC_EDIT1,g_divx,0);
		SetDlgItemInt(hwndDlg,IDC_EDIT2,g_divy,0);
		SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGE,0,MAKELONG(0,32));
  		SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_fadeout);
		return 1;
	}
	if (uMsg == WM_CLOSE || (uMsg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)))
	{
		BOOL t;
		g_divx=GetDlgItemInt(hwndDlg,IDC_EDIT1,&t,0);
		g_divy=GetDlgItemInt(hwndDlg,IDC_EDIT2,&t,0);
		g_fadeout = SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_GETPOS,0,0);
		EndDialog(hwndDlg,IDOK);
	}
	if (uMsg == WM_COMMAND)
	{
		if (LOWORD(wParam) == IDC_MODELIST && HIWORD(wParam) == CBN_SELCHANGE)
		{
			DWORD n=SendDlgItemMessage(hwndDlg,IDC_MODELIST,CB_GETCURSEL,0,0);
			if (n != CB_ERR)
			{
				n=SendDlgItemMessage(hwndDlg,IDC_MODELIST,CB_GETITEMDATA,n,0);
				if (n != CB_ERR)
				{
					g_width=LOWORD(n);
					g_height=HIWORD(n);
				}
			}
		}
	}
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam))
	{
		return TRUE;
	}
	return 0;
}

void sd_config(struct winampVisModule *this_mod)
{	
	if (running) return;
	running=1;

	readconfig(this_mod);

	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(this_mod->hDllInstance,VisNFSFLangGUID);

	WASABI_API_DIALOGBOXW(IDD_DIALOG2,this_mod->hwndParent,dlgProc1);
	writeInt(L"sdwidth", g_width);
	writeInt(L"sdheight", g_height);
	writeInt(L"sdx", g_divx);
	writeInt(L"sdy", g_divy);
	writeInt(L"sdfadeout", g_fadeout);
	running=0;
}

static int __inline is_mmx(void) {	
	_asm {
    sub eax, eax
    sub edx, edx
    inc eax
		_emit 0x0f
		_emit 0xa2
    test eax, eax
    jz noMMX
		and edx, 0x800000
		mov eax, edx
    noMMX:

    sub ebx, ebx // make sure compiler knows ebx and ecx are blown.
    sub ecx, ecx
	}	
}


int sd_init(struct winampVisModule *this_mod)
{
	if (running) return 1;
	running=1;

	if (!is_mmx())
	{
		char err[16];
		running=0;
		MessageBox(this_mod->hwndParent,WASABI_API_LNGSTRING(IDS_MMX_NOT_FOUND),
				   WASABI_API_LNGSTRING_BUF(IDS_ERROR,err,16),MB_OK);
		return 1;
	}

	readconfig(this_mod);
	warand = (int (*)())SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GET_RANDFUNC);
 
	extern void initwindow(struct winampVisModule *this_mod,int,int);
	initwindow(this_mod,g_width,g_height);
	if (!g_hwnd) 
	{
		running=0;
		return 1;
	}

	do_min(this_mod->hwndParent);

	SetForegroundWindow(g_hwnd);

	char *t=g_dd.open(g_width,g_height,g_hwnd);
	if (t)
	{
		char error[32];
		DestroyWindow(g_hwnd);
		g_hwnd=NULL;
	    do_unmin(this_mod->hwndParent);
  		UnregisterClassW(szAppName,this_mod->hDllInstance);
		MessageBox(this_mod->hwndParent,t,WASABI_API_LNGSTRING_BUF(IDS_DIRECTDRAW_ERROR,error,32),MB_OK);
		running=0;
		return 1;
	}
	moveframe_init(g_width,g_height,g_divx,g_divy,g_fadeout);
	g_dd.setpalette(getnewpalette(),5000);
	return 0;
}

// render function. This draws a frame. Returns 0 if successful, 1 if visualization should end.
int sd_render(struct winampVisModule *this_mod)
{
	unsigned char *in, *out;

	if (g_dd.palette_fadeleft() < -500)
	{
		g_dd.setpalette(getnewpalette(),3500);
	}

	if (!g_dd.lock(&in,&out)) return 0;

	moveframe(in,out,this_mod->waveformData[0]);
	drawscope(out,g_width,g_height,this_mod->waveformData[0]);

	g_dd.unlock();
	return 0;
}

// cleanup (opposite of init()). Destroys the window, unregisters the window class
void sd_quit(struct winampVisModule *this_mod)
{
	g_dd.close();
	moveframe_quit();
	ShowCursor(TRUE);
	if (g_hwnd) DestroyWindow(g_hwnd);
	g_hwnd=0;
	UnregisterClassW(szAppName,this_mod->hDllInstance);
	running=0;
	do_unmin(this_mod->hwndParent);
}