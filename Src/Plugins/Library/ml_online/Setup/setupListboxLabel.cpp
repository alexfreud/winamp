#include "./setupListboxLabel.h"
#include "../api__ml_online.h"
#include "../common.h"

#include <shlwapi.h>
#include <strsafe.h>

#define LABEL_MARGINCX			0
#define LABEL_MARGINCY			1

#define TEXT_OFFSET_LEFT	2
#define TEXT_OFFSET_BOTTOM	2
#define TEXT_ALIGN			(TA_CENTER | TA_BOTTOM)

SetupListboxLabel::SetupListboxLabel(LPCWSTR pszName) 
	: ref(1), name(NULL)
{
	SetName(pszName);
}

SetupListboxLabel::~SetupListboxLabel()
{
	SetName(NULL);
}

SetupListboxLabel *SetupListboxLabel::CreateInstance(LPCWSTR pszName)
{
	return new SetupListboxLabel(pszName);
}

ULONG SetupListboxLabel::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG SetupListboxLabel::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	return r;
}

HRESULT SetupListboxLabel::GetName(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;

	if (NULL != name && IS_INTRESOURCE(name))
	{
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)name, pszBuffer, cchBufferMax);
		return  (L'\0' != *pszBuffer) ? S_OK : E_FAIL;
	}
	return StringCchCopyExW(pszBuffer, cchBufferMax, name, NULL, NULL, STRSAFE_IGNORE_NULLS);
}
HRESULT SetupListboxLabel::SetName(LPCWSTR pszName)
{
	if (NULL != name && !IS_INTRESOURCE(name))
		Plugin_FreeString(name);

	if (IS_INTRESOURCE(pszName))
	{
		name = (LPWSTR)pszName;
	}
	else
	{
		name = Plugin_CopyString(pszName);
	}

	return S_OK;
}

BOOL SetupListboxLabel::IsNameNull()
{
	return (NULL == name);
}

void SetupListboxLabel::GetColors(HDC hdc, UINT state, COLORREF *rgbBkOut, COLORREF *rgbTextOut)
{
	COLORREF rgbBk, rgbText;
	
	rgbBk = GetSysColor(COLOR_WINDOW);
	rgbText = GetSysColor( (0 == (ODS_DISABLED & state)) ? COLOR_GRAYTEXT : COLOR_GRAYTEXT);
	
	if (NULL != rgbBkOut) *rgbBkOut = rgbBk;
	if (NULL != rgbTextOut) *rgbTextOut = rgbText;
}

HBRUSH SetupListboxLabel::GetBrush(HDC hdc, UINT state)
{	
	return GetSysColorBrush(COLOR_WINDOW);
}

BOOL SetupListboxLabel::MeasureItem(SetupListbox *instance, UINT *cx, UINT *cy)
{
	HDC hdc  = GetDCEx(instance->GetHwnd(), NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return FALSE;
		
	HFONT originalFont = (HFONT)SelectObject(hdc, instance->GetFont());
	
	if (NULL != cy)
	{
		*cy = 0;
		TEXTMETRIC tm;
		if (GetTextMetrics(hdc, &tm))
			*cy = tm.tmHeight + tm.tmExternalLeading + LABEL_MARGINCY*2;
	}

	if (NULL != cx)
	{
		*cx = 0;
		WCHAR szBuffer[128] = {0};
		if (SUCCEEDED(GetName(szBuffer, ARRAYSIZE(szBuffer))))
		{
			INT cchBuffer = lstrlenW(szBuffer);
			SIZE textSize;
			if (0 != cchBuffer && GetTextExtentPoint32(hdc, szBuffer, cchBuffer, &textSize))
			{
				*cx = textSize.cx + LABEL_MARGINCX*2;
			}
		}
	}

	SelectObject(hdc, originalFont);
	ReleaseDC(instance->GetHwnd(), hdc);
	return TRUE;
}

BOOL SetupListboxLabel::DrawItem(SetupListbox *instance, HDC hdc, const RECT *prc, UINT state)
{
	LONG paintLeft = prc->left + LABEL_MARGINCX;
		
	COLORREF rgbBk, rgbText;
	GetColors(hdc, state, &rgbBk, &rgbText);

	COLORREF origBk = SetBkColor(hdc, rgbBk);
	COLORREF origText = SetTextColor(hdc, rgbText);
	UINT textAlign = SetTextAlign(hdc, TEXT_ALIGN);

	HRGN backRgn, rgn;
	backRgn = CreateRectRgnIndirect(prc);
	rgn = CreateRectRgn(0,0,0,0);

	RECT partRect;
	WCHAR szBuffer[128] = {0};
	INT cchBuffer = 0;
	
	if (SUCCEEDED(GetName(szBuffer, ARRAYSIZE(szBuffer))))
		cchBuffer = lstrlenW(szBuffer);
	
	SetRect(&partRect, paintLeft, prc->top, prc->right - LABEL_MARGINCX, prc->bottom);
	if (ExtTextOut(hdc, partRect.left + (partRect.right - partRect.left)/2, partRect.bottom - TEXT_OFFSET_BOTTOM, 
					ETO_OPAQUE | ETO_CLIPPED, &partRect, szBuffer, cchBuffer, NULL))
	{
		if (SetRectRgn(rgn, partRect.left, partRect.top, partRect.right, partRect.bottom))
			CombineRgn(backRgn, backRgn, rgn, RGN_DIFF);
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

INT_PTR SetupListboxLabel::KeyToItem(SetupListbox *instance, const RECT *prcItem, INT vKey)
{
	return -1;
}
BOOL SetupListboxLabel::MouseMove(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupListboxLabel::MouseLeave(SetupListbox *instance, const RECT *prcItem)
{
	return FALSE;
}
BOOL SetupListboxLabel::LButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupListboxLabel::LButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupListboxLabel::LButtonDblClk(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupListboxLabel::RButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
BOOL SetupListboxLabel::RButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt)
{
	return FALSE;
}
void SetupListboxLabel::CaptureChanged(SetupListbox *instance, const RECT *prcItem, SetupListboxItem *captured)
{
}

BOOL SetupListboxLabel::GetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	WCHAR szName[128] = {0};
	if (FAILED(GetName(szName, ARRAYSIZE(szName))))
		return FALSE;

	if (NULL == pszBuffer || 
		FAILED(StringCchPrintf(pszBuffer, cchBufferMax, L"lbl_empty_%s", szName)))
	{
		return FALSE;
	}
	return TRUE;
}