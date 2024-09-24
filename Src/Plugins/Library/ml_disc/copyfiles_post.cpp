#include "main.h"
#include "./copyfiles.h"
#include "./copyinternal.h"
#include "./resource.h"
#include "../nu/trace.h"
#include <api/service/waServiceFactory.h>
#include <shlwapi.h>
#include <strsafe.h>

typedef struct _FILEMETA
{
	LPWSTR pszArtist;
	LPWSTR pszAlbum;
	LPWSTR pszTitle;
	LPWSTR pszGenre;
	LPWSTR pszAlbumArtist;
	INT nYear;
	INT nTrackNum;
	INT nTrackCount;
	LPWSTR pszDisc;
} FILEMETA;

// sets part and parts to -1 or 0 on fail/missing (e.g. parts will be -1 on "1", but 0 on "1/")
static void ParseIntSlashInt(wchar_t *string, int *part, int *parts)
{
	*part = -1;
	*parts = -1;

	if (string && string[0])
	{
		*part = _wtoi(string);
		while (string && *string && *string != '/')
		{
			string++;
		}
		if (*string == '/')
		{
			string++;
			*parts = _wtoi(string);
		}
	}
}

#define READFILEINFO(__fileName, __tag, __result, __pszBuffer, __cchBuffer)\
	(pReader->GetExtendedFileInfo((__fileName), (__tag), (__pszBuffer), (__cchBuffer)) && L'\0' != *(__pszBuffer))


static void CopyFiles_ReadFileMeta(FILEMETA *pMeta, api_metadata *pReader, LPCWSTR pszFileName)
{
	WCHAR szBuffer[2048] = {0};

	#define GETFILEINFO_STR(__tag, __result, __resId)\
	{	szBuffer[0] = L'\0';\
		READFILEINFO(pszFileName, __tag, __result, szBuffer, ARRAYSIZE(szBuffer));\
		if (TEXT('\0') == *szBuffer)\
		{	if (IS_INTRESOURCE(__resId)) WASABI_API_LNGSTRINGW_BUF(((UINT)(UINT_PTR)(__resId)), szBuffer, ARRAYSIZE(szBuffer));\
			else StringCchCopyEx(szBuffer, ARRAYSIZE(szBuffer), (__resId), NULL, NULL, STRSAFE_IGNORE_NULLS);\
		}\
		(__result) = _wcsdup(szBuffer); }
	#define GETFILEINFO_INT(__tag, __result) { szBuffer[0] = L'\0';\
		if (READFILEINFO(pszFileName, __tag, __result, szBuffer, ARRAYSIZE(szBuffer)))\
		{(__result) = _wtoi(szBuffer); }}
	#define GETFILEINFO_INTINT(__tag, __result1, __result2) { szBuffer[0] = L'\0';\
		if (READFILEINFO(pszFileName, __tag, __result, szBuffer, ARRAYSIZE(szBuffer)))\
		{ParseIntSlashInt(szBuffer, (__result1), (__result2)); }}

	if (!pMeta) return;
	ZeroMemory(pMeta, sizeof(FILEMETA));

#pragma warning(push)
#pragma warning(disable : 4127)

	GETFILEINFO_STR(L"artist",		pMeta->pszArtist, MAKEINTRESOURCE(IDS_UNKNOWN_ARTIST));
	GETFILEINFO_STR(L"album",		pMeta->pszAlbum,	 MAKEINTRESOURCE(IDS_UNKNOWN_ALBUM));
	GETFILEINFO_STR(L"title",		pMeta->pszTitle, MAKEINTRESOURCE(IDS_UNKNOWN));
	GETFILEINFO_STR(L"albumartist",	pMeta->pszAlbumArtist, pMeta->pszArtist);
	GETFILEINFO_STR(L"genre",		pMeta->pszGenre, MAKEINTRESOURCE(IDS_UNKNOWN_GENRE));
	GETFILEINFO_INT(L"year",		pMeta->nYear);
	GETFILEINFO_INTINT(L"track",	&pMeta->nTrackNum, &pMeta->nTrackCount);
	GETFILEINFO_STR(L"disc",		pMeta->pszDisc, MAKEINTRESOURCE(IDS_UNKNOWN));
#pragma warning(pop)
}

static void CopyFiles_ReleaseFileMeta(FILEMETA *pMeta)
{
	if (pMeta->pszArtist) free(pMeta->pszArtist);
	if (pMeta->pszAlbum) free(pMeta->pszAlbum);
	if (pMeta->pszTitle) free(pMeta->pszTitle);
	if (pMeta->pszGenre) free(pMeta->pszGenre);
	if (pMeta->pszDisc) free(pMeta->pszDisc);
	if (pMeta->pszAlbumArtist) free(pMeta->pszAlbumArtist);
	ZeroMemory(pMeta, sizeof(FILEMETA));
}

static BOOL CopyFiles_GetFormattedName(LPTSTR pszBuffer, INT cchBufferMax, LPCTSTR pszFileToFormat, LPCTSTR pszOrigFileName, LPCTSTR pszFormat, api_metadata *pMetaReader)
{
	HRESULT hr;
	FILEMETA meta;
	CopyFiles_ReadFileMeta(&meta, pMetaReader, pszFileToFormat);

	WCHAR szYear[16] = {0};
	StringCchPrintf(szYear, ARRAYSIZE(szYear), TEXT("%d"), meta.nYear);

	pszBuffer[0] = TEXT('\0');
	hr = FormatFileName(pszBuffer, cchBufferMax, pszFormat,
						meta.nTrackNum, meta.pszAlbumArtist, 
						meta.pszAlbum, meta.pszTitle,
						meta.pszGenre, szYear, meta.pszArtist,
						pszOrigFileName, meta.pszDisc); 

	CopyFiles_ReleaseFileMeta(&meta);

	if (S_OK != hr)  
	{ 
		SetLastError(ERROR_OUTOFMEMORY); 
		return FALSE; 
	}

	return TRUE;
}

BOOL CopyFiles_FormatFileName(LPTSTR pszNewFileName, INT cchBufferMax, LPCTSTR pszFileToRename, LPCTSTR pszOrigFileName, LPCTSTR pszDestination, LPCTSTR pszFormat, api_metadata *pMetaReader)
{
	StringCchCopy(pszNewFileName, cchBufferMax, pszDestination);
	INT l = lstrlen(pszNewFileName);
	if (l) { pszNewFileName[l] = TEXT('\\'); pszNewFileName[l + 1] = TEXT('\0'); l++; }
	if (!CopyFiles_GetFormattedName(pszNewFileName + l, cchBufferMax - l, pszFileToRename, pszOrigFileName, pszFormat, pMetaReader))
		return FALSE;
	
	LPTSTR p = PathFindFileName(pszNewFileName);
	if (p && p > pszNewFileName)
	{
		*(p - 1) = TEXT('\0');
		if (!CopyFiles_CreateDirectory(pszNewFileName)) return FALSE;
		*(p - 1) = TEXT('\\');
	}
	return TRUE;
}