#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPRECORD_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPRECORD_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./setupListbox.h"

class ifc_omservice;
class ifc_omstorageasync;
class SetupLog;

class SetupRecord :	public SetupListboxItem
					
{
protected:
	typedef enum
	{
		recordSelected = 0x0001,
		recordDownloaded = 0x0002,
		checkboxHighlighted = 0x0100,
		checkboxPressed = 0x0200,
	} RecordFlags;

protected:
	SetupRecord(ifc_omservice *serviceToUse);
	~SetupRecord();

public:
	static SetupRecord *CreateInstance(ifc_omservice *serviceToUse);

public:
	ULONG AddRef();
	ULONG Release();

	ifc_omservice *GetService() { return service; }
	
	BOOL IsModified();
	BOOL IsSelected();
	void SetSelected(BOOL fSelected);

	HRESULT GetServiceName(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetDisplayName(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT Save(SetupLog *log);
	HRESULT DownloadDetails();
		
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
	BOOL IsDisabled() { return FALSE; }
	void Command(SetupListbox *instance, INT commandId, INT eventId) {}
	HWND CreateDetailsView(HWND hParent);
	BOOL GetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax);
	

protected:
	BOOL AdjustCheckboxRect(SetupListbox *instance, RECT *prcItem);
	void GetColors(HDC hdc, UINT state, COLORREF *rgbBkOut, COLORREF *rgbTextOut);
	void InvertCheckbox(SetupListbox *instance, const RECT *prcItem);
	void OnDownloadCompleted();

private:
	friend static void CALLBACK SetupRecord_ServiceDownloadedCallback(ifc_omstorageasync *result);

protected:
	ULONG ref;
	ifc_omservice	*service;
	ifc_omstorageasync *async;
	CRITICAL_SECTION lock;
	UINT flags;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPRECORD_HEADER