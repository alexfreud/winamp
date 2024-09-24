#include "main.h"
#include "./commandbar.h"

typedef struct _CMDBAR
{
	HWND			hwndOwner;
	DLGPROC		fnDlgProc;
	ULONG_PTR	uData;
} CMDBAR;

#define GetBarData(/*HWND*/ __hwndCmdBar) ((CMDBAR*)GetPropW((__hwndCmdBar), L"MLDISCCMDBAR"))


static BOOL CommandBar_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	CMDBARCREATESTRUCT *pcbcs = (CMDBARCREATESTRUCT*)lParam;
	if (pcbcs && pcbcs->fnDialogProc)
	{
		CMDBAR *pcb = (CMDBAR*)calloc(1, sizeof(CMDBAR));
		if (pcb)
		{
			pcb->fnDlgProc = pcbcs->fnDialogProc;
			pcb->hwndOwner = pcbcs->hwndOwner;
			pcb->uData = pcbcs->uData;
			if (!SetPropW(hdlg, L"MLDISCCMDBAR", (HANDLE)pcb)) 
			{
				free(pcb);
				DestroyWindow(hdlg);
			}
			else return pcbcs->fnDialogProc(hdlg, WM_INITDIALOG, (WPARAM)hwndFocus, pcbcs->uData);
		}
	}
	return FALSE;
}

static void CommandBar_OnDestroy(HWND hdlg)
{
	CMDBAR *pcb = GetBarData(hdlg);
	if (pcb)
	{
		RemovePropW(hdlg, L"MLDISCCMDBAR");
		free(pcb);
	}
}

static BOOL CALLBACK CommandBar_EnumChildHeight(HWND hwnd, LPARAM param)
{
	if (!param) return FALSE;
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (hdc)
	{
		HFONT hf = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
		if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (NULL != hf)
		{
			TEXTMETRICW tm = {0};
			HFONT hfo = (HFONT)SelectObject(hdc, hf);
			if (GetTextMetricsW(hdc, &tm)) 
			{
				INT *pmh = (INT*)param;
				if (tm.tmHeight > *pmh) *pmh = tm.tmHeight;
			}
			SelectObject(hdc, hfo);
		}
		ReleaseDC(hwnd, hdc);
	}
	return TRUE;

}
static INT CommandBar_OnGetBestHeight(HWND hwnd)
{
	INT maxHeight = 0;
	EnumChildWindows(hwnd, CommandBar_EnumChildHeight, (LPARAM)&maxHeight);
	if (maxHeight != 0) maxHeight += 2;
	return maxHeight;
}

INT_PTR CALLBACK CommandBar_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMDBAR *pcb = GetBarData(hdlg);
	if (!pcb)
	{
		if (WM_INITDIALOG == uMsg) return CommandBar_OnInitDialog(hdlg, (HWND)wParam, lParam);
		return 0;
	}
	if (pcb->fnDlgProc(hdlg, uMsg, wParam, lParam)) 
	{
		if (WM_DESTROY == uMsg) CommandBar_OnDestroy(hdlg);
		return TRUE;
	}

	switch(uMsg)
	{
		case WM_DESTROY: CommandBar_OnDestroy(hdlg); return TRUE;
		case WM_COMMAND:
			if (pcb->hwndOwner) SendMessageW(pcb->hwndOwner, uMsg, wParam, lParam); return TRUE;
			break;
				
		case CBM_GETBESTHEIGHT: MSGRESULT(hdlg, CommandBar_OnGetBestHeight(hdlg));
		case CBM_GETOWNER:		MSGRESULT(hdlg, pcb->hwndOwner);
		case CBM_SETOWNER:		
			SetWindowLongPtrW(hdlg, DWLP_MSGRESULT, (LONGX86)(LONG_PTR)pcb->hwndOwner);
			pcb->hwndOwner = (HWND)lParam;
			return TRUE;
		case CBM_GETDATA:
			{
				CMDBAR *pBar = GetBarData(hdlg);
				MSGRESULT(hdlg, (pBar) ? pBar->uData : NULL);
			}
		case CBM_SETDATA:
			{
				CMDBAR *pBar = GetBarData(hdlg);
				if (pBar) pBar->uData = (ULONG_PTR)lParam;
				MSGRESULT(hdlg, (NULL != pBar));
			}
			
	}
	return 0;
}