#include "main.h"
#include "./copyfiles.h"
#include "./copyinternal.h"
#include "./resource.h"
#include "../nu/trace.h"
#include <shlwapi.h>
#include <strsafe.h>


#define QUESTIONBOX_PROP		TEXT("QUESTIONBOX")
#define GetQuestionBox(__hdlg)	((QUESTIONBOX*)GetProp((__hdlg), QUESTIONBOX_PROP))


#define GetResolvedString(__pszText, __pszBuffer, __chhBufferMax)\
	(IS_INTRESOURCE(__pszText) ? WASABI_API_LNGSTRINGW_BUF((UINT)(UINT_PTR)(__pszText), (__pszBuffer), (__chhBufferMax)) : (__pszText))

static INT_PTR CALLBACK CopyQuestion_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR MLDisc_ShowQuestionBox(QUESTIONBOX *pQuestionBox)
{	
	if (!pQuestionBox) return IDCANCEL;
	return WASABI_API_DIALOGBOXPARAMW(IDD_FILECOPY_QUESTION, pQuestionBox->hParent, CopyQuestion_DialogProc, (LPARAM)pQuestionBox);
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

static INT_PTR CopyQuestion_OnInitDialog(HWND hdlg, HWND hFocus, QUESTIONBOX *pqb)
{
	if (!pqb) return FALSE;
	SetProp(hdlg, QUESTIONBOX_PROP, pqb);

	HWND hctrl;
	TCHAR szBuffer[2048] = {0};
	LONG messageLeft = 0;

	if (NULL != pqb->pszTitle) SetWindowText(hdlg, GetResolvedString(pqb->pszTitle, szBuffer, ARRAYSIZE(szBuffer)));
	
	if (NULL != pqb->pszBtnOkText) SetDlgItemText(hdlg, IDOK, GetResolvedString(pqb->pszBtnOkText, szBuffer, ARRAYSIZE(szBuffer)));
	if (NULL != pqb->pszBtnCancelText) SetDlgItemText(hdlg, IDCANCEL, GetResolvedString(pqb->pszBtnCancelText, szBuffer, ARRAYSIZE(szBuffer)));
		
	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_EXTRA1)))
	{
		ShowWindow(hctrl, (QBF_SHOW_EXTRA_BUTTON & pqb->uFlags) ? SW_SHOWNA : SW_HIDE);
        if (NULL != pqb->pszBtnExtraText) SetWindowText(hctrl, GetResolvedString(pqb->pszBtnExtraText, szBuffer, ARRAYSIZE(szBuffer)));
	}
	
	
	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_PIC_ICON)))
	{
		HICON hIcon = NULL;
		if (NULL != pqb->pszIcon)
		{		
			hIcon = LoadIcon(WASABI_API_LNG_HINST, pqb->pszIcon);
			if (NULL == hIcon) hIcon = LoadIcon(WASABI_API_ORIG_HINST, pqb->pszIcon);
			if (NULL == hIcon) hIcon = LoadIcon(NULL, pqb->pszIcon);
		}
		SendMessage(hctrl, STM_SETICON, (WPARAM)hIcon, 0L); 
		ShowWindow(hctrl, (hIcon) ? SW_SHOWNA : SW_HIDE);
		RECT rw;
		GetWindowRect(hctrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rw, 2);
		messageLeft = (hIcon) ? (rw.right + 24) : rw.left;
	}

	INT shiftRight = 0, shiftBottom = 0;

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_LBL_MESSAGE)))
	{
		RECT rw;
		SIZE textSize = { 0, 0 };
		LPCTSTR pszText = (NULL != pqb->pszMessage) ? GetResolvedString(pqb->pszMessage, szBuffer, ARRAYSIZE(szBuffer)) : NULL;
		if (pszText)
		{
			FindPrefferedSize(hctrl, pszText, TEXT("\n"), &textSize);
			textSize.cx += 8; textSize.cy += 4;
		}
		SetWindowText(hctrl, pszText);
		GetWindowRect(hctrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rw, 2);
		rw.left = messageLeft;
		shiftRight = (rw.left + textSize.cx) - rw.right;
		if (shiftRight < 0) shiftRight = 0;
		shiftBottom = textSize.cy - (rw.bottom - rw.top);
		if (shiftBottom < 0) shiftBottom = 0;
		SetWindowPos(hctrl, NULL, rw.left, rw.top, (rw.right - rw.left) + shiftRight, (rw.bottom - rw.top) + shiftBottom, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_CHECKBOX1)))
	{
		
        if (NULL != pqb->pszCheckboxText) SetWindowText(hctrl, GetResolvedString(pqb->pszCheckboxText, szBuffer, ARRAYSIZE(szBuffer)));
		SendMessage(hctrl, BM_SETCHECK, (pqb->checkboxChecked) ? BST_CHECKED : BST_UNCHECKED, 0L);

		RECT rw;
		GetWindowRect(hctrl, &rw);

		if (0 == (QBF_SHOW_CHECKBOX & pqb->uFlags))
		{
			shiftBottom -= (rw.bottom - rw.top);
			ShowWindow(hctrl, SW_HIDE);
		}
		else if (shiftRight || shiftBottom)
		{
			MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rw, 2);
			SetWindowPos(hctrl, NULL, rw.left, rw.top + shiftBottom, 
						(rw.right - rw.bottom) + shiftRight, (rw.bottom - rw.top), SWP_NOACTIVATE | SWP_NOZORDER); 		
			ShowWindow(hctrl, SW_SHOWNA);
		}
	}
	
	if (shiftRight || shiftBottom)
	{
		RECT rw;
		INT idList[] = {IDC_BTN_EXTRA1, IDOK, IDCANCEL, };
		for (int i = 0; i < ARRAYSIZE(idList); i++)
		{
			if (NULL != (hctrl = GetDlgItem(hdlg, idList[i])))
			{
				GetWindowRect(hctrl, &rw);
				MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rw, 2);
				SetWindowPos(hctrl, NULL, rw.left + shiftRight, rw.top + shiftBottom, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER); 		
			}
		}
		
	}

	HWND hParent = GetParent(hdlg);
	if (hParent)
	{
		RECT rw, rc;
		GetClientRect(hParent, &rc);
		GetWindowRect(hdlg, &rw);
		rw.right += shiftRight;
		rw.bottom += shiftBottom; 
		
		SetWindowPos(hdlg, NULL, 
			rw.left + ((rc.right - rc.left) - (rw.right - rw.left))/2, 
			rw.top + ((rc.bottom - rc.top) - (rw.bottom - rw.top))/2, 
			rw.right - rw.left, rw.bottom - rw.top, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	SendMessage(hdlg, DM_REPOSITION, 0, 0L);	
	return FALSE;
}

static void CopyQuestion_OnDestroy(HWND hdlg)
{
    QUESTIONBOX *pqb = GetQuestionBox(hdlg);
	if (pqb)
	{
		pqb->checkboxChecked = (BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_CHECKBOX1));
	}
	RemoveProp(hdlg, QUESTIONBOX_PROP);
}

static void CopyQuestion_OnCommand(HWND hdlg, INT ctrlId, INT eventId, HWND hctrl)
{
	switch(ctrlId)
	{
		case IDOK:
		case IDCANCEL:
			EndDialog(hdlg, ctrlId);
			break;
		case IDC_BTN_EXTRA1:
			if (BN_CLICKED == eventId) EndDialog(hdlg, ctrlId);
			break;
	}
}
#define IDT_POSTSHOW		1985
#define DELAY_POSTSHOW		0

static void CALLBACK CopyQuestion_OnPostShowElapsed(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	QUESTIONBOX *pqb = GetQuestionBox(hdlg);
	KillTimer(hdlg, idEvent);
	if (!pqb) return;

	if (QBF_FLASH & pqb->uFlags)
	{
		FLASHWINFO flash = { sizeof(FLASHWINFO), };
		flash.hwnd = hdlg;
		flash.dwFlags = FLASHW_ALL;
		flash.uCount = 2;
		flash.dwTimeout = 300;
		FlashWindowEx(&flash);
	}
	
	if (QBF_BEEP & pqb->uFlags) MessageBeep(pqb->uBeepType);

	if ((QBF_SETFOREGROUND | QBF_TOPMOST) & pqb->uFlags)
	{
		SetForegroundWindow(hdlg);
		SetWindowPos(hdlg, (QBF_SETFOREGROUND & pqb->uFlags) ? HWND_TOP : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	
	

	

}
static void CopyQuestion_OnShowWindow(HWND hdlg, BOOL bShow, UINT nStatus)
{	
	if (bShow)
	{		
		SetTimer(hdlg, IDT_POSTSHOW, DELAY_POSTSHOW, CopyQuestion_OnPostShowElapsed);
	}
}

static INT_PTR CALLBACK CopyQuestion_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:	return CopyQuestion_OnInitDialog(hdlg, (HWND)wParam, (QUESTIONBOX*)lParam);
		case WM_DESTROY:		CopyQuestion_OnDestroy(hdlg); break;
		case WM_COMMAND:		CopyQuestion_OnCommand(hdlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_SHOWWINDOW:	CopyQuestion_OnShowWindow(hdlg, (BOOL)wParam, (UINT)lParam); break;
	}
	return 0;
}