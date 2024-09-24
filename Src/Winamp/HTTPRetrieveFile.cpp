/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/

#include "Main.h"
#include "api.h"
#include "..\Components\wac_network\wac_network_http_receiver_api.h"

#include "api/service/waServiceFactory.h"


void GetMIMEType(const char *url, char *mimeType, int mimeTypeCch)
{
	api_httpreceiver *http = 0;
	waServiceFactory *sf = 0;
	if (WASABI_API_SVC)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
		if (sf)
			http = (api_httpreceiver *)sf->getInterface();
	}

	if (!http)
		return ;

	int ret;
	http->open(API_DNS_AUTODNS, 2048, config_proxy);
	http->connect(url, 0, "HEAD");

	do
	{
		Sleep(10);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int replycode = http->getreplycode();
		switch (replycode)
		{
		case 0:
		case 100:
			break;
		case 200:
			{
				const char *contentType = http->getheader("Content-Type");
				if (contentType)
					lstrcpynA(mimeType, contentType, mimeTypeCch);
				else
					mimeType[0] = 0;

				sf->releaseInterface(http);
				return ;
			}
			break;
		default:
			sf->releaseInterface(http);
			return ;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);

	sf->releaseInterface(http);
}

#if 0
#define WM_HRF_READINGHTTP WM_USER
#define WM_HRF_DOWNLOADING (WM_USER+1)
struct RFData
{
public:
	char *url, *file;
	HWND hwnd;
};

bool killswitch;
static DWORD WINAPI rf_ThreadProc(void *p)
{
	RFData *rfData = (RFData *)p;
	char *url = rfData->url, *file = rfData->file;
	waServiceFactory *sf = 0;
	api_httpreceiver *http = 0;
	if (WASABI_API_SVC)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
		if (sf)
			http = (api_httpreceiver *)sf->getInterface();
	}

	if (!http)
		return 1;

	HANDLE hFile = CreateFile(file, GENERIC_WRITE, 0,
	                          NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		sf->releaseInterface(http);
		return 1;
	}

	http->Open(API_DNS_AUTODNS, 16384);
	http->AddHeader("User-Agent: Winamp/" APP_VERSION);
	http->Connect(url);
	int ret;
	do
	{
		Sleep(50);
		http->Run();
		ret = http->GetStatus();
		if (ret == HTTPRECEIVER_STATUS_ERROR)
			killswitch = true;

		if (killswitch)
			break;
	}
	while (ret == HTTPRECEIVER_STATUS_CONNECTING);

	if (ret == HTTPRECEIVER_STATUS_READING_HEADERS || ret == HTTPRECEIVER_STATUS_READING_CONTENT)
	{
		PostMessageW(rfData->hwnd, WM_HRF_READINGHTTP, 0, 0);
		int replycode = http->GetReplyCode();
		switch (replycode)
		{
	case 0: case 100: // shouldn't really get here
			break;
		case 200:
			break;
		default:
			killswitch = true;
			break;
		}
	}
	bool error = false;
	char block[16384] = {0};
	size_t downloadSize;
	size_t currentSize = 0, totalSize = 0;

	do
	{
		if (killswitch)
		{
			error = true;
			break;
		}
		// ---- Pause a bit and then run the http downloader ----
		Sleep(50);
		ret = http->Run();

		if (ret == -1) // connection failed
		{
			error = true;
			break;
		}

		// ---- download ----
		downloadSize = http->GetBytesAvailable();
		if (downloadSize)
		{
			totalSize = http->GetContentLength();

			downloadSize = http->GetBytes(block, downloadSize);
			currentSize += downloadSize;
			PostMessageW(rfData->hwnd, WM_HRF_DOWNLOADING, currentSize, totalSize);

			// ---- write to disk ----
			if (downloadSize) // WriteFile doesn't like 0 byte writes
			{
				DWORD numWritten = 0;
				WriteFile(hFile, block, downloadSize, &numWritten, FALSE);
				if (numWritten != downloadSize) // make sure that the block was actually written
				{
					error = true;
					break;  // failed writing
				}
			}
		}
	}
	while (ret != 1 || downloadSize); // http may be closed, but make sure we get all the data.

	CloseHandle(hFile);
	if (error)
		DeleteFile(file);
	sf->releaseInterface(http);
	return 0;

}

static LRESULT CALLBACK rf_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_STATUS, getString(IDS_HTTP_INIT,NULL,0));
		return FALSE;
	case WM_HRF_READINGHTTP:
		SetDlgItemText(hwndDlg, IDC_STATUS, getString(IDS_HTTP_READ_REQUEST,NULL,0));
		return TRUE;
	case WM_HRF_DOWNLOADING:
		{
			char temp[128] = {0};
			if (!lParam)
				StringCchPrintf(temp, 128, getString(IDS_HTTP_RET_FILE,NULL,0), wParam);
			else
				StringCchPrintf(temp, 128, getString(IDS_HTTP_RET_FILE_PERCENT,NULL,0), (100*wParam) / lParam, wParam, lParam);
			SetDlgItemText(hwndDlg, IDC_STATUS, temp);
		}
		return TRUE;
	case WM_DESTROY:
		if (GetParent(hwndDlg) == hMainWindow)
		{
			if (hMainWindow) EnableWindow(hMainWindow, 1);
			if (hPLWindow) EnableWindow(hPLWindow, 1);
			if (hEQWindow) EnableWindow(hEQWindow, 1);
			//if (hMBWindow) EnableWindow(hMBWindow,1);
		}
		else
			EnableWindow(GetParent(hwndDlg), 1);
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return 0;
		}
		return 0;
	}
	return 0;
}

int httpRetrieveFile(HWND hwnd, char *url, char *file, char *dlgtitle)
{
	killswitch = false;
	RECT r;
	HANDLE hThread = 0;
	if (!hwnd) hwnd = hMainWindow;

	if (hwnd == hMainWindow && g_dialog_box_parent) hwnd = g_dialog_box_parent;

	GetWindowRect(hwnd, &r);
	HWND dlgWnd = LPCreateDialog(IDD_HTTPGET, hwnd, rf_DlgProc);
	SetWindowText(dlgWnd, dlgtitle);
	SetDlgItemText(dlgWnd, IDC_URL, url);

	RFData data = {url, file, dlgWnd};
	DWORD id;
	hThread = CreateThread(NULL, 0, rf_ThreadProc, (void *) & data, CREATE_SUSPENDED, &id);
	if (NULL)
		return 1;

	ResumeThread(hThread);
	if (r.bottom > GetSystemMetrics(SM_CXSCREEN) / 2 && r.bottom - r.top < 100)
	{
		RECT r2;
		GetWindowRect(dlgWnd, &r2);
		r.top = r.bottom - (r2.bottom - r2.top);
	}
	SetWindowPos(dlgWnd, NULL, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	if (GetForegroundWindow() == hwnd)
		ShowWindow(dlgWnd, SW_SHOW);
	else
		ShowWindow(dlgWnd, SW_SHOWNA);

	if (hwnd == hMainWindow)
	{
		if (hMainWindow) EnableWindow(hMainWindow, 0);
		if (hPLWindow) EnableWindow(hPLWindow, 0);
		if (hEQWindow) EnableWindow(hEQWindow, 0);
		//if (hMBWindow) EnableWindow(hMBWindow,0);
	}
	else
		EnableWindow(hwnd, 0);

	while (1)
	{
		MSG msg;
		if (!IsWindow(dlgWnd))
		{
			killswitch = true;
			break;
		}
		if (WaitForSingleObject(hThread, 0) == WAIT_OBJECT_0)
		{
			DestroyWindow(dlgWnd);
			break;
		}
		GetMessage(&msg, NULL, 0, 0);
		DispatchMessage(&msg);
	}

	WaitForSingleObject(hThread, 5000);
	DWORD exitCode;
	if (GetExitCodeThread(hThread, &exitCode))
		return exitCode;
	else
	{
		// CUT: TerminateThread(hThread, 0);
		return 1;
	}
}
#endif