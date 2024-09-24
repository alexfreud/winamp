#include "./uiEraseMedium.h"
#include "api.h"
#include <api/service/waservicefactory.h>
#include "./resource.h"
#include <strsafe.h>
#include <commctrl.h>


#include "./uiCheckMedium.h"

#define ERASETIME_QUICKMODE			50 // in seconds
#define ERASETIME_FULLMODE			510 // in seconds

#define TIMER_CLOCK_ID				1979
#define TIMER_CLOCK_INTERVAL		1000

EraseMediumUI::EraseMediumUI(void)
{
	eraseMedium = NULL;
	
}

EraseMediumUI::~EraseMediumUI(void)
{
	if (eraseMedium)
	{
		delete(eraseMedium);
		eraseMedium = NULL;
	}
}
DWORD EraseMediumUI::SetEject(int ejectmode)
{
	return (eraseMedium) ? eraseMedium->SetEject(ejectmode) : 0;
}

DWORD EraseMediumUI::Erase(DWORD drive, BOOL discCheck, HWND ownerWnd)
{
	
	this->drive = drive;
	this->discCheck = discCheck;
	DWORD retCode;
	LPCDLGTEMPLATE templ = NULL;

	HRSRC hres = FindResourceExW(hResource,  MAKEINTRESOURCEW(5), MAKEINTRESOURCEW(IDD_DLG_ERASEMEDIUMPREPARE), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)); 
	templ = (hres) ? (LPCDLGTEMPLATE)LoadResource(hResource, hres) : NULL;
	retCode = (DWORD)DialogBoxIndirectParamW(dllInstance, templ, ownerWnd, (DLGPROC)PrepareWndProc, (LPARAM)this);
	if (ERASEMEDIUMUI_OK != retCode) return retCode;
	
	hres = FindResourceExW(hResource,  MAKEINTRESOURCEW(5), MAKEINTRESOURCEW(IDD_DLG_ERASEMEDIUMSTATUS), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)); 
	templ = (hres) ? (LPCDLGTEMPLATE)LoadResource(hResource, hres) : NULL;
	retCode = (DWORD)DialogBoxIndirectParamW(dllInstance, templ, ownerWnd, (DLGPROC)EraseWndProc, (LPARAM)this);
	if (ERASEMEDIUM_ABORTED == retCode) 	return ERASEMEDIUMUI_CANCELED;
	else if (retCode > ERASEMEDIUM_ERROR) return ERASEMEDIUMUI_ERROR;
    return ERASEMEDIUMUI_OK;
}

void EraseMediumUI::OnPrepareInit(HWND hwndDlg)
{
	prepareWnd = hwndDlg;
	wchar_t format[96] = {0}, buffer[98] = {0};
	
	LoadStringW(hResource, IDS_SELECTERASEMETHOD, format, 96);
	StringCchPrintfW(buffer, 98, format, drive);
	SetDlgItemTextW(prepareWnd, IDC_CAPTION, buffer);
	HWND cmbWnd = GetDlgItem(prepareWnd, IDC_CMB_ERASEMETHOD);
	
	LoadStringW(hResource, IDS_QUICKERASE, buffer, 98);
	SendMessageW(cmbWnd, CB_ADDSTRING, 0, (LPARAM)buffer);
	LoadStringW(hResource, IDS_COMPLETEERASE, buffer, 98);
	SendMessageW(cmbWnd, CB_ADDSTRING, 0, (LPARAM)buffer);
	SendMessageW(cmbWnd, CB_SETCURSEL, 0, 0);
	PostMessageW(prepareWnd, WM_COMMAND, MAKEWPARAM(IDC_CMB_ERASEMETHOD, CBN_SELCHANGE), 0);
	ShowWindow(prepareWnd, SW_SHOWNORMAL);
	SetForegroundWindow(prepareWnd);
	BringWindowToTop(prepareWnd);
}

void EraseMediumUI::OnPrepareOk()
{
	switch(SendMessage(GetDlgItem(prepareWnd, IDC_CMB_ERASEMETHOD), CB_GETCURSEL, 0,0))
	{
		case 1:	eraseMode = PRIMOSDK_ERASEFULL; break;
		default: eraseMode = PRIMOSDK_ERASEQUICK; break;
		
	}
	if (discCheck)
	{
		// check that disc is erasable
		obj_primo *primo=0;
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
		if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
		if (primo)
		{
		WAMEDIUMINFO 	mi;
		FillMemory(&mi, sizeof(WAMEDIUMINFO), 0xFF);
		mi.erasable = TRUE;
		CheckMediumUI cm;
		wchar_t buffer[256] = {0};
		LoadStringW(hResource, IDS_ERASEREQDISC, buffer, 256);
		DWORD retCode = cm.Check(primo, &drive, &mi, buffer, TRUE, FALSE, prepareWnd);
		
		switch(retCode)
		{
			
			case CHECKMEDIUMUI_MATCH: eraseCode = ERASEMEDIUMUI_OK; break;
			case CHECKMEDIUMUI_CANCELED: eraseCode = ERASEMEDIUMUI_CANCELED; break;
			default:  eraseCode = ERASEMEDIUMUI_PRIMOSDKERROR; break;
		}
		
	sf->releaseInterface(primo);
		}
		else
			eraseCode = ERASEMEDIUMUI_PRIMOSDKERROR;
	}
	else
	{
		eraseCode = ERASEMEDIUMUI_OK;
	}

	EndDialog(prepareWnd, eraseCode);
}
void EraseMediumUI::OnEraseTimerClock(void)
{
	actualTime = (GetTickCount() - startTick) / 1000;
	wchar_t time[32], *current;
	current = time;
	GetTimeString(current, 32, actualTime);
	unsigned int len = lstrlenW(current);
	current[len] = L'/';
	len++;
	current += len;
	if (estimateTime < actualTime) estimateTime = actualTime;
	GetTimeString(current, 32 - len, estimateTime);
	SetWindowTextW(GetDlgItem(eraseWnd, IDC_LBL_TIME), time);
	SendMessage(GetDlgItem(eraseWnd, IDC_PRG_PROGRESS), PBM_SETPOS, (int)(((float)actualTime/(float)estimateTime)*100), 0);
}
void EraseMediumUI::OnEraseInit(HWND hwndDlg)
{
	eraseWnd = hwndDlg;
	CheckDlgButton(eraseWnd, IDC_CHECK_EJECT, BST_CHECKED);
	EnableWindow(GetDlgItem(eraseWnd, IDC_BTN_EJECT), FALSE);
	
	eraseMedium = new(EraseMedium);
	if (!eraseMedium) EndDialog(eraseWnd, ERASEMEDIUMUI_UNABLETOCREATEOBJECT);


	startTick =  GetTickCount();
	OnEraseTimerClock();

	ShowWindow(eraseWnd, SW_SHOWNORMAL);
	SetForegroundWindow(eraseWnd);
	BringWindowToTop(eraseWnd);
	UpdateWindow(eraseWnd);


	SetTimer(eraseWnd, TIMER_CLOCK_ID, TIMER_CLOCK_INTERVAL, NULL);
	SendMessage(GetDlgItem(eraseWnd, IDC_PRG_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	eraseMedium->Start(drive, eraseMode, OnEraseNotify, this, FALSE); // will catch all errors in calback
	EnableWindow(GetDlgItem(eraseWnd, IDC_BTN_EJECT), TRUE);
	
}

void EraseMediumUI::OnEraseClose(DWORD exitCode)
{
	EnableWindow(GetDlgItem(eraseWnd, IDC_BTN_EJECT), FALSE);
	if (eraseMedium) 
	{
		HWND btnWnd = GetDlgItem(eraseWnd, IDCANCEL);
		EnableWindow(btnWnd, FALSE);
		eraseMedium->Stop();
		wchar_t buffer[24] = {0};
		LoadStringW(hResource, IDS_CLOSE, buffer, 24);
		SetWindowTextW(btnWnd, buffer);
		EnableWindow(btnWnd, TRUE);
		primoCode = eraseMedium->GetErrorCode();
		delete(eraseMedium);
		eraseMedium = NULL;
	}
	eraseCode = exitCode;
	KillTimer(eraseWnd, TIMER_CLOCK_ID);
	estimateTime = 0; // will force it to be the same with actual 
	OnEraseTimerClock();
}

LRESULT EraseMediumUI::PrepareWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static EraseMediumUI *object = NULL;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			object = (EraseMediumUI*)lParam;
			object->OnPrepareInit(hwndDlg);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
                case IDOK:
					object->OnPrepareOk();
					break;
				case IDCANCEL:
				{
					wchar_t msg[256] = {0}, caption[64] = {0};
					LoadStringW(hResource, IDS_MB_CANCELOPERATION, msg, 256);
					LoadStringW(hResource, IDS_CONFIRMATION, caption, 64);
					if (MessageBoxW(hwndDlg, msg, caption, MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						EndDialog(hwndDlg, LOWORD(wParam));
					}
					break;
				}
				case IDC_CMB_ERASEMETHOD:
					switch(HIWORD(wParam))
					{
						case CBN_SELCHANGE:
							switch(SendMessage(GetDlgItem(hwndDlg, IDC_CMB_ERASEMETHOD), CB_GETCURSEL, 0,0))
							{
								case 0:
									object->estimateTime= ERASETIME_QUICKMODE;
									break;
								case 1:
									object->estimateTime = ERASETIME_FULLMODE;
									break;
								default:
									object->estimateTime =0;
									break;
							}
							wchar_t time[16] = {0};
							SetWindowTextW(GetDlgItem(hwndDlg, IDC_LBL_TIME), GetTimeString(time, 16, object->estimateTime));
							break;
					}
					break;
			}
			break;
	}
	return 0;
}

LRESULT EraseMediumUI::EraseWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static EraseMediumUI *object = NULL;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			object = (EraseMediumUI*)lParam;
			object->OnEraseInit(hwndDlg);
			break;
		case WM_CLOSE:
			object->OnEraseClose((DWORD)wParam);
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
					if (object->eraseMedium) 
					{
						wchar_t msg[256] = {0}, caption[64] = {0};
						LoadStringW(hResource, IDS_MB_CANCELOPERATION, msg, 256);
						LoadStringW(hResource, IDS_CONFIRMATION, caption, 64);
						if (MessageBoxW(hwndDlg, msg, caption, MB_YESNO | MB_ICONQUESTION) == IDYES) object->OnEraseClose(ERASEMEDIUM_ABORTED);
					}
					EndDialog(hwndDlg, object->eraseCode);
					break;
				case IDC_BTN_EJECT:
					if (BN_CLICKED == HIWORD(wParam) && object->eraseMedium) 
					{
						object->eraseMedium->SetEject(BST_CHECKED == IsDlgButtonChecked (hwndDlg,IDC_BTN_EJECT));
					}
					break;
			}
			break;
		case WM_TIMER:
			switch(wParam)
			{
				case TIMER_CLOCK_ID:
					object->OnEraseTimerClock();
					break;	
					
			}
			break;

	}
	return 0;
}

DWORD EraseMediumUI::OnEraseNotify(void *sender, void *param,  DWORD eraseCode, DWORD primoCode)
{
	EraseMediumUI *object = (EraseMediumUI*)param;
	
	unsigned int strcode;
	BOOL finished = FALSE;

	if (ERASEMEDIUM_ERROR < eraseCode) 
	{// some error happened;
		finished = TRUE;

        switch(eraseCode)
		{
			case ERASEMEDIUM_ALREADYSTARTED:		strcode = IDS_ALREADYERASING; break;
			case ERASEMEDIUM_UNABLEINITPRIMO:	strcode = IDS_PRIMOINITFAILED;	break;
			case ERASEMEDIUM_BEGINBURNFAILED:	strcode = IDS_UNABLEINITERASE; break;
			case ERASEMEDIUM_ENDBURNFAILED:		strcode = IDS_UNABLEFINERASE; break;
			case ERASEMEDIUM_ERASEMEDIUMFAILED:	strcode = IDS_MEDIUMERASEFAILED; break;
			case ERASEMEDIUM_DEVICENOTREADY:		strcode = IDS_DRIVENOTREADY;	 break;
			case ERASEMEDIUM_DISCINFOERROR: 		strcode = IDS_UNABLEGETDISCINFO; break;
			case ERASEMEDIUM_DISCNOTERASABLE:	strcode = IDS_DISCNONERASABLE; break;
			default:							strcode = IDS_UNKNOWNERROR; break;
		}
		MessageBeep(MB_ICONHAND);
	}
	else 
	{
		switch(eraseCode)
		{
			case ERASEMEDIUM_READY:			strcode = IDS_READY; break;
			case ERASEMEDIUM_INITIALIZING:	strcode = IDS_INITIALIZING; break;
			case ERASEMEDIUM_ERASING:		strcode = IDS_ERASING; break;
			case ERASEMEDIUM_FINISHING:		strcode = IDS_FINISHING; break;
			case ERASEMEDIUM_CANCELING:		strcode = IDS_CANCELING; break;
			case ERASEMEDIUM_COMPLETED:		strcode = IDS_COMPLETED;	finished = TRUE; MessageBeep(MB_OK); break;
			case ERASEMEDIUM_ABORTED:		strcode = IDS_ABORTED;	finished = TRUE; MessageBeep(MB_OK); break;
		}
		
	}
	wchar_t buffer[224] = {0};
	LoadStringW(hResource, strcode, buffer, 224);

	if (ERASEMEDIUM_ERROR < eraseCode) 
	{	
		wchar_t txtStatus[256] = {0}, error[24] = {0};
		LoadStringW(hResource, IDS_ERROR, error, 24);
		StringCchPrintfW(txtStatus, 256, L"%s! %s", error, buffer);
	}
	else
	{
		SetWindowTextW(GetDlgItem(object->eraseWnd, IDC_LBL_STATUS), buffer);
	}
	
	if (finished) PostMessage(object->eraseWnd, WM_CLOSE, eraseCode, 0);
	return ERASEMEDIUM_CONTINUE;
}