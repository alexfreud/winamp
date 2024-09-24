#ifndef WINAMP_SKIN_SETUP_PAGE_HEADER
#define WINAMP_SKIN_SETUP_PAGE_HEADER

#include "./ifc_setuppage.h"

class setup_page_skin : public ifc_setuppage
{
public:
	setup_page_skin();
	virtual ~setup_page_skin();

public:
	size_t AddRef();
	size_t Release();
	HRESULT GetName(bool bShort, const wchar_t **pszName);
	HRESULT Save(HWND hwndText);
	HRESULT CreateView(HWND hwndParent, HWND *phwnd);
	HRESULT Revert(void);
	HRESULT IsDirty(void);
	HRESULT Validate(void);
protected:
	INT_PTR PageDlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnDestroy(void);
	INT_PTR OnColorStatic(HDC hdc, HWND hwndCtrl);
	INT_PTR OnDrawItem(INT nCtrlID, DRAWITEMSTRUCT *pdis);
	INT_PTR OnMeasureItem(INT nCtrlID, MEASUREITEMSTRUCT *pmis);
	void OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl);
	INT_PTR OnDeleteItem(INT nCtrlID, DELETEITEMSTRUCT *pdis);
	INT_PTR OnCompareItem(INT nCtrlID, COMPAREITEMSTRUCT *pcis);
	INT_PTR OnVKeyToItem(WORD vKey, INT index, HWND hwndCtrl);
	INT_PTR OnCharToItem(WORD vKey, INT index, HWND hwndCtrl);

	void ListBox_OnDrawItem(DRAWITEMSTRUCT *pdis);
	void ListBox_OnSelChange(HWND hwndCtrl);
	void ListBox_OnItemDelete(DELETEITEMSTRUCT *pdis);
	INT_PTR ListBox_OnItemCompare(COMPAREITEMSTRUCT *pcis);
	INT_PTR ListBox_OnVKeyToItem(HWND hwndLB, WORD vKey, INT index);
	INT_PTR ListBox_OnCharToItem(HWND hwndLB, WORD vKey, INT index);

private:
	size_t ref;
	HWND hwnd;
	HFONT hfLink;
	int idxSelected; // internal crap
	wchar_t szSelectionPath[MAX_PATH];
protected:
	friend static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	RECVS_DISPATCH;
};

#endif //WINAMP_SKIN_SETUP_PAGE_HEADER