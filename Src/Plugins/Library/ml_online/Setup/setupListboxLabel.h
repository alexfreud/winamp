#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLISTBOX_LABEL_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLISTBOX_LABEL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./setupListbox.h"


class SetupListboxLabel: public SetupListboxItem
{

protected:
	SetupListboxLabel(LPCWSTR pszName);
	~SetupListboxLabel();

public:
	static SetupListboxLabel *CreateInstance(LPCWSTR pszNamee);

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT GetName(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT SetName(LPCWSTR pszName);
	BOOL IsNameNull();

	void GetColors(HDC hdc, UINT state, COLORREF *rgbBkOut, COLORREF *rgbTextOut);
	HBRUSH GetBrush(HDC hdc, UINT state);


	/* SetupListboxItem */ 
	BOOL MeasureItem(SetupListbox *instance, UINT *cx, UINT *cy);
	BOOL DrawItem(SetupListbox *instance, HDC hdc, const RECT *prc, UINT state);
	INT_PTR KeyToItem(SetupListbox *instance, const RECT *prcItem, INT vKey);
	BOOL MouseMove(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt);
	BOOL MouseLeave(SetupListbox *instance, const RECT *prcItem);
	BOOL LButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt);
	BOOL LButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt);
	BOOL LButtonDblClk(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt);
	BOOL RButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt);
	BOOL RButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt);
	void CaptureChanged(SetupListbox *instance, const RECT *prcItem, SetupListboxItem *captured);
	BOOL IsDisabled() { return TRUE; }
	void Command(SetupListbox *instance, INT commandId, INT eventId) {}
	HWND CreateDetailsView(HWND hParent) { return NULL; }
	BOOL GetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax);


protected:
	ULONG ref;
	LPWSTR name;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLISTBOX_LABEL_HEADER