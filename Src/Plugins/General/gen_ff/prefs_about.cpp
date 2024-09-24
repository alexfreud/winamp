#include "precomp__gen_ff.h"
#include "gen.h"
#include "resource.h"
#include "menuactions.h"
#include "wa2frontend.h"
#include "../Agave/Language/api_language.h"

extern const wchar_t *getSkinInfoW();
extern int m_are_we_loaded;
ifc_window *skin_about_group = NULL;

void destroyskinabout()
{
	if (skin_about_group)
		WASABI_API_SKIN->group_destroy(skin_about_group);
	skin_about_group = NULL;
}

INT_PTR CALLBACK ffPrefsProc5(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		destroyskinabout();
		if (m_are_we_loaded)
		{
			if (WASABI_API_SKIN->group_exists(L"skin.about.group"))
			{
				skin_about_group = WASABI_API_SKIN->group_create(L"skin.about.group");
				if (skin_about_group)
				{
					skin_about_group->setVirtual(0);
					HWND w = GetDlgItem(hwndDlg, IDC_STATIC_GROUP);
					skin_about_group->setStartHidden(1);
					skin_about_group->init(WASABI_API_WND->main_getRootWnd(), 1);
					SetWindowLong(skin_about_group->gethWnd(), GWL_STYLE, GetWindowLong(skin_about_group->gethWnd(), GWL_STYLE) | WS_CHILD);
					SetParent(skin_about_group->gethWnd(), w);
					SetWindowLong(w, GWL_STYLE, GetWindowLong(w, GWL_STYLE) | WS_CLIPCHILDREN);
					RECT r;
					GetClientRect(w, &r);
					skin_about_group->resize(r.left, r.top, r.right - r.left, r.bottom - r.top);
					skin_about_group->setVisible(1);
					ShowWindow(skin_about_group->gethWnd(), SW_NORMAL);
					ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_EMPTY), SW_HIDE);
				}
				else
				{
					SetDlgItemTextA(hwndDlg, IDC_STATIC_EMPTY, WASABI_API_LNGSTRING(IDS_ERROR_WHILE_LOADING_SKIN_WINDOW));
				}
			}
			else
			{
				SetDlgItemTextW(hwndDlg, IDC_STATIC_EMPTY, getSkinInfoW());
			}
		}
		else
		{
			SetDlgItemTextA(hwndDlg, IDC_STATIC_EMPTY, WASABI_API_LNGSTRING(IDS_NO_SKIN_LOADED));
		}
		return 1;
	case WM_DESTROY:
		destroyskinabout();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
	case IDOK: case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return 0;
		case IDC_BUTTON_SKINSPECIFIC:
			extern void unpopulateWindowsMenus();
			extern TList<HMENU> menulist;
			HMENU menu = CreatePopupMenu();
			unpopulateWindowsMenus();
			MenuActions::installSkinOptions(menu);
			menulist.addItem(menu);
			HWND w = GetDlgItem(hwndDlg, IDC_BUTTON_SKINSPECIFIC);
			RECT r;
			GetWindowRect(w, &r);
			int n = GetMenuItemCount(menu);
			if (n == 1)
			{
				HMENU submenu = GetSubMenu(menu, 0);
				if (submenu != NULL) menu = submenu;
			}
			else if (n == 0)
			{
				InsertMenuW(menu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, WASABI_API_LNGSTRINGW(IDS_NO_OPTIONS_AVAILABLE_FOR_THIS_SKIN));
			}
			//DoTrackPopup(menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON, r.left, r.top, wa2.getMainWindow());
			TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON, r.left, r.top, 0, wa2.getMainWindow(), NULL);
			MenuActions::removeSkinOptions();
			return 0;
		}
		break;
	}
	return 0;
}