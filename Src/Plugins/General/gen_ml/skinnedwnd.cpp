#include "./skinnedwnd.h"
#include "../winamp/wa_dlg.h"
#include "../nu/trace.h"
#include "./mldwm.h"
#include "../nu/CGlobalAtom.h"

static CGlobalAtom WNDDATAPROPW(L"SWDATA");

static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;


#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define SWS_ATTACHED		0x00010000		// window attached
#define SWS_UNICODE		0x00020000		// winodow is unicode
#define SWS_THEMED		0x00040000		// was themed before 
#define SWS_REFLECT		0x00080000		// support message reflection
#define SWS_DIALOG		0x00100000		// treat this as dialog

#define BORDER_WIDTH  1

extern HRESULT(WINAPI *SetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);  //xp theme shit
extern BOOL (__stdcall *IsAppThemed)(void);


#define DWM_COMPOSITION_CHECK		((UINT)-1)
#define DWM_COMPOSITION_DISABLED	((UINT)0)
#define DWM_COMPOSITION_ENABLED		((UINT)1)

static UINT		dwmCompositionEnabled = DWM_COMPOSITION_CHECK;


static BOOL CALLBACK SkinChangedNotifyCB(HWND hwnd, LPARAM param)
{
	SendMessageW(hwnd, (UINT)WM_ML_IPC, MAKEWPARAM(TRUE, param), (LPARAM)ML_IPC_SKINNEDWND_SKINCHANGED); 
	return TRUE;
}

SkinnedWnd *SkinnedWnd::GetFromHWND(HWND hwndSkinned)
{
	return (hwndSkinned && IsWindow(hwndSkinned)) ? (SkinnedWnd*)GetPropW(hwndSkinned, WNDDATAPROPW) : NULL;
}

BOOL SkinnedWnd::IsDwmCompositionEnabled()
{
	if (DWM_COMPOSITION_CHECK == dwmCompositionEnabled)
	{
		dwmCompositionEnabled = DWM_COMPOSITION_DISABLED;
		BOOL bEnabled;
		if (S_OK == MlDwm_LoadLibrary() && S_OK == MlDwm_IsCompositionEnabled(&bEnabled) && bEnabled)
			dwmCompositionEnabled = DWM_COMPOSITION_ENABLED;
	}
	return (DWM_COMPOSITION_ENABLED == dwmCompositionEnabled);
}

SkinnedWnd::SkinnedWnd(BOOL bIsDialog) 
	: hwnd(NULL), style(SWS_NORMAL), uiState(NULL), redrawLock(0),
	  fnWndProc(NULL), wnddata(SKINNEDWND_TYPE_WINDOW | ((bIsDialog) ? SWS_DIALOG : 0))
{
	minSize.cx = 0;
	minSize.cy = 0;
	maxSize.cx = 0;
	maxSize.cy = 0;

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");
}

SkinnedWnd::~SkinnedWnd(void)
{
	if (!hwnd || !IsWindow(hwnd)) return;
	RemovePropW(hwnd, WNDDATAPROPW);
	if (fnWndProc)
	{
		INT index;
		index = (SWS_DIALOG & wnddata) ? DWLP_DLGPROC : GWLP_WNDPROC;
		(SWS_UNICODE & wnddata) ? SetWindowLongPtrW(hwnd, index, (LONGX86)(LONG_PTR)fnWndProc) : SetWindowLongPtrA(hwnd, index, (LONGX86)(LONG_PTR)fnWndProc);
	}
	if ((SWS_THEMED & wnddata) && IsAppThemed && IsAppThemed() && SetWindowTheme) SetWindowTheme(hwnd, NULL, NULL);
}

BOOL SkinnedWnd::IsUnicode(void) 
{ 
	return ( 0 != (SWS_UNICODE & wnddata)); 
}

BOOL SkinnedWnd::IsAttached(void) 
{ 
	return ( 0 != (SWS_ATTACHED & wnddata)); 
}

BOOL SkinnedWnd::Attach(HWND hwndToSkin)
{
	INT index;
	if (hwnd) return FALSE;

	hwnd = hwndToSkin;
	if(!hwnd || GetPropW(hwnd, WNDDATAPROPW)) return FALSE;

	wnddata &= (SKINNEDWND_TYPE_WINDOW | SWS_DIALOG);
	
	if(IsWindowUnicode(hwnd)) wnddata |= SWS_UNICODE;

	index = (SWS_DIALOG & wnddata) ? DWLP_DLGPROC : GWLP_WNDPROC;


	fnWndProc= (WNDPROC)(LONG_PTR)((SWS_UNICODE & wnddata) ? SetWindowLongPtrW(hwnd, index, (LONGX86)(LONG_PTR)WindowProcReal) : SetWindowLongPtrA(hwnd, index, (LONGX86)(LONG_PTR)WindowProcReal));
	if (!fnWndProc || !SetPropW(hwnd, WNDDATAPROPW, this)) return FALSE;
	
	RemoveReflector(hwnd); // we will use this refelector

	wnddata |= (SWS_ATTACHED | SWS_REFLECT);


	if (S_OK == MlDwm_LoadLibrary())
	{
		DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;
		DWORD allow = FALSE;
		MlDwm_SetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
		MlDwm_SetWindowAttribute(hwnd, DWMWA_ALLOW_NCPAINT, &allow, sizeof(allow));
	}

	if (IsAppThemed && IsAppThemed() && SetWindowTheme)
	{
		SetWindowTheme(hwnd, NULL, L"");
		wnddata |= SWS_THEMED;
	}
	
	uiState =(WORD)SendMessageW(hwnd, WM_QUERYUISTATE, 0, 0L);

	return TRUE;
}

LRESULT SkinnedWnd::CallPrevWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (SWS_UNICODE & wnddata) ? CallWindowProcW(fnWndProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(fnWndProc, hwnd, uMsg, wParam, lParam);
}

LRESULT SkinnedWnd::CallDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (SWS_UNICODE & wnddata) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void SkinnedWnd::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{	
	if (SWS_USESKINFONT & style)
	{
		HFONT skinFont;
		
		skinFont = (HFONT)MlStockObjects_Get(SKIN_FONT);
		
		if (NULL == skinFont)
			skinFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);

		if (NULL != skinFont)
		{
			HFONT windowFont = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);
			if (skinFont != windowFont)
			{
				DisableRedraw();
				
				SendMessageW(hwnd, WM_SETFONT, (WPARAM)skinFont, MAKELPARAM(0, bRedraw));
				
				if (FALSE != bRedraw)
				{
					SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
							SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | 
							SWP_FRAMECHANGED | SWP_NOREDRAW);
				}

				EnableRedraw(SWR_NONE);
			}
		}
	}
	
	if (bNotifyChildren) 
		EnumChildWindows(hwnd, SkinChangedNotifyCB, bRedraw);

	SendMessageW(hwnd, (UINT)WM_ML_IPC, 
				MAKEWPARAM(bNotifyChildren, bRedraw), 
				(LPARAM)ML_IPC_SKINNEDWND_SKINUPDATED); 

	if (bRedraw) 
		InvalidateRect(hwnd, NULL, TRUE);
}

void SkinnedWnd::OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw)
{
}

void SkinnedWnd::SkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	OnSkinChanged(bNotifyChildren, bRedraw);
}

void SkinnedWnd::EnableReflection(BOOL bEnable)
{
	wnddata = (wnddata & ~SWS_REFLECT) | ((bEnable) ? SWS_REFLECT : 0);
}

BOOL SkinnedWnd::SetStyle(UINT newStyle, BOOL bRedraw)
{ 
	style = newStyle;

	if (NULL != hwnd) 
		SkinChanged(FALSE, bRedraw);
	
	return TRUE; 
}

void SkinnedWnd::SetMinMaxInfo(MLSKINNEDMINMAXINFO *minMax)
{
	if (NULL != minMax)
	{
		minSize.cx = minMax->min.cx;
		if (minSize.cx < 0)
			minSize.cx = 0;

		minSize.cy = minMax->min.cy;
		if (minSize.cy < 0)
			minSize.cy = 0;

		maxSize.cx = minMax->max.cx;
		if (maxSize.cx < 0)
			maxSize.cx = 0;

		maxSize.cy = minMax->max.cy;
		if (maxSize.cy < 0)
			maxSize.cy = 0;
	}
	else
	{
		memset(&minSize, 0, sizeof(minSize));
		memset(&maxSize, 0, sizeof(maxSize));
	}
}


BOOL SkinnedWnd::OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult)
{
	switch(msg)
	{
		case ML_IPC_SKINNEDWND_ISSKINNED: *pResult = IsAttached(); return TRUE;
		case ML_IPC_SKINNEDWND_SKINCHANGED:	SkinChanged(LOWORD(param), HIWORD(param)); *pResult = 1; return TRUE;
		case ML_IPC_SKINNEDWND_SKINUPDATED:	OnSkinUpdated(LOWORD(param), HIWORD(param)); break;
		case ML_IPC_SKINNEDWND_GETTYPE: *pResult = GetType(); return TRUE;
		case ML_IPC_SKINNEDWND_ENABLEREFLECTION: EnableReflection((BOOL)param); *pResult = 1; return TRUE;
		case ML_IPC_SKINNEDWND_GETPREVWNDPROC:
			if (param) *((BOOL*)param) = (SWS_UNICODE & wnddata);
			*pResult = (INT_PTR)fnWndProc;
			return TRUE;
		case ML_IPC_SKINNEDWND_SETSTYLE: *pResult = style;  style = (UINT)param; return TRUE;
		case ML_IPC_SKINNEDWND_GETSTYLE: *pResult = style; return TRUE;
		case ML_IPC_SKINNEDWND_SETMINMAXINFO: 
			SetMinMaxInfo((MLSKINNEDMINMAXINFO*)param); 
			*pResult = TRUE;
			return TRUE;
	}
	return FALSE;
}


INT SkinnedWnd::OnNcHitTest(POINTS pts)
{
	return (INT)CallPrevWndProc(WM_NCHITTEST, 0, *(LPARAM*)&pts);
}

void SkinnedWnd::DrawBorder(HDC hdc, RECT *prc, UINT type, HPEN pen)
{
	HPEN penOld;
	INT o = (BORDER_WIDTH/2) + ((BORDER_WIDTH%2) ? 1 : 0);

	switch(type)
	{
		case BORDER_SUNKEN:
		case BORDER_FLAT:
			penOld = (HPEN)SelectObject(hdc, pen);

			MoveToEx(hdc, prc->right - o, prc->top, NULL);
			LineTo(hdc, prc->right - o, prc->bottom);
			MoveToEx(hdc, prc->right - BORDER_WIDTH, prc->bottom - o, NULL);
			LineTo(hdc, prc->left - 1, prc->bottom - o);
			
			if (BORDER_FLAT == type)
			{
				MoveToEx(hdc, prc->left + BORDER_WIDTH/2, prc->bottom - BORDER_WIDTH, NULL);
				LineTo(hdc, prc->left + BORDER_WIDTH/2, prc->top);
				MoveToEx(hdc, prc->left, prc->top + BORDER_WIDTH/2, NULL);
				LineTo(hdc, prc->right - BORDER_WIDTH, prc->top + BORDER_WIDTH/2);
			}
			SelectObject(hdc, penOld);
			break;
	}
}

UINT SkinnedWnd::GetBorderType(void)
{
	DWORD ws = (DWORD)GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
	if (WS_EX_STATICEDGE & ws) return BORDER_FLAT;
	else if (WS_EX_CLIENTEDGE & ws) return BORDER_SUNKEN;
	ws = (DWORD)GetWindowLongPtrW(hwnd, GWL_STYLE);
	return (WS_BORDER & ws) ? BORDER_FLAT : BORDER_NONE;
}

void SkinnedWnd::DrawBorder(HDC hdc)
{
	UINT borderType = GetBorderType();

	if (BORDER_NONE != borderType)
	{		
		RECT rc;
		GetWindowRect(hwnd, &rc);
		OffsetRect(&rc, -rc.left, -rc.top);
		DrawBorder(hdc, &rc, borderType, GetBorderPen());
	}
}

void SkinnedWnd::OnNcPaint(HRGN rgnUpdate)
{
	UINT borderType = GetBorderType();
	if (BORDER_NONE == borderType) return;

	UINT flags = DCX_PARENTCLIP | DCX_CACHE | DCX_WINDOW | DCX_CLIPSIBLINGS |
				DCX_INTERSECTUPDATE | DCX_VALIDATE;

	HDC hdc = GetDCEx(hwnd, ((HRGN)NULLREGION != rgnUpdate) ? rgnUpdate : NULL, flags);

	if (NULL == hdc) return;
	
	DrawBorder(hdc);
	ReleaseDC(hwnd, hdc);
}


INT SkinnedWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *pncsp)
{	
	UINT borderType = GetBorderType();
	switch(borderType)
	{
		case BORDER_SUNKEN:
		case BORDER_FLAT:
			if (bCalcValidRects)
			{
				SetRect(&pncsp->rgrc[0], 
						pncsp->lppos->x, pncsp->lppos->y, 
						pncsp->lppos->x + pncsp->lppos->cx - BORDER_WIDTH,  pncsp->lppos->y + pncsp->lppos->cy - BORDER_WIDTH);
			}
			else
			{
				GetWindowRect(hwnd, &pncsp->rgrc[0]);
				pncsp->rgrc[0].right -= BORDER_WIDTH;
				pncsp->rgrc[0].bottom -= BORDER_WIDTH;

			}
			if (BORDER_FLAT == borderType) 
			{ 
				pncsp->rgrc[0].left += BORDER_WIDTH; 
				pncsp->rgrc[0].top += BORDER_WIDTH;
			}
			break;
	}
	
	return 0;
}

void SkinnedWnd::OnPrint(HDC hdc, UINT options)
{
	if ((PRF_CHECKVISIBLE & options) && !IsWindowVisible(hwnd)) return;
	if (PRF_NONCLIENT & options) DrawBorder(hdc);
	if (PRF_CLIENT & options) 
	{
		CallPrevWndProc(WM_PRINT, (WPARAM)hdc, (LPARAM)(~(PRF_NONCLIENT | PRF_CHECKVISIBLE) & options));
	}
}

HPEN SkinnedWnd::GetBorderPen(void)
{
	return (HPEN)MlStockObjects_Get(HILITE_PEN);
}


void SkinnedWnd::OnUpdateUIState(UINT uAction, UINT uState)
{
	CallPrevWndProc(WM_UPDATEUISTATE, MAKEWPARAM(uAction, uState), 0L);
	uiState =(WORD)SendMessageW(hwnd, WM_QUERYUISTATE, 0, 0L);
}

void SkinnedWnd::OnStyleChanged(INT styleType, STYLESTRUCT *pss)
{
	if (0 != redrawLock)
	{
		if (0 != (GWL_STYLE & styleType) && 
			(WS_VISIBLE & pss->styleOld) != (WS_VISIBLE & pss->styleNew))
		{
			redrawLock = 0; 
		}
	}

	CallPrevWndProc(WM_STYLECHANGED, (WPARAM)styleType, (LPARAM)pss);
}

void SkinnedWnd::OnDwmCompositionChanged(void)
{
	dwmCompositionEnabled = DWM_COMPOSITION_CHECK;
}

void SkinnedWnd::DisableRedraw()
{	
	if (0 == redrawLock)
	{
		UINT windowStyle = (UINT)GetWindowLongPtrW(hwnd, GWL_STYLE);
		if (0 == (WS_VISIBLE & windowStyle))
			return;

		CallDefWndProc(WM_SETREDRAW, FALSE, 0L);
	}
	
	redrawLock++;
}

void SkinnedWnd::EnableRedraw(SkinnedWndRedraw redrawFlags)
{
	UINT rdwFlags(0);

	if (0 == redrawLock)
		return;
	
	redrawLock--;
	if (0 != redrawLock)
		return;
		
	CallDefWndProc(WM_SETREDRAW, TRUE, 0L);

	if (0 != (SWR_INVALIDATE & redrawFlags))
	{
		rdwFlags |= RDW_INVALIDATE;
		if (0 != (SWR_UPDATE & redrawFlags))
			rdwFlags |= RDW_UPDATENOW;
	}

	if (0 != (SWR_ERASE & redrawFlags))
	{
		rdwFlags |= RDW_ERASE;
		if (0 != (SWR_UPDATE & redrawFlags))
			rdwFlags |= RDW_ERASENOW;
	}

	if (0 != rdwFlags)
	{
		if (0 != (SWR_ALLCHILDREN & redrawFlags))
			rdwFlags |= RDW_ALLCHILDREN;

		RedrawWindow(hwnd, NULL, NULL, rdwFlags);
	}
	
}

BOOL SkinnedWnd::OnDirectMouseWheel(INT delta, UINT vtKey, POINTS pts)
{
	SendMessageW(hwnd, WM_MOUSEWHEEL, MAKEWPARAM(vtKey, delta), *((LPARAM*)&pts));
	return TRUE;
}

void SkinnedWnd::OnGetMinMaxInfo(MINMAXINFO *minMax)
{
	if (NULL == minMax)
		return;

	CallPrevWndProc(WM_GETMINMAXINFO, 0, (LPARAM)minMax);

	if (0 != minSize.cx)
		minMax->ptMinTrackSize.x = minSize.cx;
	
	if (0 != minSize.cy)
		minMax->ptMinTrackSize.y = minSize.cy;

	if (0 != maxSize.cx)
		minMax->ptMaxTrackSize.x = maxSize.cx;

	if (0 != maxSize.cy)
		minMax->ptMaxTrackSize.y = maxSize.cy;
}

LRESULT SkinnedWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch(uMsg)
	{
		case WM_NCHITTEST:	return OnNcHitTest(MAKEPOINTS(lParam));
		case WM_NCPAINT:		OnNcPaint((HRGN)wParam); return 0;
		case WM_NCCALCSIZE:	return OnNcCalcSize((BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam);
		case WM_PRINT:		OnPrint((HDC)wParam, (UINT)lParam); return 0;
		case WM_UPDATEUISTATE: OnUpdateUIState(LOWORD(wParam), HIWORD(wParam)); return 0;
		case WM_STYLECHANGED:	OnStyleChanged((INT)wParam, (STYLESTRUCT*)lParam); return 0;
		case WM_SUPPORTREFLECT: 
			{
				BOOL reflectionSupported = (0 != (SWS_REFLECT & wnddata) && 	CanReflect((UINT)wParam));
				if (0 != (SWS_DIALOG & wnddata))
				{
					SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, reflectionSupported);
					return TRUE;
				}
				return reflectionSupported;
			}
		case WM_ML_IPC:	
			{
				LRESULT result;
				if (OnMediaLibraryIPC((INT)lParam, (INT_PTR)wParam, &result))
				{
					if (SWS_DIALOG & wnddata) 
					{
						SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (LONGX86)result);
						return TRUE;
					}
					return result;
				}
				break;
			}

		case WM_DWMCOMPOSITIONCHANGED:	OnDwmCompositionChanged(); return 0;
		case WM_GETMINMAXINFO:			OnGetMinMaxInfo((MINMAXINFO*)lParam); return 0;

	}

	// Reflection
	if (0 != (SWS_REFLECT & wnddata))
	{
		LRESULT result;
		if (ReflectMessage(hwnd, uMsg, wParam, lParam, (0 != (SWS_DIALOG & wnddata)), &result))
			return result;
	}

	if ( 0 == (SWS_NO_DIRECT_MOUSE_WHEEL & style) && 
		 WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg && 
		 WM_NULL != WINAMP_WM_DIRECT_MOUSE_WHEEL && 
		 FALSE != OnDirectMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), GET_KEYSTATE_WPARAM(wParam), MAKEPOINTS(lParam)))
	{
		if (0 != (SWS_DIALOG & wnddata))
			SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, TRUE);
		return TRUE;
	}

	return CallPrevWndProc(uMsg, wParam, lParam);
}

LRESULT WINAPI SkinnedWnd::WindowProcReal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SkinnedWnd *pWnd = (SkinnedWnd*)GetPropW(hwnd, WNDDATAPROPW);
	if (!pWnd)
		return (IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : DefWindowProcA(hwnd, uMsg, wParam, lParam);
	
	switch(uMsg)
	{
		case WM_NCDESTROY:
			{
				WNDPROC fnWndProc = pWnd->fnWndProc;
				delete(pWnd);
				
				return IsWindowUnicode(hwnd) ? CallWindowProcW(fnWndProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(fnWndProc, hwnd, uMsg, wParam, lParam);
			}
			break;
	}
	return pWnd->WindowProc(uMsg, wParam, lParam);
}