#include "main.h"
#include "./fileview.h"
#include "./fileview_internal.h"
#include "./resource.h"
#include <strsafe.h>


typedef struct _TYPEORDERINTERNAL
{ 
	UINT index; 
	WCHAR szName[64]; 
} TYPEORDERINTERNAL;

typedef INT (CALLBACK *SHLWAPI_STRCMPLOGICALW)(LPCWSTR, LPCWSTR);

static SHLWAPI_STRCMPLOGICALW fnStrCmpLogicalW = NULL;
static UINT szFileTypesSort[FVFT_LAST + 1];

static FILEDATA *g_pCompareData = NULL;

#define FILEREC(__idx) (g_pCompareData->pRec[(*(size_t*)(__idx))])
#define FILEINFO(__idx) (FILEREC(__idx).Info)

#define COMPARE_META(__elem1, __elem2)\
{\
	if (NULL == FILEREC(__elem1).pMeta) FileViewMeta_Discover(g_pCompareData->szPath, &FILEREC(__elem1), NULL, NULL, 0);\
	if (NULL == FILEREC(__elem2).pMeta) FileViewMeta_Discover(g_pCompareData->szPath, &FILEREC(__elem2), NULL, NULL, 0);\
    if (NULL == FILEREC(__elem1).pMeta || NULL == FILEREC(__elem2).pMeta)\
		return ((INT)(ULONG_PTR)(FILEREC(__elem1).pMeta - FILEREC(__elem2).pMeta));\
}

#define COMPARE_STR_I(__str1, __str2)\
	((NULL == (__str1) || NULL == (__str2)) ? ((INT)(ULONG_PTR)((__str1) - (__str2))) :\
	(CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (__str1), -1, (__str2), -1) - 2))

#define COMPARE_META_STR_I(__elem1, __elem2, __metaField)\
{\
	LPCWSTR s1, s2;\
	BOOL b1, b2;\
	COMPARE_META(__elem1, __elem2);\
	b1 = FileViewMeta_GetString(FILEREC(__elem1).pMeta, (__metaField), &s1);\
	b2 = FileViewMeta_GetString(FILEREC(__elem2).pMeta, (__metaField), &s2);\
	return (!b1 || !b2) ? (b1 - b2) : COMPARE_STR_I(s1, s2);\
}

#define COMPARE_META_INT(__elem1, __elem2, __metaField)\
{\
	INT i1, i2;\
	BOOL b1, b2;\
	COMPARE_META(__elem1, __elem2);\
	b1 = FileViewMeta_GetInt(FILEREC(__elem1).pMeta, (__metaField), &i1);\
	b2 = FileViewMeta_GetInt(FILEREC(__elem2).pMeta, (__metaField), &i2);\
	return (!b1 || !b2) ? (b1 - b2) : (i1 - i2);\
}


__inline static int __cdecl FileRecord_CompareByName(const void *elem1, const void *elem2)
{
	return (CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, 
				FILEINFO(elem1).cFileName, -1, FILEINFO(elem2).cFileName, -1) - 2);
}

__inline static int __cdecl FileRecord_CompareByNameLogical(const void *elem1, const void *elem2)
{
	return fnStrCmpLogicalW(FILEINFO(elem1).cFileName, FILEINFO(elem2).cFileName);
}

__inline static int __cdecl FileRecord_CompareBySize(const void *elem1, const void *elem2)
{
	return ((FILEINFO(elem1).nFileSizeHigh != FILEINFO(elem2).nFileSizeHigh) ?
				(FILEINFO(elem1).nFileSizeHigh - FILEINFO(elem2).nFileSizeHigh) :
				(FILEINFO(elem1).nFileSizeLow - FILEINFO(elem2).nFileSizeLow));
}

__inline static int __cdecl FileRecord_CompareByLastWriteTime(const void *elem1, const void *elem2)
{
	return CompareFileTime(&FILEINFO(elem1).ftLastWriteTime, &FILEINFO(elem2).ftLastWriteTime);
}

__inline static int __cdecl FileRecord_CompareByType(const void *elem1, const void *elem2)
{
	return szFileTypesSort[FILEREC(elem1).fileType] - szFileTypesSort[FILEREC(elem2).fileType];
}

__inline static int __cdecl FileRecord_CompareByAttributes(const void *elem1, const void *elem2)
{
	wchar_t szTest1[32] = {0}, szTest2[32] = {0};
	FileView_FormatAttributes(FILEINFO(elem1).dwFileAttributes, szTest1, sizeof(szTest1)/sizeof(szTest1[0]));
	FileView_FormatAttributes(FILEINFO(elem2).dwFileAttributes, szTest2, sizeof(szTest2)/sizeof(szTest2[0]));
	return (CompareStringW(LOCALE_USER_DEFAULT, 0, szTest1, -1, szTest2, -1) - 2);
}
__inline static int __cdecl FileRecord_CompareByExtension(const void *elem1, const void *elem2)
{	
	if (0 == FILEREC(elem1).extOffset || 0 == FILEREC(elem2).extOffset)
			return ((INT)(FILEREC(elem1).extOffset - (INT)FILEREC(elem2).extOffset));
	
	return (CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, 
				FILEINFO(elem1).cFileName + FILEREC(elem1).extOffset, -1, 
				FILEINFO(elem2).cFileName + FILEREC(elem2).extOffset, -1) - 2);
}
__inline static int __cdecl FileRecord_CompareByCreationTime(const void *elem1, const void *elem2)
{
	return CompareFileTime(&FILEINFO(elem1).ftCreationTime, &FILEINFO(elem1).ftCreationTime);
}

__inline static int __cdecl FileRecord_CompareByAlbum(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_ALBUM);
}

__inline static int __cdecl FileRecord_CompareByArtist(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_ARTIST);
}

__inline static int __cdecl FileRecord_CompareByTitle(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_TITLE);
}

__inline static int __cdecl FileRecord_CompareByMLDB(const void *elem1, const void *elem2)
{
	COMPARE_META_INT(elem1, elem2, MF_SOURCE);
}
__inline static int __cdecl FileRecord_CompareByGenre(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_GENRE);
}

__inline static int __cdecl FileRecord_CompareByYear(const void *elem1, const void *elem2)
{
	COMPARE_META_INT(elem1, elem2, MF_YEAR);
}

__inline static int __cdecl FileRecord_CompareByLength(const void *elem1, const void *elem2)
{
	COMPARE_META_INT(elem1, elem2, MF_LENGTH);
}

__inline static int __cdecl FileRecord_CompareByBitrate(const void *elem1, const void *elem2)
{
	COMPARE_META_INT(elem1, elem2, MF_BITRATE);
}

__inline static int __cdecl FileRecord_CompareByTrack(const void *elem1, const void *elem2)
{
	COMPARE_META_INT(elem1, elem2, MF_TRACKNUM);
}

__inline static int __cdecl FileRecord_CompareByDisc(const void *elem1, const void *elem2)
{
	COMPARE_META_INT(elem1, elem2, MF_DISCNUM);
}

__inline static int __cdecl FileRecord_CompareByComment(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_COMMENT);
}

__inline static int __cdecl FileRecord_CompareByPublisher(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_PUBLISHER);
}

__inline static int __cdecl FileRecord_CompareByComposer(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_COMPOSER);
}

__inline static int __cdecl FileRecord_CompareByAlbumArtist(const void *elem1, const void *elem2)
{
	COMPARE_META_STR_I(elem1, elem2, MF_ALBUMARTIST);
}

__inline static int __cdecl CompareTypeOrderInternal(const void *elem1, const void *elem2)
{
	return (CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, ((TYPEORDERINTERNAL*)elem1)->szName, -1, ((TYPEORDERINTERNAL*)elem2)->szName, -1) - 2);
}


void FileView_SortByColumnEx(FILEDATA *pFileData, UINT uColumn, size_t *pOrder, size_t count)
{
	if (pFileData && pFileData->pRec && pOrder && count > 1)
	{
		static BOOL bLoadFailed = FALSE;
		int (__cdecl *fnComparer)(const void *, const void *) = NULL;

		switch(uColumn)
		{
			case FVCOLUMN_NAME:
				if (NULL == fnStrCmpLogicalW && !bLoadFailed)
				{
					UINT prevErrorMode;
					HMODULE hModule;
					prevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
					hModule = LoadLibraryW(L"Shlwapi.dll");
					SetErrorMode(prevErrorMode);
					if (hModule) 
					{
						fnStrCmpLogicalW = (SHLWAPI_STRCMPLOGICALW)GetProcAddress(hModule, "StrCmpLogicalW");
						FreeLibrary(hModule);
					}
					bLoadFailed = FALSE;
				}
				fnComparer = (fnStrCmpLogicalW) ? FileRecord_CompareByNameLogical : FileRecord_CompareByName;
				break;
			case FVCOLUMN_SIZE:		fnComparer = FileRecord_CompareBySize; break;
			case FVCOLUMN_MODIFIED:	fnComparer = FileRecord_CompareByLastWriteTime; break;
			case FVCOLUMN_TYPE:
				{
					TYPEORDERINTERNAL szOrder[sizeof(szFileTypesSort)/sizeof(szFileTypesSort[0])];
					for (int i = 0; i < sizeof(szFileTypesSort)/sizeof(szFileTypesSort[0]); i++)
					{
						szOrder[i].index = i;
						WASABI_API_LNGSTRINGW_BUF(i, szOrder[i].szName, sizeof(szOrder[i].szName)/sizeof(szOrder[i].szName[0]));
					}
					qsort(szOrder, sizeof(szFileTypesSort)/sizeof(szFileTypesSort[0]), sizeof(TYPEORDERINTERNAL), CompareTypeOrderInternal);
					for (int i = 0; i < sizeof(szFileTypesSort)/sizeof(szFileTypesSort[0]); i++) szFileTypesSort[szOrder[i].index] = i;
				}
				fnComparer = FileRecord_CompareByType; 
				break;
			case FVCOLUMN_CREATED:		fnComparer = FileRecord_CompareByCreationTime; break;
			case FVCOLUMN_ATTRIBUTES:	fnComparer = FileRecord_CompareByAttributes; break;
			case FVCOLUMN_EXTENSION:		fnComparer = FileRecord_CompareByExtension; break;
			case FVCOLUMN_ARTIST:		fnComparer = FileRecord_CompareByArtist; break;
			case FVCOLUMN_ALBUM:			fnComparer = FileRecord_CompareByAlbum; break;
			case FVCOLUMN_TITLE:			fnComparer = FileRecord_CompareByTitle; break;
			case FVCOLUMN_INMLDB:		fnComparer = FileRecord_CompareByMLDB; break;
			case FVCOLUMN_GENRE:			fnComparer = FileRecord_CompareByGenre; break;
			case FVCOLUMN_YEAR:			fnComparer = FileRecord_CompareByYear; break;
			case FVCOLUMN_LENGTH:		fnComparer = FileRecord_CompareByLength; break;
			case FVCOLUMN_BITRATE:		fnComparer = FileRecord_CompareByBitrate; break;
			case FVCOLUMN_TRACK:			fnComparer = FileRecord_CompareByTrack; break;
			case FVCOLUMN_DISC:			fnComparer = FileRecord_CompareByDisc; break;
			case FVCOLUMN_COMMENT:		fnComparer = FileRecord_CompareByComment; break;
			case FVCOLUMN_PUBLISHER:		fnComparer = FileRecord_CompareByPublisher; break;
			case FVCOLUMN_COMPOSER:		fnComparer = FileRecord_CompareByComposer; break;
			case FVCOLUMN_ALBUMARTIST:	fnComparer = FileRecord_CompareByAlbumArtist; break;
		}

		if (fnComparer)
		{
			g_pCompareData = pFileData;
			qsort(pOrder, count, sizeof(size_t), fnComparer);
			g_pCompareData = NULL;
		}
	}
}

void FileView_SortByColumn(FILEDATA *pFileData, UINT uColumn)
{
	if (!pFileData) return;
	FileView_SortByColumnEx(pFileData, uColumn, pFileData->pSort, pFileData->count);
}

