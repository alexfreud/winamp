#include "main.h"
#include "./import.h"
#include "./api__ml_online.h"
#include "./serviceHost.h"
#include "./serviceHelper.h"
#include "./navigation.h"
#include "./resource.h"
#include "./config.h"

#include <ifc_omstorage.h>
#include <ifc_omwebstorage.h>
#include <ifc_omstorageenum.h>
#include <ifc_omservice.h>
#include <ifc_omserviceenum.h>

#include <vector>

#include <strsafe.h>


typedef std::vector<WCHAR*> StringList;

static StringList recentList;
static BOOL recentListModified = FALSE;

typedef struct __OPENURLDLG
{
	HWND hOwner;
	LPCWSTR pszAddress;
	LPWSTR pszBuffer;
	UINT cchBufferMax;
} OPENURLDLG;

static INT_PTR ImportUrlDlg_Show(OPENURLDLG *poud);

static HRESULT ImportUrl_GetEnumerator(ifc_omstorageenumerator **enumerator)
{
	if (NULL == OMSERVICEMNGR) return E_UNEXPECTED;
	return OMSERVICEMNGR->EnumStorage(&STID_OmWebStorage, ifc_omstorage::capPublic | ifc_omstorage::capLoad, enumerator);
}

HRESULT ImportService_GetUrlSupported()
{
	if (NULL == OMSERVICEMNGR) 
		return E_UNEXPECTED;

	ifc_omstorageenumerator *enumerator;
	HRESULT hr = ImportUrl_GetEnumerator(&enumerator);
	
	if (SUCCEEDED(hr))
	{		
		ifc_omstorage *storage;
		if(S_OK == enumerator->Next(1, &storage, NULL))
		{
			storage->Release();
			hr = S_OK;
		}
		else
		{
			hr = S_FALSE;
		}

		enumerator->Release();
	}
	return hr;
}

HRESULT ImportUrl_LoadAddress(HWND hOwner, LPCWSTR pszAddress, ifc_omstorageenumerator *enumerator, 
							  ServiceHost *serviceHost, ifc_omstorage *serviceStorage, Navigation *navigation)
{
	HRESULT hr = S_OK;
	ULONG loaded(0);

	ifc_omstorage *storage;
	enumerator->Reset();

	while(S_OK == enumerator->Next(1, &storage, NULL))
	{
		ifc_omstorageasync *async;
		hr = storage->BeginLoad(pszAddress, serviceHost, NULL, NULL, &async);
		if(SUCCEEDED(hr))
		{
			ifc_omserviceenum *serviceEnum;
			hr = storage->EndLoad(async, &serviceEnum);
			async->Release();

			if (SUCCEEDED(hr))
			{
				ifc_omservice *service;
				while(S_OK == serviceEnum->Next(1, &service, NULL))
				{
					loaded++;
					if (SUCCEEDED(service->SetAddress(NULL)))
					{
						ULONG savedOk;
						if (SUCCEEDED(serviceStorage->Save(&service, 1, ifc_omstorage::saveClearModified, &savedOk)))
						{
							navigation->CreateItem(service, 1);
						}
					}
					service->Release();
				}
				serviceEnum->Release();
			}
			break;
		}
		else if (OMSTORAGE_E_UNKNOWN_FORMAT != hr)
		{
			break;
		}
		storage->Release();
	}

	return hr;
}

HRESULT ImportService_FromUrl(HWND hOwner)
{
	OPENURLDLG dlg = {0};
	WCHAR szBuffer[4096] = {0};
	
	dlg.hOwner = hOwner;
	dlg.pszAddress = NULL;
	dlg.pszBuffer = szBuffer;
	dlg.cchBufferMax = ARRAYSIZE(szBuffer);
	if (IDOK != ImportUrlDlg_Show(&dlg))
		return S_FALSE;

	ifc_omstorageenumerator *enumerator;
	HRESULT hr = ImportUrl_GetEnumerator(&enumerator);
	if (SUCCEEDED(hr))
	{
		Navigation *navigation;
		hr = Plugin_GetNavigation(&navigation);
		if (SUCCEEDED(hr))
		{
			ifc_omstorage *serviceStorage;
			hr = ServiceHelper_QueryStorage(&serviceStorage);
			if (SUCCEEDED(hr))
			{
				ServiceHost *serviceHost;
				hr = ServiceHost::GetCachedInstance(&serviceHost);
				if (SUCCEEDED(hr))
				{
					hr = ImportUrl_LoadAddress(hOwner, szBuffer, enumerator, serviceHost, serviceStorage, navigation);
					serviceHost->Release();	
				}
				serviceStorage->Release();
			}
			navigation->Release();
		}
		enumerator->Release();
	}
	return hr;
}

static INT_PTR ImportUrlDlg_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{
	OPENURLDLG *poud = (OPENURLDLG*)param;
	if (NULL != poud)
	{
		SetProp(hwnd, L"OPENURLDLG", poud);

		HWND hAddress = GetDlgItem(hwnd, IDC_ADDRESS);
		if (NULL != hAddress)
		{
			LPWSTR p;

			size_t count = recentList.size();
			if (0 == count)
			{
				char szKey[32] = {0}, szBuffer[4096] = {0};
				for(int i = 1; i < 101; i++)
				{
					if (FAILED(StringCchPrintfA(szKey, ARRAYSIZE(szKey), "entry%d", i)) ||
						0 == Config_ReadStr("RecentUrl", szKey, NULL, szBuffer, ARRAYSIZE(szBuffer)) ||
						'\0' == szBuffer[0])
					{
						break;
					}

					p = Plugin_MultiByteToWideChar(CP_UTF8, 0, szBuffer, -1);
					if (NULL != p) recentList.push_back(p);
				}
				count = recentList.size();
			}
			
			for (size_t i = 0; i < count; i ++)
			{
				p = recentList[i];
				if(NULL != p && L'\0' != *p)
					SendMessage(hAddress, CB_ADDSTRING, 0, (LPARAM)p);
			}

			if (NULL != poud->pszAddress)
				SetWindowText(hAddress, poud->pszAddress);
		}

		RECT ownerRect;
		if (NULL != poud->hOwner && IsWindowVisible(poud->hOwner) && 
			GetWindowRect(poud->hOwner, &ownerRect))
		{
			RECT myRect;
			GetWindowRect(hwnd, &myRect);
			LONG x = ownerRect.left + ((ownerRect.right - ownerRect.left) - (myRect.right - myRect.left))/2;
			LONG y = ownerRect.top + ((ownerRect.bottom - ownerRect.top) - (myRect.bottom - myRect.top))/2;
			SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	return 0;
}

static void ImportUrlDlg_OnDestroy(HWND hwnd)
{
	RemoveProp(hwnd, L"OPENURLDLG");

}
static INT_PTR ImportUrlDlg_ReturnAddress(HWND hwnd)
{
	OPENURLDLG *poud = (OPENURLDLG*)GetProp(hwnd, L"OPENURLDLG");
	if (NULL == poud) return -1;
	
	HWND hAddress = GetDlgItem(hwnd, IDC_ADDRESS);
	if (NULL == hAddress) return -2;
		
	if (0 == GetWindowText(hAddress, poud->pszBuffer, poud->cchBufferMax))
		return -3;

	if (NULL != poud->pszBuffer && L'\0' != *poud->pszBuffer)
	{
		LPWSTR p;
		size_t index = recentList.size();
		while(index--)
		{
			p = recentList[index];
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, poud->pszBuffer, -1, p, -1))
			{
				Plugin_FreeString(p);
				recentList.erase(recentList.begin() + index);
			}
		}

		p = Plugin_CopyString(poud->pszBuffer);
		if (NULL != p)
		{
			recentList.insert(recentList.begin(), p);
			recentListModified = TRUE;
		}
	}
	return IDOK;
}

static void ImportUrlDlg_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{
		case IDOK:
			EndDialog(hwnd, ImportUrlDlg_ReturnAddress(hwnd));
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
	}

}
static INT_PTR WINAPI ImportUrlDlg_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:	return ImportUrlDlg_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		ImportUrlDlg_OnDestroy(hwnd); break;
		case WM_COMMAND:		ImportUrlDlg_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
	}

	return 0;

}


static INT_PTR ImportUrlDlg_Show(OPENURLDLG *poud)
{
	if (NULL == poud || NULL == poud->pszBuffer) 
		return -1;
	
	HWND hParent = poud->hOwner;
	if (NULL == poud->hOwner)
		hParent = Plugin_GetLibrary();

	return WASABI_API_DIALOGBOXPARAMW(IDD_OPENURL, hParent, ImportUrlDlg_DialogProc, (LPARAM)poud);

}

void ImportService_SaveRecentUrl()
{
	if (FALSE == recentListModified)
		return;

	Config_WriteSection("RecentUrl", NULL);
	
	size_t count = recentList.size();
	if (count > 100) count = 100;

	char szKey[32], szBuffer[4096];
	UINT entry = 1;

	for (size_t i = 0; i < count; i++)
	{
		LPCWSTR p = recentList[i];
		if (NULL != p && L'\0' != *p && 
			0 != WideCharToMultiByte(CP_UTF8, 0, p, -1, szBuffer, ARRAYSIZE(szBuffer), NULL, NULL) &&
			SUCCEEDED(StringCchPrintfA(szKey, ARRAYSIZE(szKey), "entry%d", entry)) &&
			SUCCEEDED(Config_WriteStr("RecentUrl", szKey, szBuffer)))
		{
			entry++;
		}
	}

	recentListModified = FALSE;

}