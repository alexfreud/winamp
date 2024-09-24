#ifndef NULLSOFT_FOLDERBROWSE_SCANFILES_DIALOG_HEADER
#define NULLSOFT_FOLDERBROWSE_SCANFILES_DIALOG_HEADER

#include "./folderbrowseex.h"

typedef struct _FBUTTON FBUTTON;
class ScanFolderBrowser : public FolderBrowseEx
{
public:
	ScanFolderBrowser(void);
	ScanFolderBrowser(BOOL showBckScanOption);
	virtual ~ScanFolderBrowser(void);

	void ShowBckScanOption(BOOL show) { bkScanShow = show; }
	void SetBckScanChecked(BOOL checked) { bkScanChecked = checked; }
	BOOL GetBckScanChecked(void) { return bkScanChecked; } 
protected:
	virtual void OnInitialized(void);
	virtual void OnSelectionChanged(LPCITEMIDLIST pidl);
	virtual BOOL OnValidateFailed(LPCWSTR lpName);
	virtual void OnSelectionDone(LPCITEMIDLIST pidl);
	virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnWindowPosChanging(WINDOWPOS *lpwp);
	void OnSize(UINT nType, int cx, int cy);
	BOOL OnNotify(UINT idCtrl, LPNMHDR pnmh, LRESULT *result);
	BOOL OnCommand(UINT idCtrl, UINT idEvnt, HWND hwndCtrl);
	LRESULT OnToolBarCustomDraw(LPNMTBCUSTOMDRAW pnmcd);
	void OnToolTipGetDispInfo(LPNMTTDISPINFOW lpnmtdi);

private:
	void LoadBookmarks(void);
	void FreeBookmarks(void);
	void ShiftWindows(int cx);
	void ShrinkWindows(int cx);
	void RepositionWindows(void);

private:
	FBUTTON *buttons;
	int		buttonsCount;
	HBRUSH	hbBorder;
	BOOL	bkScanChecked;
	BOOL	bkScanShow;

	IAutoComplete	*pac;
	IACList2			*pacl2;

	wchar_t selectionPath[MAX_PATH]; // this is here only because i'm lazy

	friend static void Initialize(ScanFolderBrowser *browser, BOOL showBckScan, BOOL checkBckScan);
};

#endif //NULLSOFT_FOLDERBROWSE_SCANFILES_DIALOG_HEADER