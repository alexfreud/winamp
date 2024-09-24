#ifndef WINAMP_FEEDBACK_SETUP_PAGE_HEADER
#define WINAMP_FEEDBACK_SETUP_PAGE_HEADER

#include "./ifc_setuppage.h"

class setup_page_feedback : public ifc_setuppage
{

public:
	setup_page_feedback();
	virtual ~setup_page_feedback();

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

	void ComboBox_OnSelChange(HWND hwndCtrl);
	void UpdateUI(void);

private:
	size_t ref;
	HWND hwnd;
	WCHAR szEmail[MAX_PATH];
	WCHAR szCountry[MAX_PATH];
	INT gender;
	DWORD flags;

protected:
	friend static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	RECVS_DISPATCH;
};


#endif //WINAMP_FEEDBACK_SETUP_PAGE_HEADER