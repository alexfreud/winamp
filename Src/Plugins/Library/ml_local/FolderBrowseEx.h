#ifndef NULLSOFT_FOLDERBROWSE_EXTENDED_DIALOG_HEADER
#define NULLSOFT_FOLDERBROWSE_EXTENDED_DIALOG_HEADER

#include <windows.h>
#include <shlobj.h>

/// Standart controls 
#define IDC_TV_FOLDERS		0x3741
#define IDC_SB_GRIPPER		0x3747
#define IDC_LBL_FOLDER		0x3748
#define IDC_LBL_CAPTION		0x3742
#define IDC_EDT_PATH		0x3744


typedef struct _BFPATH
{
	int		empty;
	int		type;  
	void	*value;
}BFPATH;

class FolderBrowseEx
{
public:
	FolderBrowseEx(LPCITEMIDLIST pidlRoot, UINT ulFlags, LPCWSTR lpszCaption, LPCWSTR lpszTitle);
	FolderBrowseEx(UINT ulFlags, LPCWSTR lpszCaption, LPCWSTR lpszTitle);
	FolderBrowseEx(UINT ulFlags, LPCWSTR lpszTitle);
	FolderBrowseEx(void);
	virtual ~FolderBrowseEx(void);

public:
	virtual LPITEMIDLIST Browse(HWND hwndOwner);
	
	LPITEMIDLIST GetPIDL(void) { return pidl; }
	INT GetImage(void) { return image; }
	LPCWSTR GetDislpayName(void) { return  pszDisplayName; }
	HRESULT ParseDisplayName(LPCWSTR lpszPath, IBindCtx *pbc, LPITEMIDLIST *ppidl, SFGAOF sfgaoIn, SFGAOF *psfgaoOut);
	HWND GetDlgItem(int nIDDlgItem) { return ::GetDlgItem(hwnd, nIDDlgItem); }
	
	void SetRoot(LPCITEMIDLIST pidlRoot) { this->pidlRoot = pidlRoot; }
	void SetFlags(UINT ulFlags) { this->ulFlags = ulFlags; }
	void SetSelection(LPCITEMIDLIST pidlSelect);
	void SetSelection(LPCWSTR lpszSelect);
	void SetExpanded(LPCITEMIDLIST pidlExpand);
	void SetExpanded(LPCWSTR lpszExpand);
	void SetCaption(LPCWSTR lpszCaption);
	void SetTitle(LPCWSTR lpszTitle); 
	

protected:
	HWND GetHandle(void) { return hwnd; }
	INT_PTR CallWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetDialogResult(LRESULT result);
	LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return ::SendMessageW(hwnd, uMsg, wParam, lParam); }
	void SetWindowText(LPCWSTR lpText) { ::SetWindowTextW(hwnd, lpText); }

	void EnableOK(BOOL enable) { SendMessage(BFFM_ENABLEOK, 0, (LPARAM)enable); }
	void SetOKText(LPCWSTR lpText) { SendMessage(BFFM_SETOKTEXT, 0, (LPARAM)lpText); }
	void SetStatusText(LPCWSTR lpText)  { SendMessage(BFFM_SETSTATUSTEXTW, 0, (LPARAM)lpText); }
	
	virtual void OnInitialized(void);
	virtual void OnIUnknown(IUnknown *lpiu) {}
	virtual void OnSelectionChanged(LPCITEMIDLIST pidl);
	virtual BOOL OnValidateFailed(LPCWSTR lpName) { return FALSE; }
	virtual void OnSelectionDone(LPCITEMIDLIST pidl) { }

	virtual INT BrowseCallback(UINT uMsg, LPARAM lParam);
	virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	LPCITEMIDLIST	pidlRoot;
	
	BFPATH			pathSelection;
	BFPATH			pathExpanded;

    LPWSTR			lpszCaption;
	LPWSTR			lpszTitle;
	UINT			ulFlags;
	INT				image;
	WCHAR			pszDisplayName[MAX_PATH];
	BOOL			expand;
	LPITEMIDLIST		pidl;


private:
	HWND				hwnd;
	LONG_PTR		oldProc;
	


	friend static int WINAPI BrowseCallback_Helper(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
	friend static LRESULT WINAPI WindowProc_Helper(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend static void Initialize(FolderBrowseEx *lpfbx, LPCITEMIDLIST pidlRoot, UINT ulFlags, LPCWSTR lpszCaption, LPCWSTR lpszTitle);

};

#endif //NULLSOFT_FOLDERBROWSE_EXTENDED_DIALOG_HEADER