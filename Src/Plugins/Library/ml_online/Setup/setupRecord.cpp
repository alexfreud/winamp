#include "./main.h"
#include "../api__ml_online.h"
#include "./setupRecord.h"
#include "./setupDetails.h"
#include "./setupLog.h"
#include "../resource.h"

#include "./serviceHelper.h"
#include "./serviceHost.h"

#include <ifc_omservice.h>
#include <ifc_omwebstorage.h>
#include <ifc_omserviceenum.h>
#include <ifc_omservicecopier.h>

#include <wininet.h>
#include <strsafe.h>

#define RECORD_MARGINCX			6
#define RECORD_MARGINCY			2
#define CHECKBOX_MARGIN_RIGHT	2

#define TEXT_OFFSET_LEFT	3
#define TEXT_OFFSET_BOTTOM	RECORD_MARGINCY
#define TEXT_ALIGN			(TA_LEFT | TA_BOTTOM)

SetupRecord::SetupRecord(ifc_omservice *serviceToUse) 
	: ref(1), service(serviceToUse), flags(0), async(NULL)
{
	if (NULL != service)
	{
		if (S_OK == ServiceHelper_IsSubscribed(service))
			flags |= recordSelected;
		service->AddRef();
	}

	InitializeCriticalSection(&lock);
}

SetupRecord::~SetupRecord()
{
	EnterCriticalSection(&lock);

	if (NULL != async)
	{
		ifc_omstorage *storage = NULL;
		HRESULT hr = OMSERVICEMNGR->QueryStorage(&SUID_OmStorageUrl, &storage);
		if (SUCCEEDED(hr) && storage != NULL)
		{
			storage->RequestAbort(async, TRUE);
		}

		async->Release();
		async = NULL;
	}

    if (NULL != service)
		service->Release();

	LeaveCriticalSection(&lock);
	DeleteCriticalSection(&lock);
}

SetupRecord *SetupRecord::CreateInstance(ifc_omservice *serviceToUse)
{
	if (NULL == serviceToUse) return NULL;
	return new SetupRecord(serviceToUse);
}

ULONG SetupRecord::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG SetupRecord::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

HRESULT SetupRecord::GetServiceName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == service) return E_UNEXPECTED;
	return service->GetName(pszBuffer, cchBufferMax);
}

HRESULT SetupRecord::GetDisplayName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	HRESULT hr = GetServiceName(pszBuffer, cchBufferMax);
	if (SUCCEEDED(hr) && L'\0' == *pszBuffer)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_DEFAULT_SERVICENAME, pszBuffer, cchBufferMax);
		DownloadDetails();
	}
	return hr;
}

HRESULT SetupRecord::DownloadDetails()
{
	HRESULT hr;
	EnterCriticalSection(&lock);
	if (NULL == async && 0 == (recordDownloaded & flags))
	{		
		WCHAR szUrl[INTERNET_MAX_URL_LENGTH] = {0};
		hr = ServiceHelper_GetDetailsUrl(szUrl, ARRAYSIZE(szUrl), service, FALSE);
		if (SUCCEEDED(hr))
		{			
			ifc_omstorage *storage = NULL;
			hr = OMSERVICEMNGR->QueryStorage(&SUID_OmStorageUrl, &storage);
			if (SUCCEEDED(hr) && storage != NULL)
			{
				ServiceHost *serviceHost = NULL;
				if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
					serviceHost = NULL;

				hr = storage->BeginLoad(szUrl, serviceHost, SetupRecord_ServiceDownloadedCallback, this, &async);
				storage->Release();

				if (NULL != serviceHost)
					serviceHost->Release();
			}
		}
	}
	else
	{
		hr = S_FALSE;
	}
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT SetupRecord::Save(SetupLog *log)
{
	if (NULL == service) return E_POINTER;

	HRESULT hr = ServiceHelper_Subscribe(service, IsSelected(), SHF_SAVE);
	if (S_OK == hr)
	{
		if (NULL != log)
		{
			INT operation = (IsSelected()) ? SetupLog::opServiceAdded : SetupLog::opServiceRemoved;
			log->LogService(service, operation);
		}

	}

	return hr;
}

BOOL SetupRecord::IsModified()
{
	if (NULL == service) 
		return FALSE;

	if (S_OK == ServiceHelper_IsSubscribed(service) != (FALSE != IsSelected()))
		return TRUE;
	if (S_OK == ServiceHelper_IsModified(service))
		return TRUE;

	return FALSE;
}

BOOL SetupRecord::IsSelected()
{
	return (0 != (recordSelected & flags));
}

void SetupRecord::SetSelected(BOOL fSelected)
{
	if ((FALSE == fSelected) == !IsSelected())
		return;

	if (FALSE == fSelected)
		flags &= ~recordSelected;
	else
		flags |= recordSelected;
}

BOOL SetupRecord::AdjustCheckboxRect(SetupListbox *instance, RECT *prcItem)
{
	SIZE checkSize;
	if (!instance->GetCheckboxMetrics(NULL, IsSelected(), 0, &checkSize))
		return FALSE;

	prcItem->left += RECORD_MARGINCX;
	prcItem->right = prcItem->left + checkSize.cx;

	if (checkSize.cy > (prcItem->bottom - prcItem->top))
		checkSize.cy = (prcItem->bottom - prcItem->top);
	prcItem->top += ((prcItem->bottom - prcItem->top) - checkSize.cy) / 2;
	prcItem->bottom = prcItem->top + checkSize.cy;
	return TRUE;
}

BOOL SetupRecord::MeasureItem(SetupListbox *instance, UINT *cx, UINT *cy)
{
	HDC hdc  = GetDCEx(instance->GetHwnd(), NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return FALSE;

	HFONT originalFont = (HFONT)SelectObject(hdc, instance->GetFont());

	SIZE checkSize;
	instance->GetCheckboxMetrics(hdc, IsSelected(), 0, &checkSize);

	if (NULL != cy)
	{
		*cy = 0;
		TEXTMETRIC tm = {0};
		if (GetTextMetrics(hdc, &tm))
		{
			*cy = tm.tmHeight + tm.tmExternalLeading;
			if (checkSize.cy > (INT)*cy) *cy = checkSize.cy;
			*cy += RECORD_MARGINCY*2;
		}
	}

	if (NULL != cx)
	{
		*cx = checkSize.cx;
		WCHAR szBuffer[128] = {0};
		if (SUCCEEDED(GetDisplayName(szBuffer, ARRAYSIZE(szBuffer))))
		{
			INT cchBuffer = lstrlenW(szBuffer);
			SIZE textSize;
			if (0 != cchBuffer && GetTextExtentPoint32(hdc, szBuffer, cchBuffer, &textSize))
			{
				*cx += textSize.cx;
				if (0 != checkSize.cx)
					*cx += CHECKBOX_MARGIN_RIGHT + RECORD_MARGINCX;
			}
		}
		if (0 != *cx) *cx += RECORD_MARGINCX*2;
	}

	SelectObject(hdc, originalFont);
	ReleaseDC(instance->GetHwnd(), hdc);
	return TRUE;
}

void SetupRecord::GetColors(HDC hdc, UINT state, COLORREF *rgbBkOut, COLORREF *rgbTextOut)
{
	COLORREF rgbBk, rgbText;

	if (0 != (ODS_DISABLED & state))
	{
		rgbBk = GetBkColor(hdc);
		rgbText = GetSysColor(COLOR_GRAYTEXT);
	}
	else if (0 != (ODS_SELECTED & state))
	{
		if (0 == (ODS_INACTIVE & state))
		{
			rgbBk = GetSysColor(COLOR_HIGHLIGHT);
			rgbText = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			rgbBk = GetSysColor(COLOR_3DFACE);
			rgbText = GetTextColor(hdc);
		}
	}
	else
	{
		rgbBk = GetBkColor(hdc);
		rgbText = GetTextColor(hdc);
	}

	if (NULL != rgbBkOut) *rgbBkOut = rgbBk;
	if (NULL != rgbTextOut) *rgbTextOut = rgbText;
}

BOOL SetupRecord::DrawItem(SetupListbox *instance, HDC hdc, const RECT *prc, UINT state)
{
	LONG paintLeft = prc->left + RECORD_MARGINCX;
	RECT partRect;

	UINT checkState = state & ~ODS_SELECTED;
	if (0 == (checkboxPressed & flags))
	{
		if (0 != (checkboxHighlighted & flags))
			checkState |= ODS_HOTLIGHT;
	}
	else
	{
		checkState |= ((0 != (checkboxHighlighted & flags)) ? ODS_SELECTED : ODS_HOTLIGHT);
	}

	HRGN backRgn, rgn;
	backRgn = CreateRectRgnIndirect(prc);
	rgn = CreateRectRgn(0,0,0,0);

	SetRectEmpty(&partRect);
	instance->GetCheckboxMetrics(hdc, IsSelected(), checkState, (((SIZE*)&partRect) + 1)); 

	INT space = (prc->bottom - prc->top) - (partRect.bottom-  partRect.top);
	INT offsetY = space / 2 + space%2;
	if (offsetY < 0) offsetY = 0;

	OffsetRect(&partRect, paintLeft, prc->top + offsetY);
	if (instance->DrawCheckbox(hdc, IsSelected(), checkState, &partRect, prc))
	{
		paintLeft = partRect.right + CHECKBOX_MARGIN_RIGHT;
		if (SetRectRgn(rgn, partRect.left, partRect.top, partRect.right, partRect.bottom))
			CombineRgn(backRgn, backRgn, rgn, RGN_DIFF);
		
	}

	COLORREF rgbBk, rgbText;
	GetColors(hdc, state, &rgbBk, &rgbText);

	COLORREF origBk = SetBkColor(hdc, rgbBk);
	COLORREF origText = SetTextColor(hdc, rgbText);
	UINT textAlign = SetTextAlign(hdc, TEXT_ALIGN);

	WCHAR szBuffer[128] = {0};
	INT cchBuffer = 0;
	if (SUCCEEDED(GetDisplayName(szBuffer, ARRAYSIZE(szBuffer))))
		cchBuffer = lstrlenW(szBuffer);

	SetRect(&partRect, paintLeft, prc->top, prc->right, prc->bottom);
	if (ExtTextOut(hdc, partRect.left + TEXT_OFFSET_LEFT, partRect.bottom - TEXT_OFFSET_BOTTOM, 
					ETO_OPAQUE | ETO_CLIPPED, &partRect, szBuffer, cchBuffer, NULL))
	{
		if (SetRectRgn(rgn, partRect.left, partRect.top, partRect.right, partRect.bottom))
			CombineRgn(backRgn, backRgn, rgn, RGN_DIFF);
	}

	if (ODS_FOCUS == ((ODS_FOCUS | 0x0200/*ODS_NOFOCUSRECT*/) & state))
		DrawFocusRect(hdc, &partRect);

	if (NULL != backRgn)
	{		
		PaintRgn(hdc, backRgn);
		DeleteObject(backRgn);
	}
	if (NULL != rgn)
		DeleteObject(rgn);

	if (TEXT_ALIGN != textAlign) SetTextAlign(hdc, textAlign);
	if (origBk != rgbBk) SetBkColor(hdc, origBk);
	if (origText != rgbText) SetTextColor(hdc, origText);

	return TRUE;
}

INT_PTR SetupRecord::KeyToItem(SetupListbox *instance, const RECT *prcItem, INT vKey)
{
	switch(vKey)
	{
		case VK_SPACE:
			InvertCheckbox(instance, prcItem);
			return -2;
	}
	return -1;
}

BOOL SetupRecord::MouseMove(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	RECT checkboxRect;
	BOOL fInvalidate = FALSE;
	CopyRect(&checkboxRect, prcItem);
    AdjustCheckboxRect(instance, &checkboxRect);
	
	if (prcItem->top <= pt.y && pt.y < prcItem->bottom &&
		checkboxRect.left <= pt.x && pt.x < checkboxRect.right)
	{
		if (0 == (checkboxHighlighted & flags))
		{
			flags |= checkboxHighlighted;
			fInvalidate = TRUE;
		}
	}
	else
	{
		if (0 != (checkboxHighlighted & flags))
		{
			flags &= ~checkboxHighlighted;
			fInvalidate = TRUE;
		}
	}

	if (FALSE != fInvalidate)
		instance->InvalidateRect(&checkboxRect, FALSE);

	return FALSE;
}

BOOL SetupRecord::MouseLeave(SetupListbox *instance, const RECT *prcItem)
{
	if (0 != (checkboxHighlighted & flags))
	{
		flags &= ~checkboxHighlighted;
		RECT checkboxRect;
		CopyRect(&checkboxRect, prcItem);
        AdjustCheckboxRect(instance, &checkboxRect);
		instance->InvalidateRect(&checkboxRect, FALSE);
	}
	return FALSE;
}

BOOL SetupRecord::LButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	RECT checkboxRect;
	BOOL handled = FALSE;
	CopyRect(&checkboxRect, prcItem);
    AdjustCheckboxRect(instance, &checkboxRect);

	if (prcItem->top <= pt.y && pt.y < prcItem->bottom &&
		prcItem->left <= pt.x && pt.x < (checkboxRect.right + CHECKBOX_MARGIN_RIGHT))
	{
		handled = TRUE;
		if (checkboxRect.left <= pt.x && pt.x < checkboxRect.right)
		{
			flags |= (checkboxHighlighted | checkboxPressed);
			instance->SetCapture(this);
			instance->InvalidateRect(&checkboxRect, FALSE);	
		}
	}
	return handled;
}

BOOL SetupRecord::LButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	RECT checkboxRect;
	BOOL handled = FALSE;
	CopyRect(&checkboxRect, prcItem);
    AdjustCheckboxRect(instance, &checkboxRect);

	if (0 != (checkboxPressed & flags))
	{
		flags &= ~checkboxPressed;
		if (this == instance->GetCapture())
			instance->ReleaseCapture();

		if (prcItem->top <= pt.y && pt.y < prcItem->bottom &&
			checkboxRect.left <= pt.x && pt.x < checkboxRect.right)
		{
			SetSelected(!IsSelected());
			handled = TRUE;
		}
		instance->InvalidateRect(&checkboxRect, FALSE);
	}
	return handled;
}

BOOL SetupRecord::LButtonDblClk(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	InvertCheckbox(instance, prcItem);
	return TRUE;
}

BOOL SetupRecord::RButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}

BOOL SetupRecord::RButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}

void SetupRecord::CaptureChanged(SetupListbox *instance, const RECT *prcItem, SetupListboxItem *captured)
{
}

void SetupRecord::InvertCheckbox(SetupListbox *instance, const RECT *prcItem)
{
	SetSelected(!IsSelected());

	if (NULL != instance && NULL != prcItem)
	{
		RECT checkboxRect;
		CopyRect(&checkboxRect, prcItem);
		AdjustCheckboxRect(instance, &checkboxRect);
		instance->InvalidateRect(&checkboxRect, FALSE);
	}
}

HWND SetupRecord::CreateDetailsView(HWND hParent)
{
	DownloadDetails();

	WCHAR szName[64] = {0};
	if (FALSE == GetUniqueName(szName, ARRAYSIZE(szName)))
		szName[0] = L'\0';

	return SetupDetails_CreateServiceView(hParent, szName, service);
}

BOOL SetupRecord::GetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 
		FAILED(StringCchPrintf(pszBuffer, cchBufferMax, L"record_svc_%u", service->GetId())))
	{
		return FALSE;
	}
	return TRUE;
}

void SetupRecord::OnDownloadCompleted()
{
	ifc_omstorage *storage = NULL;
	HRESULT hr = OMSERVICEMNGR->QueryStorage(&SUID_OmStorageUrl, &storage);
	if (SUCCEEDED(hr) && service != NULL)
	{
		ifc_omserviceenum *serviceEnum = NULL;
		hr = storage->EndLoad(async, &serviceEnum);
		if (SUCCEEDED(hr) && serviceEnum != NULL)
		{
			EnterCriticalSection(&lock);

			ifc_omservice *result = NULL;
			while(S_OK == serviceEnum->Next(1, &result, NULL))
			{
				if (result)
				{
					if (result->GetId() == service->GetId())
					{
						ifc_omservicecopier *copier;
						if (SUCCEEDED(result->QueryInterface(IFC_OmServiceCopier, (void**)&copier)))
						{
							copier->CopyTo(service, NULL);
							copier->Release();
						}
						result->Release();
						flags |= recordDownloaded;
						break;
					}
					else
					{
						result->Release();
					}
				}
				result = NULL;
			}

			LeaveCriticalSection(&lock);

			serviceEnum->Release();
		}
	
		storage->Release();
	}

	EnterCriticalSection(&lock);

	async->Release();
	async = NULL;

	LeaveCriticalSection(&lock);
}

void CALLBACK SetupRecord_ServiceDownloadedCallback(ifc_omstorageasync *result)
{
	if (NULL == result) return;
	SetupRecord *record = NULL;
	if (SUCCEEDED(result->GetData((void**)&record)) && NULL != record)
	{
		record->OnDownloadCompleted();
	}
}