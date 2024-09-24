#include "main.h"
#include <shlobj.h>
#include <commctrl.h>
#include <windows.h>
#include "../winamp/wa_ipc.h"
#include "config.h"
#include "api__in_mp3.h"
#include "resource.h"

char g_http_tmp[MAX_PATH] = {0};

int config_write_mode = WRITE_UTF16;
int config_read_mode = READ_LOCAL;

int config_parse_apev2 = 1;
int config_parse_lyrics3 = 1;
int config_parse_id3v1 = 1;
int config_parse_id3v2 = 1;

int config_write_apev2 = 1;
int config_write_id3v1 = 1;
int config_write_id3v2 = 1;

int config_create_id3v1 = 1;
int config_create_id3v2 = 1;
int config_create_apev2 = 0;

int config_apev2_header = RETAIN_HEADER;
int config_lp = 0;

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassNameW(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiW(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

static int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			SetWindowText(hwnd, WASABI_API_LNGSTRINGW(IDS_SELECT_DIRECTORY_TO_SAVE_TO));
			if (g_http_tmp[0]) SendMessage(hwnd, BFFM_SETSELECTIONA, 1, (LPARAM)g_http_tmp);

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows(hwnd, browseEnumProc, 0);
		}
	}
	return 0;
}

static char app_name[] = "Nullsoft MPEG Decoder";

char *get_inifile() { return INI_FILE; }

int _r_i(char *name, int def)
{
	if (!_strnicmp(name, "config_", 7)) name += 7;
	return GetPrivateProfileIntA(app_name, name, def, INI_FILE);
}

#define RI(x) (( x ) = _r_i(#x,( x )))
void _w_i(char *name, int d)
{
	char str[120] = {0};
	wsprintfA(str, "%d", d);
	if (!_strnicmp(name, "config_", 7)) name += 7;
	WritePrivateProfileStringA(app_name, name, str, INI_FILE);
}
#define WI(x) _w_i(#x,( x ))

void _r_s(char *name, char *data, int mlen)
{
	char buf[2048] = {0};
	lstrcpynA(buf, data, 2048);
	if (!_strnicmp(name, "config_", 7)) name += 7;
	GetPrivateProfileStringA(app_name, name, buf, data, mlen, INI_FILE);
}
#define RS(x) (_r_s(#x,x,sizeof(x)))

void _w_s(char *name, char *data)
{
	if (!_strnicmp(name, "config_", 7)) name += 7;
	WritePrivateProfileStringA(app_name, name, data, INI_FILE);
}
#define WS(x) (_w_s(#x,x))

static void config_init()
{
	char *p;
	if (mod.hMainWindow &&
	    (p = (char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE))
	    && p != (char *)1)
	{
		strncpy(INI_FILE, p, MAX_PATH);
	}
	else
	{
		GetModuleFileNameA(NULL, INI_FILE, sizeof(INI_FILE));
		p = INI_FILE + strlen(INI_FILE);
		while (p >= INI_FILE && *p != '.') p--;
		strcpy(++p, "ini");
	}
}

#ifdef AAC_SUPPORT
#define DEF_EXT_LIST "MP3;MP2;MP1;AAC;VLB"
#else
#define DEF_EXT_LIST "MP3;MP2;MP1"
#endif

#define __STR2WSTR(str) L##str
#define WIDEN(str) __STR2WSTR(str)
#define DEF_EXT_LISTW WIDEN(DEF_EXT_LIST)

#ifdef AAC_SUPPORT
char config_extlist_aac[129] = DEF_EXT_LIST;
#else
char config_extlist[129] = DEF_EXT_LIST;
#endif

char config_rating_email[255] = {0};

void config_read()
{
	config_init();
	RI(allow_scartwork);
	RI(allow_sctitles);
	RI(sctitle_format);
	RI(config_http_buffersize);
	RI(config_http_prebuffer);
	RI(config_http_prebuffer_underrun);
	RI(config_downmix);
	RI(config_downsample);
	RI(config_max_bufsize_k);
	RI(config_eqmode);
	RI(config_gapless);

	if(FAILED(SHGetFolderPathA(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, config_http_save_dir)))
	{
		if(FAILED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, config_http_save_dir)))
		{
			lstrcpynA(config_http_save_dir, "C:\\", MAX_PATH);
		}
	}

	RS(config_http_save_dir);
	RI(config_miscopts);
	RI(config_fastvis);

#ifdef AAC_SUPPORT
	RS(config_extlist_aac);
#else
	RS(config_extlist);
#endif

	RI(config_write_mode);
	RI(config_read_mode);

	RI(config_parse_apev2);
	RI(config_parse_lyrics3);
	RI(config_parse_id3v1);
	RI(config_parse_id3v2);

	RI(config_write_apev2);
	RI(config_write_id3v1);
	RI(config_write_id3v2);

	RI(config_create_apev2);
	RI(config_create_id3v1);
	RI(config_create_id3v2);

	RI(config_apev2_header);

	RI(config_lp);

	RS(config_rating_email);
}

void config_write()
{
	WI(allow_scartwork);
	WI(config_fastvis);
	WI(config_miscopts);
	WI(allow_sctitles);
	WI(sctitle_format);
	WI(config_http_buffersize);
	WI(config_http_buffersize);
	WI(config_http_prebuffer);
	WI(config_http_prebuffer_underrun);
	WI(config_downmix);
	WI(config_downsample);
	WI(config_max_bufsize_k);
	WI(config_eqmode);
	WS(config_http_save_dir);
#ifdef AAC_SUPPORT
	WS(config_extlist_aac);
#else
	WS(config_extlist);
#endif

	WI(config_write_mode);
	WI(config_read_mode);

	WI(config_parse_apev2);
	WI(config_parse_lyrics3);
	WI(config_parse_id3v1);
	WI(config_parse_id3v2);

	WI(config_write_apev2);
	WI(config_write_id3v1);
	WI(config_write_id3v2);

	WI(config_create_apev2);
	WI(config_create_id3v1);
	WI(config_create_id3v2);
	
	WI(config_apev2_header);

	WI(config_lp);

	WS(config_rating_email);
}

static INT_PTR CALLBACK prefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK id3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK advancedTaggingProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK httpProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK outputProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define ISSEP(x) ((x) == ' ' || (x) == ';' || (x) == ',' || (x) == ':' || (x) == '.')
char *getfileextensions()
{
	static char list[512];
	char *op = list;
	//  char *g_fileassos="MP3;MP2;MP1\0MPEG Audio Files (*.MP3;*.MP2;*.MP1)\0";

	char *p = config_extlist;
	int s = 0;
	while (p && *p)
	{
		while (ISSEP(*p)) p++;
		if (!p || !*p) break;
		if (s) *op++ = ';';
		s = 1;
		while (p && *p && !ISSEP(*p)) *op++ = *p++;
	}
	*op++ = 0;
	strcpy(op, WASABI_API_LNGSTRING(IDS_MPEG_AUDIO_FILES));
	while (op && *op) op++;
	p = config_extlist;
	s = 0;
	while (p && *p)
	{
		while (ISSEP(*p)) p++;
		if (!p || !*p) break;
		if (s) *op++ = ';';
		s = 1;
		*op++ = '*';
		*op++ = '.';
		while (p && *p && !ISSEP(*p)) *op++ = *p++;
	}
	*op++ = ')';
	*op++ = 0;
	*op++ = 0;
	return list;
}

void config(HWND hwndParent)
{
	wchar_t title[128] = {0};
	int x;
	PROPSHEETHEADER pshead;
	PROPSHEETPAGE pspage[5];
	ZeroMemory(&pshead, sizeof(PROPSHEETHEADER));
	pshead.dwSize = sizeof(PROPSHEETHEADER);
	pshead.hwndParent = hwndParent;
	pshead.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	pshead.hInstance = WASABI_API_LNG_HINST;
	pshead.pszCaption = WASABI_API_LNGSTRINGW_BUF(IDS_MPEG_AUDIO_DECODER_SETTINGS,title,128);//"MPEG Audio Decoder Settings";
	pshead.nPages = sizeof(pspage) / sizeof(pspage[0]);
	pshead.nStartPage = config_lp;
	pshead.ppsp = pspage;

	ZeroMemory(pspage, sizeof(pspage));
	for ( x = 0; x < sizeof(pspage) / sizeof(pspage[0]); x ++)
		pspage[x].dwSize = sizeof(PROPSHEETPAGE);
	for ( x = 0; x < sizeof(pspage) / sizeof(pspage[0]); x ++)
		pspage[x].hInstance = WASABI_API_LNG_HINST;
	pspage[0].pszTemplate = MAKEINTRESOURCE(IDD_PREFS);
	pspage[1].pszTemplate = MAKEINTRESOURCE(IDD_TAGOPTS);
	pspage[2].pszTemplate = MAKEINTRESOURCE(IDD_ADVANCED_TAGGING);
	pspage[3].pszTemplate = MAKEINTRESOURCE(IDD_OUTPUT);
	pspage[4].pszTemplate = MAKEINTRESOURCE(IDD_HTTP);
	pspage[0].pfnDlgProc = prefsProc;
	pspage[1].pfnDlgProc = id3Proc;
	pspage[2].pfnDlgProc = advancedTaggingProc;
	pspage[3].pfnDlgProc = outputProc;
	pspage[4].pfnDlgProc = httpProc;
	PropertySheet((PROPSHEETHEADER*)&pshead);
	config_write();
	extern char *g_fileassos;
	mod.FileExtensions = getfileextensions();
}

static INT_PTR CALLBACK id3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		if (config_parse_id3v1) CheckDlgButton(hwndDlg, IDC_READ_ID3V1, BST_CHECKED);
		if (config_parse_id3v2) CheckDlgButton(hwndDlg, IDC_READ_ID3V2, BST_CHECKED);

		if (config_write_id3v1) CheckDlgButton(hwndDlg, IDC_WRITE_ID3V1, BST_CHECKED);
		if (config_write_id3v2) CheckDlgButton(hwndDlg, IDC_WRITE_ID3V2, BST_CHECKED);

		if (config_create_id3v1) CheckDlgButton(hwndDlg, IDC_CREATE_ID3V1, BST_CHECKED);
		if (config_create_id3v2) CheckDlgButton(hwndDlg, IDC_CREATE_ID3V2, BST_CHECKED);

		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_LATIN_1));
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_SYSTEM_LANGUAGE));
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,(config_read_mode == READ_LOCAL),0);

		SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_UNICODE_UTF_16));
		SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_LATIN_1));
		SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_SYSTEM_LANGUAGE));
		SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,config_write_mode,0);

		SetDlgItemTextA(hwndDlg,IDC_RATING_EMAIL,(config_rating_email[0] ? config_rating_email : "rating@winamp.com\0"));

		return FALSE;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if (pnmh->code == PSN_SETACTIVE)
			{
				config_lp = 1;
			}
			if (pnmh->code == PSN_APPLY)
			{
				config_parse_id3v1 = IsDlgButtonChecked(hwndDlg, IDC_READ_ID3V1);
				config_parse_id3v2 = IsDlgButtonChecked(hwndDlg, IDC_READ_ID3V2);

				config_write_id3v1 = IsDlgButtonChecked(hwndDlg, IDC_WRITE_ID3V1);
				config_write_id3v2 = IsDlgButtonChecked(hwndDlg, IDC_WRITE_ID3V2);

				config_create_id3v1 = IsDlgButtonChecked(hwndDlg, IDC_CREATE_ID3V1);
				config_create_id3v2 = IsDlgButtonChecked(hwndDlg, IDC_CREATE_ID3V2);

				GetDlgItemTextA(hwndDlg,IDC_RATING_EMAIL,config_rating_email,sizeof(config_rating_email));
				if (!stricmp(config_rating_email, "rating@winamp.com\0")) config_rating_email[0] = 0;

				return TRUE;
			}
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_COMBO1:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				int cur = (int)SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
				if(!cur) config_read_mode = READ_LATIN;
				else if(cur == 1) config_read_mode = READ_LOCAL;
			}
			break;
		case IDC_COMBO2:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				int cur = (int)SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
				if(!cur) config_write_mode = WRITE_UTF16;
				else if(cur == 1) config_write_mode = WRITE_LATIN;
				else if(cur == 2) config_write_mode = WRITE_LOCAL;
			}
			break;
		case IDC_RATING_EMAIL_RESET:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				config_rating_email[0] = 0;
				SetDlgItemTextA(hwndDlg,IDC_RATING_EMAIL,(config_rating_email[0] ? config_rating_email : "rating@winamp.com\0"));
			}
		}
		return FALSE;
	}
	return FALSE;
}

static INT_PTR CALLBACK advancedTaggingProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:

		if (config_parse_apev2) CheckDlgButton(hwndDlg, IDC_READ_APEV2, BST_CHECKED);
		if (config_write_apev2) CheckDlgButton(hwndDlg, IDC_WRITE_APEV2, BST_CHECKED);
		if (config_create_apev2) CheckDlgButton(hwndDlg, IDC_CREATE_APEV2, BST_CHECKED);

		if (config_parse_lyrics3) CheckDlgButton(hwndDlg, IDC_READ_LYRICS3, BST_CHECKED);

		SendDlgItemMessage(hwndDlg,IDC_APEV2_HEADER_OPTIONS,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_APEV2_RETAIN_HEADER));
		SendDlgItemMessage(hwndDlg,IDC_APEV2_HEADER_OPTIONS,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_APEV2_ADD_HEADER));
		SendDlgItemMessage(hwndDlg,IDC_APEV2_HEADER_OPTIONS,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_APEV2_REMOVE_HEADER));
		SendDlgItemMessage(hwndDlg,IDC_APEV2_HEADER_OPTIONS,CB_SETCURSEL,config_apev2_header, 0);

		return FALSE;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if (pnmh->code == PSN_SETACTIVE)
			{
				config_lp = 2;
			}
			if (pnmh->code == PSN_APPLY)
			{
		config_parse_apev2 = IsDlgButtonChecked(hwndDlg, IDC_READ_APEV2);
		config_write_apev2 = IsDlgButtonChecked(hwndDlg, IDC_WRITE_APEV2);
		config_create_apev2 = IsDlgButtonChecked(hwndDlg, IDC_CREATE_APEV2);

		config_parse_lyrics3 = IsDlgButtonChecked(hwndDlg, IDC_READ_LYRICS3);

				return TRUE;
			}
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_APEV2_HEADER_OPTIONS:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				int cur = (int)SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
				if(!cur) config_apev2_header = RETAIN_HEADER;
				else if(cur == 1) config_apev2_header = ADD_HEADER;
				else if(cur == 2) config_apev2_header = REMOVE_HEADER;
			}
			break;
		}
		return FALSE;
	}
	return FALSE;
}

static INT_PTR CALLBACK prefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(hwndDlg, IDC_EDIT1, config_extlist);
		SendDlgItemMessage(hwndDlg, IDC_EDIT1, EM_LIMITTEXT, 128, 0);
		{
			wchar_t str[10] = L"";
			wsprintf(str, L"%d", config_max_bufsize_k);
			SetDlgItemText(hwndDlg, IDC_BUFMAX, str);
			SendMessage(GetDlgItem(hwndDlg, IDC_BUFMAX), EM_LIMITTEXT, 5, 0);
		}

		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
			SetDlgItemText(hwndDlg, IDC_EDIT1, DEF_EXT_LISTW);
			break;
		}
		return FALSE;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if (pnmh->code == PSN_SETACTIVE)
			{
				config_lp = 0;
			}
			if (pnmh->code == PSN_APPLY)
			{
				config_max_bufsize_k = GetDlgItemInt(hwndDlg, IDC_BUFMAX, NULL, 0);
				GetDlgItemTextA(hwndDlg, IDC_EDIT1, config_extlist, 128);
				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}

static INT_PTR CALLBACK outputProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		if (config_eqmode&1) CheckDlgButton(hwndDlg, IDC_RADIO2, 1);
		else CheckDlgButton(hwndDlg, IDC_RADIO1, 1);

		if (!(config_eqmode&4)) CheckDlgButton(hwndDlg, IDC_FASTL3EQ, 1);
		if (config_eqmode&8) CheckDlgButton(hwndDlg, IDC_FASTL12EQ, 1);
		if (config_miscopts&1) CheckDlgButton(hwndDlg, IDC_CHECK1, BST_CHECKED);
		if (config_miscopts&2) CheckDlgButton(hwndDlg, IDC_CHECK2, BST_CHECKED);
		if (config_downmix == 2) CheckDlgButton(hwndDlg, IDC_REVSTEREO, BST_CHECKED);
		if (config_downsample == 1)
			CheckDlgButton(hwndDlg, IDC_HALFRATE, BST_CHECKED);
		else if (config_downsample == 2)
			CheckDlgButton(hwndDlg, IDC_QRATE, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_FULLRATE, BST_CHECKED);
		return FALSE;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if (pnmh->code == PSN_SETACTIVE)
			{
				config_lp = 3;
			}

			if (pnmh->code == PSN_APPLY)
			{
				config_miscopts &= ~3;
				config_miscopts |= IsDlgButtonChecked(hwndDlg, IDC_CHECK1) ? 1 : 0;
				config_miscopts |= IsDlgButtonChecked(hwndDlg, IDC_CHECK2) ? 2 : 0;

				config_eqmode = IsDlgButtonChecked(hwndDlg, IDC_RADIO1) ? 0 : 1;
				config_eqmode |= IsDlgButtonChecked(hwndDlg, IDC_FASTL3EQ) ? 0 : 4;
				config_eqmode |= IsDlgButtonChecked(hwndDlg, IDC_FASTL12EQ) ? 8 : 0;

				config_downmix = IsDlgButtonChecked(hwndDlg, IDC_REVSTEREO) ? 2 : 0;

				config_downsample = IsDlgButtonChecked(hwndDlg, IDC_HALFRATE) ? 1 : 0;
				config_downsample = IsDlgButtonChecked(hwndDlg, IDC_QRATE) ? 2 : config_downsample;

				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}

void SetHTTPSaveButtonText(HWND hwndDlg, char* path)
{
	HWND control = GetDlgItem(hwndDlg, IDC_BUTTON2);
	HDC hdc = GetDC(control);
	RECT r = {0};
	char temp[MAX_PATH] = {0};

	lstrcpynA(temp, path, MAX_PATH);
	SelectObject(hdc, (HFONT)SendMessage(control, WM_GETFONT, 0, 0));
	GetClientRect(control, &r);
	r.left += 5;
	r.right -= 5;
	DrawTextA(hdc, temp, -1, &r, DT_PATH_ELLIPSIS|DT_WORD_ELLIPSIS|DT_MODIFYSTRING);
	SetWindowTextA(control, temp);
	ReleaseDC(control, hdc);
}

static INT_PTR CALLBACK httpProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CHECK2:
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), IsDlgButtonChecked(hwndDlg, IDC_CHECK2));
			break;
		case IDC_BUTTON2:
			{
				BROWSEINFO bi = {0};
				wchar_t name[MAX_PATH] = {0};
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = name;
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
				bi.lpfn = BrowseCallbackProc;
				LPITEMIDLIST idlist = SHBrowseForFolder(&bi);
				if (idlist)
				{
					SHGetPathFromIDListA(idlist, g_http_tmp);
					IMalloc *m = 0;
					SHGetMalloc(&m);
					m->Free(idlist);
					SetHTTPSaveButtonText(hwndDlg, g_http_tmp);
				}
			}

			return 0;
		}
		return 0;
	case WM_INITDIALOG:
		SetDlgItemInt(hwndDlg, IDC_BUFFERS_NUMBUFS, config_http_buffersize, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER), TBM_SETRANGEMAX, 0, 50);
		SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER), TBM_SETRANGEMIN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER), TBM_SETPOS, 1, config_http_prebuffer / 2);
		SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER2), TBM_SETRANGEMAX, 0, 50);
		SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER2), TBM_SETRANGEMIN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER2), TBM_SETPOS, 1, config_http_prebuffer_underrun / 2);
		CheckDlgButton(hwndDlg, IDC_CHECK1, allow_sctitles);
		CheckDlgButton(hwndDlg, IDC_SC_ARTWORK, allow_scartwork);
		CheckDlgButton(hwndDlg, IDC_CHECK3, sctitle_format);

		if (config_miscopts&16)
		{
			CheckDlgButton(hwndDlg, IDC_CHECK2, BST_CHECKED);
		}
		EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), (config_miscopts&16));
		SetHTTPSaveButtonText(hwndDlg, config_http_save_dir);
		lstrcpynA(g_http_tmp, config_http_save_dir, MAX_PATH);

		return FALSE;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if (pnmh->code == PSN_SETACTIVE)
			{
				config_lp = 4;
			}

			if (pnmh->code == PSN_APPLY)
			{
				sctitle_format = !!IsDlgButtonChecked(hwndDlg, IDC_CHECK3);
				allow_sctitles = !!IsDlgButtonChecked(hwndDlg, IDC_CHECK1);
				allow_scartwork = !!IsDlgButtonChecked(hwndDlg, IDC_SC_ARTWORK);
				{
					int s;
					int t;
					t = GetDlgItemInt(hwndDlg, IDC_BUFFERS_NUMBUFS, &s, 0);
					if (s) config_http_buffersize = t;
					if (config_http_buffersize < 16) config_http_buffersize = 16;
				}
				config_http_prebuffer = (int)SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER), TBM_GETPOS, 0, 0) * 2;
				config_http_prebuffer_underrun = (int)SendMessage(GetDlgItem(hwndDlg, IDC_PREBUFSLIDER2), TBM_GETPOS, 0, 0) * 2;
				lstrcpynA(config_http_save_dir, g_http_tmp, MAX_PATH);
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK2))
				{
					config_miscopts |= 16;
				}
				else
				{
					config_miscopts &= ~16;
				}

				return TRUE;
			}
		}
		return FALSE;
	}

	const int controls[] = 
	{
		IDC_PREBUFSLIDER,
		IDC_PREBUFSLIDER2,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}

	return FALSE;
}

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCE(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirectW(&msgbx);
}

void about(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_MPEG_AUDIO_DECODER_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
			  mod.description, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}