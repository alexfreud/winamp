#include "ConfigDlg.h"
#include "gen_hotkeys.h"
#include "accelBlock.h"

#define ListView_InsertColumnW(hwnd, iCol, pcol) \
    (int)SNDMSG((hwnd), LVM_INSERTCOLUMNW, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMNW *)(pcol))

#define ListView_InsertItemW(hwnd, pitem)   \
    (int)SNDMSG((hwnd), LVM_INSERTITEMW, 0, (LPARAM)(const LV_ITEMW *)(pitem))

#define ListView_SetItemTextW(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEMW _macro_lvi;\
  _macro_lvi.iSubItem = (iSubItem_);\
  _macro_lvi.pszText = (pszText_);\
  SNDMSG((hwndLV), LVM_SETITEMTEXTW, (WPARAM)(i), (LPARAM)(LV_ITEMW *)&_macro_lvi);\
}

#define ListView_SetItemW(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_SETITEMW, 0, (LPARAM)(LV_ITEMW *)(pitem))

void SetListItem(HWND hwHKList, HWND hwHK, HOTKEY_DATA *hk_data, int idx = -1, int failed = 0);

static AcceleratorBlocker *pAccelBlock = NULL;
static bool changed = false;

static int setCheckedStuff(HWND hwndDlg)
{
	int checked = SendMessage(GetDlgItem(hwndDlg, IDC_ENABLED), BM_GETCHECK, 0, 0) == BST_CHECKED;
	EnableWindow(GetDlgItem(hwndDlg,IDC_HKLIST),checked);
	EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO),checked);
	EnableWindow(GetDlgItem(hwndDlg,IDC_HOTKEY),checked);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ADD),checked);
	if (!checked)
	{
		EnableWindow(GetDlgItem(hwndDlg,IDC_SAVE),0);
		EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVE),0);
	}
	EnableWindow(GetDlgItem(hwndDlg,IDC_DEFAULT),checked);
	return checked;
}

int ResizeComboBoxDropDown(HWND hwndDlg, UINT id, const char* str, int width){
	SIZE size = {0};
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	// get and select parent dialog's font so that it'll calculate things correctly
	HFONT font = (HFONT)SendMessage(hwndDlg,WM_GETFONT,0,0), oldfont = (HFONT)SelectObject(hdc,font);
	GetTextExtentPoint32(hdc, str, lstrlen(str)+1, &size);

	int ret = width;
	if(size.cx > width)
	{
		SendDlgItemMessageW(hwndDlg, id, CB_SETDROPPEDWIDTH, size.cx, 0);
		ret = size.cx;
	}

	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return ret;
}

int ResizeComboBoxDropDownW(HWND hwndDlg, UINT id, const wchar_t *str, int width){
	SIZE size = {0};
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	// get and select parent dialog's font so that it'll calculate things correctly
	HFONT font = (HFONT)SendMessage(hwndDlg,WM_GETFONT,0,0), oldfont = (HFONT)SelectObject(hdc,font);
	GetTextExtentPoint32W(hdc, str, (int)wcslen(str)+1, &size);

	int ret = width;
	if(size.cx > width)
	{
		SendDlgItemMessageW(hwndDlg, id, CB_SETDROPPEDWIDTH, size.cx, 0);
		ret = size.cx;
	}

	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return ret;
}

BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	HWND hwHKList = GetDlgItem(hwndDlg, IDC_HKLIST);
	HWND hwCombo = GetDlgItem(hwndDlg, IDC_COMBO);
	HWND hwHK = GetDlgItem(hwndDlg, IDC_HOTKEY);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			int colwidth1 = -1, colwidth2 = -1;
			if (hwHKList && hwHK)
			{
				RECT r;

				SendMessage(hwHKList, WM_SETREDRAW, FALSE, 0);
				ListView_SetExtendedListViewStyle(hwHKList, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

				GetClientRect(hwHKList, &r);

				colwidth2 = GetPrivateProfileIntW(L"gen_hotkeys", L"col2", -1, g_iniFile);
				LVCOLUMNW lc = {LVCF_TEXT | LVCF_WIDTH, 0,
							    (colwidth2==-1?((r.right / 2) - GetSystemMetrics(SM_CYHSCROLL)):colwidth2),
							    WASABI_API_LNGSTRINGW(IDS_GHK_HOTKEY_COLUMN), 0, 0
							  };

				ListView_InsertColumnW(hwHKList, 0, &lc);

				lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
				colwidth1 = GetPrivateProfileIntW(L"gen_hotkeys", L"col1", -1, g_iniFile);
				lc.cx = (colwidth1==-1?(r.right - lc.cx - GetSystemMetrics(SM_CYHSCROLL)):colwidth1);
				lc.pszText = WASABI_API_LNGSTRINGW(IDS_GHK_ACTION_COLUMN);
				lc.iSubItem = 1;

				ListView_InsertColumnW(hwHKList, 0, &lc);
			}

			changed = false;
			SubclassEditBox(hwHK);
			if (NULL != hwHK && NULL == pAccelBlock)
				pAccelBlock = new AcceleratorBlocker(hwHK);
			
			HBITMAP hBitmap = LoadBitmap(psPlugin.hDllInstance, MAKEINTRESOURCE(IDB_LVSTATE));

			HIMAGELIST hImageList = ImageList_Create(14, 14, ILC_COLOR8|ILC_MASK, 1, 0);
			ImageList_AddMasked(hImageList, hBitmap, RGB(252, 254, 252));
			ListView_SetImageList(hwHKList, hImageList, LVSIL_STATE);
			DeleteObject(hBitmap);

			ListView_SetItemCount(hwHKList, g_dwHotkeys);

			// We don't want our hot keys while in the config
			for (size_t i = 0; i < g_dwHotkeys; i++)
			{
				SetLastError(0);
				UnregisterHotkey(g_hotkeys + i);
				SetListItem(hwHKList, hwHK, &g_hotkeys[i].hkd, -1, g_hotkeys[i].failed);
			}
			hotkeysClear();

			if(colwidth1==-1)
				ListView_SetColumnWidth(hwHKList, 0, LVSCW_AUTOSIZE);
			if(colwidth2==-1)
				ListView_SetColumnWidth(hwHKList, 1, LVSCW_AUTOSIZE);

			// clear
			SendMessage(hwHK, wmHKCtlSet, 0, 0);

			ListView_SetItemState(hwHKList, -1, 0, LVIS_SELECTED);

			SendMessage(hwHKList, WM_SETREDRAW, TRUE, 0);

			int width = 0;
			for (size_t i = 0; i < GetCommandsNum(); i++)
			{
				bool unicode = 0;
				char *cmdname = GetCommandName((unsigned int)i, &unicode);
				if (*cmdname)
				{
					LRESULT pos;
					if (unicode)
					{
						pos = SendMessageW(hwCombo, CB_ADDSTRING, 0, (LPARAM) cmdname);
						width = ResizeComboBoxDropDownW(hwndDlg, IDC_COMBO, (wchar_t*)cmdname, width);
					}
					else
					{
						pos = SendMessageA(hwCombo, CB_ADDSTRING, 0, (LPARAM) cmdname);
						width = ResizeComboBoxDropDown(hwndDlg, IDC_COMBO, cmdname, width);
					}
					SendMessage(hwCombo, CB_SETITEMDATA, pos, i);
				}
			}

			// Set the enabled checkbox
			CheckDlgButton(hwndDlg,IDC_ENABLED,GetPrivateProfileIntW(L"gen_hotkeys", L"enabled", 0, g_iniFile));
			CheckDlgButton(hwndDlg,IDC_ENABLED_WM_APPCOMMAND,GetPrivateProfileIntW(L"gen_hotkeys", L"appcommand", 0, g_iniFile));
			setCheckedStuff(hwndDlg);

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(hwHKList, TRUE);

			break;
		}

		case WM_COMMAND:
		{
			// we don't want accelerator keys working when the hotkey window is focused
			// that could be a problem if someone wants to bind Alt+D for example...
			if (GetFocus() == hwHK)
				break;

			switch (LOWORD(wParam))
			{
				case IDC_ADD:
				{
					HOTKEY_DATA hkd;
					DWORD dwHotKey = (unsigned long)SendMessage(hwHK, wmHKCtlGet, 0, 0);
					UINT iCommand = (unsigned int)SendMessage(hwCombo, CB_GETITEMDATA, SendMessage(hwCombo, CB_GETCURSEL, 0, 0), 0);

					if (!dwHotKey || iCommand == CB_ERR)
						break;

					hkd.dwHotKey = dwHotKey;
					hkd.iCommand = iCommand;

					changed = true;
					SetListItem(hwHKList, hwHK, &hkd);
					break;
				}

				case IDC_REMOVE:
				{
					int i = ListView_GetNextItem(hwHKList, -1, LVNI_SELECTED);
					while (i != -1)
					{
						LVITEM lvi = {LVIF_PARAM, i};
						ListView_GetItem(hwHKList, &lvi);
						HOTKEY_DATA *hkd = (HOTKEY_DATA *) lvi.lParam;
						delete hkd;
            
						changed = true;
						ListView_DeleteItem(hwHKList, i);
						i = ListView_GetNextItem(hwHKList, -1, LVNI_SELECTED);
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), 0);
					break;
				}

				case IDC_SAVE:
				{
					if (ListView_GetSelectedCount(hwHKList) != 1)
						break;

					int iSel = ListView_GetNextItem(hwHKList, -1, LVNI_SELECTED);

					LVITEMW lvi = {LVIF_PARAM, iSel};
					ListView_GetItem(hwHKList, &lvi);
					HOTKEY_DATA *hkd = (HOTKEY_DATA *) lvi.lParam;
					if (hkd)
					{
						DWORD dwHotKey = (unsigned long)SendMessage(hwHK, wmHKCtlGet, 0, 0);
						UINT iCommand = (unsigned int)SendMessage(hwCombo, CB_GETITEMDATA, SendMessage(hwCombo, CB_GETCURSEL, 0, 0), 0);

						if (!dwHotKey || iCommand == CB_ERR)
							break;

						hkd->dwHotKey = dwHotKey;
						hkd->iCommand = iCommand;

						changed = true;
						SetListItem(hwHKList, hwHK, hkd, iSel);
					}
					break;
				}

				case IDC_ENABLED:
					if (setCheckedStuff(hwndDlg))
					{
						DWORD dwSelected = ListView_GetSelectedCount(hwHKList);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE), dwSelected == 1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), dwSelected);
						changed = true;
					}
					break;

				case IDC_DEFAULT:
				{
					DWORD i;
					SendMessage(hwndDlg, WM_SETREDRAW, FALSE, 0);

					DWORD dwCount = (DWORD)ListView_GetItemCount(hwHKList);
					if (dwCount)
					{
						for (i = 0; i < dwCount; i++)
						{
							LVITEM lvi = {LVIF_PARAM,(int)i};
							ListView_GetItem(hwHKList, &lvi);
							HOTKEY_DATA *hkd = (HOTKEY_DATA *) lvi.lParam;
							delete hkd;
						}
					}

					changed = true;
					ListView_DeleteAllItems(hwHKList);

					for (i = 0; i < DEFHKDS_NUM; i++)
					{
						SetListItem(hwHKList, hwHK, g_defhkds + i);
					}

					// restore the size of the columns on a reset as well
					ListView_SetColumnWidth(hwHKList, 0, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwHKList, 1, LVSCW_AUTOSIZE);

					ListView_SetItemState(hwHKList, -1, 0, LVIS_SELECTED);
					SendMessage(hwndDlg, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hwndDlg, 0, 0);
					break;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR nmhdr = (LPNMHDR) lParam;
			if (nmhdr && nmhdr->idFrom == IDC_HKLIST)
			{
				if (nmhdr->code == LVN_ITEMCHANGED)
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
					DWORD dwSelected = ListView_GetSelectedCount(hwHKList);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE), dwSelected == 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), dwSelected);

					SendMessage(hwHK, wmHKCtlSet, 0, 0);
					SendMessage(hwCombo, CB_SETCURSEL, -1, 0);

					if (dwSelected == 1 && (pnmv->uNewState & LVIS_SELECTED))
					{
						HOTKEY_DATA *hkd = (HOTKEY_DATA *) pnmv->lParam;
						if (hkd)
						{
							SendMessage(hwHK, wmHKCtlSet, hkd->dwHotKey, 0);
							bool unicode = 0;
							char *cmd = GetCommandName(hkd->iCommand, &unicode);
							if (*cmd)
							{
								if(unicode) SendMessageW(hwCombo, CB_SELECTSTRING, -1, (LPARAM) cmd);
								else SendMessageA(hwCombo, CB_SELECTSTRING, -1, (LPARAM) cmd);
							}
						}
					}
				}
				else if(nmhdr->code == LVN_KEYDOWN)
				{
					LPNMLVKEYDOWN pnmv = (LPNMLVKEYDOWN) lParam;
					if(pnmv->wVKey == VK_DELETE)
					{
						changed = true;
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_REMOVE,0),0);
					}
					else if(pnmv->wVKey == 'A' && GetAsyncKeyState(VK_CONTROL))
					{
						for(int i = 0; i < ListView_GetItemCount(hwHKList); i++)
						{
							ListView_SetItemState(hwHKList,i,LVIS_SELECTED,LVIS_SELECTED);
						}
					}
				}
			}
		}
			break;

		case WM_DESTROY:
		{
			int checked = (SendMessage(GetDlgItem(hwndDlg, IDC_ENABLED), BM_GETCHECK, 0, 0) == BST_CHECKED);
			writePrivateProfileInt(L"enabled", checked);
			writePrivateProfileInt(L"appcommand",
								   (SendMessage(GetDlgItem(hwndDlg, IDC_ENABLED_WM_APPCOMMAND), BM_GETCHECK, 0, 0) == BST_CHECKED));
			hotkeysClear();

			int iCount = ListView_GetItemCount(hwHKList);
			HOTKEY_DATA *hkds = NULL;
			if (iCount)
			{
				hkds = new HOTKEY_DATA[iCount]; // TODO: could alloca this
				memset(hkds, 0, iCount * sizeof(HOTKEY_DATA));
			}
			if (hkds || !iCount)
			{ 
				for (size_t i = 0; i < (unsigned int)iCount; i++)
				{
					LVITEM lvi = {LVIF_PARAM, (int)i};
					ListView_GetItem(hwHKList, &lvi);
					HOTKEY_DATA *hkd = (HOTKEY_DATA *) lvi.lParam;
					if (hkd)
					{
						hkds[i] = *hkd;
						delete hkd;
					}
				}

		        hotkeysLoad(hkds, iCount, checked);
				if (changed) hotkeysSave(hkds, iCount);
				delete [] hkds;
			}
			writePrivateProfileInt(L"col1", ListView_GetColumnWidth(hwHKList,0));
			writePrivateProfileInt(L"col2", ListView_GetColumnWidth(hwHKList,1));
			RegisterShellHookWindow(psPlugin.hwndParent);
			if (NULL != pAccelBlock && hwHK == pAccelBlock->GetHwnd())
			{
				delete(pAccelBlock);
				pAccelBlock = NULL;
			}

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(hwHKList, FALSE);
		}
			break;
	}
	return FALSE;
}

// idx = -1 for insertion
void SetListItem(HWND hwHKList, HWND hwHK, HOTKEY_DATA *hk_data, int idx, int failed)
{
	wchar_t szHK[1024] = {L""};

	SendMessage(hwHK, wmHKCtlSet, hk_data->dwHotKey, 0);
	GetWindowTextW(hwHK, szHK, sizeof(szHK));

	HOTKEY_DATA *hk_data_allocated = 0;

	if (idx < 0)
		hk_data_allocated = new HOTKEY_DATA; 

	if (idx >= 0 || hk_data_allocated)
	{
		if (idx < 0)
			*hk_data_allocated = *hk_data;

		int pos = ListView_GetItemCount(hwHKList);

		if (idx >= 0)
			pos = idx;

		// Add item to the list
		bool unicode = 0;
		LVITEM lvi = {
						LVIF_PARAM | LVIF_TEXT | LVIF_STATE,
						pos,
						0,
						LVIS_SELECTED | INDEXTOSTATEIMAGEMASK(failed ? (UINT)1 : 0) | LVIS_FOCUSED,
						LVIS_SELECTED | LVIS_STATEIMAGEMASK | LVIS_FOCUSED,
						GetCommandName(hk_data->iCommand, &unicode),
						0,
						0,
						(LPARAM) hk_data_allocated
					};

		char tmp[1024] = {0};

		if (!lvi.pszText || !*lvi.pszText)
		{
			if (hk_data->szCommand)
			{
				StringCchPrintf(tmp, 1024, "[%s]", hk_data->szCommand);
				lvi.pszText = tmp;
			}
		}
		if (!lvi.pszText || !*lvi.pszText)
			lvi.pszText = WASABI_API_LNGSTRING(IDS_GHK_ACTION_NOT_FOUND);

		if (idx >= 0)
		{
			lvi.mask ^= LVIF_PARAM;
			if(unicode) ListView_SetItemW(hwHKList, &lvi);
			else ListView_SetItem(hwHKList, &lvi);
		}
		else
		{
			ListView_SetItemState(hwHKList, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
			if(unicode) ListView_InsertItemW(hwHKList, &lvi);
			else ListView_InsertItem(hwHKList, &lvi);
		}
		ListView_SetItemTextW(hwHKList, pos, 1, szHK);
	}
}