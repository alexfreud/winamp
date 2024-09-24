#include "precomp__gen_ff.h"
#include "resource.h"
#include <windowsx.h>

extern int m_are_we_loaded;
extern int toggle_from_wa2;
extern HWND subWnd;

#define ListBox_AddStringW(hwndCtl, lpsz)            ((int)(DWORD)SendMessageW((hwndCtl), LB_ADDSTRING, 0L, (LPARAM)(LPCWSTR)(lpsz)))
#define ListBox_FindStringW(hwndCtl, indexStart, lpszFind) ((int)(DWORD)SendMessageW((hwndCtl), LB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCWSTR)(lpszFind)))
#define ListBox_GetTextW(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SendMessageW((hwndCtl), LB_GETTEXT, (WPARAM)(int)(index), (LPARAM)(LPWSTR)(lpszBuffer)))
#define ListBox_GetItemDataW(hwndCtl, index)         ((LRESULT)(ULONG_PTR)SendMessageW((hwndCtl), LB_GETITEMDATA, (WPARAM)(int)(index), 0L))
#define ListBox_SetItemDataW(hwndCtl, index, data)   ((int)(DWORD)SendMessageW((hwndCtl), LB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))


static void fillColorThemesList(HWND list)
{
	ListBox_ResetContent(list);
	if (!m_are_we_loaded) return ;
	int numsets = WASABI_API_SKIN->colortheme_getNumColorSets();
	for (int i = 0; i < numsets; i++)
	{
		const wchar_t *set = WASABI_API_SKIN->colortheme_enumColorSet(i);
		if (!_wcsnicmp(set, L"{coloredit}", 11))
		{
			int pos = ListBox_AddStringW(list, set + 11);
			ListBox_SetItemDataW(list, pos, 1);
		}
		else
		{
			ListBox_AddStringW(list, set);
		}
	}
	const wchar_t *curset = WASABI_API_SKIN->colortheme_getColorSet();
	int cur = ListBox_FindStringW(list, 0, curset);
	ListBox_SetCurSel(list, cur);
}

INT_PTR CALLBACK ffPrefsProc3(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFYFORMAT:
		return NFR_UNICODE;
	case WM_INITDIALOG:
		EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_SETTHEME), m_are_we_loaded);
		{
			HWND listWindow;
			listWindow = GetDlgItem(hwndDlg, IDC_LIST1);
			if (NULL != listWindow)
			{
				EnableWindow(listWindow, m_are_we_loaded);
				fillColorThemesList(listWindow);
				if (NULL != WASABI_API_APP)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);
			}
		}
		return 1;
	case WM_COMMAND:
		{
			toggle_from_wa2 = 1;
			int id = (int) LOWORD(wParam);
			int msg = (int)HIWORD(wParam);
			if (id == IDC_LIST1 && msg == LBN_DBLCLK || id == IDC_BUTTON_SETTHEME)
			{
				HWND ctrl = GetDlgItem(hwndDlg, IDC_LIST1);
				int sel = ListBox_GetCurSel(ctrl);
				if (sel != -1)
				{
					wchar_t newset[256 + 11] = L"";
					ListBox_GetTextW(ctrl, sel, newset);
					newset[255] = 0;
					if (*newset)
					{
						int p = ListBox_GetItemDataW(ctrl, sel);
						if (p)
						{
							WCSCPYN(newset, StringPrintfW(L"{coloredit}%s", newset), 256 + 11);
						}
						WASABI_API_SKIN->colortheme_setColorSet(newset);
					}
				}
				return 0;
			}
			toggle_from_wa2 = 0;
			break;
		}
	case WM_DESTROY:
		subWnd = NULL;
		if (NULL != WASABI_API_APP)
		{
			HWND listWindow;
			listWindow = GetDlgItem(hwndDlg, IDC_LIST1);
			if (NULL != listWindow)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
		}
		return 0;
	}
	return 0;
}

