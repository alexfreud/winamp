#include "main.h"
#ifndef IGNORE_API_GRACENOTE
#include "cddb.h"
#else
extern char config_use_cddb;
//char config_use_cddb = 0;
#endif
#include "api__in_cdda.h"
#include "../winamp/wa_ipc.h"

//int config_sample=1;
//int config_use_veritas=1;
//int config_rip_veritas=1;
//int config_maxextractspeed=4;
//int config_offset=0;
//int config_read_leadin=0;

//int config_rip_buffersize=24;
//int config_rip_buffers=256;

//int config_play_buffersize=1;
//int config_play_buffers=256;

char *INI_FILE = 0;
char app_name[] = "CDDA/Line Input Driver";

static int _r_i(char *name, int def)
{
	name += 7;
	return GetPrivateProfileIntA(app_name, name, def, INI_FILE);
}

#define RI(x) (( x ) = _r_i(#x,( x )))

static void _w_i(char *name, int d)
{
	char str[120] = {0};
	wsprintfA(str, "%d", d);
	name += 7;
	WritePrivateProfileStringA(app_name, name, str, INI_FILE);
}
#define WI(x) _w_i(#x,( x ))

void config_read()
{
	INI_FILE = (char*)SendMessage(line.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE);
	//RI(config_sample);
	RI(config_use_cddb);
	/*RI(config_use_veritas);
	config_rip_veritas=config_use_veritas;
	RI(config_rip_veritas);*/
	//RI(config_maxextractspeed);

	//RI(config_offset);

	//RI(config_rip_buffersize);
	//RI(config_rip_buffers);

	//RI(config_play_buffersize);
	//RI(config_play_buffers);
	//RI(config_read_leadin);
}

void config_write()
{
	//WI(config_sample);
	WI(config_use_cddb);
	//WI(config_use_veritas);
	//WI(config_rip_veritas);
	//WI(config_maxextractspeed);
}

// TODO need to review all of this!!!
#if 0
BOOL CALLBACK ripConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,	LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		//if (config_rip_veritas) CheckDlgButton(hwndDlg,IDC_VERITAS,BST_CHECKED);
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)L"0.5x");
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)L"1x");
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)L"2x");
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)L"4x");
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)L"8x");
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)L"16x");
		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_UNLIMITED));

		if (config_maxextractspeed<0)config_maxextractspeed=0;
		if (config_maxextractspeed>6)config_maxextractspeed=6;
		//if (config_maxextractspeed > 4 && getRegVer() < 1) config_maxextractspeed=4;

		SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,config_maxextractspeed,0);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		/*case IDC_VERITAS:
			config_rip_veritas = IsDlgButtonChecked(hwndDlg,IDC_VERITAS)?1:0;
			break;*/
		case IDC_COMBO1:
		{
			int x=(INT)SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
			if (x != CB_ERR)
			{
				if (x<0)x=0;
				if (x>6)x=6;
				/*if (x > 4 && getRegVer() < 1)
				{
					SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,4,0);
					x=4;
					config_maxextractspeed=x;
					wchar_t title[64] = {0};
					if (MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_PURCHASE_WINAMP_PRO_PROMPT),
					                WASABI_API_LNGSTRINGW_BUF(IDS_WINAMP_PRO_FEATURE,title,64),MB_YESNO) == IDYES)
					{
						SendMessage(line.hMainWindow,WM_WA_IPC,0,IPC_GETREGISTEREDVERSION);
					}
				}
				else*/ config_maxextractspeed=x;
			}
		}
		break;
		}
		return FALSE;
	case WM_DESTROY:
		config_write();
		return FALSE;
	}
	return 0;
}
#endif

static long cddbResourceID=202;
static long cddbMaxFrames=46;
static long cddbHeight=80, cddbWidth=80;
static HMODULE cddbUI = 0;


static bool GetUI()
{
	#ifndef IGNORE_API_GRACENOTE
	ICddbUIOptions *pUIOptions;
	if (SUCCEEDED(Cddb_GetIUIOptions((void**)&pUIOptions)))
	{
		if (SUCCEEDED(pUIOptions->GetCurrent(UI_DISP_PROGRESS))) // not sure what this does, but it seems to be required
		{
			pUIOptions->get_ProgressResourceID(&cddbResourceID);
			pUIOptions->get_Frames(&cddbMaxFrames);
			pUIOptions->get_Bottom(&cddbHeight);
			pUIOptions->get_Right(&cddbWidth);
			pUIOptions->get_ResourceHINSTANCE((long *)&cddbUI);
		}
		pUIOptions->Release();
		return true;
	}
	#endif
	return false;
}

static HANDLE cddbImage =0;
static int cddbFrame = 0;

BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DRAWITEM:
		if (wParam == IDC_CDDBICON)
		{
			DRAWITEMSTRUCT *drawItem = (DRAWITEMSTRUCT *) lParam;
			if (cddbImage)
			{
				HDC hdcbm = CreateCompatibleDC(drawItem->hDC);
				HGDIOBJ old = SelectObject(hdcbm, cddbImage);
				BitBlt(drawItem->hDC, 0, 0, cddbWidth, cddbHeight, hdcbm, (cddbFrame * cddbWidth), 0, SRCCOPY);
				SelectObject(hdcbm, old);
				DeleteDC(hdcbm);
			}
			else
			{
				RECT r={0,0,cddbWidth,cddbHeight};
				FillRect(drawItem->hDC, &r, (HBRUSH)GetStockObject(GRAY_BRUSH));
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		cddbImage=0;
		break;

	case WM_INITDIALOG:
		cddbFrame=0;
		cddbImage=0;
		{
			IUnknown *pUnknown = NULL;
			#ifndef IGNORE_API_GRACENOTE
			Cddb_GetIControl((void**)&pUnknown);
			#endif
			if (!pUnknown)
			{
				ShowWindow(GetDlgItem(hwndDlg,IDC_CDDBNOTE),SW_SHOWNA);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CDDB),SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_CDDBICON),SW_HIDE);
			}
			else pUnknown->Release();
		}

		//if (config_sample) CheckDlgButton(hwndDlg,IDC_SAMPLE,BST_CHECKED);
		if (config_use_cddb&1) CheckDlgButton(hwndDlg,IDC_CDDB,BST_CHECKED);
		//if (config_use_veritas) CheckDlgButton(hwndDlg,IDC_VERITAS,BST_CHECKED);

		if ((config_use_cddb&1) && GetUI())
		{
			cddbImage = LoadImage(cddbUI, MAKEINTRESOURCE(cddbResourceID), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
			SetTimer(hwndDlg, 2, 65, NULL);
		}
		SetWindowPos(GetDlgItem(hwndDlg, IDC_CDDBICON), 0, 0, 0, cddbWidth, cddbHeight, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		return TRUE;

	case WM_TIMER:
		switch (wParam)
		{
		case 2:
			if (cddbFrame < cddbMaxFrames-1)
			{
				cddbFrame++;
				InvalidateRect(GetDlgItem(hwndDlg, IDC_CDDBICON), 0, TRUE);
			}
			else
				KillTimer(hwndDlg, 2);

			break;
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CDDBICON:
			if (HIWORD(wParam) == STN_CLICKED) SendMessage(line.hMainWindow, WM_WA_IPC, (WPARAM)L"http://www.cddb.com/", IPC_OPEN_URL);
			return 0;
		case IDOK:
			//config_sample = IsDlgButtonChecked(hwndDlg,IDC_SAMPLE)?1:0;
			config_use_cddb = IsDlgButtonChecked(hwndDlg,IDC_CDDB)?1:0;
			//config_use_veritas = IsDlgButtonChecked(hwndDlg,IDC_VERITAS)?1:0;
			config_write();
		case IDCANCEL:
			EndDialog(hwndDlg,1);
			return FALSE;
		}
		return FALSE;
	}
	return 0;
}

void config(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_DIALOG1,hwndParent,ConfigProc);
}