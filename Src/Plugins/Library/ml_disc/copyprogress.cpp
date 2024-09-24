#include "main.h"
#include "./copyfiles.h"
#include "./copyinternal.h"
#include "./resource.h"
#include "./settings.h"
#include "../nu/trace.h"
#include <shlwapi.h>
#include <strsafe.h>

typedef struct _PROGDLG
{
	COPYDATA			*pCopyData;
	HBITMAP			hbmpLogo;
} PROGDLG;

#define PROGDLG_PROP		TEXT("PROGDLG")
#define GetProgDlg(__hdlg)	((PROGDLG*)GetProp((__hdlg), PROGDLG_PROP))

#define SetControlText(__hwnd, __ctrlId, __pszText)\
	SetDlgItemText((__hwnd), (__ctrlId), (IS_INTRESOURCE(__pszText) ? WASABI_API_LNGSTRINGW((UINT)(UINT_PTR)(__pszText)) : (__pszText)))

#define SetTaskText(__hwnd, __pszText) SetControlText(__hwnd, IDC_LBL_TASK, __pszText)
#define SetOperationText(__hwnd, __pszText) SetControlText(__hwnd, IDC_LBL_OPERATION, __pszText)


static INT_PTR CopyProgress_OnInitDialog(HWND hdlg, HWND hFocus, LPARAM lParam)
{
	HWND hctrl;
	PROGDLG *ppd = (PROGDLG*)calloc(1, sizeof(PROGDLG));
	if (!ppd) return 0;

	SetProp(hdlg, PROGDLG_PROP, ppd); 
	ppd->pCopyData = (COPYDATA*)lParam;
	CopyFiles_AddRef(ppd->pCopyData);
	if (ppd->pCopyData && ppd->pCopyData->hOwner)
	{
		RECT rw;
		if (!GetWindowRect(ppd->pCopyData->hOwner, &rw)) SetRect(&rw, 0, 0, 0, 0);
		if (hdlg && rw.left != rw.right)
		{
			RECT rw2;
			GetWindowRect(hdlg, &rw2);
			SetWindowPos(hdlg, HWND_TOP, 
						rw.left + ((rw.right - rw.left) - (rw2.right - rw2.left))/2, 
						rw.top + ((rw.bottom - rw.top) - (rw2.bottom - rw2.top))/2,
						0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}

	hctrl = GetDlgItem(hdlg, IDC_PRG_TOTAL);
	if (NULL != hctrl)
	{
		SendMessage(hctrl, PBM_SETRANGE32, (WPARAM)0, (LPARAM)100);
		SendMessage(hctrl, PBM_SETPOS, (WPARAM)0, 0L);
		SendMessage(hctrl, PBM_SETSTEP, (WPARAM)1, 0L);
	}
	
	SetTaskText(hdlg, MAKEINTRESOURCE(IDS_COPY_TASK_PREPARE));
	SetOperationText(hdlg, TEXT(""));
	
	SendMessage(hdlg, DM_REPOSITION, 0, 0L);	

	ppd->hbmpLogo = CopyFiles_LoadResourcePng(MAKEINTRESOURCE(IDB_FILECOPY));
	if (ppd->hbmpLogo) SendDlgItemMessage(hdlg, IDC_PIC_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ppd->hbmpLogo);
	else ShowWindow(GetDlgItem(hdlg, IDC_PIC_LOGO), SW_HIDE);

	return FALSE;
}

static void CopyProgress_OnDestroy(HWND hdlg)
{
	PROGDLG *ppd = GetProgDlg(hdlg);
	RemoveProp(hdlg, PROGDLG_PROP);
	if (ppd)
	{
		if (ppd->pCopyData) CopyFiles_Release(ppd->pCopyData);

		if (ppd->hbmpLogo)
		{
			HBITMAP hbmp = (HBITMAP)SendDlgItemMessage(hdlg, IDC_PIC_LOGO, STM_GETIMAGE, IMAGE_BITMAP, 0L);
			if (hbmp != ppd->hbmpLogo) DeleteObject(hbmp);
			DeleteObject(ppd->hbmpLogo);
		}

		free(ppd);
	}
}

static void ShowErrorBox(HWND hdlg)
{
	PROGDLG *ppd = GetProgDlg(hdlg);
	if (!ppd || !ppd->pCopyData) return;
	
	
	TCHAR szBuffer[2048] = {0}, szFormat[256] = {0}, szUnknown[64] = {0};
	LPTSTR pszMessage;

	if (ERROR_REQUEST_ABORTED == ppd->pCopyData->errorCode) return; // ignore user aborts

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
					ppd->pCopyData->errorCode, 0, (LPTSTR)&pszMessage, 0, NULL);
	
	WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN, szUnknown, ARRAYSIZE(szUnknown));
	WASABI_API_LNGSTRINGW_BUF(IDS_COPY_ERROR_MESSAGE, szFormat, ARRAYSIZE(szFormat));

	StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szFormat,  
					ppd->pCopyData->szDestination,
					(ppd->pCopyData->errorMsgId) ? WASABI_API_LNGSTRINGW(ppd->pCopyData->errorMsgId) : szUnknown,
					ppd->pCopyData->errorCode,
					(pszMessage) ? pszMessage : szUnknown);
	
	MessageBox(hdlg, szBuffer, WASABI_API_LNGSTRINGW(IDS_COPY_ERROR_CAPTION), MB_OK | MB_ICONERROR);
	if (pszMessage) LocalFree(pszMessage);
}

static void CopyProgress_OnDisplayNextFile(HWND hdlg, INT iFile, INT iCount)
{
	PROGDLG *ppd = GetProgDlg(hdlg);
	if (!ppd || !ppd->pCopyData) return;
	SetOperationText(hdlg, PathFindFileName(ppd->pCopyData->ppszFiles[iFile]));
					
}


static INT_PTR CopyProgress_OnDestiantionNotExist(HWND hdlg, LPCTSTR pszDestination)
{
	PROGDLG *ppd = GetProgDlg(hdlg);
	if (!ppd || !ppd->pCopyData) return FALSE;
    QUESTIONBOX qb = {0};
	TCHAR szFormat[MAX_PATH] = {0},  szMessage[MAX_PATH*2] = {0};
	
	WASABI_API_LNGSTRINGW_BUF(IDS_DESTINATION_NOT_EXIST_FORMAT, szFormat, ARRAYSIZE(szFormat));
	StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szFormat, pszDestination);

	qb.hParent =hdlg;
	qb.pszIcon = IDI_QUESTION;
	qb.pszTitle = MAKEINTRESOURCE(IDS_CONFIRM_CREATE_DESTINATION); 
	qb.pszMessage = szMessage;
	qb.pszBtnOkText = MAKEINTRESOURCE(IDS_YES);
	qb.pszBtnCancelText = MAKEINTRESOURCE(IDS_NO);
	qb.uBeepType = MB_ICONEXCLAMATION;
	qb.uFlags = QBF_DEFAULT_OK | QBF_SETFOREGROUND | QBF_BEEP;
	return (IDCANCEL == MLDisc_ShowQuestionBox(&qb));
}

static INT_PTR CopyProgress_OnReadOnly(HWND hdlg, LPCTSTR pszFile)
{
	PROGDLG *ppd = GetProgDlg(hdlg);
	if (!ppd || !ppd->pCopyData) return FALSE;
    QUESTIONBOX qb = {0};
	TCHAR szFormat[MAX_PATH] = {0},  szMessage[MAX_PATH*2] = {0};
	
	WASABI_API_LNGSTRINGW_BUF(IDS_READONLY_FILE_DELETE_FORMAT, szFormat, ARRAYSIZE(szFormat));
	StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szFormat, pszFile);

	qb.hParent =hdlg;
	qb.pszIcon = IDI_QUESTION;
	qb.pszTitle = MAKEINTRESOURCE(IDS_CONFIRM_FILE_DELETE); 
	qb.pszMessage = szMessage;
	qb.pszBtnOkText = MAKEINTRESOURCE(IDS_YES);
	qb.pszBtnCancelText = MAKEINTRESOURCE(IDS_CANCEL);
	qb.pszCheckboxText = MAKEINTRESOURCE(IDS_APPLY_TO_ALL_FILES);
    qb.uBeepType = MB_ICONEXCLAMATION;
	qb.uFlags = QBF_DEFAULT_OK | QBF_SETFOREGROUND | QBF_BEEP | QBF_SHOW_CHECKBOX;
	
	switch(MLDisc_ShowQuestionBox(&qb))
	{
		case IDCANCEL:	return READONLY_CANCELCOPY;
		case IDOK:		return (qb.checkboxChecked) ? READONLY_DELETEALL : READONLY_DELETE; 
	}
	
	return FALSE;
}

static LPTSTR FormatFileInfo(LPTSTR pszBuffer, size_t cchBufferMax, LPCTSTR pszFilePath)
{	
	HANDLE hFile;
	HRESULT hr;
	BY_HANDLE_FILE_INFORMATION fi;

	pszBuffer[0] = TEXT('\0');

	hFile = CreateFile(pszFilePath, FILE_READ_ATTRIBUTES | FILE_READ_EA, 
								FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
								NULL,OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	
	if (INVALID_HANDLE_VALUE != hFile && 
		GetFileInformationByHandle(hFile, &fi)) 
	{		
		TCHAR szTemp[1024] = {0}, szKeyword[64] = {0};
		SYSTEMTIME st = {0};

		LONGLONG fsize = (LONGLONG)(((__int64)fi.nFileSizeHigh<< 32) | fi.nFileSizeLow);

		WASABI_API_LNGSTRINGW_BUF(IDS_SIZE, szKeyword, ARRAYSIZE(szKeyword));
		hr = StringCchPrintfEx(pszBuffer, cchBufferMax, &pszBuffer, &cchBufferMax, STRSAFE_IGNORE_NULLS, TEXT("\n    %s: %s"), szKeyword, StrFormatByteSize64(fsize, szTemp, ARRAYSIZE(szTemp)));
		
		if (S_OK == hr && FileTimeToSystemTime(&fi.ftCreationTime, &st) && 
			GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szTemp, ARRAYSIZE(szTemp)))
		{			
			WASABI_API_LNGSTRINGW_BUF(IDS_CREATED, szKeyword, ARRAYSIZE(szKeyword));
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, &pszBuffer, &cchBufferMax, STRSAFE_IGNORE_NULLS, TEXT("\n    %s: %s"), szKeyword, szTemp);
			if (S_OK == hr && GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szTemp, ARRAYSIZE(szTemp)))
				hr = StringCchPrintfEx(pszBuffer, cchBufferMax, &pszBuffer, &cchBufferMax, STRSAFE_IGNORE_NULLS, TEXT(", %s"), szTemp);
			
		}
		

		if (S_OK == hr && FileTimeToSystemTime(&fi.ftLastWriteTime, &st) && 
			GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szTemp, ARRAYSIZE(szTemp)))
		{			
			WASABI_API_LNGSTRINGW_BUF(IDS_MODIFIED, szKeyword, ARRAYSIZE(szKeyword));
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, &pszBuffer, &cchBufferMax, STRSAFE_IGNORE_NULLS, TEXT("\n    %s: %s"), szKeyword, szTemp);
			if (S_OK == hr && GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szTemp, ARRAYSIZE(szTemp)))
				hr = StringCchPrintfEx(pszBuffer, cchBufferMax, &pszBuffer, &cchBufferMax, STRSAFE_IGNORE_NULLS, TEXT(", %s"), szTemp);
			
		}
		if (S_OK == hr)
		{
			hr = StringCchCopyEx(pszBuffer, cchBufferMax, TEXT("\n\n"), &pszBuffer, &cchBufferMax, STRSAFE_IGNORE_NULLS);
		}
	}
	else hr = S_FALSE;

	if (S_OK != hr) pszBuffer[0] = TEXT('\0');
	
	if (INVALID_HANDLE_VALUE != hFile) CloseHandle(hFile);
	return pszBuffer;

}
static INT_PTR CopyProgress_OnFileAlreadyExist(HWND hdlg, FILECONFLICT *pConflict)
{
	PROGDLG *ppd = GetProgDlg(hdlg);
	if (!ppd || !ppd->pCopyData) return FALSE;
    QUESTIONBOX qb = {0};
	TCHAR szFormat[128] = {0}, szMessage[MAX_PATH*2] = {0}, szPath[MAX_PATH] = {0}, szFileInfo1[128] = {0}, szFileInfo2[128] = {0};
	
	WASABI_API_LNGSTRINGW_BUF(IDS_FILE_REPLACE_FORMAT, szFormat, ARRAYSIZE(szFormat));

	StringCchCopy(szPath, ARRAYSIZE(szPath), pConflict->pszNameExisting);
	LPTSTR pszFileName = PathFindFileName(szPath);
	if (pszFileName && pszFileName > szPath) *(pszFileName - 1) = TEXT('\0');

	FormatFileInfo(szFileInfo1, ARRAYSIZE(szFileInfo1), pConflict->pszNameExisting);
	FormatFileInfo(szFileInfo2, ARRAYSIZE(szFileInfo2), pConflict->pszNameNew);

	StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szFormat, szPath, pszFileName, szFileInfo1, szFileInfo2);

	qb.hParent = hdlg;
	qb.pszIcon = IDI_QUESTION;
	qb.pszTitle = MAKEINTRESOURCE(IDS_CONFIRM_FILE_REPLACE); 
	qb.pszMessage = szMessage;

	qb.pszBtnExtraText = MAKEINTRESOURCE(IDS_SKIP);
	qb.pszBtnOkText = MAKEINTRESOURCE(IDS_OVERWRITE);
	qb.pszBtnCancelText = MAKEINTRESOURCE(IDS_CANCEL);
	qb.pszCheckboxText = MAKEINTRESOURCE(IDS_APPLY_TO_ALL_FILES);
	qb.uBeepType = MB_ICONEXCLAMATION;
	
	qb.uFlags = QBF_DEFAULT_OK | QBF_SETFOREGROUND | QBF_BEEP | QBF_SHOW_CHECKBOX | QBF_SHOW_EXTRA_BUTTON;
	
	INT_PTR qbr = MLDisc_ShowQuestionBox(&qb);
	INT r = 0;
	switch(qbr)
	{
		case IDCANCEL:			r = EXISTFILE_CANCELCOPY; break;
		case IDOK:				r = EXISTFILE_OVERWRITE; break;
		case IDC_BTN_EXTRA1:	r = EXISTFILE_SKIP; break;
	}
	
	if (qb.checkboxChecked) r |= EXISTFILE_APPLY_TO_ALL;
	return r;
}

static INT_PTR CopyProgress_OnCopyNotify(HWND hdlg, UINT uTask, UINT uOperation, LPARAM lParam)
{
	switch(uTask)
	{
		case CFT_INITIALIZING:
			
			switch(uOperation)
			{
				case CFO_INIT:
					SetTaskText(hdlg, MAKEINTRESOURCE(IDS_COPY_TASK_PREPARE));
					SetOperationText(hdlg, TEXT(""));
					break;
				case CFO_CACLSIZE:
					SetOperationText(hdlg, MAKEINTRESOURCE(IDS_COPY_OP_CALCULATESIZE));
					break;
				case CFO_CHECKDESTINATION:
					SetOperationText(hdlg, MAKEINTRESOURCE(IDS_COPY_OP_CHECKDESTINATION));
					break;
			}
			break;
		case CFT_COPYING:
			switch(uOperation)
			{
				case CFO_INIT:
					SetTaskText(hdlg, MAKEINTRESOURCE(IDS_COPY_TASK_COPY));
					SetOperationText(hdlg, TEXT(""));
					break;
				case CFO_NEXTFILE:
					CopyProgress_OnDisplayNextFile(hdlg, LOWORD(lParam), HIWORD(lParam));
					break;
				case CFO_PROGRESS:
					SendDlgItemMessage(hdlg, IDC_PRG_TOTAL, PBM_SETPOS, (WPARAM)lParam, 0L);
					break;
			}
			break;
		case CFT_FINISHED:
			SetTaskText(hdlg, MAKEINTRESOURCE(IDS_COPY_TASK_FINISHED));
			switch(uOperation)
			{
				case CFO_SUCCESS:	SetOperationText(hdlg, MAKEINTRESOURCE(IDS_COMPLETED)); break;
				case CFO_CANCELLED:	SetOperationText(hdlg, MAKEINTRESOURCE(IDS_CANCELLED)); break;
				case CFO_FAILED:
					SetOperationText(hdlg, MAKEINTRESOURCE(IDS_FAILED)); 
					ShowErrorBox(hdlg);
					break;
			}
			DestroyWindow(hdlg);
			break;
		case CFT_CONFLICT:
			switch(uOperation)
			{
				case CFO_DESTNOTEXIST:		return CopyProgress_OnDestiantionNotExist(hdlg, (LPCTSTR)lParam);
				case CFO_FILEALREDYEXIST:	return CopyProgress_OnFileAlreadyExist(hdlg, (FILECONFLICT*)lParam);
				case CFO_READONLY:			return CopyProgress_OnReadOnly(hdlg, (LPCTSTR)lParam);
			}
			break;
	}
	return FALSE;
}
INT_PTR CALLBACK CopyProgress_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PROGDLG *ppd = GetProgDlg(hdlg);

	switch(uMsg)
	{
		case WM_INITDIALOG:		return CopyProgress_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:			CopyProgress_OnDestroy(hdlg); break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					if (ppd && ppd->pCopyData) 
					{
						SetOperationText(hdlg, MAKEINTRESOURCE(IDS_CANCELLING)); 
						SendDlgItemMessage(hdlg, IDCANCEL, BM_SETSTATE, (WPARAM)TRUE, 0L);
						EnableWindow(GetDlgItem(hdlg, IDCANCEL), FALSE);
						CopyFiles_CancelCopy(ppd->pCopyData);
					}
					else DestroyWindow(hdlg);
					break;
			}
		case CFM_NOTIFY:		MSGRESULT(hdlg, CopyProgress_OnCopyNotify(hdlg, LOWORD(wParam), HIWORD(wParam), lParam));
	}
	return 0;
}