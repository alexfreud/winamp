#include "./uiUnitReady.h"
#include "./resource.h"
#include <strsafe.h>

#define TIMER_REFRESH_ID			1979
#define TIMER_REFRESH_INTERVAL		300

UnitReadyUI::UnitReadyUI(void)
{
	hwnd = NULL;
	drive = NULL;
	primoSDK = NULL;
	errPrimo = 0;
	errReady = 0;
	updateDlg = NULL;
}
UnitReadyUI::~UnitReadyUI(void)
{
	if (updateDlg) delete(updateDlg);
	updateDlg = NULL;
	if (hwnd) DestroyWindow(hwnd);
}

DWORD UnitReadyUI::Check(obj_primo *primoSDK, DWORD *drive, BOOL showRetry, HWND ownerWnd)
{

	if (!drive) return UNITREADYUI_DRIVENOTSET;
	if (!primoSDK) return UNITREADYUI_PRIMOSDKNOTSET;
	this->drive = drive;
	this->primoSDK = primoSDK;
	hwnd = NULL;
	errPrimo = PRIMOSDK_OK;
	errReady = UNITREADYUI_NOTREADY;
	statSense = MAXDWORD;
	statAsc = MAXDWORD;
	statAscQ = MAXDWORD;
	
	if (updateDlg) delete(updateDlg);
	updateDlg = NULL;

	Rescan();
	if (UNITREADYUI_DRIVEREADY == errReady || UNITREADYUI_CANCELED == errReady || UNITREADYUI_PRIMOSDKERROR == errReady) return errReady;

	LPCDLGTEMPLATE templ = NULL;
	HRSRC hres = FindResourceExW(hResource,  MAKEINTRESOURCEW(5), MAKEINTRESOURCEW(IDD_DLG_UNITNOTREADY), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)); 
	if (hres) templ = (LPCDLGTEMPLATE)LoadResource(hResource, hres);
	HWND dlgWnd = CreateDialogIndirectParamW(dllInstance, templ, ownerWnd, (DLGPROC)WndProc, (LPARAM)this);
	if (!dlgWnd) return UNITREADYUI_UNABLETOCREATEDIALOG;
	
	wchar_t caption[64] = {0}, buffer[48] = {0};
	LoadStringW(hResource, IDS_UNITNOTREADY, buffer, 48); 
	StringCchPrintfW(caption, 64, buffer, (char)*drive);
	SetWindowTextW(GetDlgItem(hwnd, IDC_CAPTION), caption);
	ShowWindow(GetDlgItem(hwnd, IDOK), showRetry);
	
	MSG msg;
	BOOL ret;
	while( 0 != (ret = GetMessageW(&msg, NULL, 0, 0)))
	{	 
		if (ret == -1) 
		{
			errReady = UNITREADYUI_MESSAGEPUMPERROR;
			break;
		}
		if (IsDialogMessage(hwnd, &msg)) continue;
		TranslateMessage(&msg); 
		DispatchMessageW(&msg); 
	}
	
	return errReady;
}

DWORD UnitReadyUI::Rescan(void)
{
	if (!drive) return UNITREADYUI_DRIVENOTSET;
	if (!primoSDK) return UNITREADYUI_PRIMOSDKNOTSET;
	if (hwnd) KillTimer(hwnd, TIMER_REFRESH_ID);

	errPrimo = primoSDK->UnitReady(drive);
	switch(errPrimo)
	{
		case PRIMOSDK_NOTREADY:
			if (hwnd)
			{
				DWORD cmd(0), sense(0), asc(0), ascq(0);
				primoSDK->UnitStatus(drive, &cmd, &sense, &asc, &ascq);
				if (sense != statSense || asc != statAsc || ascq != statAscQ)
				{
					statSense = sense;
					statAsc = asc;
					statAscQ = ascq;
					if ((sense == 0x02 && asc == 0x04 && ascq == 0x01) ||
						(sense == 0x06 && asc == 0x28 && ascq == 0x00))
					{
						if (!updateDlg)
						{
						//	ShowWindow(hwnd, SW_HIDE);
							UpdateWindow(GetParent(hwnd));
							updateDlg = new UpdatingDataUI;
							wchar_t buffer[64] = {0};
							LoadStringW(hResource, IDS_WAITINGFORDRIVE, buffer, 64); 
							updateDlg->Show(0, buffer, TRUE, hwnd);
						}
					}
					else
					{
						if(updateDlg)
						{
							updateDlg->Hide();
							delete(updateDlg);
							updateDlg = NULL;
						}
						
						wchar_t buffer[256] = {0}, pe[512] = {0};
						StringCchPrintfW(buffer, 256, L"%s.", GetUnitStatusText(pe, 512, sense, asc, ascq));
						SetWindowTextW(GetDlgItem(hwnd, IDC_LBL_REASON_VAL), buffer);
						ShowWindow(hwnd, SW_SHOW);
						SetForegroundWindow(hwnd);
						BringWindowToTop(hwnd);
						UpdateWindow(hwnd);
						MessageBeep(MB_ICONEXCLAMATION);
						
					}
				}
			}
			
			
			errReady = UNITREADYUI_NOTREADY;
			SetTimer(hwnd, TIMER_REFRESH_ID, TIMER_REFRESH_INTERVAL, NULL);
			break;
		case PRIMOSDK_OK:
			if (updateDlg)
			{
				updateDlg->Hide();
				delete(updateDlg);
				updateDlg = NULL;
			}
			errReady = UNITREADYUI_DRIVEREADY;
            if(hwnd) PostMessage(hwnd, WM_DESTROY, 0, 0);
			break;
		default:
			if (updateDlg)
			{
				updateDlg->Hide();
				delete(updateDlg);
				updateDlg = NULL;
			}
			errReady = UNITREADYUI_PRIMOSDKERROR; 
			if(hwnd) PostMessage(hwnd, WM_DESTROY, 0, 0);
			hwnd = NULL;
	}
	
	return errReady;
}

void UnitReadyUI::OnInitDialog(HWND hwndDlg)
{
	hwnd = hwndDlg;
	HANDLE hImage =LoadBitmapW(hResource,  MAKEINTRESOURCEW(IDB_DRIVE1));
	if(hImage==NULL){
		hImage = LoadBitmapW(dllInstance,  MAKEINTRESOURCEW(IDB_DRIVE1));
	}
	SendDlgItemMessage(hwnd, IDC_PIC, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImage);
	Rescan();
}

void UnitReadyUI::OnCancel(void)
{

	KillTimer(hwnd, TIMER_REFRESH_ID);
	wchar_t msg[256] = {0}, caption[64] = {0};
	LoadStringW(hResource, IDS_MB_CANCELOPERATION, msg, 256);
	LoadStringW(hResource, IDS_CONFIRMATION, caption, 64);
	if (MessageBoxW(hwnd, msg, caption, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		errReady = UNITREADYUI_CANCELED;
		if(hwnd) DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		SetTimer(hwnd, TIMER_REFRESH_ID, TIMER_REFRESH_INTERVAL, NULL);
	}
}

void UnitReadyUI::OnDestroy(void)
{
	ShowWindow(hwnd, SW_HIDE);
	hwnd = NULL;
	drive = NULL;
	primoSDK = NULL;
}


LRESULT UnitReadyUI::WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static UnitReadyUI *object = NULL;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			object = (UnitReadyUI*)lParam;
			object->OnInitDialog(hwndDlg);
			break;
		case WM_DESTROY:
			object->OnDestroy();
			 PostQuitMessage(object->errReady);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					object->Rescan();
					break;
				case IDCANCEL:
					object->OnCancel();
					break;
			}
			break;
		case WM_TIMER:
			switch(wParam)
			{
				case TIMER_REFRESH_ID:
					object->Rescan();
					break;
			}
			break;

	}
	return 0;
}