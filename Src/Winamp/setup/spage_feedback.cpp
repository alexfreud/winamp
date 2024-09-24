#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include "./spage_feedback.h"
#include "./setup_resource.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "./langutil.h"


#define FF_ALLOWFEEDBACK			0x0001
#define FF_ANNOUNCEMENTS			0x0010
#define FF_STATISTICS			0x0020

#define GENDER_UNKNOWN		0
#define GENDER_MALE			1
#define GENDER_FEMALE		2

setup_page_feedback::setup_page_feedback() : ref(1), hwnd(NULL)
{
	szEmail[0] = 0x00;
	szCountry[0] = 0x00;
	gender = GENDER_UNKNOWN;
	flags = FF_ALLOWFEEDBACK | FF_ANNOUNCEMENTS | FF_STATISTICS;
}

setup_page_feedback::~setup_page_feedback()
{
}

size_t setup_page_feedback::AddRef()
{
	return ++ref;
}

size_t setup_page_feedback::Release()
{
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}

HRESULT setup_page_feedback::GetName(bool bShort, const wchar_t **pszName)
{
	if (bShort)
	{
		static wchar_t szShortName[32] = {0};
		*pszName = (*szShortName) ? szShortName : getStringW(IDS_PAGE_FEEDBACK, szShortName, sizeof(szShortName)/sizeof(wchar_t));
	}
	else 
	{
		static wchar_t szLongName[64] = {0};
		*pszName = (*szLongName) ? szLongName : getStringW(IDS_PAGE_FEEDBACK_LONG, szLongName, sizeof(szLongName)/sizeof(wchar_t));
	}
	return S_OK;
}

HRESULT setup_page_feedback::Save(HWND hwndText)
{
	HRESULT hr(S_OK);

	int r, s;
	char data[8192] = {0};
	char str[32] = {0};

	if (FF_ALLOWFEEDBACK & flags)
	{
		s = GetPrivateProfileIntW(L"WinampReg", L"RegDataLen", 0, INI_FILE);
		if (s > 0) GetPrivateProfileStructA("WinampReg", "RegData2", data, s, INI_FILEA);
		if (!SetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_EMAIL), szEmail, -1)) hr = E_UNEXPECTED;
		if (!SetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_COUNTRY), szCountry, lstrlenW(szCountry)*sizeof(wchar_t))) hr = E_UNEXPECTED;
		if (!SetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_GENDER), &gender, sizeof(gender))) hr = E_UNEXPECTED;
		r = (FF_ANNOUNCEMENTS == (FF_ANNOUNCEMENTS & flags));
		if (!SetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_ANNOUNCEMENTS), &r, sizeof(r))) hr = E_UNEXPECTED;
		config_newverchk2 = (FF_STATISTICS == (FF_STATISTICS & flags));
	}
	else config_newverchk2 = 0;

	WritePrivateProfileStringW(AutoWide(app_name), L"newverchk2", (config_newverchk2) ? L"1" : L"0", INI_FILE);
	
	s = GetMetricsSize(data);
	
	StringCchPrintfA(str, 32, "%d", s);
	WritePrivateProfileStringA("WinampReg", "RegDataLen", (s) ? str : NULL, INI_FILEA);
	WritePrivateProfileStructA("WinampReg", "RegData2", (s) ? data : NULL, s, INI_FILEA);

	if (hwnd) UpdateUI();
	return hr;
}

HRESULT setup_page_feedback::Revert(void)
{
	HRESULT hr(S_OK);
	char data[8192] = {0, };
	DWORD sendinfo;

	flags = FF_ALLOWFEEDBACK;

	if (config_newverchk2) flags |= FF_STATISTICS;

	int s = GetPrivateProfileIntW(L"WinampReg", L"RegDataLen", 0, INI_FILE);
	if (s > 0) GetPrivateProfileStructA("WinampReg", "RegData2", data, s, INI_FILEA);
	
	GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_EMAIL), szEmail, sizeof(szEmail));
	GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_COUNTRY), szCountry, sizeof(szCountry));
	GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_GENDER), &gender, sizeof(gender));
	GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_ANNOUNCEMENTS), &sendinfo, sizeof(sendinfo));
	if (sendinfo) flags |= FF_ANNOUNCEMENTS;
	if (hwnd) UpdateUI();
	return hr;
}

HRESULT setup_page_feedback::IsDirty(void)
{
	char data[8192] = {0};
	wchar_t szTest[256] = {0};
	int r;
	DWORD lcid; 

	lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	if (0 != (FF_STATISTICS & flags) != !!config_newverchk2) return S_OK;

	int s = GetPrivateProfileIntW(L"WinampReg", L"RegDataLen", 0, INI_FILE);
	if (s > 0) GetPrivateProfileStructA("WinampReg", "RegData2", data, s, INI_FILEA);
	
	if (!GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_EMAIL), szTest, sizeof(szTest))) return E_UNEXPECTED;
	r = CompareStringW(lcid, NORM_IGNORECASE, szTest, -1, szEmail, -1);
	if (0 == r) return E_UNEXPECTED;
	if (CSTR_EQUAL != r) return S_OK;

	if (!GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_COUNTRY), szTest, sizeof(szTest))) return E_UNEXPECTED;
	r = CompareStringW(lcid, NORM_IGNORECASE, szTest, -1, szCountry, -1);
	if (0 == r) return E_UNEXPECTED;
	if (CSTR_EQUAL != r) return S_OK;
	r = 0;
	if (!GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_GENDER), &r, sizeof(r))) return E_UNEXPECTED;
	if (r != gender) return S_OK;
	r = 0;
	if (!GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_ANNOUNCEMENTS), &r, sizeof(r))) return E_UNEXPECTED;
	if (r != (FF_ANNOUNCEMENTS == (FF_ANNOUNCEMENTS & flags))) return S_OK;
	
	return S_FALSE;
}

HRESULT setup_page_feedback::Validate(void)
{
	wchar_t *p;

	if (0 == (FF_ALLOWFEEDBACK & flags) || !*szEmail) return S_OK;
	for (p = szEmail; *p != 0x00 && *p != L'@'; p = CharNextW(p));

	if (L'@' != *p || 0x00 == *CharNextW(p))
	{
		WCHAR szTitle[128] = {0};
		HWND hwndTop, hwndTest;
		hwndTop = hwnd;
		while(NULL != (hwndTest = GetParent(hwndTop))) hwndTop = hwndTest;
		GetWindowTextW(hwndTop, szTitle, sizeof(szTitle)/sizeof(WCHAR));
		if (hwndTop) 
		{
			MessageBoxW(hwndTop, getStringW(IDS_PAGE_INCORRECT_EMAIL_ADDRESS, NULL, 0),
						szTitle, MB_OK | MB_ICONERROR);
			return S_FALSE;	
		}
	}
	return S_OK;
}

HRESULT setup_page_feedback::CreateView(HWND hwndParent, HWND *phwnd)
{
	*phwnd = WACreateDialogParam(MAKEINTRESOURCEW(IDD_SETUP_PAGE_FEEDBACK), hwndParent, ::DialogProc, (LPARAM)this);
	return S_OK;
}

void setup_page_feedback::UpdateUI(void)
{
	
	HWND hwndCB;
	if (!hwnd || !IsWindow(hwnd)) return;
	
	CheckDlgButton(hwnd, IDC_CHK_ANNOUNCEMENTS, (FF_ANNOUNCEMENTS & flags) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHK_STATISTICS, (FF_STATISTICS & flags) ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemTextW(hwnd, IDC_EDT_EMAIL, szEmail);
	SetDlgItemTextW(hwnd, IDC_EDT_COUNTRY, szCountry);
		
	hwndCB = GetDlgItem(hwnd, IDC_CB_GENDER);
	if (hwndCB)
	{
		INT count = (INT)SendMessageW(hwndCB, CB_GETCOUNT, 0, 0L);
		if (count > 0)
		{
			INT i;
			for (i = 0; i < count && gender != (INT)SendMessageW(hwndCB, CB_GETITEMDATA, i, 0L); i++);
			if (i != count) SendMessageW(hwndCB, CB_SETCURSEL, i, 0L);
		}
	}
}


void setup_page_feedback::ComboBox_OnSelChange(HWND hwndCtrl)
{
	INT index;
	index = (INT)SendMessageW(hwndCtrl, CB_GETCURSEL, 0, 0L);
	if (CB_ERR == index) return;
	gender = (INT)SendMessageW(hwndCtrl, CB_GETITEMDATA, index, 0L);
}

INT_PTR setup_page_feedback::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	HWND hwndCB;

	SendDlgItemMessageW(hwnd, IDC_EDT_EMAIL, EM_LIMITTEXT, sizeof(szEmail)/sizeof(wchar_t), 0);
	SendDlgItemMessageW(hwnd, IDC_EDT_COUNTRY, EM_LIMITTEXT, sizeof(szCountry)/sizeof(wchar_t), 0);

	hwndCB = GetDlgItem(hwnd, IDC_CB_GENDER);
	if (hwndCB)
	{
		UINT genderStrID[] = {IDS_GENDER_UNKNOWN,  IDS_GENDER_FEMALE, IDS_GENDER_MALE, };
		INT	genderVal[] = {GENDER_UNKNOWN, GENDER_FEMALE, GENDER_MALE, };
		for(int i = 0;  i < sizeof(genderStrID)/sizeof(UINT); i++) 
		{
			INT index = (INT)SendMessageW(hwndCB, CB_ADDSTRING,0, (LPARAM)getStringW(genderStrID[i], NULL, 0));
			if (CB_ERR != index) SendMessageW(hwndCB, CB_SETITEMDATA, index, (LPARAM)genderVal[i]);
		}
	}

	UpdateUI();
	return 0;
}

void setup_page_feedback::OnSize(UINT nType, INT cx, INT cy)
{
	INT ctrlList[] = { IDC_LBL_HEADER, IDC_CHK_ANNOUNCEMENTS, IDC_CHK_STATISTICS,
						IDC_EDT_EMAIL, IDC_EDT_COUNTRY, IDC_CB_GENDER, };
	HWND hwndCtrl;
	HDWP hdwp;
	RECT rw;
	INT width, ox;

	hwndCtrl = GetDlgItem(hwnd, IDC_LBL_EMAIL);
	GetWindowRect(hwndCtrl, &rw);
	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
	ox = rw.left;

	hdwp = BeginDeferWindowPos(sizeof(ctrlList)/sizeof(INT));
	if (!hdwp) return;

	for (int i = 0; i < sizeof(ctrlList)/sizeof(INT); i++)
	{
		hwndCtrl = GetDlgItem(hwnd, ctrlList[i]);
		if (hwndCtrl)
		{
			GetWindowRect(hwndCtrl, &rw);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
			switch(ctrlList[i])
			{
				case IDC_EDT_EMAIL:
				case IDC_EDT_COUNTRY:
				case IDC_CB_GENDER:
					width = cx - rw.left - ox; 
					break;
				default: 
					width = cx - rw.left*2; 
					break;
			}
			hdwp = DeferWindowPos(hdwp, hwndCtrl, NULL, 0, 0, width, rw.bottom  - rw.top, 
									SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		}
	}
	if (hdwp) EndDeferWindowPos(hdwp);
}


void setup_page_feedback::OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl)
{
	switch(nCtrlID)
	{			
		case IDC_EDT_EMAIL:
			switch(nEvntID)
			{
				case EN_CHANGE: 
					{
						GetWindowTextW(hwndCtrl, szEmail, sizeof(szEmail)/sizeof(wchar_t));
						HWND hwndChk = GetDlgItem(hwnd, IDC_CHK_ANNOUNCEMENTS);
						if (IsWindow(hwndChk))
						{
							BOOL bEnabled = IsWindowEnabled(hwndChk);
							if (bEnabled != (0x00 != *szEmail))
							{
								CheckDlgButton(hwnd, IDC_CHK_ANNOUNCEMENTS, (0x00 != *szEmail) ? BST_CHECKED : BST_UNCHECKED);
								EnableWindow(GetDlgItem(hwnd, IDC_CHK_ANNOUNCEMENTS), 0x00 != *szEmail);
								PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_CHK_ANNOUNCEMENTS, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_CHK_ANNOUNCEMENTS));
							}
						}
					}
					break;
			}
			break;
		case IDC_EDT_COUNTRY:
			switch(nEvntID)
			{
				case EN_CHANGE: 
					GetWindowTextW(hwndCtrl, szCountry, sizeof(szCountry)/sizeof(wchar_t));
					break;
			}
			break;
		case IDC_CHK_ANNOUNCEMENTS:
			switch(nEvntID)
			{
				case BN_CLICKED: 
					if (BST_CHECKED & (INT)SendMessageW(hwndCtrl, BM_GETSTATE, 0, 0L)) flags |= FF_ANNOUNCEMENTS;
					else flags &= ~FF_ANNOUNCEMENTS;
					break;
			}
			break;
		case IDC_CHK_STATISTICS:
			switch(nEvntID)
			{
				case BN_CLICKED: 
					if (BST_CHECKED & (INT)SendMessageW(hwndCtrl, BM_GETSTATE, 0, 0L)) flags |= FF_STATISTICS;
					else flags &= ~FF_STATISTICS;
					break;
			}
			break;
		case IDC_CB_GENDER:
			switch(nEvntID)
			{
				case CBN_SELCHANGE: ComboBox_OnSelChange(hwndCtrl); break;
			}
			break;
	}
}

INT_PTR setup_page_feedback::PageDlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:		break;
		case WM_SIZE:		OnSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_COMMAND:		OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
	}
	return 0;
}

static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	setup_page_feedback *pInst = (setup_page_feedback*)GetPropW(hwnd, L"SETUPPAGE");

	switch(uMsg)
	{
		case WM_INITDIALOG:
			pInst = (setup_page_feedback*)lParam;
			if (pInst)
			{				
				pInst->hwnd = hwnd;
				SetPropW(hwnd, L"SETUPPAGE", pInst);
			}
			break;
		case WM_DESTROY:
			if (pInst) 
			{	
				pInst->PageDlgProc(uMsg, wParam, lParam);
				RemovePropW(hwnd,  L"SETUPPAGE");
				pInst = NULL;
			}
			break;
	}
	
	return (pInst) ? pInst->PageDlgProc(uMsg, wParam, lParam) : 0;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS setup_page_feedback
START_DISPATCH
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(API_SETUPPAGE_GET_NAME, GetName)
CB(API_SETUPPAGE_CREATEVIEW, CreateView)
CB(API_SETUPPAGE_SAVE, Save)
CB(API_SETUPPAGE_REVERT, Revert)
CB(API_SETUPPAGE_ISDIRTY, IsDirty)
CB(API_SETUPPAGE_VALIDATE, Validate)
END_DISPATCH
#undef CBCLASS