#include <windowsx.h>

#include "main.h"
#include "./api.h"
#include "./wa_dlg.h"
#include "./skinWindow.h"
#include "./skinWindowIPC.h"
#include "./wintheme.h"
#include "./draw.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(blah) (sizeof(blah)/sizeof(*blah))
#endif

#ifndef OCR_NORMAL
#define OCR_NORMAL          32512
#endif

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDWAIPC(__ipcMsgId, __param) SENDMSG(hMainWindow, WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#ifndef WM_NCUAHDRAWCAPTION
#define WM_NCUAHDRAWCAPTION		0x00AE
#define WM_NCUAHDRAWFRAME		0x00AF
#endif //WM_NCUAHDRAWCAPTION

// additional WM_SYSCOMMAND commands
#define SC_DRAGMOVE		0xF012
#define SC_DRAGSIZE_N	0xF003
#define SC_DRAGSIZE_S	0xF006
#define SC_DRAGSIZE_E	0xF002
#define SC_DRAGSIZE_W	0xF001
#define SC_DRAGSIZE_NW	0xF004
#define SC_DRAGSIZE_NE	0xF005
#define SC_DRAGSIZE_SW	0xF007
#define SC_DRAGSIZE_SE	0xF008

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWL_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }
#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#define RESIZESTEP_CX		25
#define RESIZESTEP_CY		29

static ATOM SKINNEDWND_ATOM = 0;

// this flags for iternal use
#define SWS_EX_MASK			0xFFFF0000
#define SWS_EX_ATTACHED		0x00010000		// window attached
#define SWS_EX_UNICODE		0x00020000		// winodow is unicode
#define SWS_EX_THEMED		0x00040000		// was themed before 
#define SWS_EX_DIALOG		0x00100000		// treat this as dialog

#define BORDERWIDTH_LEFT		11
#define BORDERWIDTH_TOP			20
#define BORDERWIDTH_RIGHT		8
#define BORDERWIDTH_BOTTOM		14

#define CHILDBORDER_LEFT		1
#define CHILDBORDER_TOP			1
#define CHILDBORDER_RIGHT		1
#define CHILDBORDER_BOTTOM		1

#define CLOSEBUTTON_HEIGHT		9
#define CLOSEBUTTON_WIDTH		9
#define CLOSEBUTTON_OFFSET_X	-11
#define CLOSEBUTTON_OFFSET_Y	3

#define SIZERGRIP_WIDTH			20
#define SIZERGRIP_HEIGHT		20

#define MOVEABLEAREA_HEIGHT		13

#define BUTTON_NORMAL		0
#define BUTTON_PUSHED		1
#define BUTTON_DISABLED		(-1)

#ifndef TME_NONCLIENT
#define TME_NONCLIENT   0x00000010
#define WM_NCMOUSELEAVE	0x02A2
#endif

#define WAMSG_CLOSE		(WM_USER + 101)

typedef struct __SKINNEDWND
{
	HWND			hwnd;
	WNDPROC		fnWndProc;
	DWORD		flags;
	POINT		movingOffset;
	INT			buttonState;
	embedWindowState embedData;
} SKINNEDWND;

#define GetSkinnedWnd(__hwnd) ((SKINNEDWND*)GetPropW((__hwnd), ((LPCWSTR)MAKEINTATOM(SKINNEDWND_ATOM))))
#define IsDialog(__skinnedWindow) (0 != (SWS_EX_DIALOG & (__skinnedWindow)->flags))
#define IsUnicode(__skinnedWindow) (0 != (SWS_EX_UNICODE & (__skinnedWindow)->flags))

#define SetWndLongPtr(__skinnedWnd, __index, __data)\
	((LONG_PTR)(IsUnicode(__skinnedWnd) ? SetWindowLongPtrW((__skinnedWnd)->hwnd, (__index), ((LONGX86)(LONG_PTR)(__data))) :\
	SetWindowLongPtrA((__skinnedWnd)->hwnd, (__index), ((LONGX86)(LONG_PTR)(__data)))))

#define CallWndProc(__skinnedWnd, __uMsg, __wParam, __lParam)\
	(IsUnicode(__skinnedWnd) ?\
		CallWindowProcW((__skinnedWnd)->fnWndProc, (__skinnedWnd)->hwnd, (__uMsg), (__wParam), (__lParam)) :\
		CallWindowProcA((__skinnedWnd)->fnWndProc, (__skinnedWnd)->hwnd, (__uMsg), (__wParam), (__lParam)))

#define DefWndProc(__skinnedWnd, __uMsg, __wParam, __lParam)\
	(IsUnicode(__skinnedWnd) ?\
		DefWindowProcW((__skinnedWnd)->hwnd, (__uMsg), (__wParam), (__lParam)) :\
		DefWindowProcA((__skinnedWnd)->hwnd, (__uMsg), (__wParam), (__lParam)))

void draw_embed_tbar(HWND hwnd, int state, int w);
void SnapWindowToAllWindows(RECT *outrc, HWND hwndNoSnap);

static LRESULT CALLBACK SkinnedWnd_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void SkinnedWindow_Delete(SKINNEDWND *pWnd)
{
	HWND hDetached = NULL;
    if (NULL == pWnd) return;

	hDetached = pWnd->hwnd;
	SetWndLongPtr(pWnd, GWLP_USERDATA, NULL);
	RemovePropW(pWnd->hwnd, ((LPCWSTR)MAKEINTATOM(SKINNEDWND_ATOM)));

	if (NULL != pWnd->embedData.me)
	{
		EnterCriticalSection(&embedcs);
		embedWindowState *p = embedwndlist;
		if (p == &pWnd->embedData)
		{
			embedwndlist=pWnd->embedData.link; // remove ourselves
			embedwndlist_cnt--;
		}
		else
		{
			while (p)
			{
				if (p->link == &pWnd->embedData)
				{
					p->link=pWnd->embedData.link;
					embedwndlist_cnt--;
					break;
				}
				p=p->link;
			}
		}
		LeaveCriticalSection(&embedcs);
	}
	
	if (NULL != pWnd->fnWndProc)
	{
		SetWndLongPtr(pWnd, GWLP_WNDPROC, pWnd->fnWndProc);
		pWnd->fnWndProc = NULL;
	}
	free(pWnd);

	if (NULL != hDetached)
	{
		SetWindowPos(hDetached, NULL, 0, 0, 0, 0, 
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

static SKINNEDWND *SkinnedWindow_Cerate(HWND hwndToSkin)
{
	if (NULL == hwndToSkin || !IsWindow(hwndToSkin))
		return NULL;
 
	if (NULL != GetSkinnedWnd(hwndToSkin))
		return NULL;

	if (0 == SKINNEDWND_ATOM)
	{
		 SKINNEDWND_ATOM = GlobalAddAtomW(L"WASKINNEDWND");
		 if (0 == SKINNEDWND_ATOM) 
			 return NULL;
	}

	SKINNEDWND *pWnd = (SKINNEDWND*)calloc(1, sizeof(SKINNEDWND));
	if (NULL == pWnd) 
		return NULL;

	pWnd->hwnd = hwndToSkin;
	if (IsWindowUnicode(hwndToSkin)) pWnd->flags |= SWS_EX_UNICODE;

	WCHAR szName[128] = {0};
	if (GetClassNameW(hwndToSkin, szName, ARRAYSIZE(szName)))
	{
		if (CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, szName, -1, L"#32770", -1))
			pWnd->flags |= SWS_EX_DIALOG;
	}

	pWnd->fnWndProc = (WNDPROC)SetWndLongPtr(pWnd, GWLP_WNDPROC, SkinnedWnd_WindowProc);
	if (NULL == pWnd->fnWndProc || !SetPropW(hwndToSkin, ((LPCWSTR)MAKEINTATOM(SKINNEDWND_ATOM)), pWnd))
	{
		SkinnedWindow_Delete(pWnd);
		return NULL;
	}
	pWnd->flags |= SWS_EX_ATTACHED;

	if (S_OK == Dwm_LoadLibrary())
	{
		DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;
		DWORD allow = FALSE;
		DwmSetWindowAttribute(pWnd->hwnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
		DwmSetWindowAttribute(pWnd->hwnd, DWMWA_ALLOW_NCPAINT, &allow, sizeof(allow));
	}

	if (S_OK == UxTheme_LoadLibrary() && IsAppThemed())
	{
		SetWindowTheme(pWnd->hwnd, NULL, L"");
		pWnd->flags |= SWS_EX_THEMED;
	}

	if (!config_embedwnd_freesize &&
		0 == (WS_CHILD & GetWindowLongPtrW(pWnd->hwnd, GWL_STYLE))) 
	{
		RECT rc;
		if (GetWindowRect(hwndToSkin, &rc))
		{		
			LONG w, h;
			OffsetRect(&rc, -rc.left, -rc.top);
			
			w = rc.right + (RESIZESTEP_CX - 1);
			w -= w % RESIZESTEP_CX;
			h = rc.bottom + (RESIZESTEP_CY - 1);
			h -= h % RESIZESTEP_CY;

			if (w != rc.right || h != rc.bottom)
				SetWindowPos(hwndToSkin, NULL, 0, 0, w, h, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		}
	}

	return pWnd;
}

BOOL SkinWindow(HWND hwndToSkin, REFGUID windowGuid, UINT flagsEx, FFCALLBACK callbackFF)
{
	SKINNEDWND *pWnd = SkinnedWindow_Cerate(hwndToSkin);

	if (NULL == pWnd)
		return FALSE;

	pWnd->embedData.me = pWnd->hwnd;
	pWnd->embedData.user_ptr = pWnd;
	pWnd->embedData.flags |= (EMBED_FLAGS_GUID | flagsEx);
    pWnd->embedData.guid = windowGuid;

	pWnd->embedData.callback = callbackFF;
	if (NULL != pWnd->embedData.callback)
		pWnd->embedData.flags |= EMBED_FLAGS_FFCALLBACK;

	GetWindowRect(pWnd->hwnd, &pWnd->embedData.r);

	EnterCriticalSection(&embedcs);

	pWnd->embedData.link = embedwndlist;
	embedwndlist = &pWnd->embedData;
	embedwndlist_cnt++;

	LeaveCriticalSection(&embedcs);

	SetWndLongPtr(pWnd, GWLP_USERDATA, &pWnd->embedData);

	return TRUE;
}

static int SkinnedWindow_GetTextWidth(LPCTSTR pszText, INT cchText)
{
	int w = 0;
	while (cchText--)
	{
		char c = *pszText++;
		if (c >= 'a' && c <= 'z') c+='A'-'a';

		if (c >= 'A' && c <= 'Z' && titlebar_font_widths[c-'A']) 
			w+=titlebar_font_widths[c-'A'];
		else if (c >= '0' && c <= '9' && titlebar_font_widths[c-'0'] && Skin_UseGenNums) 
			w+=titlebar_font_num_widths[c-'0'];
		else if (c == '-' && titlebar_font_widths[10] && Skin_UseGenNums) 
			w+=titlebar_font_num_widths[10];
		else if (c == ':' && titlebar_font_widths[11] && Skin_UseGenNums) 
			w+=titlebar_font_num_widths[11];
		else w+=titlebar_font_unknown_width;

	}
	return w;
}

static void SkinnedWindow_DrawText(HDC hdc, HDC hdcSrc, int xp, int yp, LPCTSTR pszText, INT cchText, RECT *prc, BOOL bActive)
{
	int w;
	int srcY = 88 + ((bActive) ? 8 : 0);
	int srcYnum = 72 + ((bActive) ? 8 : 0);
	int bgsrcY = ((bActive) ? 21 : 0);
	int bgX = prc->left;
	int maxw = prc->right;
	while (cchText-- && maxw > 0)
	{
		char c = *pszText++;

		if (bgX <= xp)
		{
			int bgW = 25;
			if (bgW + bgX > prc->right) bgW = prc->right - bgX;
			BitBlt(hdc, bgX, prc->top, bgW, (prc->bottom - prc->top), hdcSrc, 52, bgsrcY, SRCCOPY);
			bgX += bgW;
		}

		if (c >= 'a' && c <= 'z') c+='A'-'a';
		if (c >= 'A' && c <= 'Z' && titlebar_font_widths[c-'A']) 
		{
			w = titlebar_font_widths[c - 'A'];
			if (w > maxw) break;

			if (bgX <= (xp + w))
			{
				int bgW = 25;
				if (bgW + bgX > prc->right) bgW = prc->right - bgX;
				BitBlt(hdc, bgX, prc->top, bgW, (prc->bottom - prc->top), hdcSrc, 52, bgsrcY, SRCCOPY);
				bgX += bgW;
			}

			BitBlt(hdc, xp, yp, w, 7, hdcSrc, titlebar_font_offsets[c - 'A'], srcY, SRCCOPY);
		}
		else if (c >= '0' && c <= '9' && titlebar_font_num_widths[c - '0'] && Skin_UseGenNums)
		{
			w = titlebar_font_num_widths[c - '0'];
			if (w > maxw) break;

			if (bgX <= (xp + w))
			{
				int bgW = 25;
				if (bgW + bgX > prc->right) bgW = prc->right - bgX;
				BitBlt(hdc, bgX, prc->top, bgW, (prc->bottom - prc->top), hdcSrc, 52, bgsrcY, SRCCOPY);
				bgX += bgW;
			}

			BitBlt(hdc, xp, yp, w, 7, hdcSrc, titlebar_font_num_offsets[c - '0'], srcYnum, SRCCOPY);
		}
		else if (c == '-' && titlebar_font_num_widths[10] && Skin_UseGenNums)
		{
			w = titlebar_font_num_widths[10];
			if (w > maxw) break;

			if (bgX <= (xp + w))
			{
				int bgW = 25;
				if (bgW + bgX > prc->right) bgW = prc->right - bgX;
				BitBlt(hdc, bgX, prc->top, bgW, (prc->bottom - prc->top), hdcSrc, 52, bgsrcY, SRCCOPY);
				bgX += bgW;
			}

			BitBlt(hdc, xp, yp, w, 7, hdcSrc, titlebar_font_num_offsets[10], srcYnum, SRCCOPY);
		}
		else if (c == ':' && titlebar_font_num_widths[11] && Skin_UseGenNums)
		{
			w = titlebar_font_num_widths[11];
			if (w > maxw) break;

			if (bgX <= (xp + w))
			{
				int bgW = 25;
				if (bgW + bgX > prc->right) bgW = prc->right - bgX;
				BitBlt(hdc, bgX, prc->top, bgW, (prc->bottom - prc->top), hdcSrc, 52, bgsrcY, SRCCOPY);
				bgX += bgW;
			}

			BitBlt(hdc, xp, yp, w, 7, hdcSrc, titlebar_font_num_offsets[11], srcYnum, SRCCOPY);
		}
		else 
			w = titlebar_font_unknown_width;

		xp += w;
		maxw -= w;
	}
}

static void SkinnedWindow_DrawCloseButton(HDC hdc, HDC hdcSrc, RECT *prcWindow, BOOL bActive, INT buttonState)
{
	INT srcX, srcY;

	switch(buttonState)
	{
		case BUTTON_PUSHED:	
			srcX = 148; srcY = 42; 
			break;
		case BUTTON_NORMAL:
			srcX = 144; srcY = 3;
			if (!bActive) srcY += 21;
			break;
		case BUTTON_DISABLED:
		default:
			srcX = 144; srcY = 3; 
			break;
	}
	BitBlt(hdc, prcWindow->right - 11,prcWindow->top + 3, 9, 9, hdcSrc,srcX, srcY, SRCCOPY);
}

static void SkinnedWindow_DrawCaptionEx(HDC hdc, HDC hdcSrc, LPCTSTR pszText, INT cchText, int state, int w, int h)
{    
	state = state ? 0 : 21;
    int nt;
	int xp=0;
	int textw_exact = 0, textw = 0;
	int cchTextOrig = cchText;
	if (cchText > 0)
	{
		for(;cchText > 0; cchText--)
		{
			textw_exact = SkinnedWindow_GetTextWidth(pszText, cchText);
			textw = textw_exact + 24;
			textw -= textw % 25;
			if ((w - textw) > 100) break;
			textw = 0;
		} 
	}

	BitBlt(hdc,xp,0,25,20, hdcSrc, 0, state,SRCCOPY);
	xp+=25;
	nt = (w - 100 - textw)/25;
	if (nt > 0)
	{
		if (nt&1)
		{
			BitBlt(hdc,xp,0,12,20,hdcSrc,104,state,SRCCOPY);
			xp+=12;
		}
		nt/=2;
		while (nt-->0)
		{
			BitBlt(hdc,xp,0,25,20,hdcSrc,104,state,SRCCOPY);
			xp+=25;
		}
	}

	if (cchText > 0)
	{
		BitBlt(hdc,xp,0,25,20,hdcSrc,26,state,SRCCOPY);
		xp+=25;
		nt = textw/25;
		if (nt > 0)
		{
			
			RECT rt;
			SetRect(&rt, xp, 0, xp + textw, 20);
			
			SkinnedWindow_DrawText(hdc, hdcSrc, 
					rt.left + ((cchText == cchTextOrig) ? (textw - textw_exact)/2 : 0), 4, 
					pszText, cchText, &rt, state);
			xp += nt*25;
		}
	
		BitBlt(hdc,xp,0,25,20,hdcSrc,78,state,SRCCOPY);
		xp+=25;
	}
	else
	{	
		nt = (w - 50)/2;
		if (nt > 25) nt = 25;
		if (nt > 0)
		{
			BitBlt(hdc,xp, 0, nt,20,hdcSrc,104,state,SRCCOPY);
			xp+=nt;
			if (nt < 25 && 0 != (w - 50)%2)nt++;
			BitBlt(hdc,xp, 0, nt, 20, hdcSrc,104 + (25- nt),state,SRCCOPY);
			xp+=nt;
		}
	}

	nt = (w - 100 - textw)/25;
	if (nt > 0)
	{
		if (nt&1)
		{
			BitBlt(hdc,xp,0,13,20,hdcSrc,104,state,SRCCOPY);
			xp+=13;
		}
		nt/=2;
		while (nt-->0)
		{
			BitBlt(hdc,xp,0,25,20,hdcSrc,104,state,SRCCOPY);
			xp+=25;
		}
	}
    nt = (w - 100 - textw) % 25;
    if (nt > 0)
    {			
		BitBlt(hdc,xp,0,nt,20,hdcSrc,104,state,SRCCOPY);
		//StretchBlt(hdc,xp,0,nt,20,hdcSrc,104,state,25,20,SRCCOPY);
		xp += nt;
    }
	BitBlt(hdc,xp,0,25,20,hdcSrc,130,state,SRCCOPY);
}

static void SkinnedWindow_DrawCaption(SKINNEDWND *pWnd, BOOL bActive, POINT *cursor)
{
	RECT rc;
	GetWindowRect(pWnd->hwnd, &rc);
	OffsetRect(&rc, -rc.left, -rc.top);


	UINT flags = DCX_PARENTCLIP | DCX_CACHE | DCX_WINDOW | DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN |
							DCX_INTERSECTUPDATE | DCX_VALIDATE | DCX_NORESETATTRS;
	HDC hdc = GetDCEx(pWnd->hwnd, NULL, flags);
	if (NULL == hdc) 
		return;

	do_palmode(hdc);
	setSrcBM(embedBM);

	char szTitle[128] = {0};
	INT cchTitle = GetWindowTextA(pWnd->hwnd, szTitle, ARRAYSIZE(szTitle));
	
	INT state = pWnd->buttonState;
	if (BUTTON_PUSHED == state)
	{
		if (NULL == cursor || HTCLOSE != SendMessageW(pWnd->hwnd, WM_NCHITTEST, 0, MAKELPARAM(cursor->x, cursor->y)))
			state = BUTTON_NORMAL;
	}
	
	SkinnedWindow_DrawCaptionEx(hdc, bmDC, (LPCTSTR)szTitle, cchTitle, bActive ? 1 : (config_hilite?0:1), 
		rc.right - rc.left, rc.bottom - rc.top);
	SkinnedWindow_DrawCloseButton(hdc, bmDC, &rc, bActive, state);
	
	unsetSrcBM();
	ReleaseDC(pWnd->hwnd, hdc);

}

static void SkinnedWindow_DrawBorder(SKINNEDWND *pWnd, HDC hdc, HRGN rgnUpdate, POINT *cursor)
{
	
	DWORD style = GetWindowLongPtrW(pWnd->hwnd, GWL_STYLE);
	if (0 == (WS_BORDER & style))
		return;

	RECT rc;
	GetWindowRect(pWnd->hwnd, &rc);
	OffsetRect(&rc, -rc.left, -rc.top);
	LONG w = rc.right, h = rc.bottom;

	do_palmode(hdc);
	setSrcBM(embedBM);

	if (0 != (WS_CHILD & style))
	{
		if (!WADlg_initted())
			WADlg_init(hMainWindow);
		COLORREF rgbOld = SetBkColor(hdc, WADlg_getColor(WADLG_HILITE));
		
		RECT part;
		SetRect(&part, rc.left, rc.top, rc.right, rc.top + CHILDBORDER_TOP);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &part, NULL, 0, NULL);
		SetRect(&part, rc.right - CHILDBORDER_RIGHT, rc.top + CHILDBORDER_TOP, rc.right, rc.bottom - CHILDBORDER_BOTTOM);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &part, NULL, 0, NULL);
		SetRect(&part, rc.left, rc.bottom - CHILDBORDER_BOTTOM, rc.right, rc.bottom);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &part, NULL, 0, NULL);
		SetRect(&part, rc.left, rc.top  + CHILDBORDER_TOP, rc.left + CHILDBORDER_LEFT, rc.bottom - CHILDBORDER_BOTTOM);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &part, NULL, 0, NULL);
		
		SetBkColor(hdc, rgbOld);
		return;
	}

	if (0 != (WS_CAPTION & style))
	{
		char szTitle[128] = {0};
		INT cchTitle = GetWindowTextA(pWnd->hwnd, szTitle, ARRAYSIZE(szTitle));

		INT state = pWnd->buttonState;
		if (BUTTON_PUSHED == state)
		{
			if (NULL == cursor || HTCLOSE != SendMessageW(pWnd->hwnd, WM_NCHITTEST, 0, MAKELPARAM(cursor->x, cursor->y)))
				state = BUTTON_NORMAL;
		}

		BOOL bActive = (GetActiveWindow()==pWnd->hwnd) ? 1 : ((0 != config_hilite) ? 0 : 1);
		SkinnedWindow_DrawCaptionEx(hdc, bmDC, (LPCTSTR)szTitle, cchTitle, bActive, w, h);
		if (BUTTON_NORMAL != state)
			SkinnedWindow_DrawCloseButton(hdc, bmDC, &rc, bActive, state);
	}

	int y=(h-20-38)/29;
	int yp=20,x,xp;
	while (y-->0)
	{
		BitBlt(hdc,0,yp,11,29,bmDC,127,42,SRCCOPY);
		BitBlt(hdc,w-8,yp,8,29,bmDC,139,42,SRCCOPY);
		yp += 29;
	}
	y=(h-20-38)%29;
    if (y)
    {
			BitBlt(hdc,0,yp,11,y,bmDC,127,42,SRCCOPY);
			//StretchBlt(hdc,0,yp,11,y,bmDC,127,42,11,29,SRCCOPY);
			BitBlt(hdc,w-8,yp,8,y,bmDC,139,42,SRCCOPY);
			//StretchBlt(hdc,w-8,yp,8,y,bmDC,139,42,8,29,SRCCOPY);
			yp += y;
    }

    // 24 pixel lamity
	BitBlt(hdc,0,yp,11,24,bmDC,158,42,SRCCOPY);
	BitBlt(hdc,w-8,yp,8,24,bmDC,170,42,SRCCOPY);
    yp += 24;
	
	int realW = (w < 250) ? (w/2) : 125;
	BitBlt(hdc,0,yp,realW,14,bmDC,0,42,SRCCOPY);
	
	x=(w-125*2)/25;
	xp=realW;
	while (x-->0)
	{
		BitBlt(hdc,xp,yp,25,14,bmDC,127,72,SRCCOPY);
		xp+=25;
	}
	x=(w-125*2)%25;
    if (x > 0)
    {
		BitBlt(hdc,xp,yp,x,14,bmDC,127,72,SRCCOPY);
		//StretchBlt(hdc,xp,yp,x,14,bmDC,127,72,25,14,SRCCOPY);
		xp+=x;
    }

	if (realW < 125 && 0 != (w%2)) realW++;
	BitBlt(hdc,xp,yp,realW,14,bmDC, (125 - realW),57,SRCCOPY);
      
	if (0 != (EMBED_FLAGS_NORESIZE & pWnd->embedData.flags))
    {		
		BitBlt(hdc,xp+112,yp+2,7,7,bmDC,118,72,SRCCOPY);
    }
	  
	unsetSrcBM();
}

static BOOL SkinnedWindow_OnShowWindow(SKINNEDWND *pWnd, BOOL bShow, UINT flags)
{
	if (pWnd->embedData.reparenting)
		return TRUE;

	SENDWAIPC(((bShow) ? IPC_CB_ONSHOWWND : IPC_CB_ONHIDEWND), pWnd->hwnd);
	return FALSE;
}

static LRESULT SkinnedWindow_OnNcCalcSize(SKINNEDWND *pWnd, BOOL bCalcValidRects, NCCALCSIZE_PARAMS *pncsp)
{		

	DWORD style = (DWORD)GetWindowLongPtrW(pWnd->hwnd, GWL_STYLE);

	RECT rc;
	CopyRect(&rc, &pncsp->rgrc[0]);
	CallWndProc(pWnd, WM_NCCALCSIZE, bCalcValidRects, (LPARAM)pncsp);
	CopyRect(&pncsp->rgrc[0], &rc);

	if (bCalcValidRects)
		SetRect(&pncsp->rgrc[0], pncsp->lppos->x, pncsp->lppos->y, 
			pncsp->lppos->x + pncsp->lppos->cx, pncsp->lppos->y + pncsp->lppos->cy);
	else
	{
		GetWindowRect(pWnd->hwnd, &pncsp->rgrc[0]);
		if (0 != (WS_CHILD & style))
		{
			HWND hParent = GetParent(pWnd->hwnd);
			if (NULL != hParent)
				MapWindowPoints(HWND_DESKTOP, hParent, (POINT*)&pncsp->rgrc[0], 2);
		}
	}

	if (0 != (WS_BORDER & style))
	{
		if (0 != (WS_CHILD & style))
		{
			pncsp->rgrc[0].top += CHILDBORDER_TOP;
			pncsp->rgrc[0].left += CHILDBORDER_LEFT;
			pncsp->rgrc[0].right -= CHILDBORDER_RIGHT;
			pncsp->rgrc[0].bottom -= CHILDBORDER_BOTTOM;
		}
		else
		{
			pncsp->rgrc[0].top += BORDERWIDTH_TOP;
			pncsp->rgrc[0].left += BORDERWIDTH_LEFT;
			pncsp->rgrc[0].right -= BORDERWIDTH_RIGHT;
			pncsp->rgrc[0].bottom -= BORDERWIDTH_BOTTOM;
		}
	}

	return 0;
}

static void SkinnedWindow_OnNcPaint(SKINNEDWND *pWnd, HRGN rgnUpdate)
{	
	if (0 == (WS_BORDER & GetWindowLongPtrW(pWnd->hwnd, GWL_STYLE)))
		return;

	UINT flags = DCX_PARENTCLIP | DCX_CACHE | DCX_WINDOW | DCX_CLIPSIBLINGS |
				DCX_INTERSECTUPDATE | DCX_VALIDATE | DCX_NORESETATTRS;

	HDC hdc = GetDCEx(pWnd->hwnd, ((HRGN)NULLREGION != rgnUpdate) ? rgnUpdate : NULL, flags);
	if (NULL == hdc)
		return;
	POINT pt;
	GetCursorPos(&pt);
	SkinnedWindow_DrawBorder(pWnd, hdc, rgnUpdate, &pt);
	ReleaseDC(pWnd->hwnd, hdc);
}

static void SkinnedWindow_OnPrint(SKINNEDWND *pWnd, HDC hdc, UINT options)
{
	if ((PRF_CHECKVISIBLE & options) && !IsWindowVisible(pWnd->hwnd)) return;
	if (PRF_NONCLIENT & options) 
	{
		POINT pt = {0, 0};
		SkinnedWindow_DrawBorder(pWnd, hdc, (HRGN)NULLREGION, &pt);
	}
	if (PRF_CLIENT & options) 
		CallWndProc(pWnd, WM_PRINT, (WPARAM)hdc, (LPARAM)(~(PRF_NONCLIENT | PRF_CHECKVISIBLE) & options));
}

static LRESULT SkinnedWindow_OnNcHitTest(SKINNEDWND *pWnd, POINTS pts)
{
	POINT pt;
	RECT rw, rt;
	POINTSTOPOINT(pt, pts);

	DWORD style = (DWORD)GetWindowLongPtrW(pWnd->hwnd, GWL_STYLE);

	if (0 != (WS_DISABLED & style))
		return HTERROR;

	if (0 != (WS_CHILD & style))
	{
		if (0 != (WS_BORDER & style))
		{
			GetWindowRect(pWnd->hwnd, &rw);
			rw.left += CHILDBORDER_LEFT;
			rw.top += CHILDBORDER_TOP;
			rw.right -= CHILDBORDER_RIGHT;
			rw.bottom -= CHILDBORDER_BOTTOM;
			if (PtInRect(&rt, pt)) 
				return HTBORDER;
		}
		return HTCLIENT;
	}

	GetWindowRect(pWnd->hwnd, &rw);

	if (0 != (WS_CAPTION & style))
	{
		SetRect(&rt, rw.left, rw.top, rw.right, rw.top + BORDERWIDTH_TOP);
		if (PtInRect(&rt, pt)) // caption
		{
			SetRect(&rt, rw.right + CLOSEBUTTON_OFFSET_X, rw.top + CLOSEBUTTON_OFFSET_Y, 
				rw.right + CLOSEBUTTON_OFFSET_X + CLOSEBUTTON_WIDTH, rw.top + CLOSEBUTTON_OFFSET_Y + CLOSEBUTTON_HEIGHT);
			if (PtInRect(&rt, pt)) // close button
				return HTCLOSE;
			return HTCAPTION;
		}
	}

	if (0 != (WS_BORDER & style))
	{
		SetRect(&rt, rw.left, rw.top + BORDERWIDTH_TOP, rw.left + BORDERWIDTH_LEFT, rw.bottom - BORDERWIDTH_BOTTOM);
		if (PtInRect(&rt, pt)) 
			return HTBORDER; // left side (non resizable)

		SetRect(&rt, rw.right - BORDERWIDTH_RIGHT, rw.top + BORDERWIDTH_TOP, rw.right, rw.bottom - SIZERGRIP_HEIGHT);
		if (PtInRect(&rt, pt)) 
			return HTBORDER; // right side  (non resizable)

		SetRect(&rt, rw.right - BORDERWIDTH_RIGHT, rw.bottom - SIZERGRIP_HEIGHT, rw.right, rw.bottom);
		if (PtInRect(&rt, pt)) 
			return (0 == (EMBED_FLAGS_NORESIZE & pWnd->embedData.flags)) ? HTBOTTOMRIGHT : HTBORDER; // sizer bottomright

		SetRect(&rt, rw.left, rw.bottom - BORDERWIDTH_BOTTOM, rw.right -SIZERGRIP_WIDTH, rw.bottom);
		if (PtInRect(&rt, pt)) 
			return HTBORDER; // bottom_left + bottom (non resizable) 

		SetRect(&rt, rw.right - SIZERGRIP_WIDTH, rw.bottom - BORDERWIDTH_BOTTOM, rw.right, rw.bottom);
		if (PtInRect(&rt, pt)) 
			return (0 == (EMBED_FLAGS_NORESIZE & pWnd->embedData.flags)) ? HTBOTTOMRIGHT : HTBORDER; // sizer bottomright
	}

	SetRect(&rt, rw.left, rw.top, rw.right, rw.bottom);
	if (PtInRect(&rt, pt)) 
		return HTCLIENT;	// client

	return HTNOWHERE;
}

static LRESULT SkinnedWindow_OnSetCursor(SKINNEDWND *pWnd, HWND hwndCursor, INT hitTest, UINT uMsg)
{
	HCURSOR hCursor = NULL;

	switch(uMsg)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			DisabledWindow_OnMouseClick(pWnd->hwnd);
			break;
	}

	if (config_usecursors && !disable_skin_cursors)
	{
		int index = 15+5; // PNormal.cur
		switch(hitTest)
		{
			case HTCAPTION:  
				{ // this is required to emulate old behavior
					POINT pt;
					RECT rw;
					GetCursorPos(&pt);
					GetWindowRect(pWnd->hwnd, &rw);
					rw.bottom = rw.top + MOVEABLEAREA_HEIGHT;
					index = 15 + ((PtInRect(&rw, pt)) ? 2 : 5);  // PTBar.cur
				}
				break; 
			case HTCLOSE:
				index = 15 + 1; // PClose.cur
				break; 
			case HTLEFT:
			case HTRIGHT:
			case HTTOP:
			case HTTOPLEFT:
			case HTTOPRIGHT:
			case HTBOTTOM:
			case HTBOTTOMLEFT:
			case HTBOTTOMRIGHT:
				index = 15 + 4;// PSize.cur
				break; 
		}
		hCursor = Skin_Cursors[index];
	}

	if (NULL != hCursor) 
	{
		SetCursor(hCursor);
		return TRUE;
	}
	return CallWndProc(pWnd, WM_SETCURSOR, (WPARAM)hwndCursor, MAKELPARAM(hitTest, uMsg));
}

static LRESULT SkinnedWindow_OnNcActivate(SKINNEDWND *pWnd, BOOL bActivate)
{
	if (0 == (WS_CAPTION & GetWindowLongPtrW(pWnd->hwnd, GWL_STYLE)))
		return TRUE;

	POINT pt;
	GetCursorPos(&pt);
	SkinnedWindow_DrawCaption(pWnd, bActivate, &pt);
	return TRUE;
}

static void SkinnedWindow_OnActivate(SKINNEDWND *pWnd, unsigned int action, BOOL minimized, HWND otherWindow)
{
	if (NULL == pWnd)
		return;

	if (NULL != WASABI_API_APP)
	{
		if (WA_INACTIVE == action)
			WASABI_API_APP->ActiveDialog_Unregister(pWnd->hwnd);
		else
			WASABI_API_APP->ActiveDialog_Register(pWnd->hwnd);
	}
}

static BOOL SkinnedWindow_OnNcLButtonDown(SKINNEDWND *pWnd, INT hitTest, POINTS pts)
{
	switch(hitTest)
	{
		case HTCLOSE:
			{	
				pWnd->buttonState = BUTTON_PUSHED;

				TRACKMOUSEEVENT tm;
				ZeroMemory(&tm, sizeof(TRACKMOUSEEVENT));
				tm.cbSize = sizeof(TRACKMOUSEEVENT);
				tm.dwFlags = TME_LEAVE | TME_NONCLIENT;
				tm.hwndTrack = pWnd->hwnd;
				TrackMouseEvent(&tm);

				RedrawWindow(pWnd->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
			}
			return TRUE;
		case HTCAPTION:
			if (IsWindowEnabled(pWnd->hwnd) &&
				GetActiveWindow() != pWnd->hwnd)
				SetActiveWindow(pWnd->hwnd);
			{
				RECT rw;
				if (GetWindowRect(pWnd->hwnd, &rw))
				{
					pWnd->movingOffset.x = pts.x - rw.left;
					pWnd->movingOffset.y = pts.y - rw.top;
				}
			}
			SendMessageW(pWnd->hwnd, WM_SYSCOMMAND, (SC_MOVE | 0x0002), (*(LPARAM*)&pts));
			return TRUE;
		case HTBOTTOM:
		case HTBOTTOMLEFT:
		case HTBOTTOMRIGHT:
			if (IsWindowEnabled(pWnd->hwnd) &&
				GetActiveWindow() != pWnd->hwnd)
				SetActiveWindow(pWnd->hwnd);
			SendMessageW(pWnd->hwnd, WM_SYSCOMMAND, SC_SIZE + hitTest - (HTLEFT - WMSZ_LEFT), (*(LPARAM*)&pts));
			return TRUE;
	}
	return FALSE;
}

static BOOL SkinnedWindow_OnNcLButtonUp(SKINNEDWND *pWnd, INT hitTest, POINTS pts)
{	
	BOOL bProcessed = FALSE;
	if (HTCLOSE == hitTest && BUTTON_PUSHED == pWnd->buttonState)
	{
		SNDMSG(pWnd->hwnd, WM_SYSCOMMAND, SC_CLOSE, *(LPARAM*)&pts);
		bProcessed = TRUE;
	}

	if (BUTTON_PUSHED == pWnd->buttonState)
	{
		pWnd->buttonState = BUTTON_NORMAL;
		RedrawWindow(pWnd->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
	}

	return bProcessed;
}

static BOOL SkinnedWindow_OnNcRButtonDown(SKINNEDWND *pWnd, INT hitTest, POINTS pts)
{
	return TRUE;
}

static BOOL SkinnedWindow_OnNcRButtonUp(SKINNEDWND *pWnd, INT hitTest, POINTS pts)
{
	int ret = DoTrackPopup(main_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pts.x, pts.y, pWnd->hwnd);
	if (ret) 
		SendMessageW(hMainWindow,WM_COMMAND,ret,0);
	return TRUE;
}

static void SkinnedWindow_OnNcMouseMove(SKINNEDWND *pWnd, INT hitTest, POINTS pts)
{	
	if (BUTTON_PUSHED  == pWnd->buttonState && HTCLOSE == hitTest)
	{
		RedrawWindow(pWnd->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
	}
}

static void SkinnedWindow_OnNcMouseLeave(SKINNEDWND *pWnd)
{	
	if (BUTTON_PUSHED == pWnd->buttonState)
	{
		if(0 != (0x8000 & GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON)))
		{
			TRACKMOUSEEVENT tm;
			ZeroMemory(&tm, sizeof(TRACKMOUSEEVENT));
			tm.cbSize = sizeof(TRACKMOUSEEVENT);
			tm.dwFlags = TME_LEAVE | TME_NONCLIENT;
			tm.hwndTrack = pWnd->hwnd;
			TrackMouseEvent(&tm);
		}
		else 
		{
			pWnd->buttonState = BUTTON_NORMAL;
			RedrawWindow(pWnd->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
		}
	}
}

static void SkinnedWindow_OnWinampCustomClose(SKINNEDWND *pWnd, POINTS pts)
{
	if (0 != pWnd->embedData.reparenting)
		return;

	SNDMSG(pWnd->hwnd, WM_SYSCOMMAND, SC_CLOSE, *(LPARAM*)&pts);
}

static void SkinnedWindow_OnWindowPosChanging(SKINNEDWND *pWnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
	{		
		if (!config_embedwnd_freesize &&
			0 == (WS_CHILD & GetWindowLongPtrW(pWnd->hwnd, GWL_STYLE))) 
		{			
			pwp->cx += (RESIZESTEP_CX - 1);
			pwp->cx -= pwp->cx % RESIZESTEP_CX;
			pwp->cy += (RESIZESTEP_CY - 1);
			pwp->cy -= pwp->cy % RESIZESTEP_CY;
		}
		if (pwp->cx < RESIZESTEP_CX*2) pwp->cx = RESIZESTEP_CX*2;
		if (pwp->cy < RESIZESTEP_CY*2) pwp->cy = RESIZESTEP_CY*2;
	}
	CallWndProc(pWnd, WM_WINDOWPOSCHANGING, 0, (LPARAM)pwp);
}

static void SkinnedWindow_OnWindowPosChanged(SKINNEDWND *pWnd, WINDOWPOS *pwp)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & pwp->flags))
	{
		SetRect(&pWnd->embedData.r, pwp->x, pwp->y, pwp->x + pwp->cx, pwp->y + pwp->cy);
	}

	CallWndProc(pWnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)pwp);
}

static void SkinnedWindow_OnMoving(SKINNEDWND *pWnd, RECT *prc)
{		
	if (0 == (SWS_EX_NOSNAP & pWnd->flags) &&
		(0xFFFF != pWnd->movingOffset.x && 0xFFFF != pWnd->movingOffset.y) &&
		(!!config_snap + (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT))) == 1))
	{
		POINT pt;
		GetCursorPos(&pt);
		INT cx = prc->right - prc->left;
		INT cy = prc->bottom - prc->top;
		prc->left = pt.x - pWnd->movingOffset.x;
		prc->top = pt.y - pWnd->movingOffset.y;
		prc->right = prc->left + cx;
		prc->bottom = prc->top + cy;
		SnapWindowToAllWindows(prc, pWnd->hwnd);
	}
}

static void SkinnedWindow_OnSizing(SKINNEDWND *pWnd, UINT edge, RECT *prc)
{
	LONG cx, cy;
	cx = prc->right - prc->left;
	cy = prc->bottom - prc->top;

	if (!config_embedwnd_freesize &&
		0 == (0x8000 & GetAsyncKeyState(VK_SHIFT)))
	{
		cx += (RESIZESTEP_CX - 1);
		cx -= cx % RESIZESTEP_CX;
		cy += (RESIZESTEP_CY - 1);
		cy -= cy % RESIZESTEP_CY;
	}

	if (cx < RESIZESTEP_CX*2) cx = RESIZESTEP_CX*2;
	if (cy < RESIZESTEP_CY*2) cy = RESIZESTEP_CY*2;
	prc->right = prc->left + cx;
	prc->bottom = prc->top + cy;
}

static void SkinnedWindow_OnEnterSizeMove(SKINNEDWND *pWnd)
{	
	POINT pt;
	RECT rw;
	GetCursorPos(&pt);
	GetWindowRect(pWnd->hwnd, &rw);
	if (0xFFFF == pWnd->movingOffset.x)
		pWnd->movingOffset.x = pt.x - rw.left;
	if (0xFFFF == pWnd->movingOffset.y)
		pWnd->movingOffset.y = pt.y - rw.top;
}

static void SkinnedWindow_OnExitSizeMove(SKINNEDWND *pWnd)
{
	pWnd->movingOffset.x = 0xFFFF;
	pWnd->movingOffset.y = 0xFFFF;
}

static void SkinnedWindow_PatchCursor(SKINNEDWND *pWnd)
{
	RECT rc;
	POINT ptOrig, pt;

	if (!GetCursorPos(&ptOrig) ||
		!GetWindowRect(pWnd->hwnd, &rc))
		return;

	pt.x = rc.right + 1;
	pt.y = ptOrig.y;

	ShowCursor(FALSE);
	SetCursorPos(pt.x, pt.y);
	ShowCursor(TRUE);
	SetCursorPos(ptOrig.x, ptOrig.y);
}

static void SkinnedWindow_OnEnterMenuLoop(SKINNEDWND *pWnd, BOOL bTrackPopup)
{	
	PostMessageW(pWnd->hwnd, WM_SETCURSOR, (WPARAM)pWnd->hwnd, MAKELPARAM(HTCLIENT,WM_ENTERMENULOOP));
}

static void SkinnedWindow_OnExitMenuLoop(SKINNEDWND *pWnd, BOOL bShortcut)
{
}

static void SkinnedWindow_OnUnskin(SKINNEDWND *pWnd)
{
	if (NULL == pWnd) return;

	HWND hwnd = pWnd->hwnd;

	BOOL restoreVisible = FALSE;
	DWORD windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (0 != (WS_VISIBLE & windowStyle))
	{
		restoreVisible = TRUE;
		SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
	}

	if (NULL != pWnd->embedData.wasabi_window)
	{
		SENDWAIPC(IPC_CB_ONHIDEWND, pWnd->hwnd);
		// start looping till we get callback

		MSG msg;
		BOOL stopLoop = FALSE;

		while(FALSE == stopLoop)
		{
			DWORD status = MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
			if (WAIT_OBJECT_0 == status)
			{
				while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (WAMSG_CLOSE == msg.message && msg.hwnd == pWnd->hwnd)
					{
						stopLoop = TRUE;
						break;
					}
					else if (!CallMsgFilter(&msg, MSGF_DIALOGBOX))
					{
						if (msg.message == WM_QUIT)
						{
							PostQuitMessage((INT)msg.wParam);
							stopLoop = TRUE;
							break;
						}
						else
						{
							TranslateMessage(&msg);
							DispatchMessageW(&msg);
						}
					}
				}
			}
		}
	}

	SkinnedWindow_Delete(pWnd);

	if (FALSE != restoreVisible)
		SetWindowLongPtrW(hwnd, GWL_STYLE, WS_VISIBLE | GetWindowLongPtrW(hwnd, GWL_STYLE));
}

static LRESULT SkinnedWindow_OnWinampIPC(SKINNEDWND *pWnd, UINT uCmd, WPARAM param)
{
	switch(uCmd)
	{
		case IPC_SKINWINDOW_SETEXSTYLE:
			pWnd->flags = (SWS_EX_MASK & pWnd->flags) | (~SWS_EX_MASK & param); 
			return 0;

		case IPC_SKINWINDOW_GETEXSTYLE:
			return (~SWS_EX_MASK & pWnd->flags);

		case IPC_SKINWINDOW_SETEMBEDFLAGS:
			pWnd->embedData.flags = (INT)param; 
			return 0;

		case IPC_SKINWINDOW_GETEMBEDFLAGS:
			return pWnd->embedData.flags; 

		case IPC_SKINWINDOW_GETWASABIWND:
			return (LRESULT)pWnd->embedData.wasabi_window;

		case IPC_SKINWINDOW_UNSKIN:
			SkinnedWindow_OnUnskin(pWnd);
			break;

		case IPC_SKINWINDOW_GETGUID:
			if (NULL != param) 
			{
				CopyMemory((void*)param, &pWnd->embedData.guid, sizeof(GUID));
				return TRUE;
			}
			break;

		case IPC_SKINWINDOW_GETEMBEDNUMS:
			return Skin_UseGenNums;
	}
	return 0;
}

static void SkinnedWindow_OnSysCommand(SKINNEDWND *pWnd, UINT uCmd, LPARAM param)
{
	CallWndProc(pWnd, WM_SYSCOMMAND, (WPARAM)uCmd, param);

	switch(uCmd)
	{
		case SC_MOVE:
		case SC_SIZE:
		case SC_DRAGMOVE:
		case SC_DRAGSIZE_N:
		case SC_DRAGSIZE_S:
		case SC_DRAGSIZE_E:
		case SC_DRAGSIZE_W:
		case SC_DRAGSIZE_NW:
		case SC_DRAGSIZE_NE:
		case SC_DRAGSIZE_SW:
		case SC_DRAGSIZE_SE:
			SkinnedWindow_PatchCursor(pWnd);		
			break;
	}
}

static LRESULT SkinnedWindow_OnSetText(SKINNEDWND *pWnd, LPCWSTR pszText)
{
	LRESULT result = CallWndProc(pWnd, WM_SETTEXT, 0, (LPARAM)pszText);
	ifc_window *wnd = pWnd->embedData.wasabi_window;
	if (NULL != wnd)
	{
		ifc_window *parent = wnd->getRootParent();
		if (NULL == parent) parent = wnd;
		parent->setWindowTitle(pszText);
	}
	else
	{
		SetWindowPos(pWnd->hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	return result;
}

static LRESULT CALLBACK SkinnedWnd_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SKINNEDWND *pWnd = GetSkinnedWnd(hwnd); 
	if (NULL == pWnd)
	{
		return ((IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam));
	}

	switch(uMsg)
	{
		case WM_DESTROY:
			{
				BOOL unicode = IsUnicode(pWnd);
				WNDPROC wndProc = pWnd->fnWndProc;
				SkinnedWindow_Delete(pWnd);
				return ((unicode) ? CallWindowProcW(wndProc, hwnd, uMsg, wParam, lParam) : 
					CallWindowProcA(wndProc, hwnd, uMsg, wParam, lParam));
			}
			break;

		case WAMSG_CLOSE:		SkinnedWindow_OnWinampCustomClose(pWnd, MAKEPOINTS(lParam)); return 0;
		case WM_NCHITTEST:		return SkinnedWindow_OnNcHitTest(pWnd, MAKEPOINTS(lParam));
		case WM_NCCALCSIZE:		return SkinnedWindow_OnNcCalcSize(pWnd, (BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam);
		case WM_NCPAINT:		SkinnedWindow_OnNcPaint(pWnd, (HRGN)wParam); return 0;
		case WM_NCACTIVATE:		return SkinnedWindow_OnNcActivate(pWnd, (BOOL)wParam);  
		case WM_ACTIVATE:		SkinnedWindow_OnActivate(pWnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return 0;
		case WM_PRINT:			SkinnedWindow_OnPrint(pWnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_SETCURSOR:		return SkinnedWindow_OnSetCursor(pWnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
		case WM_NCLBUTTONDOWN:	if (SkinnedWindow_OnNcLButtonDown(pWnd, (INT)wParam, MAKEPOINTS(lParam))) return 0; break;
		case WM_NCLBUTTONUP:	if (SkinnedWindow_OnNcLButtonUp(pWnd, (INT)wParam, MAKEPOINTS(lParam))) return 0; break;
		case WM_NCRBUTTONDOWN:	if (SkinnedWindow_OnNcRButtonDown(pWnd, (INT)wParam, MAKEPOINTS(lParam))) return 0; break;
		case WM_NCRBUTTONUP:	if (SkinnedWindow_OnNcRButtonUp(pWnd, (INT)wParam, MAKEPOINTS(lParam))) return 0; break;
		case WM_NCMOUSEMOVE:	SkinnedWindow_OnNcMouseMove(pWnd, (INT)wParam, MAKEPOINTS(lParam)); break;
		case WM_NCMOUSELEAVE:	SkinnedWindow_OnNcMouseLeave(pWnd); break;
		case WM_SHOWWINDOW: 		if (SkinnedWindow_OnShowWindow(pWnd, (BOOL)wParam, (UINT)lParam)) return 0; break;
		case WM_WINDOWPOSCHANGING: SkinnedWindow_OnWindowPosChanging(pWnd, (WINDOWPOS*)lParam); return 0;
		case WM_WINDOWPOSCHANGED: SkinnedWindow_OnWindowPosChanged(pWnd, (WINDOWPOS*)lParam); return 0;
		case WM_ENTERSIZEMOVE:	SkinnedWindow_OnEnterSizeMove(pWnd); break;
		case WM_EXITSIZEMOVE:	SkinnedWindow_OnExitSizeMove(pWnd); break;
		case WM_MOVING:			SkinnedWindow_OnMoving(pWnd, (RECT*)lParam); break;
		case WM_SIZING:			SkinnedWindow_OnSizing(pWnd, (UINT)wParam, (RECT*)lParam); break;
		case WM_ENTERMENULOOP:	SkinnedWindow_OnEnterMenuLoop(pWnd, (BOOL)wParam); break;
		case WM_EXITMENULOOP:	SkinnedWindow_OnExitMenuLoop(pWnd, (BOOL)wParam); break;
		case WM_NCUAHDRAWCAPTION: return 0;
		case WM_NCUAHDRAWFRAME:	return 0;
		case WM_WA_IPC:			return SkinnedWindow_OnWinampIPC(pWnd, (UINT)lParam, wParam);
		case WM_SYSCOMMAND:		SkinnedWindow_OnSysCommand(pWnd, (UINT)wParam, lParam); return 0; 
		case WM_SETTEXT:		return SkinnedWindow_OnSetText(pWnd, (LPCWSTR)lParam);
	}

	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{			
		if (0 == (WS_CHILD & GetWindowStyle(hwnd)))
			return TRUE;
		else
		{
			HWND hParent;
			hParent = GetAncestor(hwnd, GA_PARENT);
			if (NULL != hParent)
				return SendMessageW(hwnd, uMsg, wParam, lParam);

			return FALSE;
		}
	}

	return CallWndProc(pWnd, uMsg, wParam, lParam);
}