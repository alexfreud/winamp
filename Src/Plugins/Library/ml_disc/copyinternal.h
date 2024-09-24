#ifndef NULLOSFT_MEDIALIBRARY_MLDISC_COPYFILES_INTERNAL_HEADER
#define NULLOSFT_MEDIALIBRARY_MLDISC_COPYFILES_INTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "../Agave/Metadata/api_metadata.h"
#include "../ml_local/api_mldb.h"

#ifdef __cplusplus
extern "C" {
#endif

	
#define STRCOMP_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#define CPM_UPDATEDISKSIZE		(WM_APP + 2)


typedef struct _COPYDATA
{	
	LONG		ref;
	HWND			hDialog;
	HWND			hOwner;
	HANDLE		hThread;
	BOOL		bCancel;
	DWORD		errorCode;
	UINT		errorMsgId;
	LPWSTR		*ppszFiles;
	ULONGLONG	*pFSizes;
	INT			count;
	UINT		uFlags;
	api_metadata *pMetaReader;
	api_mldb		*pMlDb;
	WCHAR		szDestination[MAX_PATH];
	WCHAR		szTitleFormat[128];
} COPYDATA;

#define FCF_ADDTOMLDB			0x00000002L
#define FCF_USETITLEFMT			0x00000004L
#define FCF_SKIPFILE			0x00010000L
#define FCF_OVERWRITEFILE		0x00020000L
#define FCF_DELETEREADONLY		0x00040000L



INT_PTR CALLBACK CopyPrepare_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CopyProgress_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


LONG CopyFiles_AddRef(COPYDATA *pcd);
LONG CopyFiles_Release(COPYDATA *pcd);
BOOL CopyFiles_StartCopy(COPYDATA *pcd);
BOOL CopyFiles_CancelCopy(COPYDATA *pcd);

BOOL CopyFiles_CreateDirectory(LPCTSTR pszDirectory);
BOOL CopyFiles_FormatFileName(LPTSTR pszNewFileName, INT cchBufferMax, LPCTSTR pszFileToRename, LPCTSTR pszOrigFileName, LPCTSTR pszDestination, LPCTSTR pszFormat, api_metadata *pMetaReader);

HBITMAP CopyFiles_LoadResourcePng(LPCTSTR pszResource);

#define CFM_NOTIFY			(WM_APP + 3)

// notify task
#define CFT_INITIALIZING	0x0001	
#define CFT_COPYING			0x0002
#define CFT_FINISHED		0x0003
#define CFT_CONFLICT		0x0004 //  conflicts always use SendMessage


// init task operations code
#define CFO_INIT				0x0000
#define CFO_CACLSIZE			0x0001	
#define CFO_CHECKDESTINATION	0x0002


// copy task operations code
#define CFO_INIT			0x0000 // time to set tast text
#define CFO_NEXTFILE		0x0001 // lParam - MAKELPARAM(file index, total count)
#define CFO_PROGRESS		0x0002 // lParam - percent 
#define CFO_POSTCOPY			0x0003


// conflicts

#define EXISTFILE_CANCELCOPY			0x0001 // almost like return FALSE  but will not produce error
#define EXISTFILE_SKIP				0x0002 // skip
#define EXISTFILE_OVERWRITE			0x0003 // overwrite
#define EXISTFILE_APPLY_ONCE			0x0000 // apply only once
#define EXISTFILE_APPLY_TO_ALL		0x0100 // apply to all files with the same conflict

#define READONLY_CANCELCOPY	0x0001
#define READONLY_DELETE		0x0002
#define READONLY_DELETEALL	0x0003

typedef struct _FILECONFLICT
{
	LPCTSTR		pszNameExisting;
	LPCTSTR		pszNameNew;
} FILECONFLICT;


#define CFO_DESTNOTEXIST		0x0000 // return FALSE to create destination or TRUE to cancel copy operation. param -pszDestionation
#define CFO_FILEALREDYEXIST		0x0001 // return FALSE to fail with access denied, or EXISTFILE_XXX, param = (FILECONFLICT*)
#define CFO_READONLY				0x0002 // return FALSE to fail, or RADONLY_XXX, param = (LPCTSTR)pszFileName






// finished task operations code
#define CFO_FAILED		0x0001
#define	CFO_SUCCESS		0x0002
#define	CFO_CANCELLED	0x0003


#ifdef __cplusplus
}
#endif



#endif // NULLOSFT_MEDIALIBRARY_MLDISC_COPYFILES_INTERNAL_HEADER