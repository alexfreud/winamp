#ifndef NULLSOFT_WINAMP_SETUP_HEADER
#define NULLSOFT_WINAMP_SETUP_HEADER

#include "./svc_setup.h"
#include <vector>

typedef struct _UI UI;

class WASetup : svc_setup
{
protected:
	WASetup(void);
	~WASetup(void);
public:
	static svc_setup *CreateInstance();
public:
	int AddRef(void);
	int Release(void);
	HRESULT InsertPage(ifc_setuppage *pPage, int*pIndex);
	HRESULT RemovePage(size_t index);
	HRESULT GetPageCount(int*pCount);
	HRESULT GetPage(size_t index, ifc_setuppage **pPage);
	HRESULT AddJob(ifc_setupjob *pJob);
	HRESULT RemoveJob(ifc_setupjob *pJob);
	HRESULT GetActiveIndex(int*pIndex);
	HRESULT Start(HWND hwndWinamp);
	HRESULT CreateStatusWnd(HWND *phwndStatus);
	HRESULT Save(HWND hwndStatus);
	HRESULT ExecJobs(HWND hwndStatus);
	HRESULT GetWinampWnd(HWND *phwndWinamp);

protected:
	INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnInitDialog(HWND hwndFocused, LPARAM lParam);
	void OnDestroy(void);
	void OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl);
	void OnCancel(void); // use it if you want prompt user first
	void OnCancel_Clicked(void);
	void OnNext_Clicked(HWND hwndCtrl);
	void OnBack_Clicked(HWND hwndCtrl);
	void OnNavigation_SelChange(HWND hwndCtrl);
	INT_PTR OnDrawItem(INT nCtrlID, DRAWITEMSTRUCT *pdis);
	INT_PTR OnMeasureItem(INT nCtrlID, MEASUREITEMSTRUCT *pmis);
	void OnDrawHeader(DRAWITEMSTRUCT *pdis);
	void OnDrawNavigationItem(DRAWITEMSTRUCT *pdis);
	INT_PTR OnColorListBox(HDC hdc, HWND hwndCtrl);

private:
	int ref;
	HWND hwnd;
	std::vector<ifc_setuppage*> pageList;
	std::vector<ifc_setupjob*> jobList;
	HWND hwndActive;
	size_t nPageActive;
	UI		*pui;
	RECT	rcUI;
	HWND	 hWinamp;
	
protected:
	friend static INT_PTR WINAPI DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	RECVS_DISPATCH;

};


#endif //WINAMP_SETUP_WIZARD_HEADER