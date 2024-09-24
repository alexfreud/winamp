#include "../common.h"
#include "./setupServicePanel.h"
#include "./setupDetails.h"
#include "./setupPage.h"
#include "../resource.h"
#include "../api__ml_online.h"

#include <ifc_omservice.h>
#include <ifc_omservicedetails.h>
#include <ifc_omcachemanager.h>
#include <ifc_omcachegroup.h>
#include <ifc_omcacherecord.h>
#include <ifc_imageloader.h>
#include <ifc_omgraphics.h>
#include <ifc_omserviceeventmngr.h>
#include <ifc_omserviceeditor.h>

#include <shlwapi.h>
#include <strsafe.h>

#define GetPanel(__hwnd) ((ServicePanel*)GetPropW((__hwnd), MAKEINTATOM(DETAILS_PROP)))

#define GET_IDETAILS(__service, __details)\
	(NULL != (service) && SUCCEEDED((service)->QueryInterface(IFC_OmServiceDetails, (void**)&(__details))))


ServicePanel::ServicePanel(LPCWSTR pszName, ifc_omservice *service) 
	: ref(1), name(NULL), service(NULL), hwnd(NULL), fontTitle(NULL), fontMeta(NULL), thumbnailCache(NULL)
{
	name = Plugin_CopyString(pszName);
	this->service = service;
	if (NULL != service)
		service->AddRef();
}

ServicePanel::~ServicePanel()
{
	Plugin_FreeString(name);

	if (NULL != service)
		service->Release();

	if (NULL != fontTitle)
		DeleteObject(fontTitle);

	if (NULL != fontMeta)
		DeleteObject(fontMeta);

	if (NULL != thumbnailCache)
		thumbnailCache->Release();
}

HWND ServicePanel::CreateInstance(HWND hParent, LPCWSTR pszName, ifc_omservice *service, ServicePanel **instance)
{
	ServicePanel *panel = new ServicePanel(pszName, service);
	if (NULL == panel)
	{
		if (NULL != instance) *instance = NULL;
		return NULL;
	}

	HWND hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_SETUP_SERVICEDETAILS, hParent, ServicePanel_DialogProc, (LPARAM)panel);
	
	if (NULL != instance)
	{
		if (NULL != hwnd)
		{
			*instance = panel;
			panel->AddRef();
		}
		else
			*instance = NULL;
	}
	
	panel->Release();
	return hwnd;
}

size_t ServicePanel::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ServicePanel::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int ServicePanel::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmServiceEvent))
		*object = static_cast<ifc_omserviceevent*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

static void CALLBACK ThreadCallback_ServiceChange(Dispatchable *instance, ULONG_PTR param1, ULONG_PTR param2)
{
	ifc_omserviceevent *panel = (ifc_omserviceevent*)instance;
	ifc_omservice *service = (ifc_omservice*)param1;
	if (NULL != service)
	{
		if (NULL != panel)
			panel->ServiceChange(service, (UINT)param2);
		service->Release();
	}
}
void ServicePanel::ServiceChange(ifc_omservice *service, unsigned int modifiedFlags)
{

	DWORD currentTID = GetCurrentThreadId();
	DWORD windowTID = GetWindowThreadProcessId(hwnd, NULL);
	if (NULL != windowTID && currentTID != windowTID)
	{
		if(NULL != OMUTILITY)
		{
			service->AddRef();
			if (FAILED(OMUTILITY->PostMainThreadCallback2(ThreadCallback_ServiceChange, (ifc_omserviceevent*)this, (ULONG_PTR)service, (ULONG_PTR)modifiedFlags)))
				service->Release();
		}
		return;
	}


	if ( 0 != (ifc_omserviceeditor::modifiedName & modifiedFlags))
	{
		UpdateName();
		HWND hPage = GetParent(hwnd);
		if (NULL != hPage)
			PostMessage(hPage, SPM_UPDATELIST, (WPARAM)service->GetId(), NULL);
	}
	
	if ( 0 != (ifc_omserviceeditor::modifiedDescription & modifiedFlags))
		UpdateDescription();

	if ( 0 != (ifc_omserviceeditor::modifiedThumbnail& modifiedFlags))
		UpdateThumbnail();

	if ( 0 != ((ifc_omserviceeditor::modifiedAuthorFirst | 
				ifc_omserviceeditor::modifiedAuthorLast | 
				ifc_omserviceeditor::modifiedUpdated | 
				ifc_omserviceeditor::modifiedPublished) & modifiedFlags))
	{
		UpdateMeta();
	}

}

HRESULT ServicePanel::LoadLocalThumbnail(LPCWSTR pszPath)
{
	HWND hThumbnail = GetDlgItem(hwnd, IDC_THUMBNAIL);
	if (NULL == hThumbnail) return E_UNEXPECTED;

	SendMessage(hThumbnail, WM_SETREDRAW, FALSE, 0L);

	HBITMAP hBitmap = NULL;

	BITMAPINFOHEADER header;
	void *pixelData;

	ifc_omimageloader *imageLoader;
	if (SUCCEEDED(OMUTILITY->QueryImageLoader(NULL, pszPath, FALSE, &imageLoader)))
	{
		imageLoader->LoadBitmapEx(&hBitmap, &header, &pixelData);
		imageLoader->Release();
	}
		
	if (NULL == hBitmap && 
		SUCCEEDED(OMUTILITY->QueryImageLoader(WASABI_API_ORIG_HINST, MAKEINTRESOURCE(IDR_SERVICE64X64_IMAGE), FALSE, &imageLoader)))
	{
		imageLoader->LoadBitmapEx(&hBitmap, &header, &pixelData);
		imageLoader->Release();
	}
	
	HBITMAP hTest = (HBITMAP)SendMessage(hThumbnail, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
	if (NULL != hTest)
		DeleteObject(hTest);

	if (NULL != hBitmap)
	{
		hTest = (HBITMAP)SendMessage(hThumbnail, STM_GETIMAGE, IMAGE_BITMAP, 0L);
		if (hTest != hBitmap)
		{	// this is XP and up image copy was created and alpha channel will be handled properly
			DeleteObject(hBitmap);
		}
		else
		{ // fix alpha channel
			if (32 == header.biBitCount)
			{
				HDC hdcFixed = CreateCompatibleDC(NULL);
				if (NULL != hdcFixed)
				{
					BITMAPINFOHEADER headerFixed;
					CopyMemory(&headerFixed, &header, sizeof(BITMAPINFOHEADER));
					BYTE *pixelsFixed;
					INT cx = header.biWidth;
					INT cy = abs(header.biHeight);
					HBITMAP bitmapFixed = CreateDIBSection(NULL, (LPBITMAPINFO)&headerFixed, DIB_RGB_COLORS, (void**)&pixelsFixed, NULL, 0);
					
					if (NULL != bitmapFixed)
					{
						HBITMAP bitmapOrig = (HBITMAP)SelectObject(hdcFixed, bitmapFixed);
						HBRUSH hb = (HBRUSH)SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)hdcFixed, (LPARAM)hwnd);
						if (NULL == hb) 
							hb = GetSysColorBrush(COLOR_3DFACE);
						RECT rect;
						SetRect(&rect, 0, 0, cx, cy);
						FillRect(hdcFixed, &rect, hb);
							
						ifc_omgraphics *graphics;
						if (SUCCEEDED(OMUTILITY->GetGraphics(&graphics)))
						{
							HDC hdcSrc = CreateCompatibleDC(NULL);
							if (NULL != hdcSrc)
							{
								HBITMAP bitmapSrcOrig = (HBITMAP)SelectObject(hdcSrc, hBitmap);
								BLENDFUNCTION bf;
								bf.BlendOp = AC_SRC_OVER;
								bf.BlendFlags = 0;
								bf.SourceConstantAlpha = 0xFF;
								bf.AlphaFormat = AC_SRC_ALPHA;

								RECT blendRect;
								SetRect(&blendRect, 0, 0, cx, cy);

								graphics->Premultiply((BYTE*)pixelData, cx, cy);
								graphics->AlphaBlend(hdcFixed, &blendRect, hdcSrc, &blendRect, bf);

								SelectObject(hdcSrc, bitmapSrcOrig);
								DeleteDC(hdcSrc);
							}
							graphics->Release();
						}

						SelectObject(hdcFixed, bitmapOrig);
						SendMessage(hThumbnail, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmapFixed);
						DeleteObject(hBitmap);
							
					}
					DeleteDC(hdcFixed);
				}
			}
		}
	}

	RECT clientRect;
	if (GetClientRect(hThumbnail, &clientRect))
	{
		INT cx = clientRect.right - clientRect.left;
		INT cy = clientRect.bottom - clientRect.top;
		if (64 != cx || 64 != cy)
		{
			SetWindowPos(hThumbnail, NULL, 0, 0, 64, 64, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}
	
	SendMessage(hThumbnail, WM_SETREDRAW, TRUE, 0L);

	if (0 != ShowWindow(hThumbnail,  (NULL != hBitmap) ? SW_SHOWNA : SW_HIDE))
		InvalidateRect(hThumbnail, NULL, TRUE);

	return S_OK;
}

static void CALLBACK ThreadCallback_PathChanged(Dispatchable *instance, ULONG_PTR param1, ULONG_PTR param2)
{
	ifc_omcachecallback *panel = (ifc_omcachecallback*)instance;
	ifc_omcacherecord *record = (ifc_omcacherecord*)param1;
	if (NULL != record)
	{
		if (NULL != panel)
			panel->PathChanged(record);
		record->Release();
	}
}

void ServicePanel::PathChanged(ifc_omcacherecord *record)
{
	if (NULL == hwnd || FALSE == IsWindow(hwnd))
		return;

	DWORD currentTID = GetCurrentThreadId();
	DWORD windowTID = GetWindowThreadProcessId(hwnd, NULL);
	if (NULL != windowTID && currentTID != windowTID)
	{
		if(NULL != OMUTILITY)
		{
			record->AddRef();
			if (FAILED(OMUTILITY->PostMainThreadCallback2(ThreadCallback_PathChanged, (ifc_omcachecallback*)this, (ULONG_PTR)record, 0L)))
				record->Release();
		}
		return;
	}

	WCHAR szPath[2048] = {0};
	if (FAILED(record->GetPath(szPath, ARRAYSIZE(szPath))))
		szPath[0] = L'\0';

	LoadLocalThumbnail(szPath);
}

void ServicePanel::Attach(HWND hwnd)
{
	this->hwnd = hwnd;
	if (NULL != hwnd && 
		FALSE != SetProp(hwnd, MAKEINTATOM(DETAILS_PROP), this))
	{
		AddRef();
	}
}

void ServicePanel::Detach()
{
	RemoveProp(hwnd, MAKEINTATOM(DETAILS_PROP));

	if (NULL != thumbnailCache)
	{
		thumbnailCache->UnregisterCallback(this);
		thumbnailCache->Release();
		thumbnailCache = NULL;
	}

	if (NULL != service)
	{
		ifc_omserviceeventmngr *eventManager;
		if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEventMngr, (void**)&eventManager)))
		{
			eventManager->UnregisterHandler(this);
			eventManager->Release();
		}
	}

	Release();
}

HFONT ServicePanel::PickTitleFont(LPCWSTR pszTitle, INT cchTitle, INT maxWidth)
{
	HFONT dialogFont =  (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	
	LOGFONT lf;
	if (0 == GetObject(dialogFont, sizeof(LOGFONT), &lf))
		return NULL;

	HFONT titleFont = NULL;
	if (cchTitle > 0)
	{		
		LOGFONT lf;
		if (0 != GetObject(dialogFont, sizeof(LOGFONT), &lf))
		{
			StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"Arial Bold");
			lf.lfWidth = 0;
			lf.lfWeight = FW_DONTCARE;
			lf.lfQuality = 5/*ANTIALIASED_QUALITY*/;
			
			HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
			if (NULL != hdc)
			{
				HFONT origFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
				SIZE textSize;

				INT heightLimit = (lf.lfHeight < 0) ? 1 : -1;
				lf.lfHeight += (lf.lfHeight < 0) ? -2 : +2;
				do
				{		
					textSize.cx = 0;
					if (NULL != titleFont) DeleteObject(titleFont);
					titleFont = CreateFontIndirect(&lf);
					if (NULL != titleFont)
					{
						SelectObject(hdc, titleFont);
						GetTextExtentPoint32(hdc, pszTitle, cchTitle, &textSize);
					}
					lf.lfHeight += (lf.lfHeight < 0) ? 1 : -1;

				} while(textSize.cx > maxWidth && lf.lfHeight != heightLimit);
				
				if (0 == textSize.cx)
				{
					DeleteObject(titleFont);
					titleFont = NULL;
				}

				SelectObject(hdc, origFont);
				ReleaseDC(hwnd, hdc);
			}
		}
	}

	if (NULL == titleFont && 
		0 != GetObject(dialogFont, sizeof(LOGFONT), &lf))
	{
		titleFont = CreateFontIndirect(&lf);	
	}
	return titleFont;
}

LPCWSTR ServicePanel::FormatDate(LPCWSTR pszDate, LPWSTR pszBuffer, INT cchBufferMax)
{
	SYSTEMTIME st;
	ZeroMemory(&st, sizeof(SYSTEMTIME));
	LPCWSTR cursor;
	
	cursor = pszDate;
	INT index = 0;

	for(;;)
	{
		
		INT iVal;

		if (FALSE == StrToIntEx(cursor, STIF_DEFAULT, &iVal) || iVal < 1)
		{
			index = 0;
			break;
		}

		if (0 == index) 
		{
			if (iVal < 2000 || iVal > 2100)
				break;
			st.wYear = iVal;
			index++;
		}
		else if (1 == index) 
		{
			if (iVal < 1 || iVal > 12)
				break;
			st.wMonth = iVal;
			index++;
		}
		else if (2 == index) 
		{
			if (iVal < 1 || iVal > 31)
				break;
			st.wDay = iVal;
			index++;
		}
		else
		{
			index = 0;
			break;
		}
		
		while(L'\0' != *cursor && L'-' != *cursor) cursor++;
		if (L'-' == *cursor) cursor++;
		if (L'\0' == *cursor)  
			break;
		
	}

	if (3 == index && 
		0 != GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pszBuffer, cchBufferMax))
	{
		return pszBuffer;	
	}

	return pszDate;
}



HRESULT ServicePanel::GetFullName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;
	*pszBuffer = L'\0';
	
	if (NULL == service) return E_UNEXPECTED;

	ifc_omservicedetails *details;
	HRESULT hr = service->QueryInterface(IFC_OmServiceDetails, (void**)&details);
    if (SUCCEEDED(hr))
	{
		hr = details->GetAuthorFirst(pszBuffer, cchBufferMax);
		if (SUCCEEDED(hr))
		{
			UINT cchBuffer = lstrlen(pszBuffer);
			LPWSTR cursor = pszBuffer + cchBuffer;
			size_t remaining = cchBufferMax - cchBuffer;

			if (cursor != pszBuffer)
			{
				hr = StringCchCopyEx(cursor, remaining, L" ", &cursor, &remaining, 0);
				if (SUCCEEDED(hr))
				{
					hr = details->GetAuthorLast(cursor, (UINT)remaining);
					if (FAILED(hr) || L'\0' == *cursor)
					{
						pszBuffer[cchBuffer] = L'\0';
					}
				}
			}
		}
	}
	return hr;
}
void ServicePanel::UpdateName()
{
	HWND hTitle = GetDlgItem(hwnd, IDC_TITLE);
	if (NULL == hTitle) return;

	WCHAR szBuffer[128] = {0};
	if (NULL == service || 
		FAILED(service->GetName(szBuffer, ARRAYSIZE(szBuffer))))
	{
		szBuffer[0] = L'\0';
	}
	
	INT cchBuffer = lstrlen(szBuffer);
	RECT rc;
	GetClientRect(hTitle, &rc);
	HFONT font = PickTitleFont(szBuffer, cchBuffer, rc.right - rc.left);
	if (NULL != font)
	{
		if (NULL != fontTitle) 
			DeleteObject(fontTitle);
		
		fontTitle = font;
		SendMessage(hTitle, WM_SETFONT, (WPARAM)fontTitle, 0L);
	}


	SetWindowText(hTitle, szBuffer);
	InvalidateRect(hTitle, NULL, TRUE);
}


void ServicePanel::UpdateDescription()
{
	HWND hDescription = GetDlgItem(hwnd, IDC_DESCRIPTION);
	if (NULL == hDescription) return;

	WCHAR szBuffer[4096] = {0};
	ifc_omservicedetails *details = 0;
	if (GET_IDETAILS(service, details))
	{
		details->GetDescription(szBuffer, ARRAYSIZE(szBuffer));
		details->Release();
	}

	SetupDetails_SetDescription(hDescription, szBuffer);
}

void ServicePanel::UpdateMeta()
{
	HWND hMeta = GetDlgItem(hwnd, IDC_SERVICEMETA);
	if (NULL == hMeta) return;

	WCHAR szBuffer[512] = {0};
	ifc_omservicedetails *svcdetails = 0;
	if (GET_IDETAILS(service, svcdetails))
	{
		WCHAR szValue[256] = {0}, szPrefix[64] = {0};
		HRESULT hr = S_OK;
		LPWSTR cursor = szBuffer;
		size_t remaining = ARRAYSIZE(szBuffer);

		if (SUCCEEDED(GetFullName(szValue, ARRAYSIZE(szValue))) && L'\0' != szValue[0])
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_SERVICE_BYAUTHOR, szPrefix, ARRAYSIZE(szPrefix));
			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
					L"%s%s", szPrefix, szValue);
		}

		if (SUCCEEDED(svcdetails->GetUpdated(szValue, ARRAYSIZE(szValue))) && L'\0' != szValue[0])
		{
			if (cursor != szBuffer)
				hr = StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

			if (SUCCEEDED(hr))
			{
				WCHAR szDate[128] = {0};
				WASABI_API_LNGSTRINGW_BUF(IDS_SERVICE_LASTUPDATED, szPrefix, ARRAYSIZE(szPrefix));
				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
					L"%s%s", szPrefix, FormatDate(szValue, szDate, ARRAYSIZE(szDate)));
			}
		}

		svcdetails->Release();
	}

	if (NULL == fontMeta)
	{
		HFONT dialogFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
		LOGFONT lf;
		if (0 != GetObject(dialogFont, sizeof(LOGFONT), &lf))
		{
			StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"Tahoma");
			lf.lfWidth = 0;
			lf.lfHeight += (lf.lfHeight < 0) ? 1 : -1;
			lf.lfQuality = ANTIALIASED_QUALITY;
			fontMeta = CreateFontIndirect(&lf);
		}

		if (NULL != fontMeta)
		{
			SendMessage(hMeta, WM_SETFONT, (WPARAM)fontMeta, 0L);
		}
	}

	SetWindowText(hMeta, szBuffer);
	if (0 != ShowWindow(hMeta,  (L'\0' != szBuffer[0]) ? SW_SHOWNA : SW_HIDE))
		InvalidateRect(hMeta, NULL, TRUE);
}

void ServicePanel::UpdateThumbnail()
{
	if (NULL != thumbnailCache)
	{
		thumbnailCache->UnregisterCallback(this);
		thumbnailCache->Release();
		thumbnailCache = NULL;
	}
	LoadLocalThumbnail(NULL);

	ifc_omservicedetails *details = 0;
	if (GET_IDETAILS(service, details))
	{
		WCHAR szPath[2048] = {0};
		if (SUCCEEDED(details->GetThumbnail(szPath, ARRAYSIZE(szPath))) && L'\0' != szPath[0])
		{
			ifc_omcachemanager *cacheManager;
			if (SUCCEEDED(OMUTILITY->GetCacheManager(&cacheManager)))
			{
				ifc_omcachegroup *cacheGroup;
				if (SUCCEEDED(cacheManager->Find(L"thumbnails", TRUE, &cacheGroup, NULL)))
				{
					
					if (SUCCEEDED(cacheGroup->Find(szPath, TRUE, &thumbnailCache, FALSE)))
					{
						thumbnailCache->RegisterCallback(this);
						if (SUCCEEDED(thumbnailCache->GetPath(szPath, ARRAYSIZE(szPath))))
							LoadLocalThumbnail(szPath);
					}
					cacheGroup->Release();
				}
				cacheManager->Release();
			}
		}
		details->Release();
	}
}




INT_PTR ServicePanel::OnInitDialog(HWND hFocus, LPARAM lParam)
{
	UpdateName();
	UpdateDescription();
	UpdateThumbnail();
	UpdateMeta();

	if (NULL != service)
	{
		ifc_omserviceeventmngr *eventManager;
		if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEventMngr, (void**)&eventManager)))
		{
			eventManager->RegisterHandler(this);
			eventManager->Release();
		}
	}

	return FALSE;
}

void ServicePanel::OnDestroy()
{
	
	HWND hThumbnail = GetDlgItem(hwnd, IDC_THUMBNAIL);
	if (NULL != hThumbnail)
	{
		HBITMAP hBitmap = (HBITMAP)SendMessage(hThumbnail, STM_SETIMAGE, IMAGE_BITMAP, 0L);
		if (NULL != hBitmap) 
			DeleteObject(hBitmap);
	}

	Detach();
}


INT_PTR ServicePanel::OnDialogColor(HDC hdc, HWND hControl)
{
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent && hParent != hwnd)
		return (INT_PTR)SendMessage(hParent, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hControl);

	return 0;
}


INT_PTR ServicePanel::OnStaticColor(HDC hdc, HWND hControl)
{
	INT_PTR result = 0;
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent && hParent != hwnd)
		result = (INT_PTR)SendMessage(hParent, WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)hControl);
	
	INT controlId = GetDlgCtrlID(hControl);
	switch(controlId)
	{
		case IDC_SERVICEMETA:
			{
				COLORREF rgbBk = GetBkColor(hdc);
				COLORREF rgbFg = GetTextColor(hdc);
								
				ifc_omgraphics *graphics;
				if (SUCCEEDED(OMUTILITY->GetGraphics(&graphics)))
				{
					graphics->BlendColor(rgbFg, rgbBk, 180, &rgbFg);
					graphics->Release();
				}

				SetTextColor(hdc, rgbFg);
			}
			break;
	}
	return result;
}


INT_PTR ServicePanel::OnGetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return FALSE;
	return SUCCEEDED(StringCchCopy(pszBuffer, cchBufferMax, (NULL != name) ? name : L""));
}

INT_PTR ServicePanel::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:			OnDestroy(); break;
		case WM_CTLCOLORDLG:		return OnDialogColor((HDC)wParam, (HWND)lParam);
		case WM_CTLCOLORSTATIC: return OnStaticColor((HDC)wParam, (HWND)lParam);

		case NSDM_GETUNIQUENAME: MSGRESULT(hwnd, OnGetUniqueName((LPWSTR)lParam, (UINT)wParam));
	}
	return 0;
}

static INT_PTR WINAPI ServicePanel_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ServicePanel *panel = GetPanel(hwnd);
	if (NULL == panel)
	{
		if (WM_INITDIALOG == uMsg) 
		{
			panel = (ServicePanel*)lParam;
			if (NULL != panel) 
				panel->Attach(hwnd);
		}
		
		if (NULL == panel) return 0;
	}

	return panel->DialogProc(uMsg, wParam, lParam);
}



#define CBCLASS ServicePanel
START_MULTIPATCH;
 START_PATCH(MPIID_SERVICEEVENT)
  M_CB(MPIID_SERVICEEVENT, ifc_omserviceevent, ADDREF, AddRef);
  M_CB(MPIID_SERVICEEVENT, ifc_omserviceevent, RELEASE, Release);
  M_CB(MPIID_SERVICEEVENT, ifc_omserviceevent, QUERYINTERFACE, QueryInterface);
  M_VCB(MPIID_SERVICEEVENT, ifc_omserviceevent, API_SERVICECHANGE, ServiceChange);
 
 
 NEXT_PATCH(MPIID_CACHECALLBACK)
  M_CB(MPIID_CACHECALLBACK, ifc_omcachecallback, ADDREF, AddRef);
  M_CB(MPIID_CACHECALLBACK, ifc_omcachecallback, RELEASE, Release);
  M_CB(MPIID_CACHECALLBACK, ifc_omcachecallback, QUERYINTERFACE, QueryInterface);
  M_VCB(MPIID_CACHECALLBACK, ifc_omcachecallback, API_PATHCHANGED, PathChanged);

 END_PATCH
END_MULTIPATCH;
#undef CBCLASS
