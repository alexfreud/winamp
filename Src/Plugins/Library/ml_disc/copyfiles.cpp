#include "main.h"
#include "./copyfiles.h"
#include "./copyinternal.h"
#include "./resource.h"
#include "./settings.h"
#include "../nu/trace.h"
#include <api/service/waServiceFactory.h>
#include <shlwapi.h>
#include <strsafe.h>

static LONG szBusyDrive[26] = {0, };
static CRITICAL_SECTION	cs_copy = { 0,}; 

static void NotifyDialog(COPYDATA *pcd, UINT uTask, UINT uOperation, LPARAM lParam)
{
	if(pcd->hDialog) PostMessage(pcd->hDialog, CFM_NOTIFY, MAKEWPARAM(uTask, uOperation), lParam);
}

static INT_PTR QueryDialog(COPYDATA *pcd, UINT uTask, UINT uOperation, LPARAM lParam)
{
	return (pcd->hDialog) ? SendMessage(pcd->hDialog, CFM_NOTIFY, MAKEWPARAM(uTask, uOperation), lParam) : FALSE;
}

static void CopyFiles_MarkDrivesBusy(LPCTSTR *ppsz, INT count, BOOL bBusy)
{
	INT i, n;
	DWORD driveMask = 0x00;

	EnterCriticalSection(&cs_copy);
	if (bBusy)
	{
		for(i = 0; i < count; i++)
		{
			n = PathGetDriveNumber(ppsz[i]);
			if (-1 != n) 
			{
				if (0 == szBusyDrive[n]) driveMask |= (((DWORD)0x01) << n);
				szBusyDrive[n]++;
			}
		}
	}
	else
	{
		for(i = 0; i < count; i++)
		{
			n = PathGetDriveNumber(ppsz[i]);
			if (-1 != n) 
			{
				if (szBusyDrive[n] <= 0) continue;
				szBusyDrive[n]--;
				if (0 == szBusyDrive[n]) driveMask |= (((DWORD)0x01) << n);
			}
		}
	}
	LeaveCriticalSection(&cs_copy);

	if (0x00 != driveMask)
	{
		static UINT uMsgCopyNotify = 0;
		if (!uMsgCopyNotify) uMsgCopyNotify = RegisterWindowMessageA("WACOPY_BROADCAST_MSG");
		if (uMsgCopyNotify)
		{
			for (CHAR c = 'A'; 0x00 != driveMask; c++, driveMask>>=1)
			{
				if (0 == (0x01 & driveMask)) continue;
				if (bBusy) SendNotifyMessage(HWND_BROADCAST, uMsgCopyNotify, MAKEWPARAM(c, 0), (LPARAM)TRUE);
				else 
				{
					SendNotifyMessage(HWND_BROADCAST, uMsgCopyNotify, MAKEWPARAM(c, 0), (LPARAM)FALSE);
					SendNotifyMessage(HWND_BROADCAST, uMsgCopyNotify, MAKEWPARAM(0, 0xffff), (LPARAM)plugin.hwndWinampParent);
				}
			}
		}
	}
}
static BOOL ReadCopyParameters(COPYDATA *pcd)
{
	BOOL bVal;
	HRESULT hr = S_OK;
	pcd->uFlags = 0;
	if (S_OK == hr) hr = Settings_ReadString(C_COPY, CF_PATH, pcd->szDestination, ARRAYSIZE(pcd->szDestination));
	if (S_OK == hr) hr = Settings_ReadString(C_COPY, CF_TITLEFMT, pcd->szTitleFormat, ARRAYSIZE(pcd->szTitleFormat));
	if (S_OK == hr && S_OK == (hr = Settings_GetBool(C_COPY, CF_ADDTOMLDB, &bVal)) && bVal) pcd->uFlags |= FCF_ADDTOMLDB;
	if (S_OK == hr && S_OK == (hr = Settings_GetBool(C_COPY, CF_USETITLEFMT, &bVal)) && bVal) pcd->uFlags |= FCF_USETITLEFMT;
	CleanupDirectoryString(pcd->szDestination);

	return (S_OK == hr);

}

void MLDisc_InitializeCopyData()
{
	InitializeCriticalSection(&cs_copy);
}

void MLDisc_ReleaseCopyData()
{
	DeleteCriticalSection(&cs_copy);
}

BOOL MLDisc_IsDiscCopying(CHAR cLetter)
{
	if (cLetter >= 'a' && cLetter <= 'z') cLetter -= 0x20;
	return (cLetter >= 'A' && cLetter <= 'Z' && szBusyDrive[cLetter - 'A'] > 0);
}


BOOL MLDisc_CopyFiles(HWND hParent, LPWSTR *ppszFiles, ULONGLONG *pFSizes, INT count)
{
	if (!ppszFiles || !count) return FALSE;
	if (NULL == hParent) hParent = plugin.hwndLibraryParent;

	COPYDATA *pcd = (COPYDATA*)CoTaskMemAlloc(sizeof(COPYDATA));
	if (!pcd) return FALSE;
	ZeroMemory(pcd, sizeof(COPYDATA));

	CopyFiles_AddRef(pcd);

	pcd->hOwner = hParent;
	pcd->ppszFiles = ppszFiles;
	pcd->pFSizes = pFSizes;
	pcd->count = count;
	pcd->bCancel = FALSE;

	waServiceFactory *factory = plugin.service->service_getServiceByGuid(api_metadataGUID);
	if (factory) pcd->pMetaReader = (api_metadata*) factory->getInterface();
	factory = plugin.service->service_getServiceByGuid(mldbApiGuid);
	if (factory) pcd->pMlDb = (api_mldb*) factory->getInterface();

	HWND hRoot = GetAncestor(hParent, GA_ROOT);
	if (NULL == hRoot) hRoot = hParent;
	
	INT_PTR result = WASABI_API_DIALOGBOXPARAMW(IDD_FILECOPY_PREPARE, hRoot, CopyPrepare_DialogProc, (LPARAM)pcd);
	if (IDCANCEL != result)
	{
		if (ReadCopyParameters(pcd))
		{
			HWND hCopy = WASABI_API_CREATEDIALOGPARAMW(IDD_FILECOPY_PROGRESS, hRoot, CopyProgress_DialogProc, (LPARAM)pcd);
			if (hCopy)
			{
				pcd->hDialog = hCopy;
				ShowWindow(hCopy, SW_SHOWNORMAL);
				RedrawWindow(hCopy, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
			CopyFiles_StartCopy(pcd);
		}
	}

	CopyFiles_Release(pcd);
	return TRUE;
}

BOOL CopyFiles_CancelCopy(COPYDATA *pcd)
{
	if (pcd) pcd->bCancel = TRUE;
	return (NULL != pcd);
}

LONG CopyFiles_AddRef(COPYDATA *pcd)
{
	return (pcd) ? InterlockedIncrement(&pcd->ref) : 0;

}
LONG CopyFiles_Release(COPYDATA *pcd)
{	
	if (pcd && pcd->ref > 0)
	{
		LONG r = InterlockedDecrement(&pcd->ref);
        if ( 0 == r)
		{
			if (pcd->ppszFiles)
			{
				for (int i = 0; i < pcd->count; i++) CoTaskMemFree(pcd->ppszFiles[i]);
				CoTaskMemFree(pcd->ppszFiles);
			}
			if (pcd->pFSizes) CoTaskMemFree(pcd->pFSizes);
			if (pcd->hThread) CloseHandle(pcd->hThread);

			if(pcd->pMetaReader)
			{
				waServiceFactory *factory = plugin.service->service_getServiceByGuid(api_metadataGUID);
				if (factory) factory->releaseInterface(pcd->pMetaReader);
			}
			if(pcd->pMlDb)
			{				
				waServiceFactory *factory = plugin.service->service_getServiceByGuid(mldbApiGuid);
				if (factory) factory->releaseInterface(pcd->pMlDb);
			}
			CoTaskMemFree(pcd);
		}
		return r;
	}
	return 0;
}

static ULONGLONG CopyFiles_CalculateTotalSize(COPYDATA *pcd)
{
	ULONGLONG total = 0;
	if (!pcd->pFSizes) 
	{
		pcd->pFSizes = (ULONGLONG*)CoTaskMemAlloc(sizeof(ULONGLONG)*pcd->count);
		if (!pcd->pFSizes)
		{
			pcd->errorCode = ERROR_NOT_ENOUGH_MEMORY;
			return 0;
		}
		LARGE_INTEGER fs;
		for(int i = 0; i < pcd->count; i++)
		{
			HANDLE hFile = CreateFile(pcd->ppszFiles[i], FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			if (INVALID_HANDLE_VALUE == hFile) 
			{
				pcd->errorCode = GetLastError();
				return 0;
			}
			if (!GetFileSizeEx(hFile, &fs)) { pcd->errorCode = GetLastError(); CloseHandle(hFile); return 0; }
			CloseHandle(hFile);
			pcd->pFSizes[i] = fs.QuadPart;
		}
	}
	for(int i = 0; i < pcd->count; i++) total += pcd->pFSizes[i];
	return total;
}

BOOL CopyFiles_CreateDirectory(LPCTSTR pszDirectory)
{
	DWORD ec = ERROR_SUCCESS;
	if (!CreateDirectory(pszDirectory, NULL))
	{
		ec = GetLastError();
		if (ERROR_PATH_NOT_FOUND == ec)
		{
			LPCTSTR pszBlock = pszDirectory;
			TCHAR szBuffer[MAX_PATH] = {0};
			
			LPCTSTR pszCursor = PathFindNextComponent(pszBlock);
			ec = (pszCursor == pszBlock || S_OK != StringCchCopyN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock))) ?
					ERROR_INVALID_NAME : ERROR_SUCCESS;
			
			pszBlock = pszCursor;
			
			while (ERROR_SUCCESS == ec && NULL != (pszCursor = PathFindNextComponent(pszBlock)))
			{
				if (pszCursor == pszBlock || S_OK != StringCchCatN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock)))
					ec = ERROR_INVALID_NAME;

				if (ERROR_SUCCESS == ec && !CreateDirectory(szBuffer, NULL))
				{
					ec = GetLastError();
					if (ERROR_ALREADY_EXISTS == ec) ec = ERROR_SUCCESS;
				}
				pszBlock = pszCursor;
			}
		}

		if (ERROR_ALREADY_EXISTS == ec) ec = ERROR_SUCCESS;
	}
	SetLastError(ec);
	return (ERROR_SUCCESS == ec);
}


static BOOL CopyFiles_CheckDestination(COPYDATA *pcd, ULONGLONG needSize)
{
	TCHAR szRoot[MAX_PATH] = {0};
	if (S_OK != StringCchCopy(szRoot, ARRAYSIZE(szRoot), pcd->szDestination))
	{
		pcd->errorCode = ERROR_OUTOFMEMORY;
		return FALSE;
	}
	PathStripToRoot(szRoot);
	ULARGE_INTEGER free, total;
	if (!GetDiskFreeSpaceEx(szRoot, &free, &total, NULL))
	{
		pcd->errorCode = GetLastError();
		return FALSE;
	}
	if (needSize > free.QuadPart)  
	{
		pcd->errorCode = ERROR_DISK_FULL;
		return FALSE;
	}

	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pcd->szDestination))
	{
		DWORD ec = GetLastError();
		if (ERROR_PATH_NOT_FOUND == ec || ERROR_FILE_NOT_FOUND == ec)
		{
			if (TRUE == QueryDialog(pcd, CFT_CONFLICT, CFO_DESTNOTEXIST, (LPARAM)pcd->szDestination))
				pcd->errorCode = ERROR_REQUEST_ABORTED;
		}
		else pcd->errorCode = ec;
		
		if (ERROR_SUCCESS != pcd->errorCode)
		{
			pcd->errorMsgId = IDS_COPY_ERRMSG_DIRECTORYCREATE_FAILED;
			return FALSE;
		}
		
	}

	if (!CopyFiles_CreateDirectory(pcd->szDestination))
	{
		pcd->errorMsgId = IDS_COPY_ERRMSG_DIRECTORYCREATE_FAILED;
		pcd->errorCode = ERROR_CANNOT_MAKE;
		return FALSE;
	}
	return TRUE;
}

static BOOL CopyFiles_FixCdAttributes(LPCTSTR pszFileName)
{
	DWORD attr = GetFileAttributes(pszFileName);
	if (INVALID_FILE_ATTRIBUTES == attr) return FALSE;
	return SetFileAttributes(pszFileName, (attr & ~FILE_ATTRIBUTE_READONLY) | FILE_ATTRIBUTE_ARCHIVE);
}



static BOOL CopyFiles_DeleteFile(LPCTSTR pszFileName, COPYDATA *pcd)
{
	DWORD attr = GetFileAttributes(pszFileName);
	if (INVALID_FILE_ATTRIBUTES == attr) return FALSE;
	if (FILE_ATTRIBUTE_READONLY & attr)
	{
		BOOL bReset = FALSE;
		if (FCF_DELETEREADONLY & pcd->uFlags) bReset = TRUE;
		else 
		{
			INT_PTR r = QueryDialog(pcd, CFT_CONFLICT, CFO_READONLY, (LPARAM)pszFileName);
			switch(r)
			{
				case READONLY_CANCELCOPY:	
					pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_USERABORT;
					SetLastError(ERROR_REQUEST_ABORTED);
					return FALSE;
				case READONLY_DELETEALL:		pcd->uFlags |= FCF_DELETEREADONLY; // no break
				case READONLY_DELETE:		bReset = TRUE; break;
			}
		}

		if (bReset)
		{
			if (!SetFileAttributes(pszFileName, (attr & ~FILE_ATTRIBUTE_READONLY)))
			return FALSE;
		}
	}
	return DeleteFile(pszFileName);
}

#define NOT_EXIST			0
#define EXIST_AND_SKIP		1
#define EXIST_AND_OVERWRITE 	2
#define EXIST_AND_CANCEL 	3

static DWORD CopyFiles_CheckIfExist(LPCTSTR pszFileNameDest, LPCTSTR pszFileNameSource, COPYDATA *pcd)
{
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pszFileNameDest))
	{
		return NOT_EXIST;
	}
	

	if (FCF_SKIPFILE & pcd->uFlags) return EXIST_AND_SKIP;
	if (FCF_OVERWRITEFILE & pcd->uFlags) return EXIST_AND_OVERWRITE;

	FILECONFLICT conflict;
	ZeroMemory(&conflict, sizeof(FILECONFLICT));

	pcd->uFlags &= ~(FCF_SKIPFILE | FCF_OVERWRITEFILE);

	conflict.pszNameExisting = pszFileNameDest;
	conflict.pszNameNew	= pszFileNameSource;
		
	INT_PTR  r = QueryDialog(pcd, CFT_CONFLICT, CFO_FILEALREDYEXIST, (LPARAM)&conflict);
		
	switch(0xFF & r)
	{
		case EXISTFILE_CANCELCOPY:	return EXIST_AND_CANCEL;
		case EXISTFILE_SKIP:		
			if (EXISTFILE_APPLY_TO_ALL & r) pcd->uFlags |= FCF_SKIPFILE;
			return EXIST_AND_SKIP;
		case EXISTFILE_OVERWRITE:	
			if (EXISTFILE_APPLY_TO_ALL & r) pcd->uFlags |= FCF_OVERWRITEFILE;
			return EXIST_AND_OVERWRITE;
	}

	return NOT_EXIST;
}

typedef struct _COPYPROGRESS
{
	ULONGLONG total;
	ULONGLONG completed;
	HWND hDialog;
	INT percent;
} COPYPROGRESS;

static DWORD CALLBACK CopyFiles_ProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize,
												LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, 
												HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
{
	COPYPROGRESS *pProgress = (COPYPROGRESS*)lpData;
	if (pProgress)
	{
		switch(dwCallbackReason)
		{
			case CALLBACK_STREAM_SWITCH:
				pProgress->completed += TotalFileSize.QuadPart;
				break;

		}
		INT p = (INT)((pProgress->completed - TotalFileSize.QuadPart + TotalBytesTransferred.QuadPart) * 100 / pProgress->total);
		if (p != pProgress->percent)
		{
			pProgress->percent = p;
			if (pProgress->hDialog) 
				PostMessage(pProgress->hDialog, CFM_NOTIFY, MAKEWPARAM(CFT_COPYING, CFO_PROGRESS), p);
		}
	}
	return PROGRESS_CONTINUE;
};

static DWORD WINAPI CopyFiles_ThreadProc(LPVOID param)
{
	
	COPYDATA *pcd = (COPYDATA*)param;
	if (!pcd) return 1;
	
	CopyFiles_MarkDrivesBusy((LPCTSTR*)pcd->ppszFiles, pcd->count, TRUE);

	ULONGLONG needSize;
	NotifyDialog(pcd, CFT_INITIALIZING, CFO_INIT, 0L);
	NotifyDialog(pcd, CFT_INITIALIZING, CFO_CACLSIZE, 0L);
	needSize = CopyFiles_CalculateTotalSize(pcd);
	BOOL bSuccess = (ERROR_SUCCESS == pcd->errorCode);
	if (bSuccess)
	{
		NotifyDialog(pcd, CFT_INITIALIZING, CFO_CHECKDESTINATION, 0L);
		bSuccess = CopyFiles_CheckDestination(pcd, needSize);
	}

	if (!bSuccess) 
	{
		if (0 == pcd->errorMsgId) 
			pcd->errorMsgId = IDS_COPY_ERRMSG_INITIALIZATION_FAILED;
		NotifyDialog(pcd, CFT_FINISHED, CFO_FAILED, pcd->errorCode);
		CopyFiles_MarkDrivesBusy((LPCTSTR*)pcd->ppszFiles, pcd->count, FALSE);
		CopyFiles_Release(pcd);
		return 0;
	}

	COPYPROGRESS progress;
	ZeroMemory(&progress, sizeof(COPYPROGRESS));
	progress.total = needSize;
	progress.hDialog = pcd->hDialog;

	NotifyDialog(pcd, CFT_COPYING, CFO_INIT, 0L);

	TCHAR szFile[MAX_PATH] = {0};

	for (int i = 0; i < pcd->count && ERROR_SUCCESS == pcd->errorCode; i++)
	{	
		if (pcd->bCancel)
		{
			pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_USERABORT;
			pcd->errorCode = ERROR_REQUEST_ABORTED;
		}

		LPCTSTR pszOrigFileName = PathFindFileName(pcd->ppszFiles[i]);
		
		NotifyDialog(pcd, CFT_COPYING, CFO_NEXTFILE, MAKELPARAM(i, pcd->count));
		
		if (NULL == PathCombine(szFile, pcd->szDestination, pszOrigFileName))
		{
			pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_FAILED;
			pcd->errorCode = ERROR_BAD_PATHNAME;
		}

		DWORD r = CopyFiles_CheckIfExist(szFile, pcd->ppszFiles[i], pcd);
		switch(r)
		{			
			case EXIST_AND_SKIP:
				continue;
				break;
			case EXIST_AND_OVERWRITE:
				if (!CopyFiles_DeleteFile(szFile, pcd))
				{
					pcd->errorMsgId = IDS_COPY_ERRMSG_DELETEFILE_FAILED;
					pcd->errorCode = GetLastError();
				}
				break;
			case EXIST_AND_CANCEL:
				pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_USERABORT;
				pcd->errorCode = ERROR_REQUEST_ABORTED;
				break;
		}	

		// copy
		if (ERROR_SUCCESS == pcd->errorCode && !CopyFileEx(pcd->ppszFiles[i], szFile, CopyFiles_ProgressRoutine, &progress, &pcd->bCancel, 0))
		{
			pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_FAILED;
			pcd->errorCode = GetLastError();
		}

		if (pcd->bCancel)
		{
			pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_USERABORT;
			pcd->errorCode = ERROR_REQUEST_ABORTED;
		}
		
		if (ERROR_SUCCESS == pcd->errorCode) // post copy
		{
			// fix attributes
			if (ERROR_SUCCESS == pcd->errorCode && !CopyFiles_FixCdAttributes(szFile))
			{
				pcd->errorMsgId = IDS_COPY_ERRMSG_SETATTRIBUTES_FAILED;
				pcd->errorCode = GetLastError();
			}

			// format title & rename
			if (ERROR_SUCCESS == pcd->errorCode && (FCF_USETITLEFMT & pcd->uFlags) && pcd->pMetaReader)
			{				
				TCHAR szBuffer[MAX_PATH] = {0};
				if (!CopyFiles_FormatFileName(szBuffer, ARRAYSIZE(szBuffer), szFile, pszOrigFileName, 
						pcd->szDestination, pcd->szTitleFormat, pcd->pMetaReader))
				{
					pcd->errorMsgId = IDS_COPY_ERRMSG_TITLEFORMAT_FAILED;
					pcd->errorCode = GetLastError();
				}

				if (pcd->bCancel)
				{
					pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_USERABORT;
					pcd->errorCode = ERROR_REQUEST_ABORTED;
				}
				
				if (ERROR_SUCCESS == pcd->errorCode && 
					CSTR_EQUAL != CompareString(STRCOMP_INVARIANT, 0, szBuffer, -1, szFile, -1))
				{

					DWORD r = CopyFiles_CheckIfExist(szBuffer, szFile, pcd);
					switch(r)
					{			
						case EXIST_AND_SKIP:
							CopyFiles_DeleteFile(szFile, pcd);
							continue;
							break;
						case EXIST_AND_OVERWRITE:
							if (!CopyFiles_DeleteFile(szBuffer, pcd))
							{
								pcd->errorMsgId = IDS_COPY_ERRMSG_DELETEFILE_FAILED;
								pcd->errorCode = GetLastError();
							}
							break;
						case EXIST_AND_CANCEL:
							pcd->errorMsgId = IDS_COPY_ERRMSG_COPYFILE_USERABORT;
							pcd->errorCode = ERROR_REQUEST_ABORTED;
							break;
					}	

					if (ERROR_SUCCESS == pcd->errorCode)
					{
						if (!MoveFileEx(szFile, szBuffer, MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED)) 
						{
							pcd->errorMsgId = IDS_COPY_ERRMSG_TITLEFORMAT_FAILED;
							pcd->errorCode = GetLastError();
						}
						else StringCchCopy(szFile, ARRAYSIZE(szFile), szBuffer);
					}
					
				}
			}
			
			
			if (ERROR_SUCCESS != pcd->errorCode)
			{
				CopyFiles_DeleteFile(szFile, pcd);
			}
			else if ((FCF_ADDTOMLDB & pcd->uFlags) && pcd->pMlDb)
			{
				if (0 == pcd->pMlDb->AddFile(szFile))
				{
					pcd->errorMsgId = IDS_COPY_ERRMSG_ADDTOMLDB_FAILED;
					pcd->errorCode = ERROR_FILE_NOT_FOUND;
				}
			}
		}
	
	}
	  
	NotifyDialog(pcd, CFT_FINISHED, (ERROR_SUCCESS == pcd->errorCode) ? CFO_SUCCESS : CFO_FAILED, pcd->errorCode);
	if ((FCF_ADDTOMLDB & pcd->uFlags) && pcd->pMlDb) pcd->pMlDb->Sync();
	CopyFiles_MarkDrivesBusy((LPCTSTR*)pcd->ppszFiles, pcd->count, FALSE);
	CopyFiles_Release(pcd);
	return 0;
}

BOOL CopyFiles_StartCopy(COPYDATA *pcd)
{
	DWORD threadId;
	
	CopyFiles_AddRef(pcd);
	
	pcd->hThread = CreateThread(NULL, 0, CopyFiles_ThreadProc, pcd, 0, &threadId);
	if (pcd->hThread) return TRUE;
	

	pcd->errorCode = GetLastError();
	NotifyDialog(pcd, CFT_FINISHED, CFO_FAILED, pcd->errorCode);
	CopyFiles_Release(pcd);
	return FALSE;
}