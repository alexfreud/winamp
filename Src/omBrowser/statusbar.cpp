#include "main.h"
#include "./statusbar.h"
#include "./graphics.h"
#include "./browserHost.h"
#include "../winamp/wa_dlg.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"

#include "./ifc_skinhelper.h"
#include "./ifc_wasabihelper.h"

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>

#define TEXTMARGIN_LEFT		3
#define TEXTMARGIN_TOP		1
#define TEXTMARGIN_RIGHT		3
#define TEXTMARGIN_BOTTOM	1


#define SBT_INFLATE_ID			31
#define SBT_INFLATE_DELAY		50
#define SBT_INFLATE_INTERVAL	20

#define SBT_MOUSEROLL_ID			32
#define SBT_MOUSEROLL_DELAY		50
#define SBT_MOUSEROLL_INTERVAL	20

#define SBF_MOUSEROLL			0x00000001
#define SBF_CACHEDFONT			0x00000002

typedef struct __STATUSBAR
{
	UINT		flags;
	RECT		parentRect;
	SIZE		textSize;
	
	LPWSTR		pszText;
	INT			cchText;
	INT			cchTextMax;

	HFONT		textFont;
	COLORREF	rgbBk;
	COLORREF	rgbText;
	HBRUSH		brushBk;

	LONG		desiredCX;
	LONG		mouseY;
	LONG		desiredY;
	HRGN		windowRegion;
	HWND		hBrowser;

	DWORD		inflateTime;
} STATUSBAR;

typedef struct __STATUSBARMOUSEHOOK
{
	HHOOK hHook;
	HWND	 hwnd;
} STATUSBARMOUSEHOOK;

static size_t tlsIndex = -1;


#define GetStatusbar(__hwnd) ((STATUSBAR*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))
static LRESULT CALLBACK Statusbar_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CALLBACK Statusbar_InflateTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD timerId);
static void CALLBACK Statusbar_MouseRollTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD timerId);
static LRESULT CALLBACK Statusbar_MouseHook(INT code, WPARAM wParam, LPARAM lParam);

BOOL Statusbar_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	ATOM klassAtom;
	ifc_wasabihelper *wasabi;

	if (GetClassInfo(hInstance, NWC_ONLINEMEDIASTATUSBAR, &wc)) 
		return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_ONLINEMEDIASTATUSBAR;
	wc.lpfnWndProc	= Statusbar_WindowProc;
	wc.style			= CS_PARENTDC | CS_SAVEBITS;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(STATUSBAR*);
	
	klassAtom = RegisterClassW(&wc);
	if (0 == klassAtom)
		return FALSE;
	
	if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)))
	{
		api_application *application;
		if (SUCCEEDED(wasabi->GetApplicationApi(&application)))
		{
			application->DirectMouseWheel_RegisterSkipClass(klassAtom);
			application->Release();
		}
		wasabi->Release();
	}
	return TRUE;
}

static BOOL Statusbar_GetTextSize(HWND hwnd, HFONT textFont, LPCWSTR pszText, INT cchText, SIZE *textSize)
{
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return FALSE;
	
	HFONT originalFont = (HFONT)SelectObject(hdc, textFont);
	
	BOOL result;
	if (0 == cchText)
	{
		TEXTMETRIC tm;
		result = GetTextMetrics(hdc, &tm);
		if (FALSE != result)
		{
			textSize->cx = 0;
			textSize->cy = tm.tmHeight;
		}
	}
	else
	{
		result = GetTextExtentPoint32(hdc, pszText, cchText, textSize);
	}
			
	SelectObject(hdc, originalFont);
	ReleaseDC(hwnd, hdc);
	
	return result;
}


static BOOL Statusbar_SetWindowPos(HWND hwnd, HWND hwndInsertAfter, INT x, INT y, INT cx, INT cy, UINT flags)
{
	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL != statusbar)
	{
		INT k = cy/4;
		if (k > cx) k = 0;

		POINT szPoint[5] = {0};
		//szPoint[0].x = 0;
		//szPoint[0].y = 0;
		szPoint[1].x = cx - k;
		//szPoint[1].y = 0;
		szPoint[2].x = cx;
		szPoint[2].y = k;
		szPoint[3].x = cx;
		szPoint[3].y = cy;
		//szPoint[4].x = 0;
		szPoint[4].y = cy;

		HRGN rgn = CreatePolygonRgn(szPoint, ARRAYSIZE(szPoint), WINDING);
		if (0 != SetWindowRgn(hwnd, rgn, TRUE))
		{
			if (NULL != statusbar->windowRegion)
				DeleteObject(statusbar->windowRegion);
			statusbar->windowRegion = rgn;
		}
		else
		{
			if (NULL != rgn)
				DeleteObject(rgn);
		}
	}

	if (0 == SetWindowPos(hwnd, hwndInsertAfter, x, y, cx, cy, flags))
		return FALSE;
	
	return TRUE;
}


static void Statusbar_Inflate(HWND hwnd)
{
	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL == statusbar) return;

	DWORD currentTime = GetTickCount();
	DWORD kMult = (currentTime - statusbar->inflateTime) / SBT_INFLATE_INTERVAL;
	if (kMult < 4) kMult = 1;

	statusbar->inflateTime = currentTime;

	RECT windowRect;
	if (!GetWindowRect(hwnd, &windowRect))
		return;

	DWORD windowStyle = GetWindowStyle(hwnd);

	LONG currentWidth = windowRect.right - windowRect.left;
	LONG targetCX = statusbar->desiredCX;

	if (0 == (SBS_ACTIVE & windowStyle) || targetCX < 16)
		targetCX = 0;

	if (currentWidth == targetCX)
		return;

	LONG width = currentWidth;
	LONG height = windowRect.bottom - windowRect.top;
	LONG step;

	while(kMult-- && width != targetCX)
	{
		if (width > targetCX)
		{
			step = (width - targetCX) / 3;
			if (step < 6) step = 6;
		
			width -= step;
			if (width < targetCX) width = targetCX;

		}
		else
		{
			step = (targetCX - width)*2 / 3;
			if (step < 48) step = 48;
			
			width += step;
			if (width > targetCX) width = targetCX;
		}
	}

	Statusbar_SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, 
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE);


	if (width < (windowRect.right - windowRect.left))
	{
		if (NULL != statusbar->hBrowser)
		{
			RECT invalidRect;
			SetRect(&invalidRect, windowRect.left + width - 4, windowRect.top, windowRect.right, windowRect.bottom);
			MapWindowPoints(HWND_DESKTOP, statusbar->hBrowser, (POINT*)&invalidRect, 2);
			RedrawWindow(statusbar->hBrowser, &invalidRect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
	else
	{
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_FRAME);
	}

	if (0 == width && 0 == (SBS_ACTIVE & windowStyle))
	{
		ShowWindow(hwnd, SW_HIDE);
	}
	else
	{
		if (width != targetCX)
		{
			SetTimer(hwnd, SBT_INFLATE_ID, SBT_INFLATE_INTERVAL, Statusbar_InflateTimer);
		}
	}
}

static void Statusbar_UpdateLayout(HWND hwnd, BOOL fForce)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_VISIBLE & windowStyle) && FALSE == fForce)
	{
		if (0 == (SBS_UPDATELAYOUT & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | SBS_UPDATELAYOUT);
		return;
	}

	if (0 != (SBS_UPDATELAYOUT & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~SBS_UPDATELAYOUT);

	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL == statusbar) return;

	if (!Statusbar_GetTextSize(hwnd, statusbar->textFont, statusbar->pszText, statusbar->cchText, &statusbar->textSize))
		ZeroMemory(&statusbar->textSize, sizeof(SIZE));
	
	RECT windowRect;
	SetRect(&windowRect, 
			statusbar->parentRect.left, 
			statusbar->parentRect.bottom - statusbar->textSize.cy + statusbar->mouseY, 
			statusbar->parentRect.left + statusbar->textSize.cx, 
			statusbar->parentRect.bottom + statusbar->mouseY);

	if (0 != statusbar->textSize.cy)
		windowRect.top -= (TEXTMARGIN_TOP + TEXTMARGIN_BOTTOM);
	if (0 != statusbar->textSize.cx)
		windowRect.right += (TEXTMARGIN_LEFT + TEXTMARGIN_RIGHT);
	
	if (windowRect.right > statusbar->parentRect.right - 2)
		windowRect.right = statusbar->parentRect.right - 2;

	if ((windowRect.right - windowRect.left) < 20)
		windowRect.right = windowRect.left;

	if (windowRect.top < statusbar->parentRect.top + 20)
		windowRect.top = statusbar->parentRect.bottom;

	RECT previousRect;
	if (!GetWindowRect(hwnd, &previousRect))
		SetRectEmpty(&previousRect);
			
	if (FALSE == EqualRect(&previousRect, &windowRect))
	{
		LONG width = windowRect.right - windowRect.left;
		LONG height = windowRect.bottom - windowRect.top;
		LONG prevWidth = previousRect.right - previousRect.left;
		LONG prevHeight = previousRect.bottom - previousRect.top;

		LONG widthAdjust = 0;
		if (statusbar->desiredCX != prevWidth)
		{
			widthAdjust = width - prevWidth;
		}

		KillTimer(hwnd, SBT_INFLATE_ID);
		statusbar->desiredCX = width;
		statusbar->inflateTime = GetTickCount();

		HWND hParent = GetParent(hwnd);
		MapWindowPoints(HWND_DESKTOP, hParent, (POINT*)&previousRect, 2);
		if (windowRect.left != previousRect.left || 	windowRect.top != previousRect.top || height != prevHeight || 0 != widthAdjust)
		{
			Statusbar_SetWindowPos(hwnd, HWND_TOP, windowRect.left, windowRect.top, prevWidth + widthAdjust, height, 
				SWP_NOACTIVATE | SWP_NOOWNERZORDER);

			if (widthAdjust < 0 && NULL != statusbar->hBrowser)
			{
				RECT invalidRect;
				SetRect(&invalidRect, previousRect.left + prevWidth + widthAdjust - 4, previousRect.top, previousRect.left + prevWidth, previousRect.bottom);
				MapWindowPoints(HWND_DESKTOP, statusbar->hBrowser, (POINT*)&invalidRect, 2);
				RedrawWindow(statusbar->hBrowser, &invalidRect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if (0 != widthAdjust)
			{
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_FRAME);
			}
		}
		
		if (width > prevWidth)
		{
			Statusbar_Inflate(hwnd);
		}
		else
		{
			SetTimer(hwnd, SBT_INFLATE_ID, SBT_INFLATE_DELAY, Statusbar_InflateTimer);
			//Statusbar_Inflate(hwnd);
		}
	}
	else
	{
        InvalidateRect(hwnd, NULL, FALSE);
	}
}



static BOOL Statusbar_InstallMouseHook(HWND hwnd)
{
	if (TLS_OUT_OF_INDEXES == tlsIndex)
	{
		tlsIndex = Plugin_TlsAlloc();
		if (TLS_OUT_OF_INDEXES == tlsIndex) 
			return FALSE;
	}

	STATUSBARMOUSEHOOK *hook = (STATUSBARMOUSEHOOK*)Plugin_TlsGetValue(tlsIndex);
    if (NULL != hook) return FALSE;

	hook = (STATUSBARMOUSEHOOK*)calloc(1, sizeof(STATUSBARMOUSEHOOK));
	if (NULL == hook) return FALSE;

	hook->hwnd = hwnd;
	hook->hHook = SetWindowsHookEx(WH_MOUSE, Statusbar_MouseHook, NULL, GetCurrentThreadId());
	if (NULL == hook->hHook)
	{
		free(hook);
		return FALSE;
	}
	
	Plugin_TlsSetValue(tlsIndex, hook); 
	return TRUE;
}

static void Statusbar_RemoveMouseHook()
{
	if (TLS_OUT_OF_INDEXES == tlsIndex) return;

	STATUSBARMOUSEHOOK *hook = (STATUSBARMOUSEHOOK*)Plugin_TlsGetValue(tlsIndex);
    if (NULL == hook) return;
	
	Plugin_TlsSetValue(tlsIndex, NULL); 

	if (NULL != hook->hHook) 
		UnhookWindowsHookEx(hook->hHook);
	free(hook);
}

static void CALLBACK Statusbar_InstallMouseHookApc(ULONG_PTR param)
{
	Statusbar_InstallMouseHook((HWND)param);
}

static void CALLBACK Statusbar_RemoveMouseHookApc(ULONG_PTR param)
{
	Statusbar_RemoveMouseHook();
}
static void Statusbar_MouseRoll(HWND hwnd)
{
	STATUSBAR *psb = GetStatusbar(hwnd);
	if (NULL == psb || 0 == (SBF_MOUSEROLL & psb->flags)) return;

	if (psb->desiredY == psb->mouseY)
	{
		psb->flags &= ~SBF_MOUSEROLL;
		return;
	}

	RECT windowRect;
	if (!GetWindowRect(hwnd, &windowRect))
		return;

	windowRect.top -= psb->mouseY;

	if (psb->mouseY > psb->desiredY)
	{
		psb->mouseY -= 4;
		if (psb->mouseY < psb->desiredY) psb->mouseY = psb->desiredY;
	}
	else
	{
		psb->mouseY += 4;
		if (psb->mouseY > psb->desiredY) psb->mouseY = psb->desiredY;
	}
			
	windowRect.top += psb->mouseY;
	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
	{
		MapWindowPoints(HWND_DESKTOP, hParent, (POINT*)&windowRect, 1);
		SetWindowPos(hwnd, HWND_TOP, windowRect.left, windowRect.top, 0, 0, 
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
	}

	if (psb->desiredY == psb->mouseY ||
		0 == SetTimer(hwnd, SBT_MOUSEROLL_ID, SBT_MOUSEROLL_INTERVAL, Statusbar_MouseRollTimer))
	{
		psb->flags &= ~SBF_MOUSEROLL;
	}
}

static void Statusbar_MouseCheck(HWND hwnd, POINT pt)
{	
	STATUSBAR *psb = GetStatusbar(hwnd);
	if (NULL == psb) return;
	
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_DISABLED & windowStyle))
	{
		if (0 != psb->desiredY)
		{
			psb->desiredY = 0;
			if (0 == (SBF_MOUSEROLL & psb->flags) &&
				0 != SetTimer(hwnd, SBT_MOUSEROLL_ID, 0, Statusbar_MouseRollTimer))
			{
				psb->flags |= SBF_MOUSEROLL;
			}
		}
		return;
	}

	RECT windowRect;
	if (!GetWindowRect(hwnd, &windowRect))
		return;
			
	windowRect.right = windowRect.left + psb->desiredCX;
	windowRect.top -= psb->mouseY;
	windowRect.bottom -= psb->mouseY;
	LONG mouseY = psb->desiredY;
		
	if (pt.y < windowRect.bottom)
	{
		if (pt.y < (windowRect.bottom - 12))
		{
			pt.y += 12;
		}
		else
		{
			pt.y = windowRect.bottom - 1;
		}
	}

	if (PtInRect(&windowRect, pt))
	{					
		psb->desiredY = pt.y - windowRect.top + 1;
	}
	else
	{
		psb->desiredY = 0;
	}

	if (psb->desiredY != mouseY)
	{
		if (0 == (SBF_MOUSEROLL & psb->flags) &&
			0 != SetTimer(hwnd, SBT_MOUSEROLL_ID, 0, Statusbar_MouseRollTimer))
		{
			psb->flags |= SBF_MOUSEROLL;
		}
	}
}

static void Statusbar_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 != (SBS_UPDATELAYOUT & windowStyle))
		Statusbar_UpdateLayout(hwnd, TRUE);

	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL == statusbar) return;

	RECT clientRect, textRect;
	GetClientRect(hwnd, &clientRect);
	CopyRect(&textRect, &clientRect);
	
	SetBkColor(hdc, statusbar->rgbBk);
	SetTextColor(hdc, statusbar->rgbText);

	HRGN rgnBack = CreateRectRgnIndirect((NULL != prcPaint) ? prcPaint : &clientRect);
	HRGN rgn = NULL;

	if (0 != statusbar->cchText)
	{
		textRect.top += TEXTMARGIN_TOP;
		textRect.right -= TEXTMARGIN_RIGHT;

		SetBkMode(hdc, TRANSPARENT);
		SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
	
		HFONT origFont = (HFONT)SelectObject(hdc, statusbar->textFont);
		
		rgn = CreateRectRgnIndirect(&textRect);

		DWORD options = (FALSE != fErase) ? ETO_OPAQUE : 0;
		if (0 != ExtTextOut(hdc, textRect.left + TEXTMARGIN_LEFT, textRect.bottom - TEXTMARGIN_BOTTOM, 
					options, &textRect, statusbar->pszText, statusbar->cchText, NULL))
		{
			CombineRgn(rgnBack, rgnBack, rgn, RGN_DIFF);
		}
		SelectObject(hdc, origFont);
	}

	if (0 != fErase)
	{
		FillRgn(hdc, rgnBack, statusbar->brushBk);
	}

	if (NULL != rgn) DeleteObject(rgn);
	if (NULL != rgnBack) DeleteObject(rgnBack);

}

static LRESULT Statusbar_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{
	STATUSBAR *statusbar = (STATUSBAR*)calloc(1, sizeof(STATUSBAR));
	if (NULL != statusbar)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)statusbar) && ERROR_SUCCESS != GetLastError())
		{
			free(statusbar);
			statusbar = NULL;
		}
	}

	if (NULL == statusbar)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	return 0;
}

static void Statusbar_OnDestroy(HWND hwnd)
{
	STATUSBAR *statusbar = GetStatusbar(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);

	if (NULL == statusbar) return;

	if (NULL != statusbar->textFont && 0 == (SBF_CACHEDFONT & statusbar->flags))
		DeleteObject(statusbar->textFont);

	if (NULL != statusbar->windowRegion)
		DeleteObject(statusbar->windowRegion);

	if (NULL != statusbar->brushBk)
		DeleteObject(statusbar->brushBk);

	Statusbar_RemoveMouseHook();
	if (NULL != statusbar->hBrowser)
		PostMessage(statusbar->hBrowser, NBHM_QUEUEAPC, (WPARAM)hwnd, (LPARAM)Statusbar_RemoveMouseHookApc);

	free(statusbar);
}

static void Statusbar_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			Statusbar_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void Statusbar_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	Statusbar_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}
static BOOL Statusbar_SetText(HWND hwnd, LPCWSTR pszText)
{
	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL == statusbar) return FALSE;

	INT cchText;
	cchText = (NULL != pszText && FALSE == IS_INTRESOURCE(pszText)) ? lstrlen(pszText) : 0;

	if (cchText >= statusbar->cchTextMax)
	{
		statusbar->cchText = 0;
		statusbar->cchTextMax = 0;
		if (NULL != statusbar->pszText)
			Plugin_FreeString(statusbar->pszText);

		INT cchMalloc = ((cchText / 1024) + 1) * 1024;
		statusbar->pszText = Plugin_MallocString(cchMalloc);
		if (NULL == statusbar->pszText)
			return FALSE;

		statusbar->cchTextMax = cchMalloc;
	}

	statusbar->cchText = cchText;
	if (0 == cchText)
	{	
		statusbar->pszText[0] = L'\0';
	}
	else
	{
		if (FAILED(StringCchCopy(statusbar->pszText, statusbar->cchTextMax, pszText)))
		{
			statusbar->pszText[0] = L'\0';
			statusbar->cchText = 0;
			return FALSE;
		}
	}

	return TRUE;
}
static BOOL Statusbar_OnSetText(HWND hwnd, LPCWSTR pszText)
{	
	if (FALSE == Statusbar_SetText(hwnd, pszText))
		return FALSE;

	Statusbar_UpdateLayout(hwnd, FALSE);
	return TRUE;
}

static INT Statusbar_OnGetText(HWND hwnd, LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax)
		return 0;

	pszBuffer[0] = L'\0';

	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL == statusbar) return 0;
	
	INT cchCopy = (statusbar ? statusbar->cchText : 0);
	if (NULL != statusbar && 0 != cchCopy)
	{		
		if (cchCopy >= cchBufferMax)
			cchCopy = (cchBufferMax - 1);

		StringCchCopyN(pszBuffer, cchBufferMax, statusbar->pszText, cchCopy);
	}
	return cchCopy;
}

static INT Statusbar_OnGetTextLength(HWND hwnd)
{
	STATUSBAR *statusbar = GetStatusbar(hwnd);
	return (NULL != statusbar) ? statusbar->cchText : 0;
}

static LRESULT Statusbar_OnShowWindow(HWND hwnd, BOOL fShow, UINT nState)
{
	STATUSBAR *psb = GetStatusbar(hwnd);
	if (NULL != psb)
	{
		if (FALSE != fShow)
		{
			Statusbar_InstallMouseHook(hwnd);
			if (NULL != psb->hBrowser)
				PostMessage(psb->hBrowser, NBHM_QUEUEAPC, (WPARAM)hwnd, (LPARAM)Statusbar_InstallMouseHookApc);
			
		}
		else
		{
			Statusbar_RemoveMouseHook();
			if (NULL != psb->hBrowser)
				PostMessage(psb->hBrowser, NBHM_QUEUEAPC, (WPARAM)hwnd, (LPARAM)Statusbar_RemoveMouseHookApc);
		}
	}

	return DefWindowProcW(hwnd, WM_SHOWWINDOW, (WPARAM)fShow, (LPARAM)nState);
}
static void Statusbar_OnUpdateSkin(HWND hwnd, BOOL fRedraw)
{
	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL == statusbar) return;

	ifc_skinhelper *skin;
	if (FAILED(Plugin_GetSkinHelper(&skin))) 
		skin = NULL;
	
	if (NULL == skin || FAILED(skin->GetColor(WADLG_WNDBG, &statusbar->rgbBk)))
		statusbar->rgbBk = GetSysColor(COLOR_WINDOW);
	
	if (NULL == skin || FAILED(skin->GetColor(WADLG_WNDFG, &statusbar->rgbText)))
		statusbar->rgbText = GetSysColor(COLOR_WINDOWTEXT);
	statusbar->rgbText = BlendColors(statusbar->rgbText, statusbar->rgbBk, 127);

	if (NULL != statusbar->textFont)
	{
		if (0 == (SBF_CACHEDFONT & statusbar->flags))
			DeleteObject(statusbar->textFont);
		statusbar->textFont = NULL; 
	}

	if (NULL != skin)
		statusbar->textFont = skin->GetFont();

	if (NULL != statusbar->textFont) 
		statusbar->flags |= SBF_CACHEDFONT;
	else
	{
		statusbar->flags &= ~SBF_CACHEDFONT;
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"Tahoma");
		statusbar->textFont = CreateFontIndirect(&lf);
	}
		
	if (NULL != skin)
		skin->Release();

	if (NULL != statusbar->brushBk)
		DeleteObject(statusbar->brushBk);

	statusbar->brushBk = CreateSolidBrush(statusbar->rgbBk);

	Statusbar_UpdateLayout(hwnd, FALSE);
}

static void Statusbar_OnSetParentRect(HWND hwnd, const RECT *parentRect)
{
	STATUSBAR *statusbar = GetStatusbar(hwnd);
	if (NULL == statusbar || NULL == parentRect) return;

	CopyRect(&statusbar->parentRect, parentRect);
	Statusbar_UpdateLayout(hwnd, FALSE);
}

static BOOL Statusbar_OnSetActive(HWND hwnd, BOOL fActive)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 == (SBS_ACTIVE & windowStyle) == (FALSE == fActive))
		return TRUE;

	if (FALSE == fActive)
		windowStyle &= ~SBS_ACTIVE;
	else
		windowStyle |= SBS_ACTIVE;

	SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);

	if (0 != (SBS_ACTIVE & windowStyle))
	{
		if (0 == (WS_VISIBLE & windowStyle))
		{
			Statusbar_UpdateLayout(hwnd, TRUE);
			ShowWindow(hwnd, SW_SHOWNA);
		}
	}
	else
	{
		if (0 != (WS_VISIBLE & windowStyle))
		{
			STATUSBAR *statusbar = GetStatusbar(hwnd);
			if (NULL != statusbar)
			{
				KillTimer(hwnd, SBT_INFLATE_ID);
				statusbar->desiredCX = 0;
				statusbar->inflateTime = GetTickCount();
				SetTimer(hwnd, SBT_INFLATE_ID, SBT_INFLATE_DELAY, Statusbar_InflateTimer);
			}
		}
	}
	return TRUE;
}

static BOOL Statusbar_OnUpdate(HWND hwnd, LPCWSTR pszText)
{
	STATUSBAR *psb = GetStatusbar(hwnd);
	if (NULL == psb) return FALSE;

	if (NULL == pszText || L'\0' == *pszText)
		return TRUE;

	if (NULL == pszText)
	{
		if (NULL == psb->pszText)
		{
			return TRUE;
		}
		pszText = L"";
	}

	if (NULL != psb->pszText &&
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, psb->pszText, -1, pszText, -1))
	{
		return TRUE;
	}

	Statusbar_SetText(hwnd, pszText);

	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
	{
		Statusbar_UpdateLayout(hwnd, FALSE);
		InvalidateRect(hwnd, NULL, TRUE);
	}
			
	return TRUE;
}

static BOOL Statusbar_OnSetBrowserHost(HWND hwnd, HWND hBrowser)
{
	STATUSBAR *psb = GetStatusbar(hwnd);
	if (NULL == psb) return FALSE;

	psb->hBrowser = hBrowser;
	return TRUE;
}

static LRESULT Statusbar_OnEnable(HWND hwnd, BOOL fEnable)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	UINT newStyle = windowStyle;
	
	if (FALSE == fEnable)
		newStyle |= WS_DISABLED;
	else
		newStyle &= ~WS_DISABLED;

	if(newStyle == windowStyle)
		return fEnable;
	
	SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);

	return !fEnable;
}

static LRESULT CALLBACK Statusbar_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:				return Statusbar_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:				Statusbar_OnDestroy(hwnd); break;
		case WM_PAINT:				Statusbar_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:			Statusbar_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_SETTEXT:				return Statusbar_OnSetText(hwnd, (LPCWSTR)lParam);
		case WM_GETTEXT:				return Statusbar_OnGetText(hwnd, (LPWSTR)lParam, (INT)wParam);
		case WM_GETTEXTLENGTH:		return Statusbar_OnGetTextLength(hwnd);
		case WM_SHOWWINDOW:			return Statusbar_OnShowWindow(hwnd, (BOOL)wParam, (UINT)lParam);

		case SBM_UPDATESKIN:			Statusbar_OnUpdateSkin(hwnd, (BOOL)lParam); return 0;
		case SBM_SETPARENTRECT:		Statusbar_OnSetParentRect(hwnd, (const RECT*)lParam); return 0;
		case SBM_SETACTIVE:			return Statusbar_OnSetActive(hwnd, (BOOL)wParam); 
		case SBM_UPDATE:				return Statusbar_OnUpdate(hwnd, (LPCWSTR)lParam); 
		case SBM_SETBROWSERHOST:		return Statusbar_OnSetBrowserHost(hwnd, (HWND)lParam);
		case SBM_ENABLE:				return Statusbar_OnEnable(hwnd, (BOOL)lParam);
	
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
static void CALLBACK Statusbar_InflateTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD timerId)
{
	KillTimer(hwnd, eventId);
	Statusbar_Inflate(hwnd);
}

static void CALLBACK Statusbar_MouseRollTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD timerId)
{
	KillTimer(hwnd, eventId);
	Statusbar_MouseRoll(hwnd);
}

static LRESULT CALLBACK Statusbar_MouseHook(INT code, WPARAM wParam, LPARAM lParam)
{
	STATUSBARMOUSEHOOK *hook = (STATUSBARMOUSEHOOK*)Plugin_TlsGetValue(tlsIndex);
	if (NULL == hook || NULL == hook->hHook) return FALSE;
	
	if (code >= 0)
	{
		MOUSEHOOKSTRUCT *mouseHook = (MOUSEHOOKSTRUCT*)lParam;
		if (NULL != hook->hwnd)
		{
			Statusbar_MouseCheck(hook->hwnd, mouseHook->pt);
		}
	}		
	return CallNextHookEx(hook->hHook, code, wParam, lParam);
}
