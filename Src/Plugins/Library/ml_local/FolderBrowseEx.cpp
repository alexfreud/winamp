#include "main.h"
#include "./folderbrowseex.h"
#include "../nu/CGlobalAtom.h"
#include <shobjidl.h>
#include "../replicant/nu/AutoWide.h"

static CGlobalAtom CLSPROP(L"FBEXDLG");

#ifdef _WIN64
#define LONGPTR_CAST LONG_PTR
#else
#define LONGPTR_CAST LONG
#endif

#define PATHTYPE_PIDL	FALSE
#define PATHTYPE_STRING	TRUE

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassName(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpi(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

static int WINAPI BrowseCallback_Helper(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	FolderBrowseEx *lpfbex = reinterpret_cast<FolderBrowseEx*>(lpData);
	if (!lpfbex) return 0;

	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			lpfbex->hwnd = hwnd;
			if(SetPropW(hwnd, CLSPROP, (void*)lpData))
			{
				lpfbex->oldProc = (LONG_PTR) ((IsWindowUnicode(hwnd)) ? SetWindowLongPtrW(hwnd, DWLP_DLGPROC, (LONGPTR_CAST)WindowProc_Helper) : SetWindowLongPtrA(hwnd, DWLP_DLGPROC, (LONGPTR_CAST)WindowProc_Helper));
				if (NULL == lpfbex->oldProc) RemovePropW(hwnd, CLSPROP);
			}

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows(hwnd, browseEnumProc, 0);
			break;
	}
	return lpfbex->BrowseCallback(uMsg, lParam);
}

static LRESULT WINAPI WindowProc_Helper(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FolderBrowseEx *lpfbex = static_cast<FolderBrowseEx*>(GetPropW(hwnd, CLSPROP));
	if (!lpfbex) return 0; 

	switch(uMsg)
	{
		case WM_NCDESTROY:
			lpfbex->DialogProc(uMsg, wParam, lParam);
			if (IsWindowUnicode(hwnd)) SetWindowLongPtrW(hwnd, DWLP_DLGPROC, (LONGPTR_CAST)lpfbex->oldProc); 			
			else SetWindowLongPtrA(hwnd, DWLP_DLGPROC, (LONGPTR_CAST)lpfbex->oldProc);
			RemovePropW(hwnd, CLSPROP);
			lpfbex->oldProc = NULL;
			lpfbex->hwnd = NULL;
			return 0;
	}
	return lpfbex->DialogProc(uMsg, wParam, lParam);
}

static void Initialize(FolderBrowseEx *lpfbx, LPCITEMIDLIST pidlRoot, UINT ulFlags, LPCWSTR lpszCaption, LPCWSTR lpszTitle)
{
	lpfbx->pidlRoot = pidlRoot;
	lpfbx->ulFlags = ulFlags;

	lpfbx->lpszCaption = NULL;
	lpfbx->lpszTitle = NULL;
	lpfbx->image = -1;
	lpfbx->pidl = NULL;
	lpfbx->hwnd = NULL;
	lpfbx->oldProc = NULL;

	CoInitialize(NULL);	

	lpfbx->pathExpanded.empty = TRUE;
	lpfbx->pathSelection.empty = TRUE;

	if (lpszCaption) lpfbx->SetCaption(lpszCaption);
	if (lpszTitle) lpfbx->SetTitle(lpszTitle);
}

FolderBrowseEx::FolderBrowseEx(LPCITEMIDLIST pidlRoot, UINT ulFlags, LPCWSTR lpszCaption, LPCWSTR lpszTitle)
{
	Initialize(this, pidlRoot, ulFlags, lpszCaption, lpszTitle);
}

FolderBrowseEx::FolderBrowseEx(UINT ulFlags, LPCWSTR lpszCaption, LPCWSTR lpszTitle)
{
	Initialize(this, NULL, ulFlags, lpszCaption, lpszTitle);
}

FolderBrowseEx::FolderBrowseEx(UINT ulFlags, LPCWSTR lpszTitle)
{
	Initialize(this, NULL, ulFlags, L"", lpszTitle);
}

FolderBrowseEx::FolderBrowseEx(void) 
{
	Initialize(this, NULL, 0, L"", L"");
}

FolderBrowseEx::~FolderBrowseEx(void)
{
	if (pidl) CoTaskMemFree(pidl);
	if (lpszTitle) free(lpszTitle);
	if (lpszCaption) free(lpszCaption);
	CoUninitialize();
}

HRESULT FolderBrowseEx::ParseDisplayName(LPCWSTR lpszPath, IBindCtx *pbc, LPITEMIDLIST *ppidl, SFGAOF sfgaoIn, SFGAOF *psfgaoOut)
{
	IShellFolder *isf = NULL;
	HRESULT	result;
	SFGAOF attrib;

	if (NOERROR != (result = SHGetDesktopFolder(&isf))) return result;
	attrib = sfgaoIn;
	result = isf->ParseDisplayName(NULL, pbc, (LPWSTR)lpszPath, NULL, ppidl, &attrib);
	isf->Release();

	if (S_OK != result) *ppidl = NULL;
	else if (psfgaoOut) *psfgaoOut = attrib;
	return result;
}

void FolderBrowseEx::SetExpanded(LPCITEMIDLIST pidlExpand)
{
	pathExpanded.empty = FALSE;
	pathExpanded.type = PATHTYPE_PIDL;
	pathExpanded.value = (void*)pidlExpand;
	if (hwnd) SendMessage(BFFM_SETEXPANDED, pathExpanded.type, (LPARAM)pathExpanded.value);
}

void FolderBrowseEx::SetExpanded(LPCWSTR lpszExpand)
{
	pathExpanded.empty = FALSE;
	pathExpanded.type = PATHTYPE_STRING;
	pathExpanded.value = (void*)lpszExpand;
	if (hwnd) SendMessage(BFFM_SETEXPANDED, pathExpanded.type, (LPARAM)pathExpanded.value);
}

void FolderBrowseEx::SetSelection(LPCITEMIDLIST pidlSelect)
{
	pathSelection.empty = FALSE;
	pathSelection.type = PATHTYPE_PIDL;
	pathSelection.value = (void*)pidlSelect;
	if (hwnd) SendMessage(BFFM_SETSELECTIONW, pathSelection.type, (LPARAM)pathSelection.value);
}

void FolderBrowseEx::SetSelection(LPCWSTR lpszSelect)
{
	pathSelection.empty = FALSE;
	pathSelection.type = PATHTYPE_STRING;
	pathSelection.value = (void*)lpszSelect;
	if (hwnd) SendMessage(BFFM_SETSELECTIONW, pathSelection.type, (LPARAM)pathSelection.value);
}

void FolderBrowseEx::SetCaption(LPCWSTR lpszCaption)
{
	if (this->lpszCaption) free(this->lpszCaption);
	this->lpszCaption = _wcsdup((lpszCaption) ? lpszCaption : L"");
	if (hwnd) SetWindowText(lpszCaption);
}

void FolderBrowseEx::SetTitle(LPCWSTR lpszTitle)
{
	if (this->lpszTitle) free(this->lpszTitle);
	this->lpszTitle = _wcsdup((lpszTitle) ? lpszTitle : L"");
}

LPITEMIDLIST FolderBrowseEx::Browse(HWND hwndOwner)
{
	BROWSEINFOW bi = {0};
	bi.hwndOwner = hwndOwner;
	bi.pidlRoot = pidlRoot; 
	bi.pszDisplayName = pszDisplayName;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = ulFlags;
	bi.lpfn = BrowseCallback_Helper;
	bi.lParam = (LPARAM)this;

	pidl = SHBrowseForFolderW(&bi);
	if (pidl) OnSelectionDone(pidl);

	image = (pidl) ? bi.iImage : -1;

	return pidl;
}

INT FolderBrowseEx::BrowseCallback(UINT uMsg, LPARAM lParam)
{
	switch(uMsg)
	{
		case BFFM_INITIALIZED:		OnInitialized(); break;
		case BFFM_IUNKNOWN:			OnIUnknown((IUnknown*)lParam); break;
		case BFFM_SELCHANGED:		OnSelectionChanged((LPCITEMIDLIST)lParam); break;
		case BFFM_VALIDATEFAILEDA:  return OnValidateFailed(AutoWide((LPCSTR)lParam));
		case BFFM_VALIDATEFAILEDW:	return OnValidateFailed((LPCWSTR)lParam);
	}
	return 0;
}

void FolderBrowseEx::OnInitialized(void)
{
	if (!pathSelection.empty) SendMessage(BFFM_SETSELECTIONW, pathSelection.type, (LPARAM)pathSelection.value);
	if (!pathExpanded.empty) SendMessage(BFFM_SETEXPANDED, pathExpanded.type, (LPARAM)pathExpanded.value);
	SetWindowText(lpszCaption);
}

void FolderBrowseEx::OnSelectionChanged(LPCITEMIDLIST pidl)
{
}

INT_PTR FolderBrowseEx::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(uMsg, wParam, lParam);
}

INT_PTR FolderBrowseEx::CallWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	return (IsWindowUnicode(hwnd))	? ::CallWindowProcW((WNDPROC)oldProc, hwnd, uMsg, wParam, lParam) 
									: ::CallWindowProcA((WNDPROC)oldProc, hwnd, uMsg, wParam, lParam); 
}

void FolderBrowseEx::SetDialogResult(LRESULT result)
{
	SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG)(LONG_PTR)result); 
}