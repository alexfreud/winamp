#include "main.h"
#include "./toolbarItem.h"
#include "./toolbar.h"
#include "./graphics.h"
#include "./resource.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"
#include <strsafe.h>

ToolbarItem::ToolbarItem(LPCSTR pszName, UINT nStyle, INT nIcon, LPCWSTR pszText, LPCWSTR pszDescription) 
	: ref(1), name(NULL), style(nStyle), iconId(nIcon), text(NULL), description(NULL)
{
	name = Plugin_CopyAnsiString(pszName);
	text = Plugin_DuplicateResString(pszText);
	description = Plugin_DuplicateResString(pszDescription);
}

ToolbarItem::~ToolbarItem()
{
	Plugin_FreeAnsiString(name);
	Plugin_FreeResString(text);
	Plugin_FreeResString(description);
}

ULONG ToolbarItem::AddRef() 
{ 
	return InterlockedIncrement((LONG*)&ref); 
}

ULONG ToolbarItem::Release() 
{ 
	if (0 == ref) return ref;
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r) delete(this);
	return r;
}

LPCSTR ToolbarItem::GetName()
{
	return name;
}

UINT ToolbarItem::GetStyle() 
{ 
	return style; 
}

void ToolbarItem::SetStyle(HWND hToolbar, UINT newStyle, UINT styleMask)
{ 
	UINT styleNew = (style & ~styleMask) | (newStyle & styleMask); 
	if (style != styleNew)
	{
		style = styleNew;
		if (NULL != hToolbar && 0 == (stateHidden & style))
			InvalidateRect(hToolbar, &rect, FALSE);
	}
}

BOOL ToolbarItem::SetRect(const RECT *prc) 
{ 
	return CopyRect(&rect, prc);
}

BOOL ToolbarItem::GetRect(RECT *prc) 
{ 
	return CopyRect(prc, &rect);
}

BOOL ToolbarItem::OffsetRect(INT dx, INT dy) 
{ 
	return ::OffsetRect(&rect, dx, dy);
}

BOOL ToolbarItem::SetRectEmpty() 
{ 
	return ::SetRectEmpty(&rect);
}

BOOL ToolbarItem::IsRectEmpty() 
{ 
	return ::IsRectEmpty(&rect);
}

BOOL ToolbarItem::PtInRect(POINT pt) 
{ 
	return ::PtInRect(&rect, pt);
}

BOOL ToolbarItem::PtInItem(POINT pt)
{
	return (pt.x >= rect.left && pt.x < rect.right && rect.bottom != rect.top);
}

BOOL ToolbarItem::IntersectRect(RECT *prcDst, const RECT *prcSrc) 
{ 
	return ::IntersectRect(prcDst, &rect, prcSrc); 
}

BOOL ToolbarItem::IsEqual(LPCSTR pszName, INT cchName) 
{ 
	return (NULL != name && CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, pszName, cchName, name, -10)); 
}
BOOL ToolbarItem::SetDescription(HWND hToolbar, LPCWSTR pszDescription)
{
	Plugin_FreeResString(description);
	description = Plugin_DuplicateResString(pszDescription);
	return TRUE;
}

HRESULT ToolbarItem::GetText(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return Plugin_CopyResString(pszBuffer, cchBufferMax, text);
}

HRESULT ToolbarItem::GetDescription(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return Plugin_CopyResString(pszBuffer, cchBufferMax, description);
}