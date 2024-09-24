#include "./skinning.h"
#include "./skinneddlg.h"
#include "./skinnedheader.h"
#include "./skinnedlistview.h"
#include "./skinnedbutton.h"
#include "./skinneddivider.h"
#include "./skinnededit.h"
#include "./skinnedstatic.h"
#include "./skinnedlistbox.h"
#include "./skinnedcombo.h"
#include "./skinnedfolder.h"
#include "./skinnedmenu.h"
#include "./skinnedtooltip.h"
#include "./skinnedprogressbar.h"

#include "./config.h"
extern C_Config *g_config;

BOOL SkinWindow(HWND hwndToSkin, UINT style)
{
	return SkinWindowEx(hwndToSkin, SKINNEDWND_TYPE_AUTO, style);
}

BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style)
{
	SkinnedWnd *psw;
	if (!hwndToSkin || !IsWindow(hwndToSkin) || SkinnedWnd::GetFromHWND(hwndToSkin)) return FALSE;
	
	if (SKINNEDWND_TYPE_AUTO == type)
	{
		wchar_t szName[256] = {0};
		
		if (!RealGetWindowClassW(hwndToSkin, szName, sizeof(szName)/sizeof(wchar_t))) 
			return FALSE;
		
		if (0== lstrcmpW(szName, L"#32770")) type = SKINNEDWND_TYPE_DIALOG;
		else if (0== lstrcmpW(szName, WC_HEADERW)) type = SKINNEDWND_TYPE_HEADER;
		else if (0== lstrcmpW(szName, WC_LISTVIEWW)) type = SKINNEDWND_TYPE_LISTVIEW;
		else if (0== lstrcmpW(szName, WC_BUTTONW)) type = SKINNEDWND_TYPE_BUTTON;
		else if (0== lstrcmpW(szName, WC_EDITW)) type = SKINNEDWND_TYPE_EDIT;
		else if (0== lstrcmpW(szName, WC_STATICW)) type = SKINNEDWND_TYPE_STATIC;
		else if (0== lstrcmpW(szName, WC_LISTBOXW)) type = SKINNEDWND_TYPE_LISTBOX;
		else if (0== lstrcmpW(szName, WC_COMBOBOXW)) type = SKINNEDWND_TYPE_COMBOBOX;
		else if (0== lstrcmpW(szName, TOOLTIPS_CLASSW)) type = SKINNEDWND_TYPE_TOOLTIP;
		else if (0== lstrcmpW(szName, FOLDERBROWSER_NAME)) type = SKINNEDWND_TYPE_FOLDERBROWSER;
		else if (0== lstrcmpW(szName, PROGRESS_CLASSW)) type = SKINNEDWND_TYPE_PROGRESSBAR;
		else 
		{
			if (0 != ((WS_VSCROLL | WS_HSCROLL) & GetWindowLongPtrW(hwndToSkin, GWL_STYLE)))
				type = SKINNEDWND_TYPE_SCROLLWND;
			else
				type = SKINNEDWND_TYPE_WINDOW;
		}
	}

	switch(type)
	{
		case SKINNEDWND_TYPE_WINDOW: psw = new SkinnedWnd(FALSE); break;
		case SKINNEDWND_TYPE_SCROLLWND: psw = new SkinnedScrollWnd(FALSE); break;
		case SKINNEDWND_TYPE_DIALOG: psw = new SkinnedDialog(); break;
		case SKINNEDWND_TYPE_HEADER: psw = new SkinnedHeader(); break;
		case SKINNEDWND_TYPE_LISTVIEW: psw = new SkinnedListView(); 	break;
		case SKINNEDWND_TYPE_BUTTON: psw = new SkinnedButton(); 	break;
		case SKINNEDWND_TYPE_DIVIDER: psw = new SkinnedDivider(); break;
		case SKINNEDWND_TYPE_EDIT: psw = new SkinnedEdit(); break;
		case SKINNEDWND_TYPE_STATIC: psw = new SkinnedStatic(); break;
		case SKINNEDWND_TYPE_LISTBOX: psw = new SkinnedListbox(); break;
		case SKINNEDWND_TYPE_COMBOBOX: psw = new SkinnedCombobox(); break;
		case SKINNEDWND_TYPE_FOLDERBROWSER: psw = new SkinnedFolderBrowser(); break;
		case SKINNEDWND_TYPE_TOOLTIP: psw = new SkinnedToolTip(); break;
		case SKINNEDWND_TYPE_PROGRESSBAR: psw = new SkinnedProgressBar(); break;
		default: psw = NULL; break;
	}

	if (!psw) return FALSE;
	
	if (!psw->Attach(hwndToSkin))
	{
		delete(psw);
		return FALSE;
	}
	psw->SetStyle(style, FALSE);
	
	return TRUE;
}

BOOL UnskinWindow(HWND hwndToUnskin)
{
	SkinnedWnd *psw;
	
	psw = SkinnedWnd::GetFromHWND(hwndToUnskin);
	if (!psw) return FALSE;
	delete (psw);

	return TRUE;
}

BOOL TrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, 
							INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
{	
	if (!hmenu) return FALSE;
	SkinnedMenu skinnedMenu;
	return skinnedMenu.TrackMenuPopupEx(hmenu, fuFlags, x, y, hwnd, lptpm, skinStyle, hmlil, width, customProc, customParam);
}

HANDLE InitSkinnedPopupHook(HWND hwndOwner, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
{
	SkinnedMenu *psm = new SkinnedMenu();
	if (NULL != psm && 
		FALSE == psm->InitializeHook(hwndOwner, skinStyle, hmlil, width, customProc, customParam))
	{
		delete(psm);
		psm = NULL;
	}

	return (HANDLE)psm;
}

void RemoveSkinnedPopupHook(HANDLE hPopupHook)
{
	SkinnedMenu *psm = (SkinnedMenu*)hPopupHook;
	if (NULL != psm)
		delete(psm);
}

#define SKINNEDMENU_DEFAULT		TRUE
BOOL IsSkinnedPopupEnabled(BOOL fIgnoreCache)
{
	static INT fUseSkinnedMenus = -1;

	if (FALSE != fIgnoreCache)
		fUseSkinnedMenus = -1;

	if (-1 == fUseSkinnedMenus)
	{
		fUseSkinnedMenus = (NULL != g_config) ? 
						(0 != g_config->ReadInt(L"skinned_menus", SKINNEDMENU_DEFAULT)) :
						SKINNEDMENU_DEFAULT;
	}

	return (FALSE != fUseSkinnedMenus);
}

BOOL EnableSkinnedPopup(BOOL fEnable)
{
	if (NULL == g_config) 
		return FALSE;

	g_config->WriteInt(L"skinned_menus", (FALSE != fEnable));
	return (fEnable == IsSkinnedPopupEnabled(TRUE));
}