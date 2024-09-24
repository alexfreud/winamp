#ifndef WINAMP_LANGUAGE_SETUP_PAGE_HEADER
#define WINAMP_LANGUAGE_SETUP_PAGE_HEADER

#include "./ifc_setuppage.h"

class setup_page_lang : public ifc_setuppage
{

public:
	setup_page_lang();
	virtual ~setup_page_lang();

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
	void OnSize(UINT nType, INT cx, INT cy);
	void OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl);
	INT_PTR OnDeleteItem(INT nCtrlID, DELETEITEMSTRUCT *pdis);
	void ListBox_OnItemDelete(DELETEITEMSTRUCT *pdis);
	void ListBox_OnSelChange(HWND hwndCtrl);
	
private:
	size_t ref;
	HWND hwnd;
	wchar_t szSelectionPath[MAX_PATH];

protected:
	friend static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	RECVS_DISPATCH;
};


#endif //WINAMP_LANGUAGE_SETUP_PAGE_HEADER