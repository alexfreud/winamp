#include "../winamp/wa_dlg.h"
#include "./skinneddlg.h"


SkinnedDialog::SkinnedDialog(void) : SkinnedWnd(TRUE)
{
}

SkinnedDialog::~SkinnedDialog(void)
{
}

BOOL SkinnedDialog::Attach(HWND hwndDialog)
{
	if(!__super::Attach(hwndDialog)) return FALSE;
	SetType(SKINNEDWND_TYPE_DIALOG);
	return TRUE;
}

HBRUSH SkinnedDialog::OnColorDialog(HDC hdc)
{
	if (hdc)
	{
		SetTextColor(hdc, WADlg_getColor(WADLG_WNDFG));
		SetBkColor(hdc, WADlg_getColor(WADLG_WNDBG));
	}
	return (HBRUSH)MlStockObjects_Get(WNDBCK_BRUSH);
}


LRESULT SkinnedDialog::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_NCPAINT:
		case WM_NCCALCSIZE:
			__super::WindowProc(uMsg, wParam, lParam);
			return TRUE;

		case WM_CTLCOLORDLG: 
			if (SWS_USESKINCOLORS & style) return (LRESULT)OnColorDialog((HDC)wParam);
			break;
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}