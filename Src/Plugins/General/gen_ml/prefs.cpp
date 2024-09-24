#include "main.h"
#include "config.h"
#include "api__gen_ml.h"
#include "resource.h"
#include "./navigation.h"
#include "./ml_ipc_0313.h"

#include <shlwapi.h>
#include <strsafe.h>

extern HNAVCTRL hNavigation;

#define PASSWORD_MAXLEN		256

void encode_mimestr(char *in, char *out)
{
	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int shift = 0;
	int accum = 0;

	while (in && *in)
	{
		if (*in)
		{
			accum <<= 8;
			shift += 8;
			accum |= *in++;
		}
		while (shift >= 6)
		{
			shift -= 6;
			*out++ = alphabet[(accum >> shift) & 0x3F];
		}
	}
	
	if (shift == 4)
	{
		*out++ = alphabet[(accum & 0xF) << 2];
		*out++ = '=';
	}

	else if (shift == 2)
	{
		*out++ = alphabet[(accum & 0x3) << 4];
		*out++ = '=';
		*out++ = '=';
	}
	*out++ = 0;
}

int ResizeComboBoxDropDown(HWND hwndDlg, UINT id, const wchar_t * str, int width){
	SIZE size = {0};
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	// get and select parent dialog's font so that it'll calculate things correctly
	HFONT font = (HFONT)SendMessage(hwndDlg,WM_GETFONT,0,0), oldfont = (HFONT)SelectObject(hdc,font);
	GetTextExtentPoint32W(hdc, str, lstrlenW(str)+1, &size);

	int ret = width;
	if(size.cx > width)
	{
		SendDlgItemMessage(hwndDlg, id, CB_SETDROPPEDWIDTH, size.cx, 0);
		ret = size.cx;
	}

	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return ret;
}

static void CenterPopup(HWND hwnd, HWND hCenter)
{
	if (NULL == hwnd || NULL == hCenter)
		return;

	RECT centerRect, windowRect;
	if (!GetWindowRect(hwnd, &windowRect) || !GetWindowRect(hCenter, &centerRect))
		return;
	windowRect.left = centerRect.left + ((centerRect.right - centerRect.left) - (windowRect.right - windowRect.left))/2;
	windowRect.top = centerRect.top + ((centerRect.bottom - centerRect.top) - (windowRect.bottom - windowRect.top))/2;

	SetWindowPos(hwnd, NULL, windowRect.left, windowRect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

static INT MessageBoxWA(HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType)
{
	WCHAR szText[128] = {0}, szCaption[2048] = {0};
	if (IS_INTRESOURCE(pszText))
	{
		if (NULL != pszText)
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszText, szText, ARRAYSIZE(szText));
			pszText = szText;
		}
	}
	if (IS_INTRESOURCE(pszCaption))
	{
		if (NULL != pszCaption)
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszCaption, szCaption, ARRAYSIZE(szCaption));
			pszCaption = szCaption;
		}
	}
	return MessageBoxW(hwnd, pszText, pszCaption, uType);
}

static INT_PTR CALLBACK Prefs1Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			CheckDlgButton(hwndDlg, IDC_PLPLAYLIST, !!g_config->ReadInt(L"plplaymode", 1));
			CheckDlgButton(hwndDlg, IDC_VIEWPLAYMODE, !!g_config->ReadInt(L"viewplaymode", 1));

			wchar_t* str = WASABI_API_LNGSTRINGW(IDS_PLAY_SELECTED);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)str);
			int width = ResizeComboBoxDropDown(hwndDlg, IDC_COMBO1, str, 0);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)(str = WASABI_API_LNGSTRINGW(IDS_ENQUEUE_SELECTED)));
			ResizeComboBoxDropDown(hwndDlg, IDC_COMBO1, str, width);
			SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_SETCURSEL, g_config->ReadInt(L"enqueuedef", 0) ? 1 : 0, 0);
			CheckDlgButton(hwndDlg, IDC_CHECK1, !!g_config->ReadInt(L"attachlbolt", 0));
			CheckDlgButton(hwndDlg, IDC_PL_SEND_TO, !!g_config->ReadInt(L"pl_send_to", 1));
			CheckDlgButton(hwndDlg, IDC_PMP_SEND_TO, !!g_config->ReadInt(L"pmp_send_to", 1));
			if (!GetModuleHandleW(L"ml_pmp.dll")) EnableWindow(GetDlgItem(hwndDlg, IDC_PMP_SEND_TO), FALSE);
			CheckDlgButton(hwndDlg, IDC_WRITE_RATINGS, !!g_config->ReadInt(L"writeratings", 0));
			CheckDlgButton(hwndDlg, IDC_GROUP_BUTTONS, !!g_config->ReadInt(L"groupbtn", 1));

			int colresmode = g_config->ReadInt(L"column_resize_mode", 0);
			width = 0;
			SendDlgItemMessageW(hwndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)(str = WASABI_API_LNGSTRINGW(IDS_COL_NORMAL)));
			width = ResizeComboBoxDropDown(hwndDlg, IDC_COMBO2, str, width);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)(str = WASABI_API_LNGSTRINGW(IDS_COL_SELONLY)));
			ResizeComboBoxDropDown(hwndDlg, IDC_COMBO2, str, width);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)(str = WASABI_API_LNGSTRINGW(IDS_COL_PROP)));
			ResizeComboBoxDropDown(hwndDlg, IDC_COMBO2, str, 0);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO2, CB_SETCURSEL, colresmode, 0);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_COMBO1:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int a = (INT)SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0),
							mode = g_config->ReadInt(L"enqueuedef", 0);
						if ((a == 1) != mode)
						{
							mode = (a == 1);
							g_config->WriteInt(L"enqueuedef", mode);
							HWND hView = (HWND)SENDMLIPC(g_hwnd, ML_IPC_GETCURRENTVIEW, 0);
							if (IsWindow(hView)) PostMessageW(hView, WM_DISPLAYCHANGE, 0, 0);
						}

						// v5.66+ - send a general notification so plug-ins can (if needed update on
						//			the play / enqueue mode change (mainly added for ml_playlists)
						pluginMessage p = {ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE, (INT_PTR)!!mode,
										   (INT_PTR)!!g_config->ReadInt(L"groupbtn", 1), 0};
						SENDMLIPC(g_hwnd, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
					}
					break;
				case IDC_PL_SEND_TO:
					g_config->WriteInt(L"pl_send_to", !!IsDlgButtonChecked(hwndDlg, IDC_PL_SEND_TO));
					break;
				case IDC_PMP_SEND_TO:
					g_config->WriteInt(L"pmp_send_to", !!IsDlgButtonChecked(hwndDlg, IDC_PMP_SEND_TO));
					break;
				case IDC_WRITE_RATINGS:
					g_config->WriteInt(L"writeratings", !!IsDlgButtonChecked(hwndDlg, IDC_WRITE_RATINGS));
					break;
				case IDC_PLPLAYLIST:
					g_config->WriteInt(L"plplaymode", !!IsDlgButtonChecked(hwndDlg, IDC_PLPLAYLIST));
					break;
				case IDC_VIEWPLAYMODE:
					g_config->WriteInt(L"viewplaymode", !!IsDlgButtonChecked(hwndDlg, IDC_VIEWPLAYMODE));
					break;
				case IDC_COMBO2:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int colresmode = (INT)SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_GETCURSEL, 0, 0);
						g_config->WriteInt(L"column_resize_mode", colresmode);
					}
					break;
				case IDC_CHECK1:
					g_config->WriteInt(L"attachlbolt", !!IsDlgButtonChecked(hwndDlg, IDC_CHECK1));
					break;
				case IDC_GROUP_BUTTONS:
				{
					g_config->WriteInt(L"groupbtn", !!IsDlgButtonChecked(hwndDlg, IDC_GROUP_BUTTONS));
					// refresh the current view as otherwise is a pain to dynamically update as needed
					PostMessage(g_hwnd, WM_USER + 30, 0, 0);
					break;
				}
			}
			break;
		}
	return 0;
}

static INT_PTR CALLBACK GetTVPassProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_GPASS), TRUE);
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			char pass1[64] = {0};
			char password[4096] = {0};
			GetDlgItemTextA(hwndDlg, IDC_GPASS, pass1, sizeof(pass1) - 1);
			encode_mimestr(pass1, password);
			if (strcmp(password, g_config->ReadString("stctka", "none")) != 0)
			{
				wchar_t titleStr[32] = {0};
				MessageBoxW(NULL,
				            WASABI_API_LNGSTRINGW(IDS_INVALID_PASSWORD),
				            WASABI_API_LNGSTRINGW_BUF(IDS_INTERNET_ACCESS,titleStr,32),
				            MB_OK);
				EndDialog(hwndDlg, 0);
				break;
			}
			else
			{
				EndDialog(hwndDlg, 1);
				break;
			}

		}
		case IDCANCEL:
		{
			EndDialog(hwndDlg, 0);
			break;
		}
		}
		break;
	}
	return 0;
}

static INT_PTR InternetPassword_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	HWND hControl = GetDlgItem(hwnd, IDC_EDIT_PASSWORD);
	if (NULL != hControl)
		PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hControl, TRUE);

	hControl = GetDlgItem(hwnd, IDOK);
	if (NULL != hControl)
		EnableWindow(hControl, FALSE);

	HWND hCenter = (HWND)lParam;
	if (NULL != hCenter && IsWindow(hCenter))
		CenterPopup(hwnd, hCenter);

	SendMessage(hwnd, DM_REPOSITION, 0, 0L);
	return FALSE;
}

static BOOL InternetPassword_GetPassword(HWND hwnd, LPWSTR pszBuffer, INT cchBufferMax)
{
	
	HWND hEdit1 = GetDlgItem(hwnd, IDC_EDIT_PASSWORD);
	HWND hEdit2 = GetDlgItem(hwnd, IDC_EDIT_PASSWORD_VERIFY);
	if (NULL != hEdit1 && NULL != hEdit2) 
	{
		WCHAR szPwd1[PASSWORD_MAXLEN] = {0}, szPwd2[PASSWORD_MAXLEN] = {0};
	
		INT cchPwd1 = GetWindowTextW(hEdit1, szPwd1, ARRAYSIZE(szPwd1));
		INT cchPwd2 = GetWindowTextW(hEdit2, szPwd2, ARRAYSIZE(szPwd2));

		if (0 != cchPwd1 && 
			cchPwd1 == cchPwd2 && 
			CSTR_EQUAL == CompareStringW(LOCALE_USER_DEFAULT, 0, szPwd1, cchPwd1, szPwd2, cchPwd2))
		{
			if (NULL != pszBuffer && cchBufferMax > 0)
			{
				if (FAILED(StringCchCopyW(pszBuffer, cchBufferMax, szPwd1)))
					return FALSE;
			}
			return TRUE;
		}
	}
	return FALSE;
}

static void InternetPassword_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch (commandId)
	{
		case IDC_EDIT_PASSWORD:
		case IDC_EDIT_PASSWORD_VERIFY:
			switch(eventId)
			{
				case EN_UPDATE:

					HWND hButton = GetDlgItem(hwnd, IDOK);
					if (NULL != hButton)
					{
						EnableWindow(hButton, InternetPassword_GetPassword(hwnd, NULL, 0));
					}
					break;
			}
			break;
		case IDOK:
			{
				WCHAR szPassword[PASSWORD_MAXLEN] = {0};
				BOOL passwordOk = InternetPassword_GetPassword(hwnd, szPassword, ARRAYSIZE(szPassword));

				if (FALSE == passwordOk)
				{
					MessageBoxWA(hwnd, MAKEINTRESOURCEW(IDS_PASSWORD_NO_MATCH), MAKEINTRESOURCEW(IDS_INTERNET_ACCESS), 
								MB_OK | MB_ICONEXCLAMATION);
				}
				else
				{
					char szPasswordAnsi[PASSWORD_MAXLEN * 2] = {0};
					BOOL fInvalidChar;
					if (0 == WideCharToMultiByte(CP_ACP, 0, szPassword, -1, szPasswordAnsi, ARRAYSIZE(szPasswordAnsi), NULL, &fInvalidChar) ||
						FALSE != fInvalidChar)
					{
						// TODO:  put better error description
						LPCWSTR pszMessage = MAKEINTRESOURCEW(IDS_INVALID_PASSWORD);
						MessageBoxWA(hwnd, pszMessage, MAKEINTRESOURCEW(IDS_INTERNET_ACCESS), MB_OK | MB_ICONERROR);

					}
					else
					{
						char szEncoded[PASSWORD_MAXLEN * 2] = {0};
						encode_mimestr(szPasswordAnsi, szEncoded);
						g_config->WriteString("stctka", szEncoded);
						EndDialog(hwnd, IDOK);
					}
				}
				break;
			}
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
	}
}

static INT_PTR CALLBACK InternetPassword_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG: return InternetPassword_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_COMMAND:		InternetPassword_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
	}
	return 0;
}

static INT_PTR CALLBACK Prefs2Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		int passprompt = g_config->ReadInt(L"tvpp", 0);
		if (passprompt && strcmp("none", g_config->ReadString("stctka", "none")) == 0)
		{
			wchar_t titleStr[32] = {0};
			MessageBoxW(hwndDlg,
			            WASABI_API_LNGSTRINGW(IDS_RATINGS_PASSWORD_MISSING),
			            WASABI_API_LNGSTRINGW_BUF(IDS_SERCURITY_ALERT,titleStr,32),
			            MB_OK);
			passprompt = 0;
		}

		CheckDlgButton(hwndDlg, IDC_CHECK_PASSPROMPT, passprompt);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ITV_CHANGEPASS), passprompt);

		{
			int rating = g_config->ReadInt(L"tvrating", 7);
			CheckDlgButton(hwndDlg, IDC_CHECK_RATING0, !!(rating&1) || !!(rating&2));
			CheckDlgButton(hwndDlg, IDC_CHECK_RATING1, !!(rating&4));
			CheckDlgButton(hwndDlg, IDC_CHECK_RATING2, !!(rating&8));
			CheckDlgButton(hwndDlg, IDC_CHECK_RATING3, !!(rating&16) || !!(rating&32));
			CheckDlgButton(hwndDlg, IDC_CHECK_RATING4, !!(rating&64));
		}

		if (passprompt && strcmp("none", g_config->ReadString("stctka", "none")) != 0)
		{
			INT_PTR result = WASABI_API_DIALOGBOX(IDD_PREFS_ITV_GETPASS, hwndDlg, GetTVPassProc);
			if (!result)
			{
				HWND next = GetWindow(hwndDlg,GW_CHILD);
				HWND warning = GetDlgItem(hwndDlg,IDC_STATIC_INFO);
				while(IsWindow(next))
				{
					if(next != warning)
					{
						HWND remove = next;
						next = GetWindow(next, GW_HWNDNEXT);
						DestroyWindow(remove);
					}
					else
					{
						next = GetWindow(next, GW_HWNDNEXT);
					}
				}
				break;
			}
			else
			{
				DestroyWindow(GetDlgItem(hwndDlg,IDC_STATIC_INFO));
			}
		}
		else
		{
			DestroyWindow(GetDlgItem(hwndDlg,IDC_STATIC_INFO));
		}
	}
	break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_CHECK_PASSPROMPT:
			{
				int passprompt = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_PASSPROMPT)) passprompt = 1;
				else g_config->WriteString("stctka", 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ITV_CHANGEPASS), passprompt);
				break;
			}
			case IDC_ITV_CHANGEPASS:
			{
				WASABI_API_DIALOGBOXPARAMW(IDD_PREFS_ITV_ASSIGNPASS, hwndDlg, InternetPassword_DialogProc, (LPARAM)hwndDlg);
				break;
			}
		}
		break;

	case WM_DESTROY:
	{
		if(!IsWindow(GetDlgItem(hwndDlg,IDC_STATIC_INFO)))
		{
			int rating = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_RATING0)) rating |= (1 | 2);
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_RATING1)) rating |= 4;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_RATING2)) rating |= 8;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_RATING3)) rating |= (16 | 32);
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_RATING4)) rating |= 64;
			g_config->WriteInt(L"tvrating", rating);
			int passprompt = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_PASSPROMPT)) passprompt = 1;
			g_config->WriteInt(L"tvpp", passprompt);
			if (passprompt)
			{
				if (strcmp("none", g_config->ReadString("stctka", "none")) == 0)
				{
					WASABI_API_DIALOGBOX(IDD_PREFS_ITV_ASSIGNPASS, hwndDlg, InternetPassword_DialogProc);
				}
			}
#if 0 // no radio
			radio_updateTvView(1);
#endif
		}
	}
	break;
	}
	return 0;
}

C_Config *g_view_metaconf = NULL;
static UINT msgNotify = 0;
static HINSTANCE cloud_hinst;
static int IPC_GET_CLOUD_HINST = -1, last_pos;
static INT_PTR CALLBACK Prefs3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int edit_inited;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		wchar_t buffer[MAX_PATH] = {0};
		edit_inited = 0;

		LPCWSTR pszPath = (LPCWSTR)SendMessageW(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORYW);
		if(PathCombineW(buffer, pszPath, L"ml_disc.dll") && PathFileExistsW(buffer))
		{			
			PathCombineW(buffer, WINAMP_INI_DIR, L"Plugins\\ml\\cdrom.vmd");
			g_view_metaconf = new C_Config(buffer);

			CheckDlgButton(hwndDlg, IDC_SHOW_EJECT_ICONS, g_view_metaconf->ReadInt(L"showeject", 1));
			CheckDlgButton(hwndDlg, IDC_GROUP_DRIVES, g_view_metaconf->ReadInt(L"showparent", 0));
			if(!msgNotify) msgNotify = RegisterWindowMessageW(L"ripburn_nav_update");
		}
		else
		{
			DestroyWindow(GetDlgItem(hwndDlg,IDC_CD_DVD_ITEMS));
			DestroyWindow(GetDlgItem(hwndDlg,IDC_SHOW_EJECT_ICONS));
			DestroyWindow(GetDlgItem(hwndDlg,IDC_GROUP_DRIVES));
			RECT r;
			HWND frame = GetDlgItem(hwndDlg,IDC_ML_TREE_OPTS);
			GetWindowRect(frame, &r);
			SetWindowPos(frame, 0, 0, 0, (r.right - r.left), (r.bottom - r.top) - 72,
						 SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
		}

		CheckDlgButton(hwndDlg, IDC_SHOW_ICONS, !!g_config->ReadInt(L"Navigation_ShowIcons", 1));
		CheckDlgButton(hwndDlg, IDC_HIGHLIGHT_FULL_TREE_ITEM, !!g_config->ReadInt(L"Navigation_FullRowSel", 1));
		CheckDlgButton(hwndDlg, IDC_RCLICK_TO_SELECT, g_config->ReadInt(L"Navigation_MouseDownSel", 0));
		SetDlgItemInt(hwndDlg, IDC_ITEM_HEIGHT, g_config->ReadInt(L"Navigation_ItemHeight", 18), 0);

		CheckDlgButton(hwndDlg, IDC_PARENT_PODCASTS, g_config->ReadInt(L"podcast_parent", 0));

		/*if (IPC_GET_CLOUD_HINST == -1) IPC_GET_CLOUD_HINST = (INT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
		if (!cloud_hinst) cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);

		SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_SETITEMDATA, SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_ADDSTRING, 0, (LPARAM)L"as a top level tree item (default)"), 0);
		if (cloud_hinst && cloud_hinst != (HINSTANCE)1)
			SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_SETITEMDATA, SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_ADDSTRING, 0, (LPARAM)L"under the 'Cloud Library' item"), 1);
		SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_SETITEMDATA, SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_ADDSTRING, 0, (LPARAM)L"under the 'Devices' item"), 2);

		last_pos = g_config->ReadInt(L"txviewpos", 0);
		int i = 0, count = SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_GETCOUNT, 0, 0);
		for (; i < count; i++)
		{
			int item = SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_GETITEMDATA, i, 0);
			if (item == last_pos)
			{
				SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_SETCURSEL, i, 0);
				break;
			}
		}
		if (i == count)
			SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_SETCURSEL, 0, 0);*/

		edit_inited = 1;
	}
	break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_SHOW_ICONS:
				g_config->WriteInt(L"Navigation_ShowIcons", !!IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
				NavCtrlI_UpdateLook(hNavigation);
				// tell ml_disc to update if present
				if(g_view_metaconf)SendMessage(plugin.hwndParent, msgNotify, 0, 0);
				break;

			case IDC_HIGHLIGHT_FULL_TREE_ITEM:
				g_config->WriteInt(L"Navigation_FullRowSel", !!IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
				NavCtrlI_UpdateLook(hNavigation);
				break;

			case IDC_RCLICK_TO_SELECT:
				g_config->WriteInt(L"Navigation_MouseDownSel", !!IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
				NavCtrlI_UpdateLook(hNavigation);
				break;

			case IDC_ITEM_HEIGHT:
				if(HIWORD(wParam) == EN_CHANGE && edit_inited)
				{
					BOOL success = 0;
					UINT val = GetDlgItemInt(hwndDlg, LOWORD(wParam), &success, 0);
					if(success)
					{
						g_config->WriteInt(L"Navigation_ItemHeight", val);
						NavCtrlI_UpdateLook(hNavigation);
					}
				}
				break;

			case IDC_SHOW_EJECT_ICONS:
				g_view_metaconf->WriteInt(L"showeject", !!IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
				SendMessage(plugin.hwndParent, msgNotify, 0, 0);
				NavCtrlI_UpdateLook(hNavigation);
				break;

			case IDC_GROUP_DRIVES:
				g_view_metaconf->WriteInt(L"showparent", !!IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
				SendMessage(plugin.hwndParent, msgNotify, 1, 0);
				NavCtrlI_UpdateLook(hNavigation);
				break;

			case IDC_PARENT_PODCASTS:
			{
				int parent = !!IsDlgButtonChecked(hwndDlg, LOWORD(wParam));
				g_config->WriteInt(L"podcast_parent", parent);

				pluginMessage p = {ML_MSG_DOWNLOADS_VIEW_POSITION, (INT_PTR)parent, (INT_PTR)1, 0};
				SENDMLIPC(g_hwnd, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
				NavCtrlI_UpdateLook(hNavigation);
				break;
			}
			case IDC_TRANSFERS_COMBO:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					int a = (INT)SendDlgItemMessage(hwndDlg, IDC_TRANSFERS_COMBO, CB_GETCURSEL, 0, 0);
					if (a != CB_ERR)
					{
						g_config->WriteInt(L"txviewpos", SendDlgItemMessageW(hwndDlg, IDC_TRANSFERS_COMBO, CB_GETITEMDATA, a, 0));
						wchar_t titleStr[32] = {0};
						if ((a != last_pos) &&
							MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_RESTART_MESSAGE),
										WASABI_API_LNGSTRINGW_BUF(IDS_RESTART, titleStr, 32),
										MB_ICONQUESTION | MB_YESNO) == IDYES)
						{
							WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
							PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
						}
					}
				}
				break;
		}
		break;

	case WM_DESTROY:
		if(g_view_metaconf)
		{
			delete(g_view_metaconf);
			g_view_metaconf = 0;
		}
		break;
	}
	return 0;
}

static INT_PTR CALLBACK Prefs4Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		CheckDlgButton(hwndDlg, IDC_CHECK_USEPLFONT, !!g_config->ReadInt(L"plfont_everywhere", 1));
		CheckDlgButton(hwndDlg, IDC_FF_SCROLLBARS, config_use_ff_scrollbars);
		CheckDlgButton(hwndDlg, IDC_ALTERNATEITEMS, config_use_alternate_colors);
		CheckDlgButton(hwndDlg, IDC_SKINNED_MENUS, IsSkinnedPopupEnabled(FALSE));
		CheckDlgButton(hwndDlg, IDC_GENO, !!GetPrivateProfileIntW(L"winamp", L"geno", 1, WINAMP_INI));
	}
	break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_USEPLFONT:
			g_config->WriteInt(L"plfont_everywhere", !!IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEPLFONT));
			PostMessage(plugin.hwndParent, WM_DISPLAYCHANGE, 0, 0);
			break;
		case IDC_FF_SCROLLBARS:
			config_use_ff_scrollbars = !!IsDlgButtonChecked(hwndDlg, IDC_FF_SCROLLBARS);
			g_config->WriteInt(L"ffsb", config_use_ff_scrollbars);
			PostMessage(plugin.hwndParent, WM_DISPLAYCHANGE, 0, 0);
			break;
		case IDC_ALTERNATEITEMS:
			config_use_alternate_colors = !!IsDlgButtonChecked(hwndDlg, IDC_ALTERNATEITEMS);
			g_config->WriteInt(L"alternate_items", config_use_alternate_colors);
			PostMessage(plugin.hwndParent, WM_DISPLAYCHANGE, 0, 0);
			break;
		case IDC_SKINNED_MENUS:
			EnableSkinnedPopup(BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_SKINNED_MENUS));
			break;
		case IDC_GENO:
		{
			wchar_t buf[64] = {L"1"};
			StringCchPrintfW(buf, 64, L"%d", !!IsDlgButtonChecked(hwndDlg, IDC_GENO));
			WritePrivateProfileStringW(L"winamp", L"geno", buf, WINAMP_INI);
			break;
		}
		case IDC_RATING_COLUMN:
			SendMessage(g_hwnd,WM_COMMAND,MAKEWPARAM(ID_SHOW_RATINGTWEAK,0),0);
			break;
		}
		break;
	}
	return 0;
}

HWND subWnd = 0, prefsWnd = 0;

static void _dosetsel(HWND hwndDlg)
{
	HWND tabwnd = GetDlgItem(hwndDlg, IDC_TAB1);
	int sel = TabCtrl_GetCurSel(tabwnd);

	if (sel >= 0 && (sel != g_config->ReadInt(L"lastprefp", 0) || !subWnd))
	{
		g_config->WriteInt(L"lastprefp", sel);
		if (subWnd) DestroyWindow(subWnd);
		subWnd = 0;

		UINT t = 0;
		DLGPROC p;
		switch (sel)
		{
			case 0: t = IDD_PREFS1; p = Prefs1Proc; break;
			case 1: t = IDD_PREFS2; p = Prefs2Proc; break;
			case 2: t = IDD_PREFS3; p = Prefs3Proc; break;
			case 3: t = IDD_PREFS4; p = Prefs4Proc; break;
		}
		if (t) subWnd = WASABI_API_CREATEDIALOGW(t, hwndDlg, p);

		if (subWnd)
		{
			RECT r;
			GetClientRect(tabwnd, &r);
			TabCtrl_AdjustRect(tabwnd, FALSE, &r);
			SetWindowPos(subWnd, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE);
			ShowWindow(subWnd, SW_SHOWNA);
		}

		if (!SendMessage(plugin.hwndParent,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC))
		{
			SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)tabwnd,IPC_USE_UXTHEME_FUNC);
			SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)subWnd,IPC_USE_UXTHEME_FUNC);
		}
	}
}

#define TabCtrl_InsertItemW(hwnd, iItem, pitem)   \
    (int)SNDMSG((hwnd), TCM_INSERTITEMW, (WPARAM)(int)(iItem), (LPARAM)(const TC_ITEMW *)(pitem))

// frame proc
INT_PTR CALLBACK PrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		TCITEMW item;
		HWND tabwnd = GetDlgItem(hwndDlg, IDC_TAB1);
		item.mask = TCIF_TEXT;
		item.pszText = WASABI_API_LNGSTRINGW(IDS_LIBRARY_OPTIONS);
		TabCtrl_InsertItemW(tabwnd, 0, &item);
		item.pszText = WASABI_API_LNGSTRINGW(IDS_ONLINE_MEDIA);
		TabCtrl_InsertItemW(tabwnd, 1, &item);
		item.pszText = WASABI_API_LNGSTRINGW(IDS_TREE_OPTIONS);
		TabCtrl_InsertItemW(tabwnd, 2, &item);
		item.pszText = WASABI_API_LNGSTRINGW(IDS_APPEARANCE);
		TabCtrl_InsertItemW(tabwnd, 3, &item);
		TabCtrl_SetCurSel(tabwnd, g_config->ReadInt(L"lastprefp", 0));
		_dosetsel(hwndDlg);

		prefsWnd = hwndDlg;
	}
	return 0;
	case WM_NOTIFY:
	{
		LPNMHDR p = (LPNMHDR) lParam;
		if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE) _dosetsel(hwndDlg);
	}
	return 0;
	case WM_DESTROY:
		subWnd = NULL;
		prefsWnd = NULL;
		return 0;
	}
	return 0;
}

void refreshPrefs(INT_PTR screen)
{
	if (subWnd && g_config->ReadInt(L"lastprefp", -1) == screen)
	{
		if (screen == 4) SendMessage(subWnd, WM_INITDIALOG, 0, 0);
	}
}

extern prefsDlgRecW myPrefsItem;

void openPrefs(INT_PTR screen)
{
	if (!subWnd)
	{
		if (screen != -1) g_config->WriteInt(L"lastprefp", (INT)screen);
	}
	else
	{
		if (screen != -1)
		{
			HWND tabwnd = GetDlgItem(prefsWnd, IDC_TAB1);
			TabCtrl_SetCurSel(tabwnd, screen);
			_dosetsel(prefsWnd);
		}
	}
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&myPrefsItem, IPC_OPENPREFSTOPAGE);
}