#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUP_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./setupRecord.h"
#include "./setupListbox.h"
#include <vector>

class SetupListboxLabel;
class SetupLog;
class SetupPage;
class ifc_omstorage;

class SetupGroup : public SetupListboxItem
{
public:
	typedef enum
	{
		styleDefaultUnsubscribed = 0x00000001,
		styleDefaultSubscribed = 0x00000002,
		styleSortAlphabetically = 0x00000008,
		styleSaveAll = 0x00000010,
	} GroupStyles;
protected:
	typedef enum
	{
		flagCollapsed = 0x0001,
		flagMenuActive = 0x0002,
		flagLoading = 0x0004,
	} GroupFlags;

protected:
	SetupGroup(INT groupId, LPCWSTR pszName, LPCWSTR pszAddress, const GUID *storageId, const GUID *filterId, UINT fStyle);
	~SetupGroup();

public:
	static SetupGroup *CreateInstance(INT groupId, LPCWSTR pszName, LPCWSTR pszAddress, const GUID *storageId, const GUID *filterId, UINT fStyle);

public:
	ULONG AddRef();
	ULONG Release();

	INT GetId() { return id; }
	HRESULT GetName(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT GetLongName(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT GetDescription(LPWSTR pszBuffer, INT cchBufferMax);

	size_t GetRecordCount();
	SetupRecord *GetRecord(size_t index) { return list[index]; }

	size_t GetListboxCount();
	SetupListboxItem *GetListboxItem(size_t index);

	BOOL IsModified();
	
	BOOL IsExpanded();
	void SetExpanded(BOOL fExpanded);
	void SelectAll(SetupListbox *instance, BOOL fSelect);

	HRESULT RequestReload();
	HRESULT Save(SetupLog *log);


	void SetEmptyText(LPCWSTR pszText, BOOL fInvalidate);
	void SetLongName(LPCWSTR pszText);
	void SetDescription(LPCWSTR pszText);
	
	void GetColors(HDC hdc, UINT state, COLORREF *rgbBkOut, COLORREF *rgbTextOut);
	HBRUSH GetBrush(HDC hdc, UINT state);

	HRESULT SignalLoadCompleted(HANDLE event);
	void ValidateSelection(SetupListbox *instance);

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
	void Command(SetupListbox *instance, INT commandId, INT eventId);
	HWND CreateDetailsView(HWND hParent);
	BOOL GetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax);

	void SetError(HRESULT code) { errorCode = code; }
	HRESULT GetError() { return errorCode; }

	void Clear(BOOL fInvalidate);

	void SetPageWnd(HWND hPage);

protected:
	void InvertExpanded(SetupListbox *instance);
	void OnLoadCompleted();
	

private:	
	friend static void CALLBACK SetupGroup_LoadCallback(ifc_omstorageasync *result);
	

protected:
	ULONG ref;
	INT id;
	LPWSTR name;
	LPWSTR longName;
	LPWSTR description;
	UINT style;
	UINT flags;
	LPWSTR address;
	GUID storageId;
	GUID filterId;
	HRESULT errorCode;
	std::vector<SetupRecord*> list;
	SetupListboxLabel *emptyLabel;
	CRITICAL_SECTION lock;
	ifc_omstorageasync *loadResult;
	HWND hPage;
	HANDLE loadComplete;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUP_HEADER