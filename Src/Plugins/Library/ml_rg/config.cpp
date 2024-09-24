#include "main.h"
#include "resource.h"

int config_ask=1;
int config_ask_each_album=1;
int config_ignore_gained_album=0;

void DoButtons(HWND hwndDlg)
{
	config_ask = IsDlgButtonChecked(hwndDlg, IDC_ASK);
	config_ask_each_album = IsDlgButtonChecked(hwndDlg, IDC_ALBUM);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ALBUM), config_ask);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ALL), config_ask);
}

INT_PTR WINAPI RGConfig(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hwndDlg, IDC_ASK, config_ask);
		if (config_ask_each_album)
			CheckDlgButton(hwndDlg, IDC_ALBUM, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_ALL, BST_CHECKED);
		DoButtons(hwndDlg);
		break;
	case WM_DESTROY:
	{
		config_ask = IsDlgButtonChecked(hwndDlg, IDC_ASK);
		config_ask_each_album = IsDlgButtonChecked(hwndDlg, IDC_ALBUM);
		WritePrivateProfileStringA("ml_rg", "config_ask", config_ask ? "1" : "0", iniFile);
		WritePrivateProfileStringA("ml_rg", "config_ask_each_album", config_ask_each_album ? "1" : "0", iniFile);
		break;
	}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case IDOK:
			EndDialog(hwndDlg, 0);
			break;
		case IDC_ASK:
		case IDC_ALBUM:
		case IDC_ALL:
			DoButtons(hwndDlg);
			break;
		}
		break;
	}
	return 0;
}