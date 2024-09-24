#include ".\smtpdlg.h"
#include ".\resource.h"
#include ".\settings.h"

#include <strsafe.h>
 
extern Settings settings;

void UpdateAuth(HWND hwndDlg, BOOL enabled)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_USER), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_USER), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_PWD), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_PWD), enabled);
}

BOOL CALLBACK smtpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			wchar_t num[16] = {0};
			CenterDialog(hwndDlg);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_SERVER), settings.smtpServer);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_USER), settings.smtpUser);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PWD), settings.smtpPwd);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PORT), _itow(settings.smtpPort, num, 10));
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_ADDRESS), settings.smtpAddress);
			CheckDlgButton(hwndDlg, IDC_CHK_AUTH, settings.smtpAuth);
			UpdateAuth(hwndDlg, settings.smtpAuth);
			break;
		}
		case WM_DESTROY:
		{
			wchar_t buf[1024] = {0};
			int len;
			if (settings.smtpServer) free(settings.smtpServer);
			settings.smtpServer = NULL;
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_SERVER), buf, 1024);
			if (len)
			{
				settings.smtpServer = (wchar_t*)malloc((len + 1)*2);
				StringCchCopy(settings.smtpServer, len+1, buf);
			}

			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PORT), buf, 1024);
			if (len) settings.smtpPort = _wtoi(buf);

			if (settings.smtpUser) free(settings.smtpUser);
			settings.smtpUser = NULL;
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_USER), buf, 1024);
			if (len)
			{
				settings.smtpUser = (wchar_t*)malloc((len + 1)*2);
				StringCchCopy(settings.smtpUser, len+1, buf);
			}

			if (settings.smtpPwd) free(settings.smtpPwd);
			settings.smtpPwd = NULL;
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PWD), buf, 1024);
			if (len)
			{
				settings.smtpPwd = (wchar_t*)malloc((len + 1)*2);
				StringCchCopy(settings.smtpPwd, len+1, buf);
			}

			if (settings.smtpAddress) free(settings.smtpAddress);
			settings.smtpAddress = NULL;
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_ADDRESS), buf, 1024);
			if (len)
			{
				settings.smtpAddress = (wchar_t*)malloc((len + 1)*2);
				StringCchCopy(settings.smtpAddress, len+1, buf);
			}
			settings.smtpAuth  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_AUTH), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.Save();
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_CHK_AUTH:
				UpdateAuth(hwndDlg, (SendMessage((HWND) lParam, BM_GETCHECK, 0,0) == BST_CHECKED));
				break;
			case IDCANCEL:
				EndDialog(hwndDlg, 0);
				break;
			}
			break;

	}
	return FALSE;
}

void CenterDialog(HWND hwndDlg)
{
	HWND hwndOwner;
	RECT rc, rcDlg, rcOwner;
	if ((hwndOwner = GetParent(hwndDlg)) == NULL) 
	{
		hwndOwner = GetDesktopWindow(); 
	}

	GetWindowRect(hwndOwner, &rcOwner); 
	GetWindowRect(hwndDlg, &rcDlg); 
	CopyRect(&rc, &rcOwner); 

	// Offset the owner and dialog box rectangles so that 
	// right and bottom values represent the width and 
	// height, and then offset the owner again to discard 
	// space taken up by the dialog box. 

	OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
	OffsetRect(&rc, -rc.left, -rc.top); 
	OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

	// The new position is the sum of half the remaining 
	// space and the owner's original position. 

	SetWindowPos(hwndDlg, 
		HWND_TOP, 
		rcOwner.left + (rc.right / 2), 
		rcOwner.top + (rc.bottom / 2), 
		0, 0,          // ignores size arguments 
		SWP_NOSIZE); 
}