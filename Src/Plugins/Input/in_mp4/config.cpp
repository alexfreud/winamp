#include "main.h"
#include "api__in_mp4.h"
#include "../nu/AutoChar.h"
#include "resource.h"

bool config_show_average_bitrate = true;

INT_PTR CALLBACK ConfigProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			wchar_t exts[1024] = {0};
			GetPrivateProfileStringW(L"in_mp4", L"extensionlist", defaultExtensions, exts, 1024, m_ini);
			SetDlgItemTextW(hwndDlg, IDC_EXTENSIONLIST, exts);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DEFAULT:
			SetDlgItemTextW(hwndDlg, IDC_EXTENSIONLIST, defaultExtensions);
			break;
		case IDOK:
			{
				wchar_t exts[1024] = {0};
				GetDlgItemTextW(hwndDlg, IDC_EXTENSIONLIST, exts, 1024);
				if (!_wcsicmp(exts, defaultExtensions)) // same as default?
					WritePrivateProfileStringW(L"in_mp4", L"extensionlist", 0, m_ini); 
				else
					WritePrivateProfileStringW(L"in_mp4", L"extensionlist", exts, m_ini);
				free(mod.FileExtensions);
				mod.FileExtensions = BuildExtensions(AutoChar(exts));
				EndDialog(hwndDlg, 0);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, 1);
			break;
		}
		break;
	}
	return 0;
}
void config(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_CONFIG, hwndParent, ConfigProc);
}