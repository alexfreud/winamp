#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include "./spage_connect.h"
#include "./setup_resource.h"
#include "../nu/ns_wc.h"
#include "./langutil.h"

setup_page_connect::setup_page_connect() : ref(1), hwnd(NULL)
{
	inetMode = 0;
	bPort80  = FALSE;
	szProxy[MAX_PATH] = 0x00;
}
setup_page_connect::~setup_page_connect()
{
}

size_t setup_page_connect::AddRef()
{
	return ++ref;
}

size_t setup_page_connect::Release()
{
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}

HRESULT setup_page_connect::GetName(bool bShort, const wchar_t **pszName)
{
	static wchar_t szName[64] = {0,};
	*pszName = (*szName) ? szName : getStringW(IDS_PAGE_CONNECTIVITY, szName, sizeof(szName)/sizeof(wchar_t));
	return S_OK;
}

HRESULT setup_page_connect::Save(HWND hwndText)
{
	HRESULT hr(S_OK);
	INT count;
	wchar_t app[MAX_PATH] = {0}, *p;

	if (S_FALSE == IsDirty()) return S_OK;

	count = MultiByteToWideCharSZ(CP_ACP, 0, app_name, -1, app, sizeof(app)/sizeof(wchar_t));
	if (!count) hr = S_FALSE;
	if (S_FALSE == hr) return hr;

	if (config_inet_mode != inetMode)
	{
		wchar_t szText[MAX_PATH] = {0};
		config_inet_mode = (char)inetMode;
		WritePrivateProfileStringW(app, L"inet_mode", _itow(inetMode, szText, 10), INI_FILE);
	}

	if (2 == inetMode) szProxy[0] = 0x00;
    if (0 == *szProxy) config_proxy[0] = 0x00;
	else
	{
		count = WideCharToMultiByteSZ(CP_ACP, 0, szProxy, -1, config_proxy, sizeof(config_proxy)/sizeof(char), NULL, NULL);
		if (!count) hr = S_FALSE;
	}
	if (!WritePrivateProfileStringW(app, L"Proxy", szProxy, INI_FILE)) hr = S_FALSE;
	if (bPort80 && 0x00 == *szProxy) bPort80 = FALSE;
	p = (bPort80) ? L"1" : L"";
	if (!WritePrivateProfileStringW(app, L"proxyonly80", p, INI_FILE)) hr = S_FALSE;
	WritePrivateProfileStringW(app, L"proxy80", p, INI_FILE); // old

	if (hwnd) UpdateUI();
	return hr;
}

HRESULT setup_page_connect::Revert(void)
{
	HRESULT hr(S_OK);
	INT count;
	wchar_t app[MAX_PATH] = {0};

	count = MultiByteToWideCharSZ(CP_ACP, 0, app_name, -1, app, sizeof(app)/sizeof(wchar_t));
	if (!count) hr = S_FALSE;

	if (config_inet_mode==3) isInetAvailable(); // autodetect
	inetMode = config_inet_mode;

	if (*config_proxy && 2 != inetMode)
	{
		count = MultiByteToWideCharSZ(CP_ACP, 0, config_proxy, -1, szProxy, sizeof(szProxy)/sizeof(wchar_t));
		if (!count) hr = S_FALSE;
	}
	else szProxy[0] = 0x00;

	bPort80 = (0x00 != *szProxy && 0 != GetPrivateProfileIntW(app, L"proxyonly80", 0, INI_FILE));

	if (hwnd) UpdateUI();

	return hr;
}

HRESULT setup_page_connect::IsDirty(void)
{
	INT res;
	wchar_t app[MAX_PATH] = {0};
	char szTest[MAX_PATH] = {0};

	if (inetMode != config_inet_mode) return S_OK;

	if (2 == inetMode || 0 == *szProxy) szTest[0] = 0x00;
	else
	{
		res = WideCharToMultiByteSZ(CP_ACP, 0, szProxy, -1, szTest, sizeof(szTest)/sizeof(char), NULL, NULL);
		if (!res) return E_OUTOFMEMORY;
	}

	res = CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
							NORM_IGNORECASE, szTest, -1, config_proxy, -1);
	if (0 == res) return E_UNEXPECTED;
	if (CSTR_EQUAL != res) return S_OK;

	res = MultiByteToWideCharSZ(CP_ACP, 0, app_name, -1, app, sizeof(app)/sizeof(wchar_t));
	if (!res) return E_OUTOFMEMORY;
	
	return ((bPort80 && 0x00 != *szProxy) != ( 0 != GetPrivateProfileIntW(app, L"proxyonly80", 0, INI_FILE))) ? S_OK : S_FALSE;
}

HRESULT setup_page_connect::Validate(void)
{
	return S_OK;
}

HRESULT setup_page_connect::CreateView(HWND hwndParent, HWND *phwnd)
{
	*phwnd = WACreateDialogParam(MAKEINTRESOURCEW(IDD_SETUP_PAGE_CONNECT), hwndParent, ::DialogProc, (LPARAM)this);
	return S_OK;
}

void setup_page_connect::UpdateUI(void)
{
	int count;

	if (!hwnd || !IsWindow(hwnd)) return;
	
	CheckDlgButton(hwnd, IDC_CHK_PORT80, (bPort80) ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemTextW(hwnd, IDC_EDT_SERVER, szProxy);

	HWND hwndCB = GetDlgItem(hwnd, IDC_CB_CONNECT);
	if (!hwndCB || !IsWindow(hwndCB)) return;

	count = (INT)SendMessageW(hwndCB, CB_GETCOUNT, 0, 0L);
	if (count > 0)
	{
		int index;
		for (index = 0; index < count && inetMode != (INT)SendMessageW(hwndCB, CB_GETITEMDATA, index, 0L); index++);
		if (index == count) 
		{
			inetMode = (INT)SendMessageW(hwndCB, CB_GETITEMDATA, 0, 0L);
			index = 0;
		}
		SendMessageW(hwndCB, CB_SETCURSEL, index, 0L);
		PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_CB_CONNECT, CBN_SELCHANGE), (LPARAM)hwndCB);
	}
}

void setup_page_connect::ComboBox_OnSelChange(HWND hwndCtrl)
{
	INT proxyList[] = { IDC_GRP_PROXY, IDC_LBL_SERVER, IDC_EDT_SERVER, IDC_CHK_PORT80, };
	INT index;

	index = (INT)SendMessageW(hwndCtrl, CB_GETCURSEL, 0, 0L);
	if (CB_ERR == index) return;

	inetMode =  (INT)SendMessageW(hwndCtrl, CB_GETITEMDATA, index, 0L);

	for (int i = 0; i < sizeof(proxyList)/sizeof(INT); i++)
	{
		HWND hwndItem = GetDlgItem(hwnd, proxyList[i]);
		if (IsWindow(hwndItem)) ShowWindow(hwndItem, (2 != inetMode) ? SW_SHOWNA : SW_HIDE);
	}
}

INT_PTR setup_page_connect::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	UINT connID[] = { IDS_INST_INET1, IDS_INST_INET2, IDS_INST_INET3, };
	HWND hwndCB = GetDlgItem(hwnd, IDC_CB_CONNECT);

	CheckDlgButton(hwnd, IDC_CHK_PORT80, (bPort80) ? BST_CHECKED : BST_UNCHECKED);
	//SendDlgItemMessageW(hwnd, IDC_EDT_SERVER, EM_LIMITTEXT, sizeof(szProxy)/sizeof(wchar_t), 0);
	SetDlgItemTextW(hwnd, IDC_EDT_SERVER, szProxy);

	if (inetMode < 0) inetMode = 0;
	else if (inetMode >= sizeof(connID)/sizeof(INT)) inetMode = sizeof(connID)/sizeof(INT) - 1;

	for(int i = 0;  i < sizeof(connID)/sizeof(INT); i++) 
	{
		INT index = (INT)SendMessageW(hwndCB, CB_ADDSTRING,0, (LPARAM)getStringW(connID[i], NULL, 0));
		if (CB_ERR != index) SendMessageW(hwndCB, CB_SETITEMDATA, index, (LPARAM)i);
		if (inetMode == i) SendMessageW(hwndCB, CB_SETCURSEL, index, 0L);
	}

	PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_CB_CONNECT, CBN_SELCHANGE), (LPARAM)hwndCB);

	return 0;
}

void setup_page_connect::OnSize(UINT nType, INT cx, INT cy)
{
	INT ctrlList[] = { IDC_CB_CONNECT, IDC_GRP_PROXY, IDC_LBL_SERVER, IDC_EDT_SERVER, IDC_CHK_PORT80, };
	RECT rw;
	HDWP hdwp = BeginDeferWindowPos(sizeof(ctrlList)/sizeof(INT));
	if (!hdwp) return;

	for (int i = 0; i < sizeof(ctrlList)/sizeof(INT); i++)
	{
		HWND hwndCtrl = GetDlgItem(hwnd, ctrlList[i]);
		if (hwndCtrl)
		{
			GetWindowRect(hwndCtrl, &rw);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
			hdwp = DeferWindowPos(hdwp, hwndCtrl, NULL, max(0, (cx - (rw.right - rw.left))/2), rw.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
		}
	}
	if (hdwp) EndDeferWindowPos(hdwp);
}

void setup_page_connect::OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl)
{
	switch(nCtrlID)
	{		
		case IDC_CB_CONNECT:
			switch(nEvntID)
			{
				case CBN_SELCHANGE: ComboBox_OnSelChange(hwndCtrl); break;
			}
			break;
		case IDC_EDT_SERVER:
			switch(nEvntID)
			{
				case EN_CHANGE: 
					GetWindowTextW(hwndCtrl, szProxy, sizeof(szProxy)/sizeof(wchar_t));
					EnableWindow(GetDlgItem(hwnd, IDC_CHK_PORT80), 0x00 != *szProxy);
					if(0x00 == *szProxy)
					{
						CheckDlgButton(hwnd, IDC_CHK_PORT80, BST_UNCHECKED);
						PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_CHK_PORT80, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_CHK_PORT80));
					}
					break;
			}
			break;
		case IDC_CHK_PORT80:
			switch(nEvntID)
			{
				case BN_CLICKED: bPort80 = (BST_CHECKED & (INT)SendMessageW(hwndCtrl, BM_GETSTATE, 0, 0L)); break;
			}
			break;
	}
}

INT_PTR setup_page_connect::PageDlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	setup_page_connect *pInst = (setup_page_connect*)GetPropW(hwnd, L"SETUPPAGE");

	switch(uMsg)
	{
		case WM_INITDIALOG:
			pInst = (setup_page_connect*)lParam;
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

#define CBCLASS setup_page_connect
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