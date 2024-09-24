#include "./uiBurnPlaylist.h"
#include "./resource.h"
#include <shlwapi.h>
#include <commctrl.h>

#include <strsafe.h>
#include "./uiCheckMedium.h"
#include "./uiUnitReady.h"

#define WM_PLBURNERCOMMAND			((WM_USER) + 26)
#define PLB_LICENSE					0
#define PLB_DECODE					1
#define PLB_BURN					2

#define TIMER_UPDATECLOCK_ID		1979
#define TIMER_UPDATECLOCK_INTERVAL	1000
#define TIMER_PROGRESS_ID			1978
#define TIMER_PROGRESS_INTERVAL		500

#define COLUMN_COUNT			0
#define COLUMN_TITLE			1
#define COLUMN_DURATION		2
#define COLUMN_STATUS		3
#define COLUMN_FILENAME		4

const int	COLUMNWIDTH[] = {20, 220, 60, 102, 280};
const int	COLUMNALLIGN[] = {LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_CENTER, LVCFMT_LEFT, LVCFMT_LEFT};
const int	COLUMNNAME[] = {IDS_COLUMN_INDEX, IDS_COLUMN_TITLE, IDS_COLUMN_DURATION, IDS_COLUMN_STATUS, IDS_COLUMN_FILE};

const COLORREF strip[] = { RGB(198, 238, 255), RGB(184, 233, 255), RGB(167, 227, 255), RGB(151, 221, 255), RGB(133, 215, 255), RGB(115, 208, 255), RGB(99, 202, 255), 
						   RGB(82, 196, 255),  RGB(64, 190, 255),  RGB(46, 184, 255),  RGB(29, 177, 255),  RGB(12, 171, 255), RGB(2, 165, 255),  RGB(0, 158, 255) };


static UINT uMsgBroadcastNotify = 0;
BurnPlaylistUI::BurnPlaylistUI(void)
{
	hwnd			= NULL;
	tmpfilename = NULL;
	hTmpFile		= NULL;
	currentPercent = -1;
	stripBmp = NULL;
	cancelOp = FALSE;
	workDone = NULL;
	readyClose = TRUE;
	ZeroMemory(&estimated, sizeof(aproxtime));
	
}

BurnPlaylistUI::~BurnPlaylistUI(void)
{
	if(hwnd) DestroyWindow(hwnd);
	if (hTmpFile)
	{
		CloseHandle(hTmpFile);
		hTmpFile=0;
	}
	if (tmpfilename)
	{
		DeleteFileW(tmpfilename);
		free(tmpfilename);
		tmpfilename=0;
	}
}

DWORD BurnPlaylistUI::Burn(obj_primo *primoSDK, DWORD drive, DWORD maxspeed, DWORD burnFlags, BurnerPlaylist *playlist,
							const wchar_t* tempPath, HWND ownerWnd)
{
	if (!primoSDK) return BURNPLAYLISTUI_PRIMOSDKNOTSET;
	DWORD retCode;
	extendedView = FALSE;
	this->primoSDK = primoSDK;
	this->drive = drive;
	this->playlist = playlist;
	this->maxspeed = maxspeed;
	this->burnFlags = burnFlags;
	this->ownerWnd = ownerWnd;

	stage = PLSTAGE_READY;
	
	wchar_t fname[64] = {0};
	DWORD uid = GetTickCount() & 0x00FFFFFF;
	StringCchPrintfW(fname, 64, L"wa%06I32X.tmp", uid);
	tmpfilename = (wchar_t*)malloc((lstrlenW(tempPath) + 48)*sizeof(wchar_t));
	if (tempPath) PathCombineW(tmpfilename, tempPath, fname);
	else 
	{
		wchar_t path[2048] = {0};
		GetTempPathW(2048, path);
		PathCombineW(tmpfilename, path, fname);
	}

	LPCDLGTEMPLATE templ = NULL;
	HRSRC hres = FindResourceExW(hResource,  MAKEINTRESOURCEW(5), MAKEINTRESOURCEW(IDD_DLG_BURNER), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)); 
	if (hres) templ = (LPCDLGTEMPLATE)LoadResource(hResource, hres);
	retCode = (DWORD)DialogBoxIndirectParamW(dllInstance, templ, NULL, (DLGPROC)WndProc, (LPARAM)this);
	return retCode;
}


void BurnPlaylistUI::OnInitDialog(HWND hwndDlg)
{
	hwnd = hwndDlg;
	errCode = BURNPLAYLISTUI_SUCCESS;

	SetPropW(hwnd, L"WABURNER", hwnd);
	SetPropW(hwnd, L"DRIVE", (HANDLE)(INT_PTR)drive);

	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ADDTODB), (PRIMOSDK_TEST != (burnFlags&PRIMOSDK_TEST)));
	CheckDlgButton(hwndDlg, IDC_CHK_ADDTODB, BST_CHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_EJECT, BST_CHECKED);

	if (ownerWnd) PostMessage(ownerWnd, WM_BURNNOTIFY, BURN_READY, (LPARAM)hwnd);
	
	if (!uMsgBroadcastNotify) uMsgBroadcastNotify = RegisterWindowMessageA("WABURNER_BROADCAST_MSG");
	if (uMsgBroadcastNotify) SendNotifyMessage(HWND_BROADCAST, uMsgBroadcastNotify, (WPARAM)(0xFF & drive), (LPARAM)TRUE);
	
	SetExtendedView(extendedView);
	SetColumns();
	FillList();
	
	wchar_t format[512] = {0}, buffer[512] = {0};
	SetReadyClose(TRUE);

	LoadStringW(hResource, IDS_BURNINGCDDA, format, 512);
	StringCchPrintfW(buffer, 512, format, drive);


	if (PRIMOSDK_TEST == (burnFlags&PRIMOSDK_TEST))
	{
		HANDLE hImage = NULL;
		hImage = LoadBitmapW(hResource,  MAKEINTRESOURCEW(IDB_TESTMODE));
		if(hImage == NULL){
			hImage = LoadBitmapW(dllInstance,  MAKEINTRESOURCEW(IDB_TESTMODE));
		}
		SendDlgItemMessage(hwnd, IDC_PIC_TESTMODE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImage);
		ShowWindow(GetDlgItem(hwnd, IDC_PIC_TESTMODE), SW_SHOW);
	}
	
	SetDlgItemTextW(hwnd, IDC_LBL_CAPTION, buffer);
	SendDlgItemMessage(hwnd, IDC_PRG_TOTAL, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SetProgress(0);

	startedTime = GetTickCount();
	/// estimation
	realSpeed = 0;
	DWORD retCode, reqspeed;
	WAMEDIUMINFO detectedMedium;
	retCode = GetMediumInfo(primoSDK, &drive, &detectedMedium);  // required before GetDiscSpeed
	if (PRIMOSDK_OK == retCode) 
	{
			
		switch(maxspeed)
		{
			case PRIMOSDK_MAX:
				reqspeed = 0xFFFFFFFF;
				break;
			case PRIMOSDK_BEST:
				reqspeed = 0xFFFFFFF0;
				break;
			case PRIMOSDK_MIN:
				reqspeed = 0x00000000;
				break;
			case PRIMOSDK_MEDIUM:
				reqspeed = 0x0000FFFF;
				break;
			default: reqspeed = maxspeed*100;
		}
		// TODO: benski> should "retCode =" go before this?
		primoSDK->GetDiscSpeed(&drive, reqspeed, &realSpeed);
		if (PRIMOSDK_OK != retCode) realSpeed = 0;
	}
	if (!realSpeed) realSpeed = 4*100;
				
	estimated.license = (DWORD)playlist->GetCount();  
	estimated.convert = playlist->GetTotalLengthMS() /(60*1000);  
	estimated.transition = 0;
	estimated.chkdisc = 1;
	estimated.init	  = 1;
	estimated.leadin  = 20;
	estimated.burn	  = playlist->GetTotalLengthMS()/(realSpeed*10);
	estimated.leadout = 20;
	estimated.finish  = 5;
	estimatedTime = 0;
	    	
	UpdateTime(TRUE);

	ShowWindow(hwnd, SW_SHOWNORMAL);
	SetForegroundWindow(hwnd);
	BringWindowToTop(hwnd);
	UpdateWindow(hwnd);

	SetTimer(hwnd, TIMER_UPDATECLOCK_ID, TIMER_UPDATECLOCK_INTERVAL, NULL);

	hTmpFile = CreateFileW(tmpfilename, GENERIC_READ|GENERIC_WRITE/*FILE_APPEND_DATA | FILE_READ_DATA*/, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN, NULL);
	if (INVALID_HANDLE_VALUE == hTmpFile)
	{
		wchar_t title[64] = {0};
		hTmpFile = NULL;
		LoadStringW(hResource, IDS_TEMPORARY_FILE, title, 64);
		MessageBoxW(NULL, tmpfilename, title, MB_OK);
		ReportError(IDS_TMPCREATEFAILED, FALSE);
		return;
	}
	PostMessage(hwnd, WM_PLBURNERCOMMAND, PLB_LICENSE, 0);
	return;
}

void BurnPlaylistUI::SetColumns(void)
{ 
	HWND ctrlWnd = GetDlgItem(hwnd, IDC_LST_DETAILS);

	LVCOLUMNW clmn = {0};
	clmn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
	int count = sizeof(COLUMNWIDTH) /sizeof(int);
	wchar_t buffer[512] = {0};
	for (int i = 0; i < count; i++)
	{
		LoadStringW(hResource, COLUMNNAME[i], buffer, 512);
		clmn.cx = COLUMNWIDTH[i];
		clmn.fmt = COLUMNALLIGN[i];
		clmn.pszText = buffer;
		clmn.iSubItem = i;
		SendMessageW(ctrlWnd, LVM_INSERTCOLUMNW, i, (LPARAM)&clmn);
	}
	
	// extra styles
	SendMessageW(ctrlWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 
							/*LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |*/  LVS_EX_LABELTIP, 
							/*LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |*/  LVS_EX_LABELTIP);

}

void BurnPlaylistUI::FillList(void)
{
	HWND ctrlWnd = GetDlgItem(hwnd, IDC_LST_DETAILS);
	LVITEMW item = {0};
	wchar_t buffer[128] = {0};
	for(size_t i = 0;  i < playlist->GetCount(); i++)
	{
		BurnerItem *bi =  playlist->at(i);
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = (int)i;
		item.iSubItem = COLUMN_COUNT;
		_i64tow_s(i + 1, buffer, 128, 10);
		item.pszText = buffer;
		item.lParam = (LPARAM)bi;
		SendMessage(ctrlWnd, LVM_INSERTITEMW, 0, (LPARAM)&item);
		
		item.mask = LVIF_TEXT;
		
		item.iSubItem = COLUMN_TITLE;
		item.pszText = const_cast<wchar_t*>(bi->GetTitle());
		SendMessage(ctrlWnd, LVM_SETITEMW, 0, (LPARAM)&item);
		item.iSubItem = COLUMN_DURATION;
		item.pszText = GetTimeString(buffer, 128, bi->GetLength()/1000);
		SendMessage(ctrlWnd, LVM_SETITEMW, 0, (LPARAM)&item);
		item.iSubItem = COLUMN_STATUS;
		item.pszText = L"";
		SendMessage(ctrlWnd, LVM_SETITEMW, 0, (LPARAM)&item);
		item.iSubItem = COLUMN_FILENAME;
		item.pszText = const_cast<wchar_t*>(bi->GetFullName());
		SendMessage(ctrlWnd, LVM_SETITEMW, 0, (LPARAM)&item);
	}

}
void BurnPlaylistUI::SetProgress(int position)
{
	if (currentPercent == position) return;
	wchar_t buffer[8] = {0};
	StringCchPrintfW(buffer, 8, L"%d%%", position);
	SetDlgItemTextW(hwnd, IDC_LBL_PERCENT, buffer);
	SendDlgItemMessage(hwnd, IDC_PRG_TOTAL, PBM_SETPOS, position, 0);
	currentPercent = position;
	
}
void BurnPlaylistUI::UpdateTime(BOOL recalcEstimates)
{
	wchar_t buffer[128] = {0};
	if (recalcEstimates)
	{
		estimatedTime = 0;
		DWORD *pe = (DWORD*)&estimated;
		for (int i = 0; i < sizeof(estimated) /sizeof(DWORD); i++) estimatedTime += pe[i];
		SetWindowTextW(GetDlgItem(hwnd, IDC_LBL_ESTIMATED_VAL), GetTimeString(buffer, 128, estimatedTime));
	}
	unsigned int elapsedTime = (GetTickCount() - startedTime)/1000;
	SetWindowTextW(GetDlgItem(hwnd, IDC_LBL_ELAPSED_VAL), GetTimeString(buffer, 128, elapsedTime));
	if (estimatedTime <= elapsedTime) 
	{
		estimatedTime = elapsedTime;
		SetWindowTextW(GetDlgItem(hwnd, IDC_LBL_ESTIMATED_VAL), buffer);
	}
}


void BurnPlaylistUI::ReportError(unsigned int stringCode, BOOL allowContinue)
{
	wchar_t buffer[512] = {0};
	LoadStringW(hResource, stringCode, buffer, 512);
	ReportError(buffer, allowContinue);
}
void BurnPlaylistUI::ReportError(const wchar_t *errorString, BOOL allowContinue)
{
	HWND ctrlWnd;
	if (!allowContinue)
	{
		// stop timer and set progress to the end
		estimatedTime = 0;
		KillTimer(hwnd, TIMER_UPDATECLOCK_ID);
		SetProgress(100);
		// update time 
		UpdateTime(FALSE);
	}
	else
	{
		ShowWindow(GetDlgItem(hwnd, IDC_BTN_CONTINUE), SW_SHOW);
	}
	
	// set caption to the Burning canceled.
	wchar_t buffer[128] = {0}, format[128] = {0};
	LoadStringW(hResource, (allowContinue) ? IDS_BURNINGSTOPPED : IDS_BURNINGCANCELED, format, 128);
	StringCchPrintfW(buffer, 128, format, drive);
	SetDlgItemTextW(hwnd, IDC_LBL_CAPTION, buffer);
	LoadStringW(hResource, IDS_REASON, buffer, 128);
	
	// set operation info to Cancelation cause: error message
	SetDlgItemTextW(hwnd, IDC_LBL_CURRENTOPERATION, buffer);
	SetDlgItemTextW(hwnd, IDC_LBL_CURRENTOPERATION_VAL, errorString);
		
	// set cancel/close button to 'close' mode and enable it
	LoadStringW(hResource, IDS_CLOSE, buffer, 128);
	ctrlWnd = GetDlgItem(hwnd, IDCANCEL); 
	SetWindowTextW(ctrlWnd, buffer);
    EnableWindow(ctrlWnd, TRUE);
	// set extended view (show all info)
	SetExtendedView(TRUE);
	// make some noise
	MessageBeep(MB_ICONHAND);
	// set status to ready for closing
	SetReadyClose(TRUE);
	// if somebody waiting - we done!
    if(workDone) SetEvent(workDone);
	if (!allowContinue) if (uMsgBroadcastNotify) SendNotifyMessage(HWND_BROADCAST, uMsgBroadcastNotify, (WPARAM)(0xFF & drive), (LPARAM)FALSE);
}

void BurnPlaylistUI::UpdateItemStatus(int index)
{
	LVITEMW lvi;
	lvi.mask		= LVIF_TEXT;
	lvi.iItem	= index;
	lvi.iSubItem= COLUMN_STATUS;
	lvi.pszText = NULL;
	SendDlgItemMessage(hwnd, IDC_LST_DETAILS, LVM_SETITEMW, 0, (LPARAM)&lvi);
}
void BurnPlaylistUI::SetItemStatusText(int index, unsigned int stringCode, BOOL redraw)
{
	wchar_t buffer[128] = {0};
	LoadStringW(hResource, stringCode, buffer, 128);
	LVITEMW lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iItem =  index;
	lvi.iSubItem = COLUMN_STATUS;
	lvi.pszText = buffer;

	HWND lstWnd = GetDlgItem(hwnd, IDC_LST_DETAILS);
	SendMessage(lstWnd, LVM_SETITEMW, 0, (LPARAM)&lvi);
	if (redraw) ListView_RedrawItems(lstWnd, index, index);
	
}
void BurnPlaylistUI::SetCurrentOperation(unsigned int stringCode)
{
	wchar_t buffer[128] = {0};
	LoadStringW(hResource, stringCode, buffer, 128);
	SetDlgItemTextW(hwnd, IDC_LBL_CURRENTOPERATION_VAL, buffer);
}
int BurnPlaylistUI::MessageBox(unsigned int messageCode, unsigned int captionCode, unsigned int uType)
{
	wchar_t message[512] = {0}, caption[64] = {0};
	LoadStringW(hResource, messageCode, message, 512);
	LoadStringW(hResource, captionCode, caption, 64);
	return MessageBoxW(hwnd, message, caption, uType);
}
void BurnPlaylistUI::OnCancel(void)
{
	ShowWindow(GetDlgItem(hwnd, IDC_BTN_CONTINUE), SW_HIDE);
	if (!readyClose && workDone)
	{

		HWND btnWnd = GetDlgItem(hwnd, IDCANCEL); 
		EnableWindow(btnWnd, FALSE);
		wchar_t message[512] = {0}, caption[64] = {0};
		LoadStringW(hResource, IDS_MB_CANCELBURNING, message, 512);
		LoadStringW(hResource, IDS_CONFIRMATION, caption, 64);
		if (IDYES != MessageBoxW(hwnd, message, caption, MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_SETFOREGROUND | MB_TOPMOST)) 
		{
			EnableWindow(btnWnd, TRUE);
			return;
		}
			
		wchar_t buffer[64] = {0};
		LoadStringW(hResource, IDS_BURNINGABORTEDBYUSER, buffer, 64);
		SetDlgItemTextW(hwnd, IDC_LBL_CAPTION, buffer);
		SetCurrentOperation(IDS_CANCELING);
		cancelOp = TRUE; 
		MSG msg;
		SetTimer(hwnd, TIMER_PROGRESS_ID, TIMER_PROGRESS_INTERVAL, NULL);
		while (WAIT_TIMEOUT == WaitForSingleObject(workDone, 20))
		{
			while (PeekMessageW(&msg, NULL, 0,0, PM_REMOVE))
			{
				if(IsDialogMessage(hwnd, &msg)) continue;
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		KillTimer(hwnd, TIMER_PROGRESS_ID);
		cancelOp = FALSE; 
		CloseHandle(workDone);
		workDone = NULL;
		KillTimer(hwnd, TIMER_UPDATECLOCK_ID);
		SetProgress(100);
		estimatedTime = 0;
		UpdateTime(FALSE);
		if (hTmpFile)
		{
			CloseHandle(hTmpFile);
			hTmpFile = NULL;
		}
		if (tmpfilename) DeleteFileW(tmpfilename);
		SetReadyClose(TRUE);
		EnableWindow(btnWnd, TRUE);
		MessageBeep(MB_OK);
		DWORD errorCode;
		DWORD status = playlist->GetStatus(&errorCode);
		if (ownerWnd) PostMessage(ownerWnd, WM_BURNNOTIFY, BURN_FINISHED, errorCode);
		if (uMsgBroadcastNotify)
		{
			SendNotifyMessage(HWND_BROADCAST, uMsgBroadcastNotify, (WPARAM)(0xFF & drive), (LPARAM)FALSE);
		}
		if(BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CHK_AUTOCLOSE) || BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CHK_HIDEWINDOW))
		{
			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDCANCEL));
		}
	}
	else
	{
		if (workDone) CloseHandle(workDone);
		workDone = NULL;
		if (tmpfilename) DeleteFileW(tmpfilename);
		PostMessage(hwnd, WM_DESTROY, 0,0);
	}
}
void BurnPlaylistUI::SetReadyClose(BOOL ready)
{
	readyClose = ready;
	wchar_t buffer[32] = {0};
	LoadStringW(hResource, (readyClose) ? IDS_CLOSE : IDS_CANCEL, buffer, 32);
	SetDlgItemTextW(hwnd, IDCANCEL, buffer);
}
void BurnPlaylistUI::OnDestroy(void)
{
	if (stripBmp) 
	{
		DeleteObject(stripBmp);
		stripBmp = NULL;
	}
	if (hTmpFile)
	{
		CloseHandle(hTmpFile);
		hTmpFile = NULL;
	}

	if (tmpfilename)
	{
		DeleteFileW(tmpfilename);
		free(tmpfilename);
		tmpfilename = NULL;
	}
	EndDialog(hwnd, errCode);
	if (ownerWnd) PostMessage(ownerWnd, WM_BURNNOTIFY, BURN_DESTROYED, 0);
}

void BurnPlaylistUI::SetExtendedView(BOOL extView)
{
	extendedView = extView;
	if (!hwnd) return;
	
	ShowWindow(GetDlgItem(hwnd, IDC_LST_DETAILS), extView);
	ShowWindow(GetDlgItem(hwnd, IDC_GRP_OPTIONS), extView);
	ShowWindow(GetDlgItem(hwnd, IDC_CHK_AUTOCLOSE), extView);
	ShowWindow(GetDlgItem(hwnd, IDC_CHK_EJECT), extView);
	ShowWindow(GetDlgItem(hwnd, IDC_CHK_ADDTODB), extView);
	ShowWindow(GetDlgItem(hwnd, IDC_CHK_HIDEWINDOW), extView);

	RECT rw;
	HWND ctrlWnd;
	GetWindowRect(hwnd, &rw);
	int height = (extView) ? 413 : 147;

	SetWindowPos(hwnd, HWND_TOP, 0, 0, rw.right - rw.left, height, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	
	ctrlWnd = GetDlgItem(hwnd, IDC_BTN_EXTENDEDVIEW);
	wchar_t buffer[32] = {0};
	LoadStringW(hResource, (extView) ? IDS_SHOWLESS : IDS_SHOWMORE, buffer, 32);
	SetWindowTextW(ctrlWnd, buffer);
}

HBITMAP BurnPlaylistUI::CreateStripBmp(HDC compDC)
{
	HDC hdc = CreateCompatibleDC(compDC);
	if (!hdc) return NULL;

	BITMAPINFO info;
    // create DIB Section
    info.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth         = 1; 
    info.bmiHeader.biHeight        = -14; 
    info.bmiHeader.biPlanes        = 1; 
    info.bmiHeader.biBitCount      = 24; 
    info.bmiHeader.biCompression   = BI_RGB; 
    info.bmiHeader.biSizeImage     = 0; 
    info.bmiHeader.biXPelsPerMeter = 0; 
    info.bmiHeader.biYPelsPerMeter = 0; 
    info.bmiHeader.biClrUsed       = 0; 
    info.bmiHeader.biClrImportant  = 0; 
	void *data;
	HBITMAP bmp = CreateDIBSection(hdc, (const BITMAPINFO*) &info, DIB_RGB_COLORS, &data, NULL, 0);
	if (bmp)
	{
		CopyMemory(data, strip, sizeof(strip));
	}
    DeleteDC(hdc);
	return bmp;
}

DWORD BurnPlaylistUI::DrawList(NMLVCUSTOMDRAW* listDraw)
{
	switch(listDraw->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT: 
			return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;

		case CDDS_ITEMPREPAINT: 
			if ((int)listDraw->nmcd.dwItemSpec%2)
			{
				listDraw->clrTextBk = RGB(238, 238, 238);//GetSysColor(COLOR_WINDOW);
			}
			{
				BurnerItem *bi = (BurnerItem*)listDraw->nmcd.lItemlParam;
				DWORD biCode;
				int status = bi->GetStatus(&biCode);
				switch(status)
				{
					case BURNERITEM_LICENSING:
					case BURNERITEM_DECODING:
					case BURNERITEM_BURNING:
						listDraw->clrText  = RGB(30, 120, 40);
						break;
					case BURNERITEM_LICENSED:
					case BURNERITEM_DECODED:
					case BURNERITEM_BURNED:
						switch(biCode)
						{
							case BURNERITEM_SUCCESS:
								listDraw->clrText = RGB(10, 10, 60);
								break;
							case BURNERITEM_ABORTED:
								listDraw->clrText = RGB(100, 10, 40);
								break;
							default:
								listDraw->clrText = RGB(220, 0, 0);
								break;
						}
						break;
					case BURNERITEM_ABORTED:
						listDraw->clrText = RGB(96, 24, 24);
						break;
					case BURNERITEM_SKIPPED:
						listDraw->clrText = RGB(204, 204, 220);
						break;
				}
			}
			return CDRF_DODEFAULT | CDRF_NOTIFYSUBITEMDRAW;
		
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
			{
				BurnerItem *bi = (BurnerItem*)listDraw->nmcd.lItemlParam;
				DWORD biCode;
				int status = bi->GetStatus(&biCode);
				switch(status)
				{
					case BURNERITEM_DECODING:
					case BURNERITEM_BURNING:
						if (listDraw->iSubItem == COLUMN_STATUS)
						{
							if (!stripBmp) stripBmp = CreateStripBmp(listDraw->nmcd.hdc);
							HDC hdc = CreateCompatibleDC(listDraw->nmcd.hdc);
							HGDIOBJ tmpBmp = SelectObject(hdc, stripBmp);
							
							RECT rc;
							ListView_GetSubItemRect(listDraw->nmcd.hdr.hwndFrom, (int)listDraw->nmcd.dwItemSpec, COLUMN_STATUS, LVIR_BOUNDS, &rc);
							
							HBRUSH  hb = ((int)listDraw->nmcd.dwItemSpec%2) ? CreateSolidBrush(RGB(240, 242, 245)) :GetSysColorBrush(COLOR_WINDOW);
							RECT rb;
							SetRect(&rb, rc.left, rc.top, rc.left + 4, rc.bottom);
							FillRect(listDraw->nmcd.hdc, &rb, hb);
							SetRect(&rb, rc.left + 4, rc.top, rc.right - 4, rc.top + 1);
							FillRect(listDraw->nmcd.hdc, &rb, hb);
							SetRect(&rb, rc.left + 4, rc.bottom - 1, rc.right - 4, rc.bottom);
							FillRect(listDraw->nmcd.hdc, &rb, hb);

							int len = int (((float)(rc.right - rc.left - 8)) * (((float)biCode)/100.0f));
							
							SetRect(&rb, rc.left + 4 + len, rc.top, rc.right, rc.bottom);
							FillRect(listDraw->nmcd.hdc, &rb, hb);
							
							for (int i = rc.left + 4; i < rc.left + 4 + len; i++)
							{
								BitBlt(listDraw->nmcd.hdc, i, rc.top + 1, 1, rc.bottom - rc.top -2, hdc, 0, 0, SRCCOPY);
							}
							SelectObject(hdc, tmpBmp);
							DeleteDC(hdc);
							wchar_t buffer[32] = {0}, text[64] = {0};
							LoadStringW(hResource, (BURNERITEM_DECODING == status) ? IDS_PREPARING : IDS_BURNING, buffer, 32);

							StringCchPrintfW(text, 64, L"%s (%d%%)" , buffer, biCode);
							SetTextColor(listDraw->nmcd.hdc, RGB(30, 120, 0));
							SetBkMode(listDraw->nmcd.hdc,TRANSPARENT);
							InflateRect(&rc, -4, 0);
							DrawTextW(listDraw->nmcd.hdc, text, -1, &rc, DT_CENTER | DT_VCENTER |DT_END_ELLIPSIS |DT_SINGLELINE);
							if((int)listDraw->nmcd.dwItemSpec%2) DeleteObject(hb);
							return CDRF_SKIPDEFAULT;
							
						}		
						break;
				}
			}

			break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}

void BurnPlaylistUI::OnLicense(void)
{
	SetReadyClose(FALSE);
	if (workDone) ResetEvent(workDone);
	if (ownerWnd) PostMessage(ownerWnd, WM_BURNNOTIFY, BURN_WORKING, 0);
	controlTime =  GetTickCount();
	if (!workDone) workDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	playlist->CheckLicense(OnLicensingPlaylist, this);
}

void BurnPlaylistUI::OnDecode(void)
{	
	prevRefresh = 0;
	SetReadyClose(FALSE);
	if (workDone) ResetEvent(workDone);
	if (ownerWnd) PostMessage(ownerWnd, WM_BURNNOTIFY, BURN_WORKING, 0);
	controlTime =  GetTickCount();
	ShowWindow(GetDlgItem(hwnd, IDC_BTN_CONTINUE), SW_HIDE);
	DWORD retCode = playlist->Decode(hTmpFile, OnDecodePlaylist, this, FALSE);
	if (BURNERPLAYLIST_DECODESTARTING == retCode)
	{
		if(!workDone) workDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
}

void BurnPlaylistUI::OnBurn(void)
{
	// check disc
	SetReadyClose(FALSE);
	if (workDone) ResetEvent(workDone);
	if (ownerWnd) PostMessage(ownerWnd, WM_BURNNOTIFY, BURN_WORKING, 0);
	estimated.transition = (GetTickCount() - controlTime)/1000;
	UpdateTime(TRUE);
	ShowWindow(GetDlgItem(hwnd, IDC_BTN_CONTINUE), SW_HIDE);
	DWORD totalSectors = playlist->GetTotalSectors();
	SetReadyClose(FALSE);
	controlTime = GetTickCount();
	WAMEDIUMINFO mi;
	FillMemory(&mi, sizeof(WAMEDIUMINFO), 0xFF);
	mi.recordable = TRUE;
	mi.isCD = TRUE;
	mi.freeSectors = totalSectors; // plus lead-in
	CheckMediumUI cm;
	SetProgress(0);
	SetCurrentOperation(IDS_CHECKINGDISC);
	DWORD time =totalSectors/75;
	int min = time / 60 + ((time % 60) ? 1 : 0);
	wchar_t string[512] = {0};
	LoadStringW(hResource, IDS_BURNREQDISC, string, 512);
	wchar_t buffer[512] = {0};
	StringCchPrintfW(buffer, 512, string, min);
	DWORD retCode = cm.Check(primoSDK, &drive, &mi, buffer,  FALSE, TRUE, hwnd);
	estimated.chkdisc = (GetTickCount() - controlTime)/1000;
	UpdateTime(TRUE);
	switch(retCode)
	{
		case CHECKMEDIUMUI_MATCH:
		{
			// start burning
			DWORD reqspeed;
			unsigned int strcode;
			switch(maxspeed)
			{
				case PRIMOSDK_MAX:
					reqspeed = 0xFFFFFFFF;
					strcode = IDS_SPEEDMAX;
					break;
				case PRIMOSDK_BEST:
					reqspeed = 0xFFFFFFF0;
					strcode = IDS_SPEEDBEST;
					break;
				case PRIMOSDK_MIN:
					reqspeed = 0x00000000;
					strcode = IDS_SPEEDMIN;
					break;
				case PRIMOSDK_MEDIUM:
					reqspeed = 0xFFFFF000;
					strcode = IDS_SPEEDMED;
					break;
				default: 
					reqspeed = maxspeed*100;
					strcode = IDS_SPEED;
					break;
			}
			primoSDK->GetDiscSpeed(&drive, reqspeed, &realSpeed);
			wchar_t at[8] = {0}, spd[16] = {0}, spddesc[32] = {0}; 
			StringCchPrintfW(spd, 16, L"%d.%02dx", realSpeed/100, realSpeed%100);

			LoadStringW(hResource, strcode, spddesc, 32);
			LoadStringW(hResource, IDS_BURNINGCDDA, buffer, 512);
			StringCchPrintfW(string, 512, buffer, drive);
			LoadStringW(hResource, IDS_AT, at, 8);
			switch(maxspeed)
			{
				case PRIMOSDK_BEST:
					StringCchPrintfW(buffer, 512, L"%s %s %s", string, at, spddesc);
					break;			
				case PRIMOSDK_MAX:
				case PRIMOSDK_MEDIUM:
				case PRIMOSDK_MIN:
					StringCchPrintfW(buffer,512, L"%s %s %s (%s)", string, at, spddesc, spd);
					break;
				default: 
					StringCchPrintfW(buffer,512, L"%s %s %s %s", string, at, spd, spddesc);
					break;
			}			
			SetDlgItemTextW(hwnd, IDC_LBL_CAPTION, buffer);
			
			retCode = playlist->Burn(primoSDK, drive, maxspeed, burnFlags, hTmpFile,OnBurnPlaylist, this, FALSE);
			if (BURNERPLAYLIST_BURNSTARTING == retCode)
			{
                if (!workDone) workDone = CreateEvent(NULL, FALSE, FALSE, NULL);
				SetReadyClose(FALSE);
                return;
			}
			break;
		}
		case CHECKMEDIUMUI_CANCELED: ReportError(IDS_BURNINGABORTEDBYUSER, FALSE); break;
		case CHECKMEDIUMUI_DISCNOTSET: ReportError(IDS_CHKDSCWRONGPARAMETER, FALSE); break;
		case CHECKMEDIUMUI_DRIVENOTSET: ReportError(IDS_CHKDSCWRONGPARAMETER, FALSE); break;
		case CHECKMEDIUMUI_PRIMOSDKNOTSET: ReportError(IDS_CHKDSCWRONGPARAMETER, FALSE); break;
		case CHECKMEDIUMUI_UNABLETOCREATEDIALOG: ReportError(IDS_CHKDSCDIALOGFAILED, FALSE); break;
		case CHECKMEDIUMUI_MESSAGEPUMPERROR: ReportError(IDS_CHKDSCMSGPUMPFAILED, FALSE); break;
		default: 
			{
				ReportError(IDS_PRIMOINITFAILED, FALSE); 
				
				DWORD statCode, cmd, sense, asc, ascq;
				statCode = primoSDK->UnitStatus(&drive, &cmd, &sense, &asc, &ascq);
				wchar_t caption[64] = {0}, message[512] = {0}, myerror[128] = {0}, libprfx[32] = {0}, liberror[128] = {0}, drvprfx[32] = {0}, drverror[128] = {0};
				LoadStringW(hResource, IDS_BURNERROR, caption, 64);
				LoadStringW(hResource, IDS_PRIMOINITFAILED, myerror, 128);
				LoadStringW(hResource, IDS_LIBERRORPREFIX, libprfx, 32);
				LoadStringW(hResource, IDS_DRVERRORPREFIX, drvprfx, 32);
				GetPrimoCodeText(liberror, 128, statCode);
				GetUnitStatusText(drverror, 128, sense, asc, ascq);
				StringCchPrintfW(message, 512, L"%s%s%s%s%s", myerror, libprfx, liberror, drvprfx, drverror);
				MessageBoxW(hwnd, message, caption, MB_OK | MB_ICONEXCLAMATION);
			}
			break;
	}
	SetReadyClose(TRUE);
	if (workDone) SetEvent(workDone);
}
DWORD BurnPlaylistUI::OnLicensingPlaylist(void *sender, void *userparam, DWORD notifyCode, DWORD errorCode, ULONG_PTR param)
{
	BurnPlaylistUI *dlg = (BurnPlaylistUI *) userparam;
	switch (notifyCode)
	{
		case BURNERPLAYLIST_LICENSINGFINISHED:
			dlg->stage = PLSTAGE_LICENSED;
			dlg->estimated.license = (GetTickCount() - dlg->controlTime)/1000;
			dlg->UpdateTime(TRUE);
            KillTimer(dlg->hwnd, TIMER_PROGRESS_ID);
			dlg->SetProgress(100);
			switch(errorCode)
			{
			
				case BURNERPLAYLIST_FILENOTLICENSED:  // param contains number of successfully processed files
					dlg->ReportError(IDS_LICENSEFAILED, (param) ? TRUE : FALSE);
					if(param)
					{
						dlg->estimated.convert = ((int)param)/(60*1000);  
						dlg->estimated.burn = ((int)param)/(dlg->realSpeed*10);
						dlg->controlTime = GetTickCount();
						dlg->MessageBox(IDS_LICENSEFAILEDMSG, IDS_BURNERROR, MB_OK | MB_ICONERROR);
						dlg->estimated.transition += (GetTickCount() - dlg->controlTime)/1000;
						dlg->UpdateTime(TRUE);
					}
					break;
				case BURNERPLAYLIST_SUCCESS: dlg->SetCurrentOperation(IDS_LICENSESUCCESS); break;
				case BURNERPLAYLIST_NOFILES: dlg->ReportError(IDS_NOFILES, FALSE); break;
				case BURNERPLAYLIST_DECODESERVICEFAILED: dlg->ReportError(IDS_DECODESERVICEFAILED, FALSE); break;
				case BURNERPLAYLIST_WRONGFILECOUNT: dlg->ReportError(IDS_LICENSEWRONGFILECOUNT, FALSE); break;
				default: dlg->ReportError(IDS_LICENSEFAILED, FALSE); break;
			}
			PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, 0);		
			if (BURNERPLAYLIST_SUCCESS == errorCode) PostMessage(dlg->hwnd, WM_PLBURNERCOMMAND, PLB_DECODE, 0);
			break;
		case BURNERPLAYLIST_LICENSINGSTARTING:
			switch(errorCode)
			{
				case BURNERPLAYLIST_ITEMADDED:		dlg->SetItemStatusText((int)param, IDS_LICENSEITEMPROGRESS, TRUE); break;
				case BURNERPLAYLIST_ADDITEMSKIPPED: dlg->SetItemStatusText((int)param, IDS_SKIPPED, TRUE); break;
				case BURNERPLAYLIST_SUCCESS: 		
					dlg->SetCurrentOperation(IDS_LICENSEPROGRESS); 
					SetTimer(dlg->hwnd, TIMER_PROGRESS_ID, 100, NULL);
					break;
			}
			break;
		case BURNERPLAYLIST_LICENSINGPROGRESS:
			unsigned int strcode(IDS_UNKNOWN);
			switch(errorCode)
			{
				case BURN_OK:				dlg->SetItemStatusText((int)param, IDS_LICENSEITEMSUCCESS, TRUE); break;
				case BURN_FILE_NOT_FOUND:	dlg->SetItemStatusText((int)param, IDS_FILENOTFOUND, TRUE); break;
				case BURN_DRM_NO_LICENSE:	dlg->SetItemStatusText((int)param, IDS_DRMNOLICENSE, TRUE); break;
				case BURN_DRM_NOT_ALLOWED:	dlg->SetItemStatusText((int)param, IDS_DRMNOBURNING, TRUE); break;
				case BURN_DRM_BURN_COUNT_EXCEEDED: dlg->SetItemStatusText((int)param, IDS_DRMBURNCOUNTEXCEEDED, TRUE); break;
				case BURN_NO_DECODER:		dlg->SetItemStatusText((int)param, IDS_NODECODER, TRUE); break;
				default: dlg->SetItemStatusText((int)param, IDS_LICENSEITEMFAILED, TRUE); break;
			}
			break;
	}
	if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, notifyCode);
	return (!dlg->cancelOp) ? BURNERPLAYLIST_CONTINUE : BURNERPLAYLIST_STOP;
}
DWORD BurnPlaylistUI::OnDecodePlaylist(void *sender, void *userparam, DWORD notifyCode, DWORD errorCode, ULONG_PTR param)
{
	BurnPlaylistUI *dlg = (BurnPlaylistUI *) userparam;
	switch (notifyCode)
	{
		case BURNERPLAYLIST_DECODESTARTING:
			switch(errorCode)
			{
				case BURNERPLAYLIST_SUCCESS:
					dlg->SetCurrentOperation(IDS_PREPARINGDATA); 
					dlg->processed = 0; 
					dlg->count = 0;
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, notifyCode);
					break;
				case BURNERPLAYLIST_ITEMADDED:  dlg->SetItemStatusText((int)param, IDS_SCHEDULED, TRUE); dlg->count++; break;
				case BURNERPLAYLIST_ADDITEMSKIPPED:  dlg->SetItemStatusText((int)param, IDS_SKIPPED, TRUE); break;
			}
			break;
		case BURNERPLAYLIST_DECODEFINISHED:
			dlg->stage = PLSTAGE_DECODED;
			dlg->SetProgress(100);
			dlg->estimated.convert = (GetTickCount() - dlg->controlTime)/1000;
			dlg->UpdateTime(TRUE);
			dlg->controlTime = GetTickCount();
			dlg->estimated.burn = dlg->playlist->GetStateLengthMS(BURNERITEM_DECODED, BURNERITEM_SUCCESS)/(dlg->realSpeed*10);
			switch(errorCode)
			{
				case BURNERPLAYLIST_SUCCESS:
					dlg->SetCurrentOperation(IDS_PREPARESUCCESS);
					PostMessage(dlg->hwnd,  WM_PLBURNERCOMMAND, PLB_BURN, 0);
					break;
				case BURNERPLAYLIST_NOFILES: dlg->ReportError(IDS_NOFILES, FALSE); break;
				case BURNERPLAYLIST_ABORTED: dlg->ReportError(IDS_BURNINGABORTEDBYUSER, FALSE); break;
				case BURNERPLAYLIST_DECODESERVICEFAILED: dlg->ReportError(IDS_DECODESERVICEFAILED, FALSE); break;
				case BURNERPLAYLIST_THREADCREATEFAILED: dlg->ReportError(IDS_DECODESTARTFAILED, FALSE); break;
					
					
				default:	
					dlg->ReportError(IDS_PREPAREFAILED, TRUE);
					dlg->MessageBox(IDS_PREPAREFAILEDMSG, IDS_BURNERROR, MB_OK | MB_ICONERROR);
					dlg->estimated.transition += (GetTickCount() - dlg->controlTime)/1000;
					dlg->UpdateTime(TRUE);
					break;

			}
			if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, notifyCode);
            break;
		case BURNERPLAYLIST_DECODEPROGRESS:
			{			
				BPLDECODEINFO *info = (BPLDECODEINFO*)param;		
				switch(errorCode)
				{
					case BURNERPLAYLIST_DECODENEXTITEM:
						if(!dlg->cancelOp) 
						{
							wchar_t text[64] = {0}, of[16] = {0}, buffer[128] = {0};
							BPLDECODEINFO *info = (BPLDECODEINFO*)param;		
							LoadStringW(hResource, IDS_PREPARINGDATA, text, 64);
							LoadStringW(hResource, IDS_OF, of, 16);
							StringCchPrintfW(buffer, 128, L"%s ( %d %s %d )", text, dlg->processed + 1, of, dlg->count);
							SetDlgItemTextW(dlg->hwnd, IDC_LBL_CURRENTOPERATION_VAL, buffer);
							if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMSTATECHANGED, info->iIndex);
						}
						break;
					case BURNERPLAYLIST_DECODEITEM:
						switch(info->iNotifyCode)
						{
							case BURNERITEM_DECODESTARTING:
								ListView_RedrawItems(GetDlgItem(dlg->hwnd, IDC_LST_DETAILS),  info->iIndex, info->iIndex);
								if(!dlg->cancelOp) dlg->SetProgress((int)info->percentCompleted);
								break;
							case BURNERITEM_DECODEPROGRESS:
								if (GetTickCount() - dlg->prevRefresh < 100) break;
								dlg->UpdateItemStatus(info->iIndex);
								if(!dlg->cancelOp)
								{
									dlg->SetProgress((int)info->percentCompleted);
									if ((int)info->percentCompleted)
									{
										dlg->estimated.convert = (GetTickCount() - dlg->controlTime)/((int)(info->percentCompleted*10));
										dlg->UpdateTime(TRUE);
									}
								}
								dlg->prevRefresh = GetTickCount();
								if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMDECODEPROGRESS, info->iIndex);
								break;
							case BURNERITEM_DECODEFINISHED:
								switch(info->iErrorCode)
								{
									case BURNERITEM_SUCCESS: dlg->SetItemStatusText(info->iIndex, IDS_PREPAREITEMSUCCESS, TRUE); break;
									case BURNERITEM_ABORTED: dlg->SetItemStatusText(info->iIndex, IDS_CANCELED, TRUE); break;
									case BURNERITEM_BADFILENAME: dlg->SetItemStatusText(info->iIndex, IDS_BADFILENAME, TRUE); break;
									case BURNERITEM_UNABLEOPENFILE: dlg->SetItemStatusText(info->iIndex, IDS_NODECODER, TRUE); break;
									case BURNERITEM_WRITEERROR: dlg->SetItemStatusText(info->iIndex, IDS_CACHEWRITEFAILED, TRUE); break;
									default:
										dlg->SetItemStatusText(info->iIndex, IDS_PREPAREITEMFAILED, TRUE);
										dlg->estimated.burn -= info->iInstance->GetLength()/(dlg->realSpeed*10);
										dlg->UpdateTime(TRUE);
										break;
								}
								dlg->processed++;
								if(!dlg->cancelOp) dlg->SetProgress((int)info->percentCompleted);
								if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMSTATECHANGED, info->iIndex);
								break;
						}
						break;
				}
			}
			break;
		case BURNERPLAYLIST_DECODECANCELING: dlg->SetCurrentOperation(IDS_CANCELING); break;
	}
	return (!dlg->cancelOp) ? BURNERPLAYLIST_CONTINUE : BURNERPLAYLIST_STOP;
}

DWORD BurnPlaylistUI::OnBurnPlaylist(void *sender, void *userparam, DWORD notifyCode, DWORD errorCode, ULONG_PTR param)
{
	BurnPlaylistUI *dlg = (BurnPlaylistUI *) userparam;
	BurnerPlaylist *playlist = (BurnerPlaylist*)sender;
	switch (notifyCode)
	{
		case BURNERPLAYLIST_BURNSTARTING:
			switch(errorCode)
			{
				case BURNERPLAYLIST_SUCCESS:			dlg->SetCurrentOperation(IDS_MASTERINGDISC); break;
				case BURNERPLAYLIST_ITEMADDED:		
					dlg->SetItemStatusText((int)param, IDS_SCHEDULED, TRUE); 
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMSTATECHANGED, param);
					break;
				case BURNERPLAYLIST_ADDITEMSKIPPED: 
					dlg->SetItemStatusText((int)param, IDS_SKIPPED, TRUE); 
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMSTATECHANGED, param);
					break;
				case BURNERPLAYLIST_ADDITEMFAILED:	
					dlg->SetItemStatusText((int)param, IDS_BURNITEMADDFAILED, TRUE);
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMSTATECHANGED, param);
					break;
				case BURNERPLAYLIST_BEGINBURN:		dlg->SetCurrentOperation(IDS_INITIALIZINGBURNER); break;
			}
			break;
		case BURNERPLAYLIST_BURNPROGRESS:
			switch(errorCode)
			{
				case BURNERPLAYLIST_WRITELEADIN:
					
					dlg->estimated.init =  (GetTickCount() - dlg->controlTime)/1000;
					dlg->controlTime = GetTickCount();
					dlg->UpdateTime(TRUE);
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, notifyCode);
					if (dlg->cancelOp) break;
					dlg->SetCurrentOperation(IDS_WRITELEADIN);
					break;					
				case BURNERPLAYLIST_WRITELEADOUT:
					dlg->controlTime = GetTickCount();
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, notifyCode);
					if (dlg->cancelOp) break;
					dlg->SetCurrentOperation(IDS_WRITELEADOUT);
					break;
				case BURNERPLAYLIST_DISCOPEN: 
					if (!dlg->cancelOp) dlg->SetCurrentOperation(IDS_DISCOPEN); 
					break;
				case BURNERPLAYLIST_DISCCLOSE: 
					if (!dlg->cancelOp) dlg->SetCurrentOperation(IDS_DISCCLOSE); 
					break;
				case BURNERPLAYLIST_WRITEITEMBEGIN:
					if (((BPLRUNSTATUS*)param)->iIndex == 0)
					{
						dlg->estimated.leadin =  (GetTickCount() - dlg->controlTime)/1000;
						dlg->controlTime = GetTickCount();
					}
					ListView_RedrawItems(GetDlgItem(dlg->hwnd, IDC_LST_DETAILS), ((BPLRUNSTATUS*)param)->iIndex, ((BPLRUNSTATUS*)param)->iIndex);
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMSTATECHANGED, ((BPLRUNSTATUS*)param)->iIndex);
					break;
				case BURNERPLAYLIST_WRITEITEMEND:
					if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMSTATECHANGED, ((BPLRUNSTATUS*)param)->iIndex);
					if (dlg->cancelOp) break;
					dlg->SetItemStatusText(((BPLRUNSTATUS*)param)->iIndex, IDS_BURNITEMSUCCESS, TRUE);
					break;	
				case BURNERPLAYLIST_WRITEDATA:
					{
						BPLRUNSTATUS *status = (BPLRUNSTATUS*)param;
						int percent = (int)(status->sCurrent * 100 / status->sTotal);
						if (!percent) break;
						if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_ITEMBURNPROGRESS, ((BPLRUNSTATUS*)param)->iIndex);
						if (dlg->cancelOp) break;
						if (dlg->currentPercent != percent)
						{
							dlg->SetProgress(percent);
							wchar_t format[128] = {0}, buffer[164] = {0};
							LoadStringW(hResource, IDS_BURNPROGRESS, format, 128);
							StringCchPrintfW(buffer, 164, format , percent);
							SetDlgItemTextW(dlg->hwnd, IDC_LBL_CURRENTOPERATION_VAL, buffer);
						}
						dlg->UpdateItemStatus(((BPLRUNSTATUS*)param)->iIndex);
						dlg->estimated.burn = (GetTickCount() - dlg->controlTime)/((int)(percent*10));
						dlg->UpdateTime(TRUE);
						
					}
					break;
			}
			break;
		case BURNERPLAYLIST_BURNFINISHING:
			dlg->estimated.leadout =  (GetTickCount() - dlg->controlTime)/1000;
			dlg->UpdateTime(TRUE);
			dlg->controlTime = GetTickCount();
			if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, notifyCode);
			playlist->SetEjectWhenDone((BST_CHECKED == IsDlgButtonChecked(dlg->hwnd, IDC_CHK_EJECT)) ? PRIMOSDK_OPENTRAYEJECT : 0); 
			if (dlg->cancelOp) break;
			dlg->SetCurrentOperation(IDS_RELEASINGBURNER);
			break;
		case BURNERPLAYLIST_BURNFINISHED:
			dlg->stage = PLSTAGE_BURNED;
			dlg->estimated.finish =  (GetTickCount() - dlg->controlTime)/1000;
			dlg->SetProgress(100);
			
			if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_FINISHED, errorCode);
			
			switch(errorCode)
			{
				case BURNERPLAYLIST_ABORTED:				dlg->ReportError(IDS_BURNINGABORTEDBYUSER, FALSE); return BURNERPLAYLIST_STOP;
				case BURNERPLAYLIST_NOFILES:				dlg->ReportError(IDS_NOFILES, FALSE); return BURNERPLAYLIST_STOP;
				case BURNERPLAYLIST_ADDITEMFAILED:		dlg->ReportError(IDS_MASTERINGFAILED, FALSE); return BURNERPLAYLIST_STOP;
				case BURNERPLAYLIST_THREADCREATEFAILED: dlg->ReportError(IDS_BURNSTARTFAILED, FALSE); return BURNERPLAYLIST_STOP;
			}

			if (BURNERPLAYLIST_SUCCESS != errorCode)  // rest of the errors
			{
				unsigned int strcode(IDS_BURNFAILED);
				switch(errorCode)
				{
					case BURNERPLAYLIST_BEGINBURNFAILED:	  strcode = IDS_BEGINBURNFAILED; break; 
					case BURNERPLAYLIST_WRITEAUDIOFAILED: strcode = IDS_WRITEAUDIOFAILED; break;
					case BURNERPLAYLIST_ENDBURNFAILED:	  strcode = IDS_ENDBURNFAILED; break;
				}
				dlg->ReportError(strcode, FALSE);

				wchar_t caption[64] = {0}, message[512] = {0}, myerror[128] = {0}, libprfx[32] = {0}, liberror[128] = {0}, drvprfx[32] = {0}, drverror[128] = {0};
				LoadStringW(hResource, IDS_BURNERROR, caption, 64);
				LoadStringW(hResource, strcode, myerror, 128);
				LoadStringW(hResource, IDS_LIBERRORPREFIX, libprfx, 32);
				LoadStringW(hResource, IDS_DRVERRORPREFIX, drvprfx, 32);
				GetPrimoCodeText(liberror, 128, (0x000000FF & (((DWORD)param) >> 24)));
				GetUnitStatusText(drverror, 128, (0x000000FF & (((DWORD)param) >> 16)), (0x000000FF & (((DWORD)param) >> 8)), (0x000000FF & (DWORD)param));
				StringCchPrintfW(message, 512, L"%s%s%s%s%s", myerror, libprfx, liberror, drvprfx, drverror);
				MessageBoxW(dlg->hwnd, message, caption, MB_OK | MB_ICONEXCLAMATION);
				return BURNERPLAYLIST_STOP;
			}

			// this is a happy end :)
			if(BST_CHECKED == IsDlgButtonChecked(dlg->hwnd, IDC_CHK_ADDTODB)) dlg->playlist->AddCompilationToCDDB();
			
			KillTimer(dlg->hwnd, TIMER_UPDATECLOCK_ID);
			dlg->estimatedTime = 0;
			dlg->UpdateTime(FALSE);
			// set caption to the Burning completed.
			{
				wchar_t buffer[128] = {0}, format[128] = {0};
				LoadStringW(hResource, IDS_BURNINGCOMPLETED, format, 128);
				StringCchPrintfW(buffer, 128, format, dlg->drive);
				SetDlgItemTextW(dlg->hwnd, IDC_LBL_CAPTION, buffer);
			}
						
			dlg->SetCurrentOperation(IDS_BURNSUCCESS); 
	
			dlg->SetReadyClose(TRUE);
			if (dlg->workDone) SetEvent(dlg->workDone);

			if (uMsgBroadcastNotify) 
				SendNotifyMessage(HWND_BROADCAST, uMsgBroadcastNotify, (WPARAM)(0xFF & dlg->drive), (LPARAM)FALSE);

			if(BST_CHECKED == IsDlgButtonChecked(dlg->hwnd, IDC_CHK_AUTOCLOSE) ||
			    BST_CHECKED == IsDlgButtonChecked(dlg->hwnd, IDC_CHK_HIDEWINDOW))
			{
				PostMessage(dlg->hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), (LPARAM)GetDlgItem(dlg->hwnd, IDCANCEL));
			}
			
			MessageBeep(MB_OK);
			break;
		case BURNERPLAYLIST_BURNCANCELING: 
			dlg->SetItemStatusText(((BPLRUNSTATUS*)param)->iIndex, 
				(BURNERITEM_CANCELING == dlg->playlist->at(((BPLRUNSTATUS*)param)->iIndex)->GetStatus(NULL)) ? IDS_CANCELING : IDS_CANCELED,
				TRUE); 
			if (dlg->ownerWnd) PostMessage(dlg->ownerWnd, WM_BURNNOTIFY, BURN_STATECHANGED, notifyCode);
			break;
	}
	return (!dlg->cancelOp) ? BURNERPLAYLIST_CONTINUE : BURNERPLAYLIST_STOP;
}

LRESULT BurnPlaylistUI::WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BurnPlaylistUI *pl = NULL;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			pl = (BurnPlaylistUI*)lParam;
			pl->OnInitDialog(hwndDlg);
            break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
					pl->OnCancel();
					break;
				case IDC_BTN_EXTENDEDVIEW:
					if (BN_CLICKED == HIWORD(wParam)) pl->SetExtendedView(!pl->extendedView);
					break;
				case IDC_BTN_CONTINUE:
					if (BN_CLICKED == HIWORD(wParam)) 
					{
						
						wchar_t buffer[128] = {0}, format[128] = {0};
						LoadStringW(hResource, IDS_BURNINGCDDA, format, 128);
						StringCchPrintfW(buffer, 128, format, pl->drive);
						SetDlgItemTextW(hwndDlg, IDC_LBL_CAPTION, buffer);
						LoadStringW(hResource, IDS_CURRENTOPERATION, buffer, 128);
						SetDlgItemTextW(hwndDlg, IDC_LBL_CURRENTOPERATION, buffer);
						ShowWindow((HWND)lParam, SW_HIDE);
						PostMessage(hwndDlg,  WM_PLBURNERCOMMAND, PLB_LICENSE, 0);
					}
					break;
				case IDC_CHK_AUTOCLOSE:
					if (BN_CLICKED == HIWORD(wParam))
					{
						if (pl->ownerWnd)
							PostMessage(pl->ownerWnd, WM_BURNNOTIFY, 
								BURN_CONFIGCHANGED,
								MAKELPARAM(BURNCFG_AUTOCLOSE, (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_AUTOCLOSE))));
					}
					break;
				case IDC_CHK_EJECT:
					if (BN_CLICKED == HIWORD(wParam))
					{
						if (pl->ownerWnd)
							PostMessage(pl->ownerWnd, WM_BURNNOTIFY, 
								BURN_CONFIGCHANGED,
								MAKELPARAM(BURNCFG_AUTOEJECT, (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_EJECT))));
					}
					break;
				case IDC_CHK_ADDTODB:
					if (BN_CLICKED == HIWORD(wParam))
					{
						if (pl->ownerWnd)
							PostMessage(pl->ownerWnd, WM_BURNNOTIFY, 
								BURN_CONFIGCHANGED,
								MAKELPARAM(BURNCFG_ADDTODB, (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_ADDTODB))));
					}
					break;
				case IDC_CHK_HIDEWINDOW:
					if (BN_CLICKED == HIWORD(wParam))
					{
						if (pl->ownerWnd)
							PostMessage(pl->ownerWnd, WM_BURNNOTIFY, 
								BURN_CONFIGCHANGED,
								MAKELPARAM(BURNCFG_HIDEVIEW, (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEWINDOW))));
						ShowWindow(hwndDlg, (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEWINDOW)) ? SW_HIDE : SW_SHOW);
					}
					break;
			}
			break;
		case WM_DESTROY:
			pl->OnDestroy();
			break;
		case WM_TIMER:
			switch(wParam)
			{
				case TIMER_UPDATECLOCK_ID:
					pl->UpdateTime(FALSE);
					break;
				case TIMER_PROGRESS_ID:
					if (pl->currentPercent == 100) pl->currentPercent = 0;
					pl->SetProgress(pl->currentPercent + 1);
					break;
			}
			break;
		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->idFrom == IDC_LST_DETAILS)
			{
				if(((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
				{
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG)(LONG_PTR)pl->DrawList((NMLVCUSTOMDRAW*)lParam));
					return TRUE;
				}
				if(((LPNMHDR)lParam)->code == LVN_ITEMCHANGED)
				{
					int index = (int)SendMessage(((LPNMHDR)lParam)->hwndFrom, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
					if(index == -1) return FALSE;
					ListView_SetItemState(((LPNMHDR)lParam)->hwndFrom, index, 0, LVIS_SELECTED | LVIS_FOCUSED);
					return TRUE;
				}
			}
			break;
		case WM_PLBURNERCOMMAND:
			switch(wParam)
			{
				case PLB_LICENSE:
					pl->OnLicense();
					break;
				case PLB_DECODE:
					pl->OnDecode();
					break;
				case PLB_BURN:
					pl->OnBurn();
					break;
			}
			break;
		case WM_BURNUPDATEOWNER:
			{
				LONG tmpWnd = (LONG)(ULONG_PTR)pl->ownerWnd;
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, tmpWnd);
				pl->ownerWnd = (HWND)lParam;
				return tmpWnd;
			}
		case WM_BURNGETSTATUS:
			{
				DWORD retCode = 0;
				switch(wParam)
				{
					case BURNSTATUS_DRIVE:
						retCode = pl->drive;
						break;
					case BURNSTATUS_ELAPSED:
						retCode =  (GetTickCount() - pl->startedTime)/1000;
						break;
					case BURNSTATUS_ESTIMATED:
						retCode = pl->estimatedTime;
						break;
					case BURNSTATUS_STATE:
						retCode = pl->playlist->GetStatus(NULL);
						break; 
					case BURNSTATUS_ERROR:
						pl->playlist->GetStatus(&retCode);
						break; 
					case BURNSTATUS_PROGRESS:
						retCode = (GetTickCount() - pl->startedTime)/(pl->estimatedTime*10);
						break; 
				}
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, retCode);
				return retCode;
			}
		case WM_BURNGETITEMSTATUS:
			{
				DWORD retCode = 0;
				if (((DWORD)lParam) >= pl->playlist->GetCount()) break;
				{
					BurnerItem *item = pl->playlist->at(lParam); 
					if (!item) break;
					switch(wParam)
					{
						case BURNSTATUS_STATE:
							retCode = item->GetStatus(NULL);
							break; 
						case BURNSTATUS_PROGRESS:
						case BURNSTATUS_ERROR:
							item->GetStatus(&retCode);
							break; 
						
					}
				}
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, retCode);
				return retCode;
			}
		case WM_BURNCONFIGCHANGED:
			switch(wParam)
			{
				case BURNCFG_AUTOCLOSE:
					CheckDlgButton(hwndDlg, IDC_CHK_AUTOCLOSE, (lParam) ? BST_CHECKED : BST_UNCHECKED);
					break;
				case BURNCFG_AUTOEJECT:
					CheckDlgButton(hwndDlg, IDC_CHK_EJECT, (lParam) ? BST_CHECKED : BST_UNCHECKED);
					break;
				case BURNCFG_ADDTODB:
					if (PRIMOSDK_TEST == (pl->burnFlags&PRIMOSDK_TEST)) lParam = FALSE;
					CheckDlgButton(hwndDlg, IDC_CHK_ADDTODB, (lParam) ? BST_CHECKED : BST_UNCHECKED);
					break;
				case BURNCFG_HIDEVIEW:
					CheckDlgButton(hwndDlg, IDC_CHK_HIDEWINDOW, (lParam) ? BST_CHECKED : BST_UNCHECKED);
					break;
			}
			break;
	}
	if (uMsgBroadcastNotify && uMsgBroadcastNotify == uMsg && HIWORD(wParam) && pl && pl->playlist)
	{
		CHAR cLetter;
		cLetter = (CHAR)LOWORD(wParam);
		if (!cLetter || (cLetter == (CHAR)(0xFF & pl->drive)))
		{
			if (!cLetter) cLetter = (CHAR)(0xFF & pl->drive);
			if (BURNERPLAYLIST_BURNFINISHED != pl->playlist->GetStatus(NULL)) 
				SendNotifyMessage((HWND)lParam, uMsgBroadcastNotify, (WPARAM)cLetter, TRUE); 
		}
	}
	return 0;
}

