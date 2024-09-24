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
//#include "../Wasabi/bfc/util/timefmt.h"
//#include <bfc/util/timefmt.h>

#include <strsafe.h>	// should be last

HWND hwndDlgCurrent = 0;
bool optionsVisible = true;
bool isGenerating = false;
int originalWidth = 877;
//#define DIALOG_WIDTH_OPTIONS		877 // use originalWidth instead
#define DIALOG_WIDTH_NO_OPTIONS		610
#define DIALOG_HIDDEN_COLUMN_ID		4

// Pass in 0 for width or height in order to preserve its current dimension
void SizeWindow(HWND hwnd, int width, int height)
{
	if (width == 0 || height == 0)		// Preserve only if one of the items is 0
	{
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
	
		if (width == 0)					// Preserve the width
			width = windowRect.right - windowRect.left;
		if (height == 0)				// Preserve the height
			height = windowRect.bottom - windowRect.top;
	}
	SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void ClientResize(HWND hWnd, int nWidth, int nHeight)
{
	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
	MoveWindow(hWnd,rcWind.left, rcWind.top, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);
}

void SetMarqueeProgress(bool isMarquee)
{
	HWND hwndProgress = GetDlgItem(hwndDlgCurrent,IDC_PROGRESS_GENERATE);
	static long state = GetWindowLongW(hwndProgress, GWL_STYLE);		// Capture the initial state of the progress bar
	
	
	if (isMarquee)		// Set it to marquee style
	{
		SetWindowLong (hwndProgress, GWL_STYLE, GetWindowLong(hwndProgress, GWL_STYLE) | PBS_MARQUEE);
		//SendMessage(hwndProgress, PBM_SETMARQUEE, 1, 10);
		SendMessage(hwndProgress, PBM_SETMARQUEE, 1, 30);
	}
	else				// Restore the normal progress bar
	{
		SetWindowLong (hwndProgress, GWL_STYLE, state);
		//SendMessage(hwndProgress, WM_PAINT, 0, 0);
		InvalidateRect(hwndProgress, 0, 1);					// Force a repaint of the marquee after turning it off because there are stuck pixels in XP
	}
}

// Sets the query check state as well as enabling all the controls involved
void SetMLQueryCheckState(HWND hwndDlg, unsigned int checked)
{
	// Get the handles to all the child controls we want to enable / disable
	HWND hwndButtonMlQuery = GetDlgItem(hwndDlg, IDC_BUTTON_ML_QUERY);
	HWND hwndEditMlQuery = GetDlgItem(hwndDlg, IDC_EDIT_ML_QUERY);
	HWND hwndButtonRestoreQueryDefault = GetDlgItem(hwndDlg, IDC_BUTTON_RESTORE_QUERY_DEFAULT);
	
	if (checked)			// enable all the controls related to ML query
	{
		CheckDlgButton(hwndDlg, IDC_CHECK_ML_QUERY, TRUE);
		EnableWindow (hwndButtonMlQuery, TRUE );
		EnableWindow (hwndEditMlQuery, TRUE );
		EnableWindow (hwndButtonRestoreQueryDefault, TRUE );
		useMLQuery = true;
	}
	else					// disable all the controls related to ML query
	{
		CheckDlgButton(hwndDlg, IDC_CHECK_ML_QUERY, FALSE);
		EnableWindow (hwndButtonMlQuery, FALSE );
		EnableWindow (hwndEditMlQuery, FALSE );
		EnableWindow (hwndButtonRestoreQueryDefault, FALSE );
		useMLQuery = false;
	}
}

void SetButtonsEnabledState(bool enabled_flag)
{
	int itemIds[] =
	{
		IDC_BUTTON_PLAY_NOW,
		IDC_BUTTON_ENQUEUE_NOW,
		IDC_BUTTON_SAVEAS,
		IDC_BUTTON_REGENERATE
	};

	for(int i = 0; i < sizeof(itemIds) / sizeof(itemIds[0]); i++)
		EnableWindow(GetDlgItem(hwndDlgCurrent, itemIds[i]), enabled_flag);
}

void ToggleOptions(bool reset)
{
	if (reset)
		optionsVisible = false;
	else
		optionsVisible = !optionsVisible;			// Toggle the options visible state

	// to resolve tabbing issues when in the collapsed
	// state we need to disable some of the controls (dro)
	int itemIds[] = {
		IDC_RADIO_PLAYLIST_ITEMS,
		IDC_RADIO_PLAYLIST_LENGTH,
		IDC_RADIO_PLAYLIST_SIZE,
		IDC_COMBO_LENGTH,
		IDC_CHECK_USE_SEED,
		IDC_CHECK_MULTIPLE_ARTISTS,
		IDC_CHECK_MULTIPLE_ALBUMS,
		IDC_CHECK_ML_QUERY,
		IDC_EDIT_ML_QUERY,
		IDC_BUTTON_ML_QUERY,
		IDC_BUTTON_RESTORE_QUERY_DEFAULT
	};
	for(int i = 0; i < sizeof(itemIds) / sizeof(itemIds[0]); i++)
		EnableWindow(GetDlgItem(hwndDlgCurrent, itemIds[i]), optionsVisible);

	SetMLQueryCheckState(hwndDlgCurrent, useMLQuery);
	
	if (optionsVisible)
	{
		SizeWindow(hwndDlgCurrent, originalWidth, 0);		// Resize the window to the correct width
		SetDlgItemText(hwndDlgCurrent, IDC_BUTTON_OPTIONS, WASABI_API_LNGSTRINGW(IDS_OPTIONS));		// Set the dialog button to show the correct options mode
	}
	else
	{
		SizeWindow(hwndDlgCurrent, DIALOG_WIDTH_NO_OPTIONS, 0);
		SetDlgItemText(hwndDlgCurrent, IDC_BUTTON_OPTIONS, WASABI_API_LNGSTRINGW(IDS_NO_OPTIONS));		// Set the dialog button to show the correct options mode
	}
}

// ToDo: Make this more human readable
void FormatToMinutesAndSeconds(const int lengthInSeconds, wchar_t *buff, const size_t cchBuf)
{
	//StringCchPrintfW(buff, cchBuf, L"%d:%02d", lengthInSeconds / 60, lengthInSeconds % 60);

	int total_length_s = lengthInSeconds;
	int uncert = 0;

	// Minutes and seconds
	if (total_length_s < 60*60) StringCchPrintfW(buff, 64, L"%s%u:%02u", uncert ? L"~" : L"", total_length_s / 60, total_length_s % 60);
	// Hours minutes and seconds
	else if (total_length_s < 60*60*24) StringCchPrintfW(buff, 64, L"%s%u:%02u:%02u", uncert ? L"~" : L"", total_length_s / 60 / 60, (total_length_s / 60) % 60, total_length_s % 60);
	else
	{
		wchar_t days[16] = {0};
		int total_days = total_length_s / (60 * 60 * 24);		// Calculate days
		total_length_s -= total_days * 60 * 60 * 24;			// Remove days from length
		StringCchPrintfW(buff, 64,
			//WASABI_API_LNGSTRINGW(IDS_LENGTH_DURATION_STRING),
			L"%s%u %s+%u:%02u:%02u",
			((uncert) ? L"~" : L""), total_days,											// Approximate
			WASABI_API_LNGSTRINGW_BUF(total_days == 1 ? IDS_DAY : IDS_DAYS, days, 16),		// Days
			total_length_s / 60 / 60,														// Hours
			(total_length_s / 60) % 60,														// Minutes
			total_length_s % 60);															// Seconds
	}
}

// Refreashed the statistics about the generated playlist
void UpdateStats(void)
{
	const int MAX_STATS = 512;
	wchar_t stats[MAX_STATS] = {0};
	wchar_t lengthText[MAX_STATS] = {0};
	wchar_t sizeText[MAX_STATS] = {0};
	int count = (int)currentPlaylist.GetNumItems();
	uint64_t length = currentPlaylist.GetPlaylistLengthMilliseconds();
	uint64_t size = currentPlaylist.GetPlaylistSizeBytes();

	// Add the seed stats?
	if (useSeed == TRUE)
	{
		count += (int)seedPlaylist.GetNumItems();
		length += seedPlaylist.GetPlaylistLengthMilliseconds();
		size += seedPlaylist.GetPlaylistSizeBytes();
	}
	
	FormatToMinutesAndSeconds((int)(length / 1000), lengthText, MAX_STATS);		// / 1000 because we have it in milliseconds and not seconds
	StrFormatByteSizeW(size, sizeText, MAX_STATS);								// Get the human readable formatting for filesize
	
	StringCchPrintf(stats, MAX_STATS, WASABI_API_LNGSTRINGW(IDS_STATS), count, lengthText, sizeText);

	SetDlgItemText(hwndDlgCurrent, IDC_STATIC_STATS, stats);		// Set the dialog button to show the correct options mode
}

// Update the progress to the current
static void doProgressBar(HWND h, int x, int t=-1) {
	h = GetDlgItem(h,IDC_PROGRESS_GENERATE);
	if(t!=-1 && SendMessage(h,PBM_GETRANGE,0,0) != t)
		SendMessage(h,PBM_SETRANGE32,0,t);
	SendMessage(h,PBM_SETPOS,x,0);
}

// Update the status while id scanner is active 
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
			if (!isGenerating)				// Only set the done state if the gneeration has not started yet
			{
				WASABI_API_LNGSTRINGW_BUF(IDS_DONE,status,1024);
				doProgressBar(hwndDlg,0,0);	// Turn off the progress bar to 0
			}

			KillTimer(hwndDlg, 1);
			break;
		}
		if (!isGenerating)					// Only set the done state if the gneeration has not started yet
		{
			SetDlgItemTextW(hwndDlg, IDC_STATIC_PROGRESS_STATE, status);
		}
	}
}

// Function calls appropriate items when a generation is requested
void Regenerate(HWND hwndDlg)
{
	SendMessage(GetDlgItem(hwndDlgCurrent, IDC_LIST_RESULTS2),LVM_DELETEALLITEMS,0,0);		// Clear the listview of all playlist items
	
	SetTimer(hwndDlg, 1, 500, 0);		// Set the progress timer for the scanner
	StartScan();

	MoreLikeTheseSongs(&seedPlaylist);
}

// Function draws in colors for the seed listview items
LRESULT CustomDrawListViewColors(LPARAM lParam)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	switch(lplvcd->nmcd.dwDrawStage) 
	{
	case CDDS_PREPAINT : //Before the paint cycle begins
		return CDRF_NOTIFYITEMDRAW;	//request notifications for individual listview items

	case CDDS_ITEMPREPAINT: //Before an item is drawn
		if (lplvcd->nmcd.dwItemSpec < seedPlaylist.entries.size())	// Check how many seeds we have, thats how we know which rows in the view to color paint
		{
			if (useSeed == TRUE)
			{
				lplvcd->clrText   = RGB(0,0,255);		// Color seed tracks blue
			}
			else
			{
				lplvcd->clrText   = RGB(100,100,100);	// Color seed tracks a faded grey
			}
		}
		return CDRF_NEWFONT;
		break;
	}
	return CDRF_DODEFAULT;
}

int SetRadioControlsState(HWND hwndDlg)
{
		// Set the radio buttons for playlist length type, items or minutes
		if(plLengthType == PL_ITEMS)
		{
			CheckDlgButton(hwndDlg,IDC_RADIO_PLAYLIST_ITEMS,TRUE);
			SendMessage(hwndDlg, WM_COMMAND, IDC_RADIO_PLAYLIST_ITEMS, 0);
			SetPlLengthTypeComboToItems(hwndDlg, plItems);
		}
		else if(plLengthType == PL_MINUTES)
		{
			CheckDlgButton(hwndDlg,IDC_RADIO_PLAYLIST_LENGTH,TRUE);
			SendMessage(hwndDlg, WM_COMMAND, IDC_RADIO_PLAYLIST_LENGTH, 0);
			SetPlLengthTypeComboToMinutes(hwndDlg, plMinutes);
		}
		else if(plLengthType == PL_MEGABYTES)
		{
			CheckDlgButton(hwndDlg,IDC_RADIO_PLAYLIST_SIZE,TRUE);
			SendMessage(hwndDlg, WM_COMMAND, IDC_RADIO_PLAYLIST_SIZE, 0);
			SetPlLengthTypeComboToMegabytes(hwndDlg, plMegabytes);
		}	

	return 0;
}

// Update the combo box contents depending on which lengthType we are using
int UpdateComboLength(HWND hwndDlg)
{
	const int BUF_SIZE = 32;
	ComboBox combo(hwndDlg, IDC_COMBO_LENGTH);
	wchar_t buf[BUF_SIZE] = {0};
	combo.GetEditText(buf, BUF_SIZE);
	
	switch(plLengthType)
	{
	case PL_ITEMS:
		plItems = _wtoi(buf);
		return 0;
		break;
	case PL_MINUTES:
		plMinutes = _wtoi(buf);
		return 0;
		break;
	case PL_MEGABYTES:
		plMegabytes = _wtoi(buf);
		return 0;
		break;
	}
	return 1;
}

LRESULT tab_fix_proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CHAR)
	{
		if(wParam == VK_TAB)
		{
			SendMessage(hwndDlgCurrent, WM_NEXTDLGCTL, (GetAsyncKeyState(VK_SHIFT)&0x8000), FALSE);
			return TRUE;
		}
	}

	return CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"tab_fix_proc"), hwndDlg, uMsg, wParam, lParam);
}

// this will prevent the hidden column (for making the headers work better)
// from appearing as sizeable / disabled (as it effectively is)
LRESULT header_block_proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_SETCURSOR)
	{
		HDHITTESTINFO hitTest;
		GetCursorPos(&hitTest.pt);
		ScreenToClient(hwndDlg, &hitTest.pt);
		hitTest.flags = hitTest.iItem = 0;
		SendMessage(hwndDlg, HDM_HITTEST, FALSE, (LPARAM)&hitTest);
		if(hitTest.iItem == DIALOG_HIDDEN_COLUMN_ID || hitTest.iItem == -1)
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return TRUE;
		}
	}

	return CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"header_block_proc"), hwndDlg, uMsg, wParam, lParam);
}

INT_PTR CALLBACK GenerateProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			RECT r;
			GetWindowRect(hwndDlg, &r);
			originalWidth = r.right - r.left;

			// bit hacky but it will resolve issues with tabbing and the combobox in a
			// dropdown style still not 100% sure why it's failing to work though (dro)
			HWND combobox = GetWindow(GetDlgItem(hwndDlg, IDC_COMBO_LENGTH), GW_CHILD);
			SetPropW(combobox, L"tab_fix_proc",(HANDLE)SetWindowLongPtrW(combobox, GWLP_WNDPROC, (LONG_PTR)tab_fix_proc));

			hwndDlgCurrent = hwndDlg;		// Set the global so that we have a window open
			
			// this will make sure that we've got thr aacplus logo shown even when using a localised version
			SendDlgItemMessage(hwndDlg,IDC_LOGO,STM_SETIMAGE,IMAGE_BITMAP,
							   (LPARAM)LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(IDB_GN_LOGO),IMAGE_BITMAP,0,0,LR_SHARED));

			BoldStatusText(GetDlgItem(hwndDlg, IDC_STATIC_PROGRESS_STATE) );
			
			SetRadioControlsState(hwndDlg);				// Set the playlist length state

			if(multipleArtists)
				CheckDlgButton(hwndDlg,IDC_CHECK_MULTIPLE_ARTISTS,TRUE);
			if(multipleAlbums)
				CheckDlgButton(hwndDlg,IDC_CHECK_MULTIPLE_ALBUMS,TRUE);
			if(useSeed)
				CheckDlgButton(hwndDlg,IDC_CHECK_USE_SEED,TRUE);

			// Set up the colums for the playlist listing
			#define ListView_InsertColumnW(hwnd, iCol, pcol) \
				(int)SNDMSG((hwnd), LVM_INSERTCOLUMNW, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMNW *)(pcol))
			//SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);

			// Add the columns to the listbox
			HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST_RESULTS2);
			ListView_SetExtendedListViewStyle(hwndlist, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
			LVCOLUMNW lvc = {0, };
			lvc.mask = LVCF_TEXT|LVCF_WIDTH;
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_TITLE);		// Initialize the columns of the listview
			lvc.cx = 160;
			ListView_InsertColumnW(hwndlist, 0, &lvc);
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_LENGTH);
			lvc.cx = 80;
			ListView_InsertColumnW(hwndlist, 1, &lvc);
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_SIZE);
			lvc.cx = 80;
			ListView_InsertColumnW(hwndlist, 2, &lvc);
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_SEED);
			lvc.cx = 80;
			ListView_InsertColumnW(hwndlist, 3, &lvc);
			lvc.pszText = 0;
			lvc.cx = 0;
			ListView_InsertColumnW(hwndlist, DIALOG_HIDDEN_COLUMN_ID, &lvc);

			// Autosize the columns taking the header into consideration
			ListView_SetColumnWidth(hwndlist,0,LVSCW_AUTOSIZE_USEHEADER);
			ListView_SetColumnWidth(hwndlist,1,LVSCW_AUTOSIZE_USEHEADER);
			ListView_SetColumnWidth(hwndlist,2,LVSCW_AUTOSIZE_USEHEADER);
			ListView_SetColumnWidth(hwndlist,3,LVSCW_AUTOSIZE_USEHEADER);

			HWND hwndListHeader = ListView_GetHeader(hwndlist);
			SetPropW(hwndListHeader, L"header_block_proc",(HANDLE)SetWindowLongPtrW(hwndListHeader, GWLP_WNDPROC, (LONG_PTR)header_block_proc));

			// Background color for highlighting seed tracks.
			//hbrBkcolor = CreateSolidBrush ( RGB(255,0,0) );
			
			BoldStatusText(GetDlgItem(hwndDlg, IDC_STATIC_STATS) );

			// Populate the query textbox
			SetDlgItemTextW(hwndDlg, IDC_EDIT_ML_QUERY, mlQuery);						// Set the text for the query

			// Disable the regenerate button because we will be scanning the library and generating on initialization
			//EnableWindow (GetDlgItem(hwndDlgCurrent, IDC_BUTTON_REGENERATE), FALSE );		// This is for initialization
			SetButtonsEnabledState(false);

			// Set up the window with the options hidden
			ToggleOptions(true);

			// Show the window since we are modeless
			POINT pt = {(LONG)GetPrivateProfileInt(L"ml_plg", L"generate_x",-1, mediaLibrary.GetWinampIniW()),
						(LONG)GetPrivateProfileInt(L"ml_plg", L"generate_y",-1, mediaLibrary.GetWinampIniW())};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
			else
				ShowWindow(hwndDlg, SW_SHOW);

			Regenerate(hwndDlg);

			if (WASABI_API_APP)		// Add direct mousewheel support for the main tracklist view of seed and generated tracks
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(GetDlgItem(hwndDlg, IDC_LIST_RESULTS2), TRUE);
			
			return TRUE;
		}
		break;

		// Trying to change the background color of seed tracks here
		/*case WM_CTLCOLORSTATIC:
			{
				HDC hdc = (HDC) wParam;
				HWND hwndStatic = (HWND) lParam;

				if ( hwndStatic == GetDlgItem ( hwndDlg, IDC_LIST_RESULTS2 ))
				{
					SetBkMode ( hdc, TRANSPARENT );
					return (LRESULT) hbrBkcolor;
				}
			}
			break;*/

		case WM_TIMER:
			FillStatus(hwndDlg);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
				{
					RECT rect = {0};
					GetWindowRect(hwndDlg, &rect);
					char buf[16] = {0};
					StringCchPrintfA(buf, 16, "%d", rect.left);
					WritePrivateProfileStringA("ml_plg", "generate_x", buf, mediaLibrary.GetWinampIni());
					StringCchPrintfA(buf, 16, "%d", rect.top);
					WritePrivateProfileStringA("ml_plg", "generate_y", buf, mediaLibrary.GetWinampIni());

					EndDialog(hwndDlg, 0);
					hwndDlgCurrent = 0;			// Set to null so new instance can be opened

					WriteSettingsToIni(hwndDlg);
					
					// We need to free up our seed tracks because we no longer require them
					seedPlaylist.Clear();			// Clear the global seed list
				}
				break;
				case IDC_BUTTON_CANCEL:
					SendMessage(hwndDlg, WM_COMMAND, IDCANCEL, 0);
					break;
				case IDC_BUTTON_REGENERATE:
					Regenerate(hwndDlg);
					break;
				case IDC_RADIO_PLAYLIST_ITEMS:
					SetDlgItemText(hwndDlg, IDC_LENGTH_TYPE, WASABI_API_LNGSTRINGW(IDS_ITEMS));
					plLengthType = PL_ITEMS;		// Set to # of items
					SetPlLengthTypeComboToItems(hwndDlg, plItems);
					break;
				case IDC_RADIO_PLAYLIST_LENGTH:
					SetDlgItemText(hwndDlg, IDC_LENGTH_TYPE, WASABI_API_LNGSTRINGW(IDS_MINUTES));
					plLengthType = PL_MINUTES;		// Set to minutes
					SetPlLengthTypeComboToMinutes(hwndDlg, plMinutes);
					break;
				case IDC_RADIO_PLAYLIST_SIZE:
					SetDlgItemText(hwndDlg, IDC_LENGTH_TYPE, WASABI_API_LNGSTRINGW(IDS_MEGABYTES));
					plLengthType = PL_MEGABYTES;	// Set to megabytes
					SetPlLengthTypeComboToMegabytes(hwndDlg, plMegabytes);
					break;
				case IDC_COMBO_LENGTH:
				{
					UpdateComboLength(hwndDlg);
				}
				break;
				case IDC_BUTTON_OPTIONS:
					ToggleOptions(false);
					break;
				case IDC_BUTTON_PLAY_NOW:
					playPlaylist(currentPlaylist, false, 0, /*seed,*/ useSeed);		// Play the current playlist taking the seed track into consideration
					SendMessage(hwndDlg, WM_COMMAND, IDCANCEL, 0);				// Close up the dialog because we are done
					break;
				case IDC_BUTTON_ENQUEUE_NOW:
					playPlaylist(currentPlaylist, true, 0, /*seed,*/ useSeed);		// Enqueue the current playlist taking the seed track into consideration
					SendMessage(hwndDlg, WM_COMMAND, IDCANCEL, 0);				// Close up the dialog because we are done
					break;
				case IDC_BUTTON_SAVEAS:
					{
						// ToDo spawn a dialog to save the current playlist as a ML playlist
						int save_result = WASABI_API_DIALOGBOXPARAM(IDD_ADD_PLAYLIST, hwndDlg, AddPlaylistDialogProc, (LPARAM)&seedPlaylist/*seed*/);
						if (save_result == IDOK)		// If the user accepted that playlist dialog then go ahead and close up everything
						{
							SendMessage(hwndDlg, WM_COMMAND, IDCANCEL, 0);				// Close up the dialog because we are done
						}
					}
					break;
				case IDC_BUTTON_ML_QUERY:
					{
						char temp[1024] = {0};
						GetDlgItemTextA(hwndDlg, IDC_EDIT_ML_QUERY, temp, sizeof(temp) - 1);		// Retreive the current custom ML query
						
						ml_editview meq = {hwndDlg, (temp[0] == 0) ? DEFAULT_ML_QUERY : temp, "ML Query", -1};							// Create the editview
						meq.name = WASABI_API_LNGSTRING(IDS_ML_QUERY);								// Set a custom title
						if(!(int)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (LPARAM)&meq, ML_IPC_EDITVIEW))
							return 0;	// Spawn the edit view
						SetDlgItemTextA(hwndDlg, IDC_EDIT_ML_QUERY, meq.query);						// Set the text back to the edited query
					}
					break;
				case IDC_BUTTON_RESTORE_QUERY_DEFAULT:
					SetDlgItemTextW(hwndDlg, IDC_EDIT_ML_QUERY, _T(DEFAULT_ML_QUERY));				// Set the text back to the edited query
					break;
				case IDC_CHECK_USE_SEED:
					useSeed = IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_SEED);
					UpdateStats();																// Update the track stats, because the seed status can change them
					RedrawWindow(GetDlgItem(hwndDlg,IDC_LIST_RESULTS2), 0, 0, RDW_INVALIDATE);	// Refresh the colors in the list view
					break;
				case IDC_CHECK_MULTIPLE_ARTISTS:												// Set the multiple tracks per artist option when checked
					multipleArtists = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MULTIPLE_ARTISTS);
					break;
				case IDC_CHECK_MULTIPLE_ALBUMS:													// Set the multiple tracks per album option when checked
					multipleAlbums = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MULTIPLE_ALBUMS);
					break;
				case IDC_CHECK_ML_QUERY:
					SetMLQueryCheckState(hwndDlg, IsDlgButtonChecked(hwndDlg, IDC_CHECK_ML_QUERY));
					break;
				case IDC_EDIT_ML_QUERY:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						GetDlgItemTextW(hwndDlg, IDC_EDIT_ML_QUERY, mlQuery, MAX_ML_QUERY_SIZE);						// Set the text back to the edited query
						break;
					}
			}
			break;

		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == HDN_BEGINTRACKW || ((LPNMHDR)lParam)->code == HDN_BEGINTRACKA ||
			   ((LPNMHDR)lParam)->code == HDN_ITEMCHANGINGW || ((LPNMHDR)lParam)->code == HDN_ITEMCHANGINGA)
			{
				LPNMHEADER pNMHeader = (LPNMHEADER)lParam;
				if(pNMHeader->iItem == DIALOG_HIDDEN_COLUMN_ID)
				{
					SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)TRUE);
					return TRUE;
				}
			}
			else if(((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)					// Notify for List View custom redraw (seed track colors)
			{
#if defined(_WIN64)
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG)CustomDrawListViewColors(lParam));
#else
				SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG)CustomDrawListViewColors(lParam));
#endif
				return TRUE;
			}

			{
				const int controls[] = 
				{
					IDC_LIST_RESULTS2,
				};
				if (WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, msg, wParam, lParam, controls, ARRAYSIZE(controls)) != FALSE)
				{
					return TRUE;
				}
			}
			break;
		case WM_DESTROY:
			{
			if (WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(GetDlgItem(hwndDlg, IDC_LIST_RESULTS2), FALSE);
			}
			break;
	}

	return 0;
}

int AddResultListItem(Playlist *playlist, int index, int position, bool seed)
{
	const unsigned int MAX_INFO = 256;
	wchar_t filename[MAX_INFO] = {0};
	wchar_t info[MAX_INFO] = {0};
	wchar_t *seedText = 0;
	LVITEMW lvi={LVIF_TEXT, position, 0};

	playlist->GetItem(index,filename,MAX_INFO);

	// Add the title column
	playlist->GetItemTitle(index, info, MAX_INFO);
	lvi.pszText=info;
	lvi.cchTextMax=sizeof(info) / sizeof(*info);
	SendMessage(GetDlgItem(hwndDlgCurrent,IDC_LIST_RESULTS2),LVM_INSERTITEMW,0,(LPARAM)&lvi);

	// Add the length column
	int length = playlist->GetItemLengthMilliseconds(index);
	if (length <= 0)
		StringCchCopyW(info, MAX_INFO, WASABI_API_LNGSTRINGW(IDS_UNKNOWN));
	else
		FormatToMinutesAndSeconds(length / 1000, info, MAX_INFO);		// / 1000 because we have it in milliseconds and not seconds
	
	lvi.pszText=info;
	lvi.cchTextMax=sizeof(info) / sizeof(*info);
	lvi.iSubItem = 1;
	SendMessage(GetDlgItem(hwndDlgCurrent,IDC_LIST_RESULTS2),LVM_SETITEMW,0,(LPARAM)&lvi);
	
	// Add the size column
	int size = playlist->GetItemSizeBytes(index);
	if (size <= 0)
		StringCchCopyW(info, MAX_INFO, WASABI_API_LNGSTRINGW(IDS_UNKNOWN));
	else
		StrFormatByteSizeW(size, info, MAX_INFO);
	lvi.pszText=info;
	lvi.cchTextMax=sizeof(info) / sizeof(*info);
	lvi.iSubItem = 2;
	SendMessage(GetDlgItem(hwndDlgCurrent,IDC_LIST_RESULTS2),LVM_SETITEMW,0,(LPARAM)&lvi);	

	// Add the seed track column
	if (seed == true)
		seedText = WASABI_API_LNGSTRINGW(IDS_YES);
	else
		seedText = WASABI_API_LNGSTRINGW(IDS_NO);
	lvi.pszText=seedText;
	lvi.cchTextMax=sizeof(seedText) / sizeof(*seedText);
	lvi.iSubItem = 3;
	SendMessage(GetDlgItem(hwndDlgCurrent,IDC_LIST_RESULTS2),LVM_SETITEMW,0,(LPARAM)&lvi);	
	
	return 0;
}

void CantPopulateResults(void)
{
	wchar_t message[256] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_EXCUSE_ME, message, 256);
	MessageBoxW(hwndDlgCurrent, message, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_PLAYLIST_GENERATOR), MB_OK | MB_ICONINFORMATION);
}

void PopulateResults(Playlist *playlist)
{
	// Add all of the seed tracks to the listview
	int listLength = (playlist) ? (int)playlist->GetNumItems() : 0;
	int seedLength = (int)seedPlaylist.GetNumItems();
	for (int i = 0; i < seedLength; i++)
	{
		AddResultListItem(&seedPlaylist, i, i, true);
	}

	// Add all of the generated tracks to the listview
	for (int i = 0; i < listLength; i++)
	{
		AddResultListItem(playlist, i, seedLength + i, false);
	}

	// After we are done populating the data then we can size the columns accordingly
	HWND hwndlist = GetDlgItem(hwndDlgCurrent,IDC_LIST_RESULTS2);
	ListView_SetColumnWidth(hwndlist,0,(listLength ? LVSCW_AUTOSIZE : LVSCW_AUTOSIZE_USEHEADER));
	ListView_SetColumnWidth(hwndlist,1,LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(hwndlist,2,LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(hwndlist,3,LVSCW_AUTOSIZE_USEHEADER);

	// Refresh the playlist stats
	UpdateStats();

	// Change the progress status to read done 'generated'
	SetDlgItemText(hwndDlgCurrent,IDC_STATIC_PROGRESS_STATE, WASABI_API_LNGSTRINGW(IDS_DONE));

	SetMarqueeProgress(false);			// Turn the marquee off because we are actually generating the tracks

	//EnableWindow (GetDlgItem(hwndDlgCurrent, IDC_BUTTON_REGENERATE), TRUE );
	SetButtonsEnabledState(true);		// Renable the buttons

}
