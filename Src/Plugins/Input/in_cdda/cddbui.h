#ifndef NULLSOFT_CDDB_UI_HEADER
#define NULLSOFT_CDDB_UI_HEADER


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

#define AUTOCLOSE_NOW		0x00000000
#define AUTOCLOSE_NEVER		0xFFFFFFFF


#define STATE_INACTIVE		((UINT)0)
#define STATE_ACTIVE		((UINT)1)
#define STATE_COMPLETED		((UINT)2)
#define STATE_ABORTING		((UINT)3)

typedef void (CALLBACK *CDDBDLG_ONBTNCLICK)(HWND /*hwndDlg*/, BSTR /*bstrUser*/); // return TRUE to close dialog or FALSE to stay in STATE_ABORTING

// all functions can accept String IDS and will resolve it using WASABI_API_LNGSTRINGW
HWND CddbProgressDlg_Create(HWND hwndParent, INT nCmdShow);
BOOL CddbProgressDlg_Initialize(HWND hwnd, LPCWSTR pszCaption, CDDBDLG_ONBTNCLICK fnOnAbort, BSTR bstrAbortUser); // 
BOOL CddbProgressDlg_Completed(HWND hwnd, LPCWSTR pszResult, LPCWSTR pszReason, DWORD nAutoCloseDelay, HRESULT rCode); 
BOOL CddbProgressDlg_SetStatus(HWND hwnd, LPCWSTR pszStatus, INT nPercentCompleted);
BOOL CddbProgressDlg_EnableAbortButton(HWND hwnd, BOOL bEnable);
BOOL CddbProgressDlg_ShowButton1(HWND hwnd, LPCWSTR pszCaption, CDDBDLG_ONBTNCLICK fnOnButton1, BSTR bstrUser); // set pszCaption = NULL and/or fnOnButton1 = NULL to hide it
UINT CddbProgressDlg_GetState(HWND hwnd);
BOOL CddbProgressDlg_SetUserData(HWND hwnd, HANDLE user);
HANDLE CddbProgressDlg_GetUserData(HWND hwnd);
BOOL CddbProgressDlg_ShowInTaskbar(HWND hwnd, BOOL bShow);
BOOL CddbProgressDlg_SetExtendedMode(HWND hwnd, BOOL bEnable);
BOOL CddbProgressDlg_AddRecord(HWND hwnd, LPCWSTR pszArtist, LPCWSTR pszTitle, LPCWSTR pszLanguage);
INT CddbProgressDlg_GetSelRecordIndex(HWND hwnd);
HRESULT CddbProgressDlg_DoModal(HWND hwnd, RECT *prc); // if prc != NULL will contain window rect before it closed
BOOL CddbProgressDlg_ExitModal(HWND hwnd, HRESULT rCode, BOOL bDestroy); /// exits modal loop without destroying window
BOOL CddbProgressDlg_IsModal(HWND hwnd);


#define FINDWND_ONLY_VISIBLE		0x01
#define FINDWND_ONLY_ENABLED		0x02

BOOL FindAllOwnedWindows(HWND hwndHost, HWND *hwndList, INT cList, UINT flags);

#endif //NULLSOFT_CDDB_UI_HEADER
