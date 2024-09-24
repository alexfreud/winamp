#include "./skinnedfolder.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"


SkinnedFolderBrowser::SkinnedFolderBrowser(void) : SkinnedScrollWnd(FALSE)
{
}

SkinnedFolderBrowser::~SkinnedFolderBrowser(void)
{

}

BOOL SkinnedFolderBrowser::Attach(HWND hwndFB)
{
	if(!SkinnedScrollWnd::Attach(hwndFB)) return FALSE;
	
	SetType(SKINNEDWND_TYPE_FOLDERBROWSER);
	SetMode(SCROLLMODE_STANDARD);
	DisableNoScroll(TRUE);
	
	FOLDERBROWSERINFO fbi;
	fbi.cbSize = sizeof(FOLDERBROWSERINFO);
	if (FolderBrowser_GetInfo(hwnd, &fbi))
	{
		if (NULL != fbi.hwndActive) SkinWindowEx(fbi.hwndActive, SKINNEDWND_TYPE_LISTBOX, style);
		if (NULL != fbi.hwndDraw) SkinWindowEx(fbi.hwndDraw, SKINNEDWND_TYPE_LISTBOX, style);
	}

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}


BOOL SkinnedFolderBrowser::SetStyle(UINT newStyle, BOOL bRedraw)
{
	BOOL result = __super::SetStyle(newStyle, bRedraw);
	if (hwnd)
	{
		FOLDERBROWSERINFO fbi;
		fbi.cbSize = sizeof(FOLDERBROWSERINFO);
		if (FolderBrowser_GetInfo(hwnd, &fbi))
		{
			if (NULL != fbi.hwndActive) MLSkinnedWnd_SetStyle(fbi.hwndActive, style);
			if (NULL != fbi.hwndDraw) MLSkinnedWnd_SetStyle(fbi.hwndDraw, style);
		}
	}
	return result;
}

void SkinnedFolderBrowser::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	if (SWS_USESKINCOLORS & style)
	{
		FolderBrowser_SetBkColor(hwnd,  WADlg_getColor(WADLG_ITEMBG));
		FolderBrowser_SetTextColor(hwnd,  WADlg_getColor(WADLG_ITEMFG));
	}
	__super::OnSkinChanged(bNotifyChildren, bRedraw);
}

