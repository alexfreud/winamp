#ifndef WINAMP_ASSOCIATIONS_SETUP_PAGE_HEADER
#define WINAMP_ASSOCIATIONS_SETUP_PAGE_HEADER

#include "./ifc_setuppage.h"
#include <commctrl.h>

#define TYPE_CATEGORIES_NUM			5

class setup_page_assoc : public ifc_setuppage
{
public:
	setup_page_assoc();
	virtual ~setup_page_assoc();

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
	void OnSize(UINT nType, INT cx, INT cy);
	void OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl);
	BOOL OnNotify(INT nCtrlID, NMHDR *pnmh, LRESULT *pResult);

	BOOL TreeView_OnClick(NMHDR *pnmh);
	void TreeView_OnItemStateClick(HWND hwndTree, HTREEITEM hItem);
	BOOL TreeView_OnKeyDown(NMTVKEYDOWN *ptvkd);
	INT TreeView_OnCustomDraw(NMTVCUSTOMDRAW *ptvcd);

	void UpdateUI(void);

private:
	size_t ref;
	HWND hwnd;
	wchar_t *pszTypes;
	WORD *pMeta;
	BYTE expanded[TYPE_CATEGORIES_NUM];
	wchar_t szTopExt[32];
	wchar_t szCaretExt[32];
	BOOL bRegCD;
	BOOL bAgent;
	BOOL bExplorerMenu;
	
protected:
	friend static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	RECVS_DISPATCH;
};

#endif //WINAMP_ASSOCIATIONS_SETUP_PAGE_HEADER