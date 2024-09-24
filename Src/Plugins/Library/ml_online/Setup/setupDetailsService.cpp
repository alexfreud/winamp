#include "./setupDetails.h"
#include "../common.h"
#include "../resource.h"
#include "../wasabi.h"

#include <ifc_omservice.h>
#include <ifc_omservicedetails.h>
#include <ifc_omcachemanager.h>
#include <ifc_omserviceevent.h>
#include <ifc_omcachegroup.h>
#include <ifc_omcacherecord.h>
#include <ifc_imageloader.h>
#include <ifc_omgraphics.h>
#include <ifc_omserviceeventmngr.h>

#include <shlwapi.h>
#include <strsafe.h>



#define GetPanel(__hwnd) ((ServicePanel*)GetPropW((__hwnd), MAKEINTATOM(DETAILS_PROP)))

#define GET_IDETAILS(__service, __details)\
	(NULL != (service) && SUCCEEDED((service)->QueryInterface(IFC_OmServiceDetails, (void**)&(__details))))

static INT_PTR WINAPI ServicePanel_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



HWND OmSetupDetails_CreateServiceView(HWND hParent, ifc_omservice *service)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_SETUP_SERVICEDETAILS, hParent, ServicePanel_DialogProc, (LPARAM)service);
}

static HFONT ServicePanel_PickTitleFont(HWND hwnd, LPCWSTR pszTitle, INT cchTitle, INT maxWidth)
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

static void ServicePanel_SetServiceName(HWND hwnd, ifc_omservice *service)
{
	HWND hTitle = GetDlgItem(hwnd, IDC_TITLE);
	if (NULL == hTitle) return;

	WCHAR szBuffer[128];
	if (NULL == service || 
		FAILED(service->GetName(szBuffer, ARRAYSIZE(szBuffer))))
	{
		szBuffer[0] = L'\0';
	}

	
	
	SERVICEDETAILS *details = GetDetails(hwnd);
	if (NULL != details)
	{
		INT cchBuffer = lstrlen(szBuffer);
		RECT rc;
		GetClientRect(hTitle, &rc);
		HFONT titleFont = ServicePanel_PickTitleFont(hwnd, szBuffer, cchBuffer, rc.right - rc.left);
		if (NULL != titleFont)
		{
			if (NULL != details->fontTitle) DeleteObject(details->fontTitle);
			details->fontTitle = titleFont;
			SendMessage(hTitle, WM_SETFONT, (WPARAM)details->fontTitle, 0L);
		}
	}

	SetWindowText(hTitle, szBuffer);
	InvalidateRect(hTitle, NULL, TRUE);
}


static void ServicePanel_SetServiceDescription(HWND hwnd, ifc_omservice *service)
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

	OmSetupDetails_SetDescription(hDescription, szBuffer);
}

static LPCWSTR ServicePanel_FormatDate(LPCWSTR pszDate, LPWSTR pszBuffer, INT cchBufferMax)
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



static HRESULT ServicePanel_GetFullName(ifc_omservicedetails *details, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;
	
	*pszBuffer = L'\0';

	HRESULT hr = S_OK;
	if (NULL != details)
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

static void ServicePanel_SetServiceMeta(HWND hwnd, ifc_omservice *service)
{
	HWND hMeta = GetDlgItem(hwnd, IDC_SERVICEMETA);
	if (NULL == hMeta) return;

	WCHAR szBuffer[512] = {0};

	ifc_omservicedetails *svcdetails = 0;
	if (GET_IDETAILS(service, svcdetails))
	{
		HRESULT hr = S_OK;
		LPWSTR cursor = szBuffer;
		WCHAR szValue[256] = {0}, szPrefix[64] = {0};
		size_t remaining = ARRAYSIZE(szBuffer);

		if (SUCCEEDED(ServicePanel_GetFullName(svcdetails, szValue, ARRAYSIZE(szValue))) && L'\0' != szValue[0])
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
					L"%s%s", szPrefix, ServicePanel_FormatDate(szValue, szDate, ARRAYSIZE(szDate)));
			}
		}

		svcdetails->Release();

	}

	SERVICEDETAILS *details = GetDetails(hwnd);
	if (NULL != details && NULL == details->fontMeta)
	{
		HFONT dialogFont =  (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
		LOGFONT lf;
		if (0 != GetObject(dialogFont, sizeof(LOGFONT), &lf))
		{
			StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"Tahoma");
			lf.lfWidth = 0;
			lf.lfHeight += (lf.lfHeight < 0) ? 1 : -1;
			lf.lfQuality = ANTIALIASED_QUALITY;
			details->fontMeta = CreateFontIndirect(&lf);
		}

		if (NULL != details->fontMeta)
		{
			SendMessage(hMeta, WM_SETFONT, (WPARAM)details->fontMeta, 0L);
		}
	}

	SetWindowText(hMeta, szBuffer);
	if (0 != ShowWindow(hMeta,  (L'\0' != szBuffer[0]) ? SW_SHOWNA : SW_HIDE))
		InvalidateRect(hMeta, NULL, TRUE);
}

static void ServicePanel_SetThumbnail(HWND hwnd, ifc_omservice *service)
{	
	HWND hThumbnail = GetDlgItem(hwnd, IDC_THUMBNAIL);
	if (NULL == hThumbnail) return;

	SendMessage(hThumbnail, WM_SETREDRAW, FALSE, 0L);

	HBITMAP hBitmap;

	BITMAPINFOHEADER header;
	void *pixelData;

	WCHAR szPath[2048];

	ifc_omservicedetails *details = 0;
	if (GET_IDETAILS(service, details))
	{
		if (SUCCEEDED(details->GetThumbnail(szPath, ARRAYSIZE(szPath))))
		{
			ifc_omcachemanager *cacheManager;
			if (SUCCEEDED(OMUTILITY->GetCacheManager(&cacheManager)))
			{
				ifc_omcachegroup *cacheGroup;
				if (SUCCEEDED(cacheManager->Find(L"thumbnail", TRUE, &cacheGroup, NULL)))
				{
					ifc_omcacherecord *cacheRecord;
					if (SUCCEEDED(cacheGroup->Find(szPath, TRUE, &cacheRecord, FALSE)))
					{
						cacheRecord->Release();
					}
					cacheGroup->Release();
				}
				cacheManager->Release();
			}
		}
		details->Release();
	}

	ifc_omimageloader *imageLoader;
	if (SUCCEEDED(OMUTILITY->QueryImageLoader(NULL, szPath, FALSE, &imageLoader)))
	{
		if (FAILED(imageLoader->LoadBitmapEx(&hBitmap, &header, &pixelData)))
			hBitmap = NULL;
		imageLoader->Release();
	}
		
	if (NULL == hBitmap && 
		SUCCEEDED(OMUTILITY->QueryImageLoader(WASABI_API_ORIG_HINST, MAKEINTRESOURCE(IDR_SERVICE64X64_IMAGE), FALSE, &imageLoader)))
	{
		if (FAILED(imageLoader->LoadBitmapEx(&hBitmap, &header, &pixelData)))
			hBitmap = NULL;
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
}


static void CALLBACK ServicePanel_OnServiceNotify(UINT serviceUid, UINT callbackType, UINT callbackParam, ULONG_PTR user)
{
	HWND hwnd = (HWND)user;
	if (NULL == hwnd) return;

	SERVICEDETAILS *details = GetDetails(hwnd);
		
	if (NULL == details ||
		NULL == details->service ||
		serviceUid != details->service->GetId())
	{
		return;
	}

	switch(callbackType)
	{
		case OmService::eventServiceModified:
			if (0 != (OmService::modifiedName & callbackParam))
				ServicePanel_SetServiceName(hwnd, details->service);
			if (0 != (OmService::modifiedDescription & callbackParam))
				ServicePanel_SetServiceDescription(hwnd, details->service);
			if (0 != ((OmService::modifiedAuthor | OmService::modifiedDate) & callbackParam))
				ServicePanel_SetServiceMeta(hwnd, details->service);
			if (0 != (OmService::modifiedThumbnail & callbackParam))
				ServicePanel_SetThumbnail(hwnd, details->service);
			break;
	}
}

static INT_PTR ServicePanel_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	ifc_omservice *service = (ifc_omservice*)lParam;

	SERVICEDETAILS *details = (SERVICEDETAILS*)malloc(sizeof(SERVICEDETAILS));
	if (NULL == details) return FALSE;
	ZeroMemory(details, sizeof(SERVICEDETAILS));

	if (!SetProp(hwnd, MAKEINTATOM(DETAILS_PROP), details))
		return FALSE;

	details->service = service;
	details->service->AddRef();
	
	ServicePanel_SetServiceName(hwnd, service);
	ServicePanel_SetServiceDescription(hwnd, service);
	ServicePanel_SetThumbnail(hwnd, service);
	ServicePanel_SetServiceMeta(hwnd, service);

	if (NULL != service)
	{
		ifc_omserviceeventmngr *eventManager;
		if (SUCCEEDED(service->GetEventManager(&eventManager)))
		{
			if (SUCCEEDED(eventManager->RegisterHandler(eventHander)))
			{
				details->eventHandler = eventHandler;
			}
			else
			{
				eventHandler->Release();
			}
		}
	}

	return FALSE;
}

static void ServicePanel_OnDestroy(HWND hwnd)
{
	OMSERVICEMNGR->UnregisterCallback(ServicePanel_OnServiceNotify, (ULONG_PTR)hwnd);

	SERVICEDETAILS *details = GetDetails(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(DETAILS_PROP));
	
	if (NULL != details)
	{
		if (NULL != details->service)
			details->service->Release();
		if (NULL != details->fontTitle)
			DeleteObject(details->fontTitle);
		if (NULL != details->fontMeta)
			DeleteObject(details->fontMeta);
	}

	HWND hThumbnail = GetDlgItem(hwnd, IDC_THUMBNAIL);
	if (NULL != hThumbnail)
	{
		HBITMAP hBitmap = (HBITMAP)SendMessage(hThumbnail, STM_SETIMAGE, IMAGE_BITMAP, 0L);
		if (NULL != hBitmap) 
			DeleteObject(hBitmap);
	}
}


static INT_PTR ServicePanel_OnDialogColor(HWND hwnd, HDC hdc, HWND hControl)
{
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent && hParent != hwnd)
		return (INT_PTR)SendMessage(hParent, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hControl);
	return 0;
}


static INT_PTR ServicePanel_OnStaticColor(HWND hwnd, HDC hdc, HWND hControl)
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

static INT_PTR WINAPI ServicePanel_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return ServicePanel_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		ServicePanel_OnDestroy(hwnd); break;
		case WM_CTLCOLORDLG:	return ServicePanel_OnDialogColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_CTLCOLORSTATIC: return ServicePanel_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
	}
	return 0;
}