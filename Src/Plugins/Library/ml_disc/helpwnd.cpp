#include "main.h"
#include "./resource.h"
#include "../nu/trace.h"
#include <strsafe.h>

#define MLHELP_PROP			TEXT("MLHELP")

typedef struct __SIMPLEHELP
{
	LPCWSTR pszTitle;
	LPCWSTR pszCaption;
	LPCWSTR pszText;
	HWND	hOwner;
	UINT	uFlags;
} SIMPLEHELP;

typedef struct __MLHELP
{
	HWND	hOwner;
	UINT	uFlags;
	LONG	width;
	LONG	height;
} MLHELP;

#define GetHelp(__hwnd) ((MLHELP*)GetProp((__hwnd), MLHELP_PROP))

#define CLIENT_MIN_WIDTH		280
#define CLIENT_MIN_HEIGHT	200
#define CLIENT_MAX_WIDTH		800
#define CLIENT_MAX_HEIGHT	600
#define BORDER_SPACE		10

static INT_PTR CALLBACK SimpleHelp_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND MLDisc_ShowHelp(HWND hOwner, LPCWSTR pszWindowTitle, LPCWSTR pszCaption, LPCWSTR pszText, UINT uFlags)
{
	SIMPLEHELP help;
	help.pszTitle = pszWindowTitle;
	help.pszCaption = pszCaption;
	help.pszText = pszText;
	help.uFlags = uFlags;
	help.hOwner = hOwner;
	
	if (HF_DOMODAL & uFlags)
	{
		WASABI_API_DIALOGBOXPARAMW(IDD_SIMPLEHELP, hOwner, SimpleHelp_DialogProc, (LPARAM)&help);
		return NULL;	
	}

	return WASABI_API_CREATEDIALOGPARAMW(IDD_SIMPLEHELP, hOwner, SimpleHelp_DialogProc, (LPARAM)&help);
}

static BOOL FindPrefferedSizeEx(HDC hdc, LPCTSTR pszText, LPCTSTR pszNewLine, SIZE *pSize)
{
	if (!pSize) return FALSE;
	pSize->cx = 0; pSize->cy = 0;
	if (!hdc || !pszText || !pszNewLine) return FALSE;
	LPCTSTR pszBlock = pszText;
	LPCTSTR pszCursor = pszBlock;
	INT cchSep = lstrlenW(pszNewLine);
	INT matched = 0;
	for(;;)
	{
		if (*pszCursor)
		{
			if (*pszCursor == pszNewLine[matched]) matched++;
			else matched = 0;
			pszCursor++;
		}
		if (matched == cchSep || TEXT('\0') == *pszCursor)
		{
			SIZE sz;

			INT l = (INT)(size_t)((pszCursor - pszBlock) - matched);
			if (l > 0)
			{
				if (!GetTextExtentPoint32(hdc, pszBlock, l, &sz)) return FALSE;
			}
			else 
			{
				if (!GetTextExtentPoint32(hdc, TEXT("\n"), 1, &sz)) return FALSE;
				sz.cx = 0;
			}

			
			if (pSize->cx < sz.cx) pSize->cx= sz.cx;
			pSize->cy += sz.cy;

			if (TEXT('\0') == *pszCursor) break;
			else 
			{
				matched = 0;
				pszBlock = pszCursor;
			}
		}
	}
	return TRUE;
}

static BOOL FindPrefferedSize(HWND hwnd, LPCTSTR pszText, LPCTSTR pszNewLine, SIZE *pSize)
{
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_PARENTCLIP);
	if (!hdc) return FALSE;
	HFONT hf, hfo;
	hf = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;

	BOOL br = FindPrefferedSizeEx(hdc, pszText, pszNewLine, pSize);
	
	if (hfo) SelectObject(hdc, hfo);
	ReleaseDC(hwnd, hdc);

	return br;
}

static INT_PTR SimpleHlp_OnInitDialog(HWND hdlg, HWND hFocus, LPARAM lParam)
{
	SIMPLEHELP *pHelp = (SIMPLEHELP*)lParam;
	SIZE sizeCaption = { 0, 0 };
	SIZE sizeText = { 0, 0 };
	SIZE sizeClient = {0, 0};
	SIZE sizeButton = {0, 0};

	MLHELP *pmlh = (MLHELP*)calloc(1, sizeof(MLHELP));
	if (pmlh)
	{
		pmlh->hOwner = pHelp->hOwner;
		pmlh->uFlags = pHelp->uFlags;
		pmlh->height = 0;
		pmlh->width = 0;	
	}
    SetProp(hdlg, MLHELP_PROP, (HANDLE)pmlh);
	
	HWND hctrl;
	if(pHelp)
	{
		
		WCHAR szBuffer[4096] = {0};
		if (pHelp->pszTitle) 
		{
			if (IS_INTRESOURCE(pHelp->pszCaption))
			{
				WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pHelp->pszTitle, szBuffer, ARRAYSIZE(szBuffer));
				pHelp->pszTitle = szBuffer;
			}
			SetWindowText(hdlg, pHelp->pszTitle);
		}

		if (pHelp->pszCaption) 
		{
			if (IS_INTRESOURCE(pHelp->pszCaption))
			{
				WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pHelp->pszCaption, szBuffer, ARRAYSIZE(szBuffer));
				pHelp->pszCaption = szBuffer;
			}
			
			if (NULL != (hctrl = GetDlgItem(hdlg, IDC_LBL_CAPTION)))
			{
				FindPrefferedSize(hctrl, pHelp->pszCaption, TEXT("\n"), &sizeCaption);
				SetWindowText(hctrl, pHelp->pszCaption);
			}
		}

		if (pHelp->pszText) 
		{
			if (IS_INTRESOURCE(pHelp->pszText))
			{
				WCHAR form_szBuffer[4096] = {0}, *fszB = form_szBuffer, *szB = szBuffer;
				WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pHelp->pszText, form_szBuffer, ARRAYSIZE(form_szBuffer));
				while(fszB && *fszB){
					if(*fszB == L'\n' && *CharPrevW(form_szBuffer,fszB) != L'\r'){
						*szB = L'\r';
						szB = CharNextW(szB);
					}
					*szB = *fszB;
					szB = CharNextW(szB);
					fszB = CharNextW(fszB);
				}
				*szB = 0;
				pHelp->pszText = szBuffer;
			}
			if (NULL != (hctrl = GetDlgItem(hdlg, IDC_EDT_TEXT)))
			{
				FindPrefferedSize(hctrl, pHelp->pszText, TEXT("\r\n"), &sizeText);
				SetWindowText(hctrl, pHelp->pszText);
			}
		}

		if (0 == (HF_ALLOWRESIZE & pHelp->uFlags))
		{
			SetWindowLongPtrW(hdlg, GWL_EXSTYLE, GetWindowLongPtrW(hdlg, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME);
			SetWindowLongPtrW(hdlg, GWL_STYLE, (GetWindowLongPtrW(hdlg, GWL_STYLE) & ~WS_THICKFRAME) | DS_MODALFRAME);
		}
	}

	if (sizeText.cx > 0) sizeText.cx += GetSystemMetrics(SM_CXVSCROLL);

	sizeClient.cx = ((sizeText.cx > sizeCaption.cx) ? sizeText.cx : sizeCaption.cx) + 8;
	if (sizeClient.cx < CLIENT_MIN_WIDTH) sizeClient.cx = CLIENT_MIN_WIDTH;
	if (sizeClient.cx > CLIENT_MAX_WIDTH) sizeClient.cx = CLIENT_MAX_WIDTH;
	sizeText.cx = sizeClient.cx;
	sizeCaption.cx = sizeClient.cx;
	sizeClient.cx += BORDER_SPACE * 2;

	if (sizeCaption.cy > 0) sizeCaption.cy += 16;
	if (sizeCaption.cy > CLIENT_MAX_HEIGHT/3) sizeCaption.cy = CLIENT_MAX_HEIGHT/3;
	
	if (sizeText.cy > 0) sizeText.cy += 16;
	if (sizeText.cy < (CLIENT_MIN_HEIGHT - sizeCaption.cy)) sizeText.cy = (CLIENT_MIN_HEIGHT - sizeCaption.cy);
	if (sizeText.cy > (CLIENT_MAX_HEIGHT - sizeCaption.cy)) sizeText.cy = (CLIENT_MAX_HEIGHT - sizeCaption.cy);
	
	if (NULL != (hctrl = GetDlgItem(hdlg, IDCANCEL)))
	{
		RECT rw;
		if (GetWindowRect(hctrl, &rw)) { sizeButton.cx = rw.right - rw.left; sizeButton.cy = rw.bottom - rw.top; }
	}

	LONG top = BORDER_SPACE;
	sizeClient.cy = BORDER_SPACE +sizeCaption.cy + sizeText.cy + 8 + sizeButton.cy + BORDER_SPACE;

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_LBL_CAPTION)))
	{
		if (0 == sizeCaption.cy) EnableWindow(hctrl, FALSE);
		SetWindowPos(hctrl, NULL, BORDER_SPACE, top, sizeCaption.cx, sizeCaption.cy, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
		top += sizeCaption.cy;
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_EDT_TEXT)))
	{
		if (0 == sizeText.cy) EnableWindow(hctrl, FALSE);
		SetWindowPos(hctrl, NULL, BORDER_SPACE, top, sizeText.cx, sizeText.cy, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDCANCEL)))
	{
		SetWindowPos(hctrl, NULL, sizeClient.cx - BORDER_SPACE - sizeButton.cx, sizeClient.cy - BORDER_SPACE - sizeButton.cy,
						sizeButton.cx, sizeButton.cy, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}
	
	RECT rw, rc;
	if (GetClientRect(hdlg, &rc) && GetWindowRect(hdlg, &rw))
	{						
		sizeClient.cx += ((rw.right - rw.left) - (rc.right - rc.left));
		sizeClient.cy += ((rw.bottom - rw.top) - (rc.bottom - rc.top));
	}

	SetRect(&rw, 0, 0, 0, 0);
	
	if (pHelp->hOwner && GetWindowRect(pHelp->hOwner, &rw))
	{		
		rw.left += ((rw.right - rw.left) - sizeClient.cx)/2;
		rw.top += ((rw.bottom - rw.top) - sizeClient.cy)/2;
	}
	UINT swpFlags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | ((HF_DOMODAL & pHelp->uFlags) ? SWP_NOZORDER  : 0);
	pmlh->width = sizeClient.cx;
	pmlh->height = sizeClient.cy;
	SetWindowPos(hdlg, HWND_TOP, rw.left, rw.top, sizeClient.cx, sizeClient.cy, swpFlags);
	return FALSE;
}

static void SimpleHelp_OnDestroy(HWND hdlg)
{
	MLHELP *pHelp = GetHelp(hdlg);
	RemoveProp(hdlg, MLHELP_PROP);
	
	if (pHelp && 0 == (HF_DOMODAL & pHelp->uFlags) && pHelp->hOwner && IsWindow(pHelp->hOwner)) 
		SendMessageW(pHelp->hOwner, WM_PARENTNOTIFY, MAKEWPARAM(WM_DESTROY, 0), (LPARAM)hdlg);
	if (pHelp) free(pHelp);
}

static void SimpleHlp_OnCommand(HWND hdlg, INT ctrlId, INT eventId, HWND hctrl)
{
	MLHELP *pHelp = GetHelp(hdlg);
	switch(ctrlId)
	{
		case IDCANCEL:
		case IDOK:
			if (!pHelp || (HF_DOMODAL & pHelp->uFlags)) EndDialog(hdlg, ctrlId); 
			else DestroyWindow(hdlg);
			break;
	}
}

/*static void SimpleHlp_OnWindowPosChanging(HWND hdlg, WINDOWPOS *pwp)
{
	MLHELP *pHelp = GetHelp(hdlg);
	if (!pHelp) return;

	if (0 == (SWP_NOSIZE & pwp->flags))
	{
		if (pwp->cx < CLIENT_MIN_WIDTH) pwp->cx = CLIENT_MIN_WIDTH;
		if (pwp->cx > CLIENT_MAX_WIDTH) pwp->cx = CLIENT_MAX_WIDTH;
		if (pwp->cy < CLIENT_MIN_HEIGHT) pwp->cy = CLIENT_MIN_HEIGHT;
		if (pwp->cy > CLIENT_MAX_HEIGHT) pwp->cy = CLIENT_MAX_HEIGHT;
	}
}

static void SimpleHlp_OnWindowPosChanged(HWND hdlg, WINDOWPOS *pwp)
{
	MLHELP *pHelp = GetHelp(hdlg);
	if (!pHelp) return;
	if (0 == (SWP_NOSIZE & pwp->flags))
	{
		RECT rw;
		GetWindowRect(hdlg, &rw);
		LONG dx = (rw.right - rw.left) - pHelp->width;
		LONG dy = (rw.bottom - rw.top) - pHelp->height;
		pHelp->width = rw.right - rw.left;
		pHelp->height = rw.bottom - rw.top;

		HDWP hdwp = BeginDeferWindowPos(3);
		HWND hctrl;
		if (hdwp && 0 != dx && NULL != (hctrl = GetDlgItem(hdlg, IDC_LBL_CAPTION)) && GetWindowRect(hctrl, &rw))
		{
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, 0, 0, 
				(rw.right - rw.left) + dx, (rw.bottom - rw.top), SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
			InvalidateRect(hctrl, NULL, TRUE);
		}
		if (hdwp && (0 != dx || 0 != dy) && 
			NULL != (hctrl = GetDlgItem(hdlg, IDC_EDT_TEXT)) && GetWindowRect(hctrl, &rw))
		{
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, 0, 0, 
						(rw.right - rw.left) + dx, (rw.bottom - rw.top) + dy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
		}
		if (hdwp && (0 != dx || 0 != dy) && 
			NULL != (hctrl = GetDlgItem(hdlg, IDCANCEL)) && GetWindowRect(hctrl, &rw))
		{
			MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rw, 2);
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, rw.left + dx, rw.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
		}
		if (hdwp && EndDeferWindowPos(hdwp))
		{	
			RedrawWindow(hdlg, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_INTERNALPAINT | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW); 
		}
	}
}*/

static INT_PTR CALLBACK SimpleHelp_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return SimpleHlp_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:			SimpleHelp_OnDestroy(hdlg); break;
		case WM_COMMAND:			SimpleHlp_OnCommand(hdlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		/*case WM_WINDOWPOSCHANGING:	SimpleHlp_OnWindowPosChanging(hdlg, (WINDOWPOS*)lParam); return 0;
		case WM_WINDOWPOSCHANGED:	SimpleHlp_OnWindowPosChanged(hdlg, (WINDOWPOS*)lParam); return 0;*/
	}
	return 0;
}