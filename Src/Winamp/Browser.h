#ifndef NULLSOFT_BROWSERH
#define NULLSOFT_BROWSERH

#include "../nu/HTMLContainer.h"
#include "wa_ipc.h"

class Browser : public HTMLContainer
{
public:
	Browser();
	~Browser();
	static WNDCLASS *wc;
	static HRESULT CALLBACK WindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void NavigateToName(LPCTSTR pszUrl);
	void Resized(unsigned long width, unsigned long height);
	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
	void ToggleVisible(int showing);
	embedWindowState state;
	bool minimised;
	void SetMenuCheckMark();
	virtual void OnNavigateComplete();
	HWND CreateHWND();
	DWORD threadId;
};

class UpdateBrowser : public HTMLContainer
{
public:
	HWND CreateHWND();
	static WNDCLASS *wc;
	static HRESULT CALLBACK WindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void NavigateToName(LPCTSTR pszUrl);
	void Resized(unsigned long width, unsigned long height);
	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
	embedWindowState state;
	virtual void OnNavigateComplete();

};

HRESULT UpdateWindow_Show(LPCSTR pszUrl);

#endif