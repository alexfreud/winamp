#include "main.h"
#include <windowsx.h>
#include "./resource.h"
#include "./commandbar.h"
#include <shlwapi.h>
#include <strsafe.h>

static HMLIMGLST hmlilButton = NULL;

static LPCTSTR GetImageTagStr(INT resId)
{
	switch(resId)
	{
		case IDB_PLAY_NORMAL:			return TEXT("button.play");
		case IDB_PLAY_HIGHLIGHTED:		return TEXT("button.play.highlighted");
		case IDB_PLAY_PRESSED:			return TEXT("button.play.pressed");
		case IDB_PLAY_DISABLED:			return TEXT("button.play.disabled");
		case IDB_ENQUEUE_NORMAL:			return TEXT("button.enqueue");
		case IDB_ENQUEUE_HIGHLIGHTED:	return TEXT("button.enqueue.highlighted");
		case IDB_ENQUEUE_PRESSED:		return TEXT("button.enqueue.pressed");
		case IDB_ENQUEUE_DISABLED:		return TEXT("button.enqueue.disabled");
		case IDB_EJECT2_NORMAL:			return TEXT("button.eject");
		case IDB_EJECT2_HIGHLIGHTED:		return TEXT("button.eject.highlighted");
		case IDB_EJECT2_PRESSED:			return TEXT("button.eject.pressed");
		case IDB_EJECT2_DISABLED:		return TEXT("button.eject.disabled");
	}
	return NULL;
}

static HMLIMGLST DataCmdBar_CreateImageList()
{
	HMLIMGLST hmlil;
	MLIMAGELISTCREATE mlilCreate;
	MLIMAGESOURCE mlis;
	MLIMAGELISTITEM mlilItem;
				
	mlilCreate.cx = g_view_metaconf->ReadIntEx(TEXT("artwork"), TEXT("button.icon.cx"), 12);
	mlilCreate.cy = g_view_metaconf->ReadIntEx(TEXT("artwork"), TEXT("button.icon.cy"), 12);
	mlilCreate.cInitial = 12;
	mlilCreate.cGrow = 3;
	mlilCreate.cCacheSize = 4;
	mlilCreate.flags = MLILC_COLOR32;

	hmlil = MLImageList_Create(plugin.hwndLibraryParent, &mlilCreate);
	if (NULL == hmlil) return NULL;
	
	
	ZeroMemory(&mlilItem, sizeof(MLIMAGELISTITEM));
	mlilItem.cbSize		= sizeof(MLIMAGELISTITEM);
	mlilItem.hmlil		= hmlil;
	mlilItem.filterUID	= MLIF_BUTTONBLENDPLUSCOLOR_UID;
	mlilItem.pmlImgSource = &mlis;

	ZeroMemory(&mlis, sizeof(MLIMAGESOURCE));
	mlis.cbSize		= sizeof(MLIMAGESOURCE);
	mlis.type		= SRC_TYPE_PNG;
	mlis.hInst		= plugin.hDllInstance;
	

	INT imageList[] = 
	{	IDB_PLAY_NORMAL, IDB_PLAY_PRESSED, IDB_PLAY_HIGHLIGHTED, IDB_PLAY_DISABLED,
		IDB_ENQUEUE_NORMAL, IDB_ENQUEUE_PRESSED, IDB_ENQUEUE_HIGHLIGHTED, IDB_ENQUEUE_DISABLED,
		IDB_EJECT2_NORMAL, IDB_EJECT2_PRESSED, IDB_EJECT2_HIGHLIGHTED, IDB_EJECT2_DISABLED,
	};
	
	
	TCHAR szResource[MAX_PATH] = {0}, szPath[MAX_PATH] = {0}, szFullPath[MAX_PATH] = {0};
	g_view_metaconf->ReadCchStringEx(szPath, ARRAYSIZE(szPath), TEXT("artwork"), TEXT("path"), NULL);
	for(int i = 0; i < sizeof(imageList)/sizeof(imageList[0]); i++) 
	{
		mlilItem.nTag	= imageList[i];
		g_view_metaconf->ReadCchStringEx(szResource, ARRAYSIZE(szResource), TEXT("artwork"), GetImageTagStr(imageList[i]), NULL);
		if (TEXT('\0') != szResource[0]) 
		{	
			PathCombine(szFullPath, szPath, szResource);
			mlis.lpszName = szFullPath;
			mlis.flags |= ISF_LOADFROMFILE;
		}
		else 
		{
			mlis.lpszName = MAKEINTRESOURCE(imageList[i]);
			mlis.flags &= ~ISF_LOADFROMFILE;
		}

		MLImageList_Add(plugin.hwndLibraryParent, &mlilItem);
	}
	return hmlil;
}

static HMLIMGLST DataCmdBar_CreateDropDownImageList(HMENU hMenu)
{
	HMLIMGLST hmlil;
	MLIMAGELISTCREATE mlilCreate;
	MLIMAGESOURCE mlis;
				
	if (!hMenu) return NULL;

	mlilCreate.cx = 16;
	mlilCreate.cy = 16;
	mlilCreate.cInitial = 2;
	mlilCreate.cGrow = 1;
	mlilCreate.cCacheSize = 3;
	mlilCreate.flags = MLILC_COLOR32;

	hmlil = MLImageList_Create(plugin.hwndLibraryParent, &mlilCreate);
	if (NULL == hmlil) return NULL;
		
	ZeroMemory(&mlis, sizeof(MLIMAGESOURCE));
	mlis.cbSize		= sizeof(MLIMAGESOURCE);
	mlis.type		= SRC_TYPE_PNG;
	mlis.hInst		= plugin.hDllInstance;
	

	INT imageList[] = { IDB_PLAY_MENU, IDB_ENQUEUE_MENU, };
	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_ID;
	for(int i = 0; i < sizeof(imageList)/sizeof(imageList[0]); i++) 
	{		
		if (GetMenuItemInfoW(hMenu, i, TRUE, &mii))
		{
			mlis.lpszName	= MAKEINTRESOURCEW(imageList[i]);
			MLImageList_Add2(plugin.hwndLibraryParent, hmlil, MLIF_FILTER1_UID, &mlis, mii.wID);
		}
	}
	return hmlil;

}



static void DataCmdBar_SetButtonImages(HWND hButton, HMLIMGLST hmlil, INT normal, INT hover, INT pressed, INT disabled)
{
	MLBUTTONIMAGELIST bil;
	MLIMAGELISTTAG t;
	bil.hmlil = hmlil;
	t.hmlil = bil.hmlil;
	
	t.nTag = normal;
	bil.normalIndex	= (MLImageList_GetIndexFromTag(plugin.hwndLibraryParent, &t)) ? t.mlilIndex : -1;
	
	if (disabled == normal) bil.disabledIndex = bil.normalIndex;
	else
	{
		t.nTag = disabled;
		bil.disabledIndex = (MLImageList_GetIndexFromTag(plugin.hwndLibraryParent, &t)) ? t.mlilIndex : bil.normalIndex;
	}
	
	if (hover == normal) bil.hoverIndex = bil.normalIndex;
	else if (hover == disabled) bil.hoverIndex = bil.disabledIndex;
	else
	{
		t.nTag = hover;
		bil.hoverIndex = (MLImageList_GetIndexFromTag(plugin.hwndLibraryParent, &t)) ? t.mlilIndex : bil.normalIndex;
	}

	if (pressed == normal) bil.pressedIndex = bil.normalIndex;
	else if (pressed == disabled) bil.pressedIndex = bil.disabledIndex;
	else if (pressed == hover) bil.pressedIndex = bil.hoverIndex;
	else
	{
		t.nTag = pressed;
		bil.pressedIndex = (MLImageList_GetIndexFromTag(plugin.hwndLibraryParent, &t)) ? t.mlilIndex : bil.normalIndex;
	}

	SENDMLIPC(hButton, ML_IPC_SKINNEDBUTTON_SETIMAGELIST, (LPARAM)&bil);
}

static void PlayEx_Initialize(HWND hdlg)
{
	HWND hButton = GetDlgItem(hdlg, IDC_BTN_PLAYEX);
	if (!hButton) return;

	HWND hFileView = (HWND)CommandBar_GetData(hdlg);
	if (NULL == hFileView) return;

	HMENU hMenu = FileView_GetMenu(hFileView, FVMENU_PLAY);
	if (!hMenu) return;

	BOOL bPlay = (!hFileView || 0 == (FVS_ENQUEUE & FileView_GetStyle(hFileView)));

	WCHAR szBuffer[256] = {0};
	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_STRING;
	mii.dwTypeData = szBuffer;
	mii.cch = sizeof(szBuffer)/sizeof(szBuffer[0]);
	if (GetMenuItemInfoW(hMenu, (bPlay) ? 0 : 1, TRUE, &mii))
	{
		while(mii.cch && L'\t' != szBuffer[mii.cch]) mii.cch--; 
		if (mii.cch > 0) szBuffer[mii.cch] = L'\0'; 
		SetWindowTextW(hButton, szBuffer);
	}

	if (bPlay)
	{
		DataCmdBar_SetButtonImages(hButton, hmlilButton, IDB_PLAY_NORMAL, 
					IDB_PLAY_PRESSED, IDB_PLAY_HIGHLIGHTED, IDB_PLAY_DISABLED);
	}
	else
	{
		DataCmdBar_SetButtonImages(hButton, hmlilButton, IDB_ENQUEUE_NORMAL, 
					IDB_ENQUEUE_PRESSED, IDB_ENQUEUE_HIGHLIGHTED, IDB_ENQUEUE_DISABLED);
	}
}

static void DataCmdBar_UpdateControls(HWND hdlg, BOOL bRedraw)
{
	INT buttonList[] = { IDC_BTN_PLAYEX, IDC_BTN_COPY, };
	HDWP hdwp;
	DWORD flags, size;
	INT w;

	flags = SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | ((bRedraw) ? 0 : SWP_NOREDRAW);
	hdwp = BeginDeferWindowPos(sizeof(buttonList)/sizeof(buttonList[0]));
	if (!hdwp) return;

	for(int i =0; i < sizeof(buttonList)/sizeof(buttonList[0]); i++)
	{
		HWND hctrl = GetDlgItem(hdlg, buttonList[i]);
		if (NULL != hctrl)
		{
			size = 0;
			switch(buttonList[i])
			{
				case IDC_BTN_PLAYEX:
					{
						HWND hFileView = (HWND)CommandBar_GetData(hdlg);
						if (NULL == hFileView) return;

						HMENU hMenu = FileView_GetMenu(hFileView, FVMENU_PLAY);
						if (hMenu)
						{
							WCHAR szText[256] = {0};
							INT count = GetMenuItemCount(hMenu);
							MENUITEMINFO mii = {0};

							mii.cbSize = sizeof(MENUITEMINFO);
							mii.fMask = MIIM_STRING;
							mii.dwTypeData = szText;

							w = 0;
							for (int i = 0; i < count; i++)
							{
								mii.cch = sizeof(szText)/sizeof(szText[0]);
								if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
								{
									while(mii.cch && L'\t' != szText[mii.cch]) mii.cch--; 
									if (mii.cch > 0) szText[mii.cch] = L'\0'; 
									size = MLSkinnedButton_GetIdealSize(hctrl, szText);
									if (w < LOWORD(size)) w = LOWORD(size);
								}
							}
							size = MAKELONG(w + 8, HIWORD(size));
						}
					}
					break;
				default:
					size = MLSkinnedButton_GetIdealSize(hctrl, NULL);
					break;
			}

			INT width = LOWORD(size), height = HIWORD(size);
			if (width < 82) width = 82;
			if (height < 14) height = 14;
			
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, 0, 0, width, height, flags);
		}
	}

	EndDeferWindowPos(hdwp);

	SetWindowPos(hdlg, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE |
					SWP_NOZORDER | SWP_FRAMECHANGED | ((bRedraw) ? 0 : SWP_NOREDRAW));
}

static BOOL DataCmdBar_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_AUTO;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hdlg;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);
	
	
	sw.hwndToSkin = GetDlgItem(hdlg, IDC_BTN_EJECT);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	sw.style |= SWBS_SPLITBUTTON;
	sw.hwndToSkin = GetDlgItem(hdlg, IDC_BTN_PLAYEX);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);
	
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hdlg, IDC_BTN_COPY);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);
	

	sw.skinType = SKINNEDWND_TYPE_STATIC;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
    sw.hwndToSkin = GetDlgItem(hdlg, IDC_LBL_STATUS);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	hmlilButton = DataCmdBar_CreateImageList();

	PlayEx_Initialize(hdlg);

	DataCmdBar_SetButtonImages(GetDlgItem(hdlg, IDC_BTN_EJECT), hmlilButton, IDB_EJECT2_NORMAL, 
						IDB_EJECT2_PRESSED, IDB_EJECT2_HIGHLIGHTED, IDB_EJECT2_DISABLED);

	DataCmdBar_UpdateControls(hdlg, FALSE);
	return FALSE;
}

static void DataCmdBar_OnDestroy(HWND hdlg)
{	
	if (hmlilButton)
	{
		MLImageList_Destroy(plugin.hwndLibraryParent, hmlilButton);
		hmlilButton = NULL;
	}
}

static void DataCmdBar_OnWindowPosChanged(HWND hdlg, WINDOWPOS *pwp)
{
	if (0 == (SWP_NOSIZE & pwp->flags) || 0 != (SWP_FRAMECHANGED & pwp->flags))
	{
		HWND hctrl;
		HDWP hdwp;
		RECT rc, rw;
		DWORD flags;

		if (!GetClientRect(hdlg, &rc)) return;
		InflateRect(&rc, -2, 0);
		LONG left = rc.left-2;
		LONG right = rc.right;

		hdwp = BeginDeferWindowPos(4);
		if (!hdwp) return;

		flags = SWP_NOACTIVATE | SWP_NOZORDER | ((SWP_NOREDRAW | SWP_NOCOPYBITS) & pwp->flags);

		if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_PLAYEX)) && GetWindowRect(hctrl, &rw))
		{
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, left, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags);
			left += ((rw.right - rw.left) + 8);
		}

		if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_COPY)) && GetWindowRect(hctrl, &rw))
		{
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, left, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags);
			left += ((rw.right - rw.left) + 8);
		}

		if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_EJECT)) && GetWindowRect(hctrl, &rw))
		{			
			right -= (rw.right - rw.left);
			if (right < (left + 16)) right = left + 16;

			hdwp = DeferWindowPos(hdwp, hctrl, NULL, right, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags);
			right -= 4;
		}

		if (NULL != (hctrl = GetDlgItem(hdlg, IDC_LBL_STATUS)) && GetWindowRect(hctrl, &rw))
		{				
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, left, rc.top, right - left, rc.bottom - rc.top, flags);
		}
		EndDeferWindowPos(hdwp);
	}
	if (0 == (SWP_NOREDRAW & pwp->flags)) InvalidateRect(GetDlgItem(hdlg, IDC_LBL_STATUS), NULL, TRUE);
}

static void DataCmdBar_OnPlayDropDown(HWND hdlg, HWND hctrl)
{
	RECT r;
	if (!GetWindowRect(hctrl, &r)) return;

	HWND hFileView = (HWND)CommandBar_GetData(hdlg);
	if (NULL == hFileView) return;


	HMENU hMenu = FileView_GetMenu(hFileView, FVMENU_PLAY);
	if (!hMenu) return;
	
	
	MLSkinnedButton_SetDropDownState(hctrl, TRUE);

	HMLIMGLST hmlilDropDown = DataCmdBar_CreateDropDownImageList(hMenu);

	MLTrackSkinnedPopupMenuEx(plugin.hwndLibraryParent, hMenu, 
								TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_NONOTIFY, 
								r.left, r.top - 2, hFileView, NULL, hmlilDropDown, r.right - r.left, 
								SMS_USESKINFONT, NULL, 0L);

	MLSkinnedButton_SetDropDownState(hctrl, FALSE);
	MLImageList_Destroy(plugin.hwndLibraryParent, hmlilDropDown);
}

static void DataCmdBar_OnPlayClick(HWND hdlg, HWND hButton)
{
	EnableWindow(hButton, FALSE);

	HWND hFileView = (HWND)CommandBar_GetData(hdlg);
	if (NULL != hFileView)
	{
		HMENU hMenu = FileView_GetMenu(hFileView, FVMENU_PLAY);
		if (NULL != hMenu)
		{			
			UINT uCmd = (FVS_ENQUEUE & FileView_GetStyle(hFileView)) ? FVA_ENQUEUE : FVA_PLAY;
			SendMessageW(hFileView, WM_COMMAND, MAKEWPARAM(FileView_GetActionCommand(hFileView, uCmd), 0), 0L);
		}
	}

	EnableWindow(hButton, TRUE);

}

static void DataCmdBar_OnCommand(HWND hdlg, INT eventId, INT ctrlId, HWND hwndCtrl)
{
	switch (ctrlId)
	{				
		case IDC_BTN_PLAYEX:
			switch(eventId)
			{
				case MLBN_DROPDOWN:	DataCmdBar_OnPlayDropDown(hdlg, hwndCtrl); break;
				case BN_CLICKED:	DataCmdBar_OnPlayClick(hdlg, hwndCtrl); break;
			}
			break;
		case IDC_BTN_EJECT:
			switch(eventId)
			{
				case BN_CLICKED: SendMessageW(GetParent(hdlg), WM_COMMAND, MAKEWPARAM(ID_EJECT_DISC, 0), 0L); break; // straight to container...
			}
			break;
		case IDC_BTN_COPY:
			switch(eventId)
			{
				case BN_CLICKED: SendMessageW(hdlg, WM_COMMAND, MAKEWPARAM(ID_COPY_SELECTION, 0), 0L); break;
			}
			break;
	}
}


static INT DataCmdBar_OnGetBestHeight(HWND hdlg)
{
	INT h, height = 0;
	INT buttonList[] = { IDC_BTN_PLAYEX, };

	for(int i =0; i < sizeof(buttonList)/sizeof(buttonList[0]); i++)
	{
		HWND hctrl = GetDlgItem(hdlg, buttonList[i]);
		if (NULL != hctrl)
		{
			DWORD sz = MLSkinnedButton_GetIdealSize(hctrl, NULL);
			h = HIWORD(sz);
			if (height < h) height = h;
		}
	}

	return height;
}

INT_PTR WINAPI DataCmdBar_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:			return DataCmdBar_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:				DataCmdBar_OnDestroy(hdlg); break;
		case WM_WINDOWPOSCHANGED:	DataCmdBar_OnWindowPosChanged(hdlg, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:				DataCmdBar_OnCommand(hdlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam); break;
		case CBM_GETBESTHEIGHT:		SetWindowLongPtrW(hdlg, DWLP_MSGRESULT, DataCmdBar_OnGetBestHeight(hdlg)); return TRUE;
		case WM_DISPLAYCHANGE:		
			PlayEx_Initialize(hdlg); 
			break;
		case WM_SETFONT:				
			DataCmdBar_UpdateControls(hdlg, LOWORD(lParam)); 
			return 0;
		case WM_SETTEXT:
		case WM_GETTEXT:
		case WM_GETTEXTLENGTH:
			SetWindowLongPtrW(hdlg, DWLP_MSGRESULT, (LONGX86)(LONG_PTR)SendDlgItemMessageW(hdlg, IDC_LBL_STATUS, uMsg, wParam, lParam));
			return TRUE;
	
	}
	
	return 0;
}