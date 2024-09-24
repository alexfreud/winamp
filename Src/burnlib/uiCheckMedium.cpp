#include "./uiCheckMedium.h"
#include "./resource.h"
#include <strsafe.h>

#include "./uiEraseMedium.h"
#include "./uiUnitReady.h"

#define AINSTATE_DISABLE	PRIMOSDK_LOCK
#define AINSTATE_ENABLE		PRIMOSDK_UNLOCK


CheckMediumUI::CheckMediumUI(void)
{
	hwnd = NULL;
	drive = NULL;
	primoSDK = NULL;
	errPrimo = 0;
	errReady = 0;
}
CheckMediumUI::~CheckMediumUI(void)
{
	if (hwnd) DestroyWindow(hwnd);
}

DWORD CheckMediumUI::Check(obj_primo *primoSDK, DWORD *drive, WAMEDIUMINFO *medium, const wchar_t *description, BOOL disableAIN, BOOL showErase, HWND ownerWnd)
{

	if (!medium) return CHECKMEDIUMUI_DISCNOTSET;
	if (!drive) return CHECKMEDIUMUI_DRIVENOTSET;
	if (!primoSDK) return CHECKMEDIUMUI_PRIMOSDKNOTSET;
	this->desiredMedium	= medium;
	this->drive = drive;
	this->primoSDK = primoSDK;
	this->disableAIN = (disableAIN) ? AINSTATE_DISABLE : 0 ;
	this->showErase = showErase;
	this->description = description;
	this->ownerWnd = ownerWnd;
	
	hwnd = NULL;
	errPrimo = PRIMOSDK_OK;
	errReady = CHECKMEDIUMUI_NOMATCH;
	
	deadLoop = 0;

	LPCDLGTEMPLATE templ = NULL;
	HRSRC hres = FindResourceExW(hResource,  MAKEINTRESOURCEW(5), MAKEINTRESOURCEW(IDD_DLG_WRONGMEDIUM), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)); 
	if (hres) templ = (LPCDLGTEMPLATE)LoadResource(hResource, hres);
	HWND dlgWnd = CreateDialogIndirectParamW(dllInstance, templ, ownerWnd, (DLGPROC)WndProc, (LPARAM)this);
	
	if (!dlgWnd) return CHECKMEDIUMUI_UNABLETOCREATEDIALOG;
	
	MSG msg;
	BOOL ret;
	while( 0 != (ret = GetMessageW(&msg, hwnd, 0, 0)))
	{	 
		if (ret == -1) 
		{
			errReady = CHECKMEDIUMUI_MESSAGEPUMPERROR;
			break;
		}
		if (IsDialogMessage(hwnd, &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return errReady;
}

DWORD CheckMediumUI::Rescan(void)
{
	wchar_t buffer[256] = {0};
	if (hwnd)
	{
		EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
		
		LoadStringW(hResource, IDS_UPDATINGDATA, buffer, 256);
		SetWindowLong(GetDlgItem(hwnd, IDC_LBL_DETECTEDMEDIUM), GWL_STYLE, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE);
		SetDlgItemTextW(hwnd, IDC_LBL_DETECTEDMEDIUM, buffer);
	}
	if (!drive) return CHECKMEDIUMUI_DRIVENOTSET;
	if (!primoSDK) return CHECKMEDIUMUI_PRIMOSDKNOTSET;
	
	
	UpdatingDataUI updateDlg;
	LoadStringW(hResource, IDS_READINGDISCINFO, buffer, 256);
	updateDlg.Show(800, buffer, TRUE, hwnd);

	WAMEDIUMINFO detectedMedium;
	ZeroMemory(&detectedMedium, sizeof(WAMEDIUMINFO));
	
	errPrimo = GetMediumInfo(primoSDK, drive, &detectedMedium);
    updateDlg.Hide();


	if (PRIMOSDK_OK != errPrimo)
	{

		SetWindowLong(GetDlgItem(hwnd, IDC_LBL_DETECTEDMEDIUM), GWL_STYLE,  WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE);
		LoadStringW(hResource, IDS_UNABLEGETDISCINFO, buffer, 256);
		SetDlgItemTextW(hwnd, IDC_LBL_DETECTEDMEDIUM, buffer);
		EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_BTN_ERASE), FALSE);
		EnableWindow(hwnd, FALSE);
		if (AINSTATE_DISABLE == disableAIN && primoSDK) 
		{
			primoSDK->UnitAIN(drive, disableAIN);
			disableAIN = AINSTATE_ENABLE;
		}
		UnitReadyUI ready;
		DWORD test = ready.Check(primoSDK, drive, FALSE, hwnd);
		if (UNITREADYUI_DRIVEREADY == test) deadLoop++;
		if (deadLoop == 5) 
		{ /// dead loop detected - PRIMOSDK broken - recomended restart of the computer
			errReady = CHECKMEDIUMUI_DEADLOOP; 
			if(hwnd) PostMessage(hwnd, WM_DESTROY, 0, 0);
			return errReady;
		}
		EnableWindow(hwnd, TRUE);
		UpdateWindow(hwnd);
		switch(test)
		{
			case UNITREADYUI_DRIVEREADY:
				break;	
			case UNITREADYUI_PRIMOSDKERROR:
				errReady = CHECKMEDIUMUI_PRIMOSDKERROR; 
				break;
			case UNITREADYUI_CANCELED:
				errReady = CHECKMEDIUMUI_CANCELED; 
				break;
			default:
				errReady = CHECKMEDIUMUI_PRIMOSDKERROR; 
				break;

		}
		if (UNITREADYUI_DRIVEREADY != test)
		{
			if(hwnd) PostMessage(hwnd, WM_DESTROY, 0, 0);
			return errReady;
		}
		
		PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0); // start other rescan
		errReady = CHECKMEDIUMUI_DRIVENOTREADY; 
		return errReady;

	}

	if (hwnd)
	{
		EnableWindow(GetDlgItem(hwnd, IDCANCEL), TRUE);
		SetDlgItemTextW(hwnd, IDC_LBL_DETECTEDMEDIUM, L"");
	}
	
	errReady = CHECKMEDIUMUI_MATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->medium && desiredMedium->medium != detectedMedium.medium)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->mediumType && desiredMedium->mediumType != detectedMedium.mediumType)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->mediumFormat && desiredMedium->mediumFormat != detectedMedium.mediumFormat)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->tracks && desiredMedium->tracks != detectedMedium.tracks)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->erasable && desiredMedium->erasable != detectedMedium.erasable)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->freeSectors && desiredMedium->freeSectors > detectedMedium.freeSectors)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->usedSectors && desiredMedium->usedSectors < detectedMedium.usedSectors)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->recordable && desiredMedium->recordable != detectedMedium.recordable)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->isCD && desiredMedium->isCD != detectedMedium.isCD)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->isDCD && desiredMedium->isDCD != detectedMedium.isDCD)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->isDVD && desiredMedium->isDVD != detectedMedium.isDVD)  errReady = CHECKMEDIUMUI_NOMATCH;
	if (CHECKMEDIUMUI_MATCH == errReady && MAXDWORD != desiredMedium->isDLDVD && desiredMedium->isDLDVD != detectedMedium.isDLDVD)  errReady = CHECKMEDIUMUI_NOMATCH;
	

	if (CHECKMEDIUMUI_MATCH == errReady)
	{
		if(hwnd) PostMessage(hwnd, WM_DESTROY, 0, 0);
	}
	else if (hwnd)
	{
		wchar_t buffer[4096] = {0};
		SetWindowLong(GetDlgItem(hwnd, IDC_LBL_DETECTEDMEDIUM), GWL_STYLE, WS_CHILD | WS_VISIBLE | SS_LEFT);
		SetDlgItemTextW(hwnd, IDC_LBL_DETECTEDMEDIUM, GetMediumInfoText(buffer, 4096, &detectedMedium));

		if (showErase) EnableWindow(GetDlgItem(hwnd, IDC_BTN_ERASE), (detectedMedium.erasable && detectedMedium.usedSectors > 0));
		ShowWindow(hwnd, SW_SHOWNORMAL);
		SetForegroundWindow(hwnd);
		BringWindowToTop(hwnd);
		MessageBeep(MB_ICONEXCLAMATION);
		UpdateWindow(hwnd);
	}
	return errReady;
}

wchar_t* CheckMediumUI::GetMediumInfoText(wchar_t *buffer, unsigned int cchBuffer, WAMEDIUMINFO *info)
{
	buffer[0] = 0x0000;

	wchar_t format[64] = {0}, freeSize[256] = {0}, usedSize[256] = {0}, medium[32] = {0}, mtype[128] = {0}, mformat[128] = {0}, erasable[32] = {0};
	LoadStringW(hResource, IDS_DISCINFOFORMAT, format, 64);
	LoadStringW(hResource, (info->erasable) ? IDS_DISCERASABLE : IDS_DISCNONERASABLE, erasable, 32);
	StringCchPrintfW(buffer, cchBuffer, format, GetMediumText(medium, 32, info->medium),
																erasable,
																GetSizeText(freeSize, 256, info->freeSectors),
																GetSizeText(usedSize, 256, info->usedSectors),
																GetMediumFormatText(mformat, 128,info->mediumFormat), 
																GetMediumTypeText(mtype, 128, info->mediumType));
	return buffer;
}

wchar_t* CheckMediumUI::GetSizeText(wchar_t *buffer, unsigned int cchBuffer, unsigned int sectors)
{
	wchar_t format[32] = {0},tmp[256] = {0};
	DWORD minutes = sectors/(75*60);
	DWORD kb = MulDiv(sectors, 2048,1024);
	DWORD mb = kb / 1024;
	LoadStringW(hResource, IDS_DISCSIZEFORMAT, format, 32);
	StringCchPrintfW(tmp, 256, format, minutes, mb, sectors);
	StringCchCatW(buffer, cchBuffer, tmp);
	return buffer;
}
void CheckMediumUI::OnInitDialog(HWND hwndDlg)
{
	hwnd = hwndDlg;

	HANDLE hImage = LoadBitmapW(hResource,  MAKEINTRESOURCEW(IDB_DISC1));
	if(hImage==NULL){
		hImage = LoadBitmapW(dllInstance,  MAKEINTRESOURCEW(IDB_DISC1));
	}
	SendDlgItemMessage(hwnd, IDC_PIC, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImage);

	wchar_t format[64] = {0}, buffer[66] = {0};
	
	LoadStringW(hResource, IDS_WRONGMEDIUM, format, 64);
	StringCchPrintfW(buffer, 64, format, (char)*drive);
	SetWindowTextW(GetDlgItem(hwnd, IDC_CAPTION), buffer);
	SetWindowTextW(GetDlgItem(hwnd, IDC_LBL_REQUESTEDMEDIUM), description);
	HWND btnWnd = GetDlgItem(hwnd, IDC_BTN_ERASE);
	EnableWindow(btnWnd, showErase);
	ShowWindow(btnWnd, showErase);
	Rescan();
}

void CheckMediumUI::OnCancel(void)
{
	wchar_t msg[256] = {0}, caption[64] = {0};
	LoadStringW(hResource, IDS_MB_CANCELOPERATION, msg, 256);
	LoadStringW(hResource, IDS_CONFIRMATION, caption, 64);
	if (MessageBoxW(hwnd, msg, caption, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		errReady = CHECKMEDIUMUI_CANCELED;
		if(hwnd) DestroyWindow(hwnd);
		hwnd = NULL;
	}
}

void CheckMediumUI::OnDestroy(void)
{
	if (AINSTATE_ENABLE == disableAIN && primoSDK) 
	{
		primoSDK->UnitAIN(drive, disableAIN);
		disableAIN = AINSTATE_DISABLE;
	}
	ShowWindow(hwnd, SW_HIDE);
	hwnd = NULL;
	drive = NULL;
	primoSDK = NULL;
	deadLoop = 0;
}

void CheckMediumUI::OnEraseClicked(void)
{
	EraseMediumUI eraseMedium;
	eraseMedium.Erase(*drive, FALSE, hwnd); // do not perform disc check
	UpdateWindow(hwnd);
	Rescan();
}

LRESULT CheckMediumUI::WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CheckMediumUI *object = NULL;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			object = (CheckMediumUI*)lParam;
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
				case IDC_BTN_ERASE:
					if (BN_CLICKED == HIWORD(wParam)) object->OnEraseClicked();
					break;
			}
			break;
	}
	return 0;
}