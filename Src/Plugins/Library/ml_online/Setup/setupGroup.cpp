#include "./setupGroup.h"
#include "./setupGroupFilter.h"
#include "./setupListboxLabel.h"
#include "./setupDetails.h"
#include "./setupPage.h"
#include "../common.h"
#include "../api__ml_online.h"
#include "../resource.h"
#include "../serviceHost.h"
#include "../serviceHelper.h"

#include "../../nu/menuHelpers.h"
#include <vector>

#include <ifc_omservice.h>
#include <ifc_omstorage.h>
#include <ifc_omstorageasync.h>
#include <ifc_omserviceenum.h>
#include <ifc_omfilestorage.h>

#include <shlwapi.h>
#include <strsafe.h>
#include <algorithm>

typedef std::vector<ifc_omservice*> ServiceList;

#define GROUP_MARGINCX			0
#define GROUP_MARGINCY			1

#define TEXT_OFFSET_LEFT	2
#define TEXT_OFFSET_BOTTOM	2
#define TEXT_ALIGN			(TA_LEFT | TA_BOTTOM)

SetupGroup::SetupGroup(INT groupId, LPCWSTR pszName, LPCWSTR pszAddress, const GUID *storageId, const GUID *filterId, UINT fStyle) 
	: ref(1), id(groupId), name(NULL), flags(0), emptyLabel(NULL), errorCode(S_OK), hPage(NULL),
	longName(NULL), description(NULL), address(NULL), loadResult(NULL), style(fStyle), loadComplete(NULL)
{
	name = Plugin_DuplicateResString(pszName);
	address = Plugin_DuplicateResString(pszAddress);
	this->storageId = (NULL != storageId) ? *storageId : GUID_NULL;
	this->filterId = (NULL != filterId) ? *filterId : GUID_NULL;
	
	InitializeCriticalSection(&lock);
}

SetupGroup::~SetupGroup()
{
	Plugin_FreeResString(name);
	Plugin_FreeResString(address);

	SetLongName(NULL);
	SetDescription(NULL);

	EnterCriticalSection(&lock);

	size_t index = list.size();
	while(index--)
	{
		list[index]->Release();
	}

	if (NULL != emptyLabel)
		emptyLabel->Release();

	if (NULL != loadResult)
	{		
		ifc_omstorage *storage;
		HRESULT hr = OMSERVICEMNGR->QueryStorage(&storageId, &storage);
		if (SUCCEEDED(hr))
		{
			storage->RequestAbort(loadResult, TRUE);
		}

		loadResult->Release();
		loadResult = NULL;
	}

	if (NULL != loadComplete)
		CloseHandle(loadComplete);

	LeaveCriticalSection(&lock);

	DeleteCriticalSection(&lock);
	
}

SetupGroup *SetupGroup::CreateInstance(INT groupId, LPCWSTR pszName, LPCWSTR pszAddress, const GUID *storageId, const GUID *filterId, UINT fStyle)
{
	return new SetupGroup(groupId, pszName, pszAddress, storageId, filterId, fStyle);
}

ULONG SetupGroup::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG SetupGroup::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	return r;
}

HRESULT SetupGroup::GetName(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;

	HRESULT hr;
	if (NULL != name)
	{
		if (IS_INTRESOURCE(name))
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)name, pszBuffer, cchBufferMax);
			hr = (L'\0' != *pszBuffer) ? S_OK : E_FAIL;
		}
		else
		{
			hr = StringCchCopyW(pszBuffer, cchBufferMax, name);
		}
	}
	else
	{
		hr = StringCchCopyW(pszBuffer, cchBufferMax, L"Unknown");
	}
	return hr;
}

HRESULT SetupGroup::GetLongName(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;

	HRESULT hr;
	if (NULL == longName)
		return GetName(pszBuffer, cchBufferMax);
	
	if (IS_INTRESOURCE(longName))
	{
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)longName, pszBuffer, cchBufferMax);
		hr = (L'\0' != *pszBuffer) ? S_OK : E_FAIL;
	}
	else
	{
		hr = StringCchCopyW(pszBuffer, cchBufferMax, longName);
	}
	return hr;
}

HRESULT SetupGroup::GetDescription(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;

	HRESULT hr;
	
	if (NULL != description && IS_INTRESOURCE(description))
	{
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)description, pszBuffer, cchBufferMax);
		hr = (L'\0' != *pszBuffer) ? S_OK : E_FAIL;
	}
	else
	{
		hr = StringCchCopyEx(pszBuffer, cchBufferMax, description, NULL, NULL, STRSAFE_IGNORE_NULLS);
	}
	return hr;
}
size_t SetupGroup::GetRecordCount()
{ 	
	return list.size(); 
}

size_t SetupGroup::GetListboxCount()
{
	if (0 != (flagCollapsed & flags)) return 0;
	size_t listSize = list.size();

	if (0 == listSize)
	{
		return (NULL != emptyLabel && FALSE == emptyLabel->IsNameNull()) ? 1 : 0;
	}

	return listSize;
}

SetupListboxItem *SetupGroup::GetListboxItem(size_t index)
{
	if (0 != (flagCollapsed & flags)) return NULL;
	size_t listSize = list.size();

	if (0 == listSize)
	{
		return (NULL != emptyLabel && FALSE == emptyLabel->IsNameNull())  ? emptyLabel : NULL;
	}
	return list[index];
}

BOOL SetupGroup::IsModified()
{
	size_t index = list.size();
	while(index--)
	{
		if (list[index]->IsModified()) 
			return TRUE;
	}

	return FALSE;
}

BOOL SetupGroup::IsExpanded()
{
	return (0 == (flagCollapsed & flags));
}

void SetupGroup::SetExpanded(BOOL fExpanded)
{
	if ((FALSE == fExpanded) == (FALSE == IsExpanded()))
		return;

	if (FALSE == fExpanded)
		flags |= flagCollapsed;
	else
		flags &= ~flagCollapsed;
}

void SetupGroup::Clear(BOOL fInvalidate)
{
	size_t index = list.size();
	if (0 == index) return;

	EnterCriticalSection(&lock);

	while(index--)
	{
		SetupRecord *record = list[index];
		if (NULL != record)
		{
			record->Release();
		}
	}
	list.clear();
	
	LeaveCriticalSection(&lock);

	SetEmptyText(MAKEINTRESOURCE(IDS_SETUP_EMPTYGROUP), FALSE);

	if (FALSE != fInvalidate && NULL != hPage)
		PostMessage(hPage, SPM_UPDATELIST, (WPARAM)id, NULL);
}

static void CALLBACK SetupGroup_LoadCallback(ifc_omstorageasync *result)
{
	if (NULL == result) return;
	SetupGroup *group;
	if (SUCCEEDED(result->GetData((void**)&group)) && NULL != group)
	{
		group->OnLoadCompleted();
	}
}


__inline static int __cdecl SetupGroup_AlphabeticalSorter(const void *elem1, const void *elem2)
{
	SetupRecord *record1 = (SetupRecord*)elem1;
	SetupRecord *record2 = (SetupRecord*)elem2;
	
	if (NULL == record1 || NULL == record2) 
		return (INT)(INT_PTR)(record1 - record2);
	
	ifc_omservice *svc1 = record1->GetService();
	ifc_omservice *svc2 = record2->GetService();

	if (NULL == svc1 || NULL == svc2) 
		return (INT)(INT_PTR)(svc1 - svc2);

	WCHAR szBuffer1[256] = {0}, szBuffer2[256] = {0};
	if (FAILED(svc1->GetName(szBuffer1, ARRAYSIZE(szBuffer1))))
		szBuffer1[0] = L'\0';
	if (FAILED(svc2->GetName(szBuffer2, ARRAYSIZE(szBuffer2))))
		szBuffer2[0] = L'\0';

	return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, szBuffer1, -1, szBuffer2, -1) - 2;
}

__inline static bool __cdecl SetupGroup_AlphabeticalSorter_V2(const void* elem1, const void* elem2)
{
	return SetupGroup_AlphabeticalSorter(elem1, elem2) < 0;
}
void SetupGroup::OnLoadCompleted()
{
	ifc_omstorage *storage;
	HRESULT hr = OMSERVICEMNGR->QueryStorage(&storageId, &storage);
	if (SUCCEEDED(hr))
	{		
		ifc_omserviceenum *serviceEnum;
		hr = storage->EndLoad(loadResult, &serviceEnum);
		if (SUCCEEDED(hr))
		{
			SetupGroupFilter *filter;
			if (FAILED(SetupGroupFilter::CreateInstance(&filterId, &filter)))
			{
				filter = NULL;
			}
			else if (FAILED(filter->Initialize()))
			{
				filter->Release();
				filter = NULL;
			}
			
			EnterCriticalSection(&lock);

			ifc_omservice *service;
			UINT filterResult, defaultFilter;
			defaultFilter = SetupGroupFilter::serviceInclude;
			if (0 != (styleDefaultUnsubscribed & style))
				defaultFilter |= SetupGroupFilter::serviceForceUnsubscribe;
			else if (0 != (styleDefaultSubscribed & style))
				defaultFilter |= SetupGroupFilter::serviceForceSubscribe;

			while(S_OK == serviceEnum->Next(1, &service, NULL))
			{
				
				filterResult = defaultFilter;
				if (NULL != filter && FAILED(filter->ProcessService(service, &filterResult)))
					filterResult = defaultFilter;

				if (0 == (SetupGroupFilter::serviceInclude & filterResult))
				{
					service->Release();
					service = NULL;
					continue;
				}
				
				if (0 != (SetupGroupFilter::serviceForceUnsubscribe & filterResult))
					ServiceHelper_Subscribe(service, TRUE, 0);
				else if (0 != (SetupGroupFilter::serviceForceSubscribe & filterResult))
					ServiceHelper_Subscribe(service, FALSE, 0);

				if (0 != (styleSaveAll & style))
					ServiceHelper_MarkModified(service, (UINT)-1, (UINT)-1);
			
				SetupRecord *record = SetupRecord::CreateInstance(service);
				if (NULL != record)
				{
					if (0 != (SetupGroupFilter::serviceForceUnsubscribe & filterResult))
						record->SetSelected(FALSE);
					else if (0 != (SetupGroupFilter::serviceForceSubscribe & filterResult))
						record->SetSelected(TRUE);
                    list.push_back(record);
				}

				service->Release();
			}

			if (0 != (styleSortAlphabetically & style))
			{
				//qsort(list.first(), list.size(), sizeof(SetupRecord*), SetupGroup_AlphabeticalSorter);
				std::sort(list.begin(), list.end(), SetupGroup_AlphabeticalSorter_V2);
			}

			LeaveCriticalSection(&lock);

			serviceEnum->Release();
			if (NULL != filter)
				filter->Release();
		}
	
		storage->Release();
	}

	EnterCriticalSection(&lock);

	loadResult->Release();
	loadResult = NULL;

	if (NULL != loadComplete)
	{
		SetEvent(loadComplete);
		CloseHandle(loadComplete);
		loadComplete = NULL;
	}

	LeaveCriticalSection(&lock);

	LPCWSTR pszText = MAKEINTRESOURCE(((FAILED(hr)) ? IDS_SETUP_GROUPLOADFAILED : IDS_SETUP_EMPTYGROUP));
	SetEmptyText( pszText, TRUE);
}

HRESULT SetupGroup::RequestReload()
{
	if (NULL == OMSERVICEMNGR) 
		return E_UNEXPECTED;
	
	HRESULT hr;

	EnterCriticalSection(&lock);

	if (NULL != loadResult)
		hr = E_PENDING;
	else
	{
		if (NULL != loadComplete)
		{
			CloseHandle(loadComplete);
			loadComplete = NULL;
		}

		Clear(FALSE);
		SetEmptyText(MAKEINTRESOURCE(IDS_SETUP_LOADINGGROUP), TRUE);
		
		ifc_omstorage *storage;
		hr = OMSERVICEMNGR->QueryStorage(&storageId, &storage);
		if (SUCCEEDED(hr))
		{
			ServiceHost *serviceHost;
			if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
				serviceHost = NULL;

			hr = storage->BeginLoad(address, serviceHost, SetupGroup_LoadCallback, this, &loadResult);
			storage->Release();

			if (NULL != serviceHost)
				serviceHost->Release();
		}

		if (FAILED(hr))
		{
			SetEmptyText(MAKEINTRESOURCE(IDS_SETUP_GROUPLOADFAILED), TRUE);
		}
	}

	LeaveCriticalSection(&lock);

	return hr;
}

void SetupGroup::SetPageWnd(HWND hPage)
{
	this->hPage = hPage;
}

HRESULT SetupGroup::SignalLoadCompleted(HANDLE event)
{
	HRESULT hr;
	if (NULL == event) return E_INVALIDARG;

	EnterCriticalSection(&lock);

	if (NULL == loadResult)
	{
		SetEvent(event);
		hr = S_OK;
	}
	else
	{
		if (NULL != loadComplete)
			CloseHandle(loadComplete);

		if (FALSE == DuplicateHandle(GetCurrentProcess(), event, GetCurrentProcess(), &loadComplete, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32(error);
		}

		hr = S_OK;
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT SetupGroup::Save(SetupLog *log)
{
	HRESULT hr(S_OK);
	size_t index = list.size();
	while(index--)
	{
		if (FAILED(list[index]->Save(log)))
			hr = E_FAIL;
	}
	return hr;
}

void SetupGroup::GetColors(HDC hdc, UINT state, COLORREF *rgbBkOut, COLORREF *rgbTextOut)
{
	COLORREF rgbBk, rgbText;
		

	if (0 != (ODS_DISABLED & state))
	{
		rgbBk = GetBkColor(hdc);
		rgbText = GetSysColor(COLOR_GRAYTEXT);
	}
	else
	{
		if (0 != (ODS_SELECTED & state))
		{
			if (0 == (ODS_INACTIVE & state))
			{
				rgbBk = GetSysColor(COLOR_HIGHLIGHT);
				rgbText = GetSysColor(COLOR_HIGHLIGHTTEXT);
			}
			else
			{
				rgbBk = GetSysColor(COLOR_3DFACE);
				rgbText = GetSysColor(COLOR_WINDOWTEXT);
			}
		}
		else
		{
			rgbBk = GetSysColor(COLOR_WINDOW);
			rgbText = GetSysColor(COLOR_WINDOWTEXT);
		}
	}

	if (NULL != rgbBkOut) *rgbBkOut = rgbBk;
	if (NULL != rgbTextOut) *rgbTextOut = rgbText;
}

HBRUSH SetupGroup::GetBrush(HDC hdc, UINT state)
{	
	if (0 != (ODS_DISABLED & state))
	{
		return GetSysColorBrush(COLOR_WINDOW);
	}
	if (0 != (ODS_COMBOBOXEDIT & state))
	{
		return GetSysColorBrush(COLOR_WINDOWTEXT);
	}
	if (0 != (ODS_SELECTED & state))
	{
		return GetSysColorBrush( (0 == (ODS_INACTIVE & state)) ? COLOR_HIGHLIGHT : COLOR_3DFACE);
	}

	return GetSysColorBrush(COLOR_WINDOW);
}

BOOL SetupGroup::MeasureItem(SetupListbox *instance, UINT *cx, UINT *cy)
{
	HDC hdc  = GetDCEx(instance->GetHwnd(), NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return FALSE;
		
	HFONT originalFont = (HFONT)SelectObject(hdc, instance->GetFont());
	SIZE imageSize;
	if (!instance->GetExpandboxMetrics(hdc, IsExpanded(), &imageSize))
		ZeroMemory(&imageSize, sizeof(SIZE));

	if (NULL != cy)
	{
		*cy = 0;
		TEXTMETRIC tm;
		if (GetTextMetrics(hdc, &tm))
		{
			*cy = tm.tmHeight + tm.tmExternalLeading;
			if (imageSize.cy > (INT)*cy) *cy = imageSize.cy;
			*cy += GROUP_MARGINCY*2;
		}
	}

	if (NULL != cx)
	{
		*cx = imageSize.cx;
		WCHAR szBuffer[128] = {0};
		if (SUCCEEDED(GetName(szBuffer, ARRAYSIZE(szBuffer))))
		{
			INT cchBuffer = lstrlenW(szBuffer);
			SIZE textSize;
			if (0 != cchBuffer && GetTextExtentPoint32(hdc, szBuffer, cchBuffer, &textSize))
			{
				*cx += textSize.cx;
			}
		}
		if (0 != *cx) *cx += GROUP_MARGINCX*2;
	}

	SelectObject(hdc, originalFont);
	ReleaseDC(instance->GetHwnd(), hdc);
	return TRUE;
}


static void SetupGroup_DrawFrame(HDC hdc, const RECT *prc, INT width, COLORREF rgbFrame)
{	
	if (width > 0)
	{		
		COLORREF rgbOld = SetBkColor(hdc, rgbFrame);	

		RECT rcPart;
		SetRect(&rcPart, prc->left, prc->top, prc->right, prc->top + width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->left, prc->bottom - width, prc->right, prc->bottom); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->left, prc->top + width, 	prc->left + width, prc->bottom - width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->right - width, prc->top + width, prc->right, prc->bottom - width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		
		if (rgbOld != rgbFrame)
			SetBkColor(hdc, rgbOld);
	}
}
BOOL SetupGroup::DrawItem(SetupListbox *instance, HDC hdc, const RECT *prc, UINT state)
{
	LONG paintLeft = prc->left + GROUP_MARGINCX;
	RECT partRect;

	SetRectEmpty(&partRect);

	COLORREF rgbBk, rgbText;
	GetColors(hdc, state, &rgbBk, &rgbText);

	COLORREF origBk = SetBkColor(hdc, rgbBk);
	COLORREF origText = SetTextColor(hdc, rgbText);
	UINT textAlign = SetTextAlign(hdc, TEXT_ALIGN);

	HRGN backRgn, rgn;
	backRgn = CreateRectRgnIndirect(prc);
	rgn = CreateRectRgn(0,0,0,0);

	SetRectEmpty(&partRect);
	if (instance->GetExpandboxMetrics(hdc, IsExpanded(), (((SIZE*)&partRect) + 1)))
	{
		INT space = (prc->bottom - prc->top) - (partRect.bottom-  partRect.top);
		INT offsetY = space / 2 + space%2;
		if (offsetY < 0) offsetY = 0;
		OffsetRect(&partRect, paintLeft, prc->top + offsetY);
		if (instance->DrawExpandbox(hdc, IsExpanded(), &partRect, rgbBk, rgbText))
		{
			paintLeft = partRect.right;
			if (SetRectRgn(rgn, partRect.left, partRect.top, partRect.right, partRect.bottom))
				CombineRgn(backRgn, backRgn, rgn, RGN_DIFF);
		}
	}
		
	WCHAR szBuffer[128] = {0};
	INT cchBuffer = 0;
	if (SUCCEEDED(GetName(szBuffer, ARRAYSIZE(szBuffer))))
		cchBuffer = lstrlenW(szBuffer);
	
	SetRect(&partRect, paintLeft, prc->top, prc->right, prc->bottom);
	if (ExtTextOut(hdc, partRect.left + TEXT_OFFSET_LEFT, partRect.bottom - TEXT_OFFSET_BOTTOM, 
					ETO_OPAQUE | ETO_CLIPPED, &partRect, szBuffer, cchBuffer, NULL))
	{
		if (SetRectRgn(rgn, partRect.left, partRect.top, partRect.right, partRect.bottom))
			CombineRgn(backRgn, backRgn, rgn, RGN_DIFF);
	}

	

	COLORREF rgbLine = ColorAdjustLuma(rgbBk, -150, TRUE);
	if (rgbLine != rgbBk)
	{
		RECT lineRect;
		SetRect(&lineRect, prc->left, prc->bottom - 1, prc->right, prc->bottom);
		
		SetBkColor(hdc, rgbLine);
		if (ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &lineRect, NULL, 0, NULL))
		{
			if (SetRectRgn(rgn, lineRect.left, lineRect.top, lineRect.right, lineRect.bottom))
				CombineRgn(backRgn, backRgn, rgn, RGN_DIFF);
		}

		SetBkColor(hdc, rgbBk);
	}
	
	if (0 != (flagMenuActive & flags))
	{
		COLORREF rgbFrame = rgbLine; //ColorAdjustLuma(GetSysColor(COLOR_HIGHLIGHT), 100, TRUE);
		SetupGroup_DrawFrame(hdc, prc, 1, rgbFrame);
		if (SetRectRgn(rgn, prc->left + 1, prc->top + 1, prc->right - 1, prc->bottom - 1))
			CombineRgn(backRgn, backRgn, rgn, RGN_AND);
	}
	

	if (NULL != backRgn)
	{		
		FillRgn(hdc, backRgn, GetBrush(hdc, state));
		DeleteObject(backRgn);
	}
	if (NULL != rgn)
		DeleteObject(rgn);



	if (ODS_FOCUS == ((ODS_FOCUS | 0x0200/*ODS_NOFOCUSRECT*/) & state))
		DrawFocusRect(hdc, prc);

	if (TEXT_ALIGN != textAlign) SetTextAlign(hdc, textAlign);
	if (origBk != rgbBk) SetBkColor(hdc, origBk);
	if (origText != rgbText) SetTextColor(hdc, origText);
	return TRUE;
}

INT_PTR SetupGroup::KeyToItem(SetupListbox *instance, const RECT *prcItem, INT vKey)
{
	switch(vKey)
	{
		case VK_SPACE:
			InvertExpanded(instance);
			return -2;
	}
	return -1;
}
BOOL SetupGroup::MouseMove(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupGroup::MouseLeave(SetupListbox *instance, const RECT *prcItem)
{
	return FALSE;
}
BOOL SetupGroup::LButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupGroup::LButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupGroup::LButtonDblClk(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	InvertExpanded(instance);
	return TRUE;
}

BOOL SetupGroup::RButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return TRUE;
}

BOOL SetupGroup::RButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	HMENU hMenu = instance->GetContextMenu(SetupListbox::menuGroupContext);
	if (NULL == hMenu) 
		return FALSE;
	
	hMenu = MenuHelper_DuplcateMenu(hMenu);
	if (NULL == hMenu) return FALSE;

	MENUITEMINFO mi;
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_STATE;
	GetMenuItemInfo(hMenu, ID_GROUP_TOGGLE, FALSE, &mi);

	mi.fMask = 0;
	if (0 == (MFS_DEFAULT & mi.fState))
	{
		mi.fMask |= MIIM_STATE;
		mi.fState |= MFS_DEFAULT;
	}

	WCHAR szBuffer[128] = {0};
	WASABI_API_LNGSTRINGW_BUF(((IsExpanded()) ? IDS_COLLAPSE : IDS_EXPAND), szBuffer, ARRAYSIZE(szBuffer));
	mi.fMask |= MIIM_STRING;
	mi.dwTypeData = szBuffer;

	if (0 != mi.fMask)
		SetMenuItemInfo(hMenu, ID_GROUP_TOGGLE, FALSE, &mi);

	if (0 == list.size())
	{
		EnableMenuItem(hMenu, ID_GROUP_SELECTALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hMenu, ID_GROUP_UNSELECTALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (0 != (flagLoading & flags))
	{
		EnableMenuItem(hMenu, ID_GROUP_RELOAD, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
		
	MapWindowPoints(instance->GetHwnd(), HWND_DESKTOP, &pt, 1);
	
	flags |= flagMenuActive;
	instance->InvalidateRect(prcItem, TRUE);

	INT cmd = TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, instance->GetHwnd(), NULL);
	if (0 != cmd)
	{
		Command(instance, cmd, 0);
	}

	DestroyMenu(hMenu);

	flags &= ~flagMenuActive;
	instance->InvalidateRect(prcItem, TRUE);

	return TRUE;
}

void SetupGroup::CaptureChanged(SetupListbox *instance, const RECT *prcItem, SetupListboxItem *captured)
{
}

void SetupGroup::InvertExpanded(SetupListbox *instance)
{
	if (FALSE != IsExpanded())
		ValidateSelection(instance);

	SetExpanded(!IsExpanded());
	if (NULL != instance)
	{
		instance->UpdateCount();
		UpdateWindow(instance->GetHwnd());
	}
}
void SetupGroup::SelectAll(SetupListbox *instance, BOOL fSelect)
{
	size_t index = list.size();

	INT baseIndex;
	if (NULL == instance || FALSE == instance->GetIndex(this, &baseIndex))
		baseIndex = -1;
	else 
		baseIndex++;

	while(index--)
	{
		SetupRecord *record = list[index];
		if (NULL != record && !record->IsDisabled())
		{
			if ((FALSE == fSelect) != (FALSE == record->IsSelected()))
			{
				record->SetSelected(fSelect);
				if (0 == (flagCollapsed & flags) && NULL != instance && -1 != baseIndex)
				{
					instance->InvalidateItem((INT)(baseIndex + index), TRUE);
				}
			}
		}
	}
}


void SetupGroup::SetEmptyText(LPCWSTR pszText, BOOL fInvalidate)
{
	if (NULL == emptyLabel)
		emptyLabel = SetupListboxLabel::CreateInstance(pszText);
	else
		emptyLabel->SetName(pszText);

	if (FALSE != fInvalidate && NULL != hPage)
		PostMessage(hPage, SPM_UPDATELIST, (WPARAM)id, NULL);
}

HWND SetupGroup::CreateDetailsView(HWND hParent)
{
	WCHAR szName[64] = {0};
	if (FALSE == GetUniqueName(szName, ARRAYSIZE(szName)))
		szName[0] = L'\0';

	return SetupDetails_CreateGroupView(hParent, szName, this);
}

BOOL SetupGroup::GetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 
		FAILED(StringCchPrintf(pszBuffer, cchBufferMax, L"grp_id_%d", id)))
	{
		return FALSE;
	}
	return TRUE;
}


void SetupGroup::SetLongName(LPCWSTR pszText)
{
	if (NULL != longName && !IS_INTRESOURCE(longName))
		Plugin_FreeString(longName);
	longName = NULL;
	
	if (NULL != pszText)
		longName = (IS_INTRESOURCE(pszText)) ? (LPWSTR)pszText : Plugin_CopyString(pszText);
}	

void SetupGroup::SetDescription(LPCWSTR pszText)
{
	if (NULL != description && !IS_INTRESOURCE(description))
		Plugin_FreeString(description);
	description = NULL;
	if (NULL != pszText)
		description = (IS_INTRESOURCE(pszText)) ? (LPWSTR)pszText : Plugin_CopyString(pszText);
}	

void SetupGroup::ValidateSelection(SetupListbox *instance)
{
	if (NULL == instance)
		return;

	SetupListboxItem *selection = instance->GetSelection();
	if (NULL != selection)
	{
		EnterCriticalSection(&lock);
		size_t index = list.size();
		while(index--)
		{
			if (list[index] == selection)
			{
				instance->SetSelection(this);
				break;
			}
		}

		LeaveCriticalSection(&lock);
	}
}

void SetupGroup::Command(SetupListbox *instance, INT commandId, INT eventId)
{
	switch(commandId)
	{
		case ID_GROUP_TOGGLE:
			InvertExpanded(instance);
			break;
		case ID_GROUP_SELECTALL:
			SelectAll(instance, TRUE);
			break;
		case ID_GROUP_UNSELECTALL:
			SelectAll(instance, FALSE);
			break;
		case ID_GROUP_RELOAD:
			ValidateSelection(instance);
			RequestReload();
			break;
	}
}