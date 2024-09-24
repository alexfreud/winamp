#include "../gracenote/gracenote.h"
#include "api__ml_plg.h"
#include <windows.h>
#include "resource.h"
#include "../../General/gen_ml/ml.h"
#include "../winamp/wa_ipc.h"
#include "../Agave/Language/api_language.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/ComboBox.h"

#include "main.h"
#include <shlwapi.h>
#include <assert.h>
#include "playlist.h"
#include <atlbase.h>
#include "IDScanner.h"
#include <strsafe.h>

extern bool pluginEnabled;
extern int scanMode;
extern int plLengthType;
volatile bool is_scan_running=false;

static bool IsScanRunning()
{
	return is_scan_running; 
}

void ShowErrorDlg(HWND parent)
{
	wchar_t title[32] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_INITIALIZING,title,32);
	MessageBox(parent, WASABI_API_LNGSTRINGW(IDS_INITFAILMSG),title,0);
}

IDScanner scanner;

int ShutdownScanner(HANDLE handle, void *user_data, intptr_t id)
{
	HANDLE event = (HANDLE)user_data;
	scanner.Shutdown();
	ShutdownPlaylistSDK();
	SetEvent(event);
	return 0;
}

static int ScanThread(HANDLE handle, void *user_data, intptr_t id)
{
	is_scan_running=true;
	scanner.ScanDatabase();
	is_scan_running=false;
	return 0;
}

static void doProgressBar(HWND h, int x, int t=-1) {
	h = GetDlgItem(h,IDC_PROGRESS1);
	if(t!=-1 && SendMessage(h,PBM_GETRANGE,0,0) != t)
		SendMessage(h,PBM_SETRANGE32,0,t);
	SendMessage(h,PBM_SETPOS,x,0);
}

static void FillStatus(HWND hwndDlg)
{
	long state, track, tracks;
	if (scanner.GetStatus(&state, &track, &tracks))
	{
		static int x=0;
		wchar_t *ticker;
		switch (x++)
		{
			case 0:			ticker=L""; break;
			case 1:			ticker=L"."; break;
			case 2:			ticker=L".."; break;
			default:		ticker=L"...";
		}
		x%=4;
		wchar_t status[1024]=L"";
		switch (state)
		{
		case IDScanner::STATE_ERROR:
			WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_INITIALIZING,status,1024);
			KillTimer(hwndDlg, 1);
			doProgressBar(hwndDlg,0,1);
			ShowErrorDlg(hwndDlg);
			break;
		case IDScanner::STATE_IDLE:
			WASABI_API_LNGSTRINGW_BUF(IDS_IDLE,status,1024);
			doProgressBar(hwndDlg,0);
			break;
		case IDScanner::STATE_INITIALIZING:
			StringCchPrintfW(status, 1024, WASABI_API_LNGSTRINGW(IDS_INITIALIZING), ticker);
			doProgressBar(hwndDlg,0);
			break;
		case IDScanner::STATE_SYNC:
			StringCchPrintfW(status, 1024, WASABI_API_LNGSTRINGW(IDS_SYNC), track, tracks, ticker);
			doProgressBar(hwndDlg,track,tracks);
			break;
		case IDScanner::STATE_METADATA:
			StringCchPrintfW(status, 1024, WASABI_API_LNGSTRINGW(IDS_METADATA), track, tracks, ticker);
			doProgressBar(hwndDlg,track,tracks);
			break;
		case IDScanner::STATE_MUSICID:
			StringCchPrintfW(status, 1024, WASABI_API_LNGSTRINGW(IDS_MUSICID), track, tracks, ticker);
			doProgressBar(hwndDlg,track,tracks);
			break;
		case IDScanner::STATE_DONE:
			WASABI_API_LNGSTRINGW_BUF(IDS_DONE,status,1024);
			doProgressBar(hwndDlg,100,100);
			KillTimer(hwndDlg, 1);
			SetDlgItemText(hwndDlg,IDC_TEST,WASABI_API_LNGSTRINGW(IDS_SCAN));
			break;
		}
		SetDlgItemTextW(hwndDlg, IDC_STATUS, status);
	}
}
#if 0 // TODO: reimplement contract requirement
LRESULT CALLBACK DeviceMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
	{
		if (wParam == 0)
		{
			KillTimer(hwnd, 0);
		if (CheckThread())
		{
			PostMessage(plugin.hwndLibraryParent, WM_USER+641, 0, 0);
			SetTimer(hwnd, 1, 3000, 0);
		}
		else
			DestroyWindow(hwnd);
		}
		else if (wParam == 1)
		{
			KillTimer(hwnd, 1);
			PostMessage(plugin.hwndLibraryParent, WM_USER+641, 1, 0);
			SetTimer(hwnd, 0, 27000, 0);
		}
	}
	else if (uMsg == WM_DESTROY)
	{
		PostMessage(plugin.hwndLibraryParent, WM_USER+641, 1, 0);
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static bool classRegistered=0;
HWND CreateDummyWindow()
{
	if (!classRegistered)
	{
		WNDCLASSW wc = {0, };

		wc.style = 0;
		wc.lpfnWndProc = DeviceMsgProc;
		wc.hInstance = plugin.hDllInstance;
		wc.hIcon = 0;
		wc.hCursor = NULL;
		wc.lpszClassName = L"ml_plg_window";

		if (!RegisterClassW(&wc))
			return 0;

		classRegistered = true;
	}
	HWND dummy = CreateWindowW(L"ml_plg_window", L"ml_plg_window", 0, 0, 0, 0, 0, NULL, NULL, plugin.hDllInstance, NULL);

	return dummy;
}
#endif

bool StartScan()
{
	if (IsScanRunning())
	{
		//run_full_scan_flag = true;			// Not really necessary since we are already running the scan
		return false;
	}

	if (reset_db_flag)							// See if we have the flag set to reset the DB.
	{
		SetDlgItemTextW(hwndDlgCurrent, IDC_STATIC_PROGRESS_STATE, WASABI_API_LNGSTRINGW(IDS_RESETDB));
		NukeDB();
		//ResetDBOnThread(true);
		reset_db_flag = false;
	}
	
	if (run_full_scan_flag)						// See if we have the flag set for running the whole scan
	{
		// Call the scan procedure on the reserved gracenote specific thread
		if (WASABI_API_THREADPOOL->RunFunction(plg_thread, ScanThread, 0, 0, api_threadpool::FLAG_REQUIRE_COM_STA) == 0)
		{
			run_full_scan_flag = false;			// Reset the flag to false since we (will) complete the run
			return true;
		}
	}
	else if (run_pass2_flag)					// If we have the pass2 scan flag set then run the second pass only
	{
		if (WASABI_API_THREADPOOL->RunFunction(plg_thread, IDScanner::Pass2OnThread, 0, (intptr_t)&scanner, api_threadpool::FLAG_REQUIRE_COM_STA) == 0)
		{
			run_pass2_flag = false;				// Set the flag back to false so we know that we wont have to run it again
			return true;
		}
	}


	return false;
}

void StopScan()
{
	if (IsScanRunning())
	{
		scanner.Kill();
		while (IsScanRunning())
			Sleep(500);
		run_full_scan_flag = true;			// Set the flag so that we rerun everything on a rescan
	}
}

void SetPlLengthTypeComboToItems(HWND hwndDlg, int value)
{
		ComboBox combo(hwndDlg, IDC_COMBO_LENGTH);
		combo.Clear();
		combo.SetItemData(combo.AddString("10"),10);
		combo.SetItemData(combo.AddString("20"),20);
		combo.SetItemData(combo.AddString("50"),50);
		combo.SetItemData(combo.AddString("100"),100);
		combo.SetItemData(combo.AddString("200"),200);
		wchar_t buf[32] = {0};
		_itow(value,buf,10);
		combo.SetEditText(buf);
}

void SetPlLengthTypeComboToMinutes(HWND hwndDlg, int value)
{
		ComboBox combo(hwndDlg, IDC_COMBO_LENGTH);
		combo.Clear();
		combo.SetItemData(combo.AddString("10"),10);
		combo.SetItemData(combo.AddString("20"),20);
		combo.SetItemData(combo.AddString("30"),30);
		combo.SetItemData(combo.AddString("60"),60);
		combo.SetItemData(combo.AddString("74"),60);
		combo.SetItemData(combo.AddString("80"),60);
		combo.SetItemData(combo.AddString("90"),60);
		combo.SetItemData(combo.AddString("120"),120);
		wchar_t buf[32] = {0};
		_itow(value,buf,10);
		combo.SetEditText(buf);
}

void SetPlLengthTypeComboToMegabytes(HWND hwndDlg, int value)
{
		ComboBox combo(hwndDlg, IDC_COMBO_LENGTH);
		combo.Clear();
		combo.SetItemData(combo.AddString("100"),10);
		combo.SetItemData(combo.AddString("650"),20);
		combo.SetItemData(combo.AddString("703"),30);
		combo.SetItemData(combo.AddString("4489"),60);
		combo.SetItemData(combo.AddString("8147"),120);
		
		wchar_t buf[32] = {0};
		_itow(value,buf,10);
		combo.SetEditText(buf);
}

// Set a control to bold text and a more bold font
void BoldStatusText(HWND hwndControl)
{
	HFONT hFont ;

	LOGFONT lfFont;

	memset(&lfFont, 0x00, sizeof(lfFont));
	memcpy(lfFont.lfFaceName, TEXT("Microsoft Sans Serif"), 24);

	lfFont.lfHeight   = 14;
	lfFont.lfWeight   = FW_BOLD;
	lfFont.lfCharSet  = ANSI_CHARSET;
	lfFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfFont.lfQuality  = DEFAULT_QUALITY;

	// Create the font from the LOGFONT structure passed.
	hFont = CreateFontIndirect (&lfFont);

	//SendMessageW(GetDlgItem(hwndDlg,IDC_STATUS), WM_SETFONT, (LPARAM)hFont, MAKELONG(TRUE, 0 ) );
	SendMessageW(hwndControl, WM_SETFONT, (LPARAM)hFont, MAKELONG(TRUE, 0 ) );
}

void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;
	if (p || sr || wnd)
	{
		HMONITOR hm = NULL;
		if (sr)
			hm = MonitorFromRect(sr, MONITOR_DEFAULTTONEAREST);
		else if (wnd)
			hm = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
		else if (p)
			hm = MonitorFromPoint(*p, MONITOR_DEFAULTTONEAREST);

		if (hm)
		{
			MONITORINFOEXW mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfoW(hm, &mi))
			{
				if (!full)
					*r = mi.rcWork;
				else
					*r = mi.rcMonitor;
				return ;
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top = r->left = 0;
		r->right = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfoW(SPI_GETWORKAREA, 0, r, 0);
	}
}

BOOL windowOffScreen(HWND hwnd, POINT pt)
{
	RECT r = {0}, wnd = {0}, sr = {0};
	GetWindowRect(hwnd, &wnd);
	sr.left = pt.x;
	sr.top = pt.y;
	sr.right = sr.left + (wnd.right - wnd.left);
	sr.bottom = sr.top + (wnd.bottom - wnd.top);
	getViewport(&r, hwnd, 0, &sr);
	return !PtInRect(&r, pt);
}

#define STATUS_MS 500
INT_PTR CALLBACK PrefsProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// this will make sure that we've got thr aacplus logo shown even when using a localised version
			SendDlgItemMessage(hwndDlg,IDC_LOGO,STM_SETIMAGE,IMAGE_BITMAP,
							   (LPARAM)LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(IDB_GN_LOGO),IMAGE_BITMAP,0,0,LR_SHARED));

			SetDlgItemText(hwndDlg,IDC_BLURB,WASABI_API_LNGSTRINGW(IDS_SCANNER_BLURB));
			if (IsScanRunning())
			{
				//EnableWindow(GetDlgItem(hwndDlg, IDC_TEST), FALSE);
				SetDlgItemText(hwndDlg,IDC_TEST,WASABI_API_LNGSTRINGW(IDS_PAUSE));
				SetTimer(hwndDlg, 1, STATUS_MS, 0);
			}
			
			BoldStatusText(GetDlgItem(hwndDlg, IDC_STATUS) );
			FillStatus(hwndDlg);

			/*if(!pluginEnabled)
				CheckDlgButton(hwndDlg,IDC_SCANDISABLE,TRUE);
			else */
			if(scanMode == 1)
				CheckDlgButton(hwndDlg,IDC_SCANLAUNCH,TRUE);
			else
				CheckDlgButton(hwndDlg,IDC_SCANONUSE,TRUE);

			SetRadioControlsState(hwndDlg);				// Set the playlist length radio buttons state

			if(scanMode == 0 || pluginEnabled == false)
			{
				pluginEnabled = true;
				scanMode = 2;
			}
			if(multipleArtists)
				CheckDlgButton(hwndDlg,IDC_CHECK_MULTIPLE_ARTISTS,TRUE);
			if(multipleAlbums)
				CheckDlgButton(hwndDlg,IDC_CHECK_MULTIPLE_ALBUMS,TRUE);
			if(useSeed)
				CheckDlgButton(hwndDlg,IDC_CHECK_USE_SEED,TRUE);

			// show config window and restore last position as applicable
			POINT pt = {(LONG)GetPrivateProfileInt(L"ml_plg", L"prefs_x", -1, mediaLibrary.GetWinampIniW()),
						(LONG)GetPrivateProfileInt(L"ml_plg", L"prefs_y", -1, mediaLibrary.GetWinampIniW())};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
		break;
		case WM_TIMER:
			FillStatus(hwndDlg);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_TEST:
				{
					if(StartScan())			// Start the scanning
					{
						SetDlgItemText(hwndDlg,IDC_TEST,WASABI_API_LNGSTRINGW(IDS_PAUSE));
						//EnableWindow(GetDlgItem(hwndDlg, IDC_TEST), FALSE);
						SetTimer(hwndDlg, 1, STATUS_MS, 0);		// Start the timer ?
					}
					else					// Stop the scanning
					{
						StopScan();
						SetDlgItemText(hwndDlg,IDC_TEST,WASABI_API_LNGSTRINGW(IDS_SCAN));
						SetDlgItemText(hwndDlg,IDC_STATUS,WASABI_API_LNGSTRINGW(IDS_PAUSE));
					}
				}
				break;
				case IDOK:
				case IDCANCEL:
				{
					RECT rect = {0};
					GetWindowRect(hwndDlg, &rect);
					char buf[16] = {0};
					StringCchPrintfA(buf, 16, "%d", rect.left);
					WritePrivateProfileStringA("ml_plg", "prefs_x", buf, mediaLibrary.GetWinampIni());
					StringCchPrintfA(buf, 16, "%d", rect.top);
					WritePrivateProfileStringA("ml_plg", "prefs_y", buf, mediaLibrary.GetWinampIni());

					EndDialog(hwndDlg, 0);

					StringCchPrintfA(buf, 10, "%d", scanMode);
					WritePrivateProfileStringA("ml_plg", "scanmode", buf, mediaLibrary.GetWinampIni());
					WritePrivateProfileStringA("ml_plg", "enable", pluginEnabled ? "1" : "0", mediaLibrary.GetWinampIni());
				}
				break;
				case IDC_SCANDISABLE:
					pluginEnabled = false;
					scanMode = 2;
					break;
				case IDC_SCANONUSE:
					pluginEnabled = true;
					scanMode = 2;
					break;
				case IDC_SCANLAUNCH:
					pluginEnabled = true;
					scanMode = 1;
					break;
				
				case IDC_RESETDB:
				{
					wchar_t title[32] = {0};
					WASABI_API_LNGSTRINGW_BUF(IDS_RESETDB,title,32);
					if(MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_RESETDB_TEXT),title,MB_YESNO) == IDNO)
						break;

					StopScan();
					SetDlgItemText(hwndDlg,IDC_TEST,WASABI_API_LNGSTRINGW(IDS_SCAN));
					ResetDBOnThread(false);
					//NukeDB();
				}
				break;
			}
			break;
	}
	return 0;
}

int DeleteGracenoteMLDBOnThread(HANDLE handle, void *user_data, intptr_t id)
{
	HANDLE event = (HANDLE)user_data;
	SetupPlaylistSDK();				// Need to set up the playlist SDK so that we have the MLDB manager ready to go, which is linked to the playlist manager
	DeleteGracenoteMLDB(!!id);		// This will Shutdown the playlist manager in order to allow for a delete, id is recreating the silent boolean double NOT is creating int -> bool
	ShutdownPlaylistSDK();			// Hence we need to do a complete and proper shutdown afterwards
	SetEvent(event);
	return 0;
}

// silent - pass in true if the user should not be prompted on errors
int ResetDBOnThread(bool silent)
{
	int silentInt = (silent) ? 1 : 0;		// Convert silent to int to pass it to wasabi thread runner
	/*HANDLE wait_event = CreateEvent(NULL, FALSE, FALSE, 0);
	WASABI_API_THREADPOOL->RunFunction(plg_thread, ShutdownScanner, (void *)wait_event, 0, api_threadpool::FLAG_REQUIRE_COM_STA);
	WaitForSingleObject(wait_event, INFINITE);*/

	HANDLE wait_event = CreateEvent(NULL, FALSE, FALSE, 0);
	WASABI_API_THREADPOOL->RunFunction(plg_thread, DeleteGracenoteMLDBOnThread, (void *)wait_event, silentInt, api_threadpool::FLAG_REQUIRE_COM_STA);
	WaitForSingleObject(wait_event, INFINITE);

	run_full_scan_flag = true;			// Set the flag so that we rerun everything on a rescan
		
	return 0;
}

int ResetDB(bool silent)
{
	int silentInt = silent;

	HANDLE wait_event = CreateEvent(NULL, FALSE, FALSE, 0);
	DeleteGracenoteMLDBOnThread (0, (void *)wait_event, silentInt);
	
	run_full_scan_flag = true;			// Set the flag so that we rerun everything on a rescan
		
	return 0;
}

BOOL DeleteGracenoteFile(char *filename)
{
	BOOL result;
	char path[MAX_PATH] = {0};
	PathCombineA(path,mediaLibrary.GetIniDirectory(),"Plugins\\Gracenote");
	PathAppendA(path, filename);
	result = DeleteFileA(path);
	return result;
}

int NukeDB(void)
{
	ShutdownPlaylistSDK();

	DeleteGracenoteFile("cddb.db");
	DeleteGracenoteFile("cddbplm.chk");
	DeleteGracenoteFile("cddbplm.gcf");
	DeleteGracenoteFile("cddbplm.idx");
	DeleteGracenoteFile("cddbplm.pdb");
	DeleteGracenoteFile("elists.db");
	
	run_full_scan_flag = true;			// Set the flag so that we rerun everything on a rescan
		
	return 0;
}



INT_PTR CALLBACK BGScanProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
		if(!StartScan())
		{
			EndDialog(hwndDlg,0);
			return 0;
		}
		SetTimer(hwndDlg,1,STATUS_MS,NULL);
		ShowWindow(hwndDlg,SW_HIDE);

		break;
	case WM_TIMER:
		switch(wParam)
		{
		case 1:
		{
			long state, track, tracks;
			if (scanner.GetStatus(&state, &track, &tracks))
			{
				if(state == IDScanner::STATE_MUSICID && tracks != 0 && track != 0)
				{
					KillTimer(hwndDlg,1);
					ShowWindow(hwndDlg,SW_SHOWNA);
					SetTimer(hwndDlg,2,3000,NULL);
				}
				else if(state == IDScanner::STATE_DONE)
					EndDialog(hwndDlg,0);
				else if(state == IDScanner::STATE_ERROR)
				{
					EndDialog(hwndDlg,0);
					if(!GetWindowLongPtr(hwndDlg,GWLP_USERDATA)){
						KillTimer(hwndDlg,1);
						ShowErrorDlg(hwndDlg);
					}
				}
			}
		}
		break;
		case 2:
		EndDialog(hwndDlg,0);
		break;
		}
	break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON1:
			EndDialog(hwndDlg,0);
			WASABI_API_DIALOGBOXW(IDD_PREFS, plugin.hwndLibraryParent, PrefsProcedure);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,0);
			break;
		}
		break;
	}
	return 0;
}