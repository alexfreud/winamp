#include "main.h"
#include "./fileview.h"
#include "./fileview_internal.h"
#include <vector>
#include "../playlist/svc_playlisthandler.h"
#include "../playlist/api_playlistmanager.h"
#include <api/service/waservicefactorybase.h>
#include <api/service/services.h>

#include <shlwapi.h>
#include <strsafe.h>
#include <algorithm>

#define FILEREC_ALLOCATION_STEP		1000


#define FILETYPE_REREAD		((UINT)(0 - 1))

typedef struct _FILETYPEREC
{
	WCHAR	szExtension[32];
	WCHAR	szFamily[128];
	UINT	Type;
} FILETYPEREC;

static std::vector<FILETYPEREC> supportedFilesList;



//__inline static int __cdecl QSort_StrCmpI(const void *elem1, const void *elem2)
//{
//	return (CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, (LPCWSTR)elem1, -1, (LPCWSTR)elem2, -1) - 2);
//}

static BOOL FileView_GetPLExtensionName(LPCWSTR pszExt, LPWSTR pszDest, INT cchDest)
{	
	BOOL result(FALSE);
	DWORD lcid;
	int n(0);
    waServiceFactory *sf = 0;
	LPCWSTR ext;
	lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

	while (NULL != (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++)))
	{
		svc_playlisthandler * handler = static_cast<svc_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			int k(0);
			while (NULL != (ext = handler->EnumerateExtensions(k++)))
			{
				if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszExt, -1, ext, -1))
				{
					result = (S_OK == StringCchCopyW(pszDest, cchDest, handler->GetName()));
					if (result && CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszExt, -1, L"M3U8", -1)) // ugly...
							result = (S_OK == StringCchCatW(pszDest, cchDest, L" (Unicode)"));
					break;
				}
			}
			sf->releaseInterface(handler);
		}
	}
	return result;
}

static void FileView_ReadSupportedTypesInfo()
{
	LPWSTR pszTypes, p;
	pszTypes = (LPWSTR)SendMessageW(plugin.hwndParent, WM_WA_IPC,  0, IPC_GET_EXTLISTW);
	if (pszTypes)
	{
		INT i;
		for(i = 0, p = pszTypes; *p != 0 && *(p+1) != 0; p += lstrlenW(p) + 1, i++)
		{
			FILETYPEREC ftr;
			if (S_OK == StringCchCopyW(ftr.szExtension, sizeof(ftr.szExtension) / sizeof(ftr.szExtension[0]), p))
			{
				ftr.Type = FILETYPE_REREAD;
				supportedFilesList.push_back(ftr);
			}
		}
		GlobalFree(pszTypes);
	}

	if (WASABI_API_SVC)
	{
		api_playlistmanager *plMngr = 0;
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
		if (factory) plMngr = (api_playlistmanager*) factory->getInterface();
		if (plMngr)
		{
			FILETYPEREC ftr;
			ftr.Type = FVFT_PLAYLIST;
			size_t playlistEnum=0;
			const wchar_t *playlistExt=0;
			while (NULL != (playlistExt = plMngr->EnumExtension(playlistEnum++))) 
			{ 
				if (S_OK ==StringCchCopyW(ftr.szExtension,  sizeof(ftr.szExtension) / sizeof(ftr.szExtension[0]), playlistExt))
				{
					FileView_GetPLExtensionName(ftr.szExtension, ftr.szFamily, sizeof(ftr.szFamily)/sizeof(ftr.szFamily[0]));
					supportedFilesList.push_back(ftr);
				}
			}
			factory->releaseInterface(plMngr);
		}
	}
}


static UINT FileView_GetFileType(LPCWSTR pszExtension)
{
	FILETYPEREC *pftr = nullptr;

	if (!pszExtension) return FVFT_UNKNOWN;

	if ( 0 == supportedFilesList.size()) 
	{
		FileView_ReadSupportedTypesInfo();
		if (supportedFilesList.size()) {
			//qsort(supportedFilesList.begin(), supportedFilesList.size(), sizeof(FILETYPEREC), QSort_StrCmpI);
			std::sort(supportedFilesList.begin(), supportedFilesList.end(),
				[&](const FILETYPEREC &lhs, const FILETYPEREC &rhs) -> bool
				{
					return CSTR_LESS_THAN == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, (LPCWSTR)(&lhs), -1, (LPCWSTR)(&rhs), -1);
				}
			);
		}
	}

	//pftr = (FILETYPEREC*)bsearch(pszExtension, supportedFilesList.begin(), supportedFilesList.size(), sizeof(FILETYPEREC), QSort_StrCmpI);
	auto it = std::find_if(supportedFilesList.begin(), supportedFilesList.end(),
		[&](const FILETYPEREC& rec) -> bool
		{
			return CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, (LPCWSTR)(&rec), -1, (LPCWSTR)(pszExtension), -1);
		}
	);
	if (it != supportedFilesList.end())
	{
		pftr = &(*it);
	}

	if (pftr)
	{
		if (FILETYPE_REREAD == pftr->Type)
		{
			wchar_t szTest[MAX_PATH] = {0};

			pftr->Type = FVFT_UNKNOWN;
			
			if (S_OK == StringCchPrintfW(szTest, sizeof(szTest)/sizeof(wchar_t), L"test.%s", pftr->szExtension))
			{
				wchar_t szResult[MAX_PATH] = {0};
				extendedFileInfoStructW efis = { szTest, L"type", szResult, sizeof(szResult)/sizeof(szResult[0]), };
				if (SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&efis, IPC_GET_EXTENDED_FILE_INFOW))
				{
					switch(szResult[0])
					{
						case L'0':	pftr->Type = FVFT_AUDIO; break;
						case L'1':	pftr->Type = FVFT_VIDEO; break;
					}
					pftr->szFamily[0] = L'\0';
					switch(pftr->Type)
					{
						case FVFT_AUDIO:
						case FVFT_VIDEO:
							extendedFileInfoStructW efis = { szTest, L"family", pftr->szFamily, sizeof(pftr->szFamily)/sizeof(pftr->szFamily[0]), };
							SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&efis, IPC_GET_EXTENDED_FILE_INFOW);
							break;
					}
				}
			}

		}
		return pftr->Type;
	}
	return FVFT_UNKNOWN;
}


size_t FileView_ReadFileData(FILEDATA *pfd, LPCWSTR pszPath, UINT fStyle, FILESYSTEMINFO *pfsi)
{
	HANDLE hFile;
	wchar_t szSearch[2 * MAX_PATH + 4] = {0};
	
	if (!pfd) return ((size_t)-1);
	
	size_t c = 0;
	pfd->count = 0;
	pfd->folderSize = 0;
	
	if (S_OK != StringCchPrintfW(szSearch, sizeof(szSearch)/sizeof(szSearch[0]), L"%s\\*", pszPath)) return ((size_t)-1);
	
	if (0 == pfd->allocated)
	{
		pfd->pRec = (FILERECORD*)calloc(FILEREC_ALLOCATION_STEP, sizeof(FILERECORD));
		if (!pfd->pRec)
		{			
			return ((size_t)-1);
		}
		pfd->pSort = (size_t*)calloc(FILEREC_ALLOCATION_STEP, sizeof(size_t));
		if (!pfd->pSort)
		{			
			if (pfd->pRec) free(pfd->pRec);
			ZeroMemory(&pfd, sizeof(FILEDATA));
			return ((size_t)-1);
		}
		pfd->allocated = FILEREC_ALLOCATION_STEP;
	}
	
	hFile = pfsi->fnFindFirstFile(szSearch, &pfd->pRec[c].Info);
	if (INVALID_HANDLE_VALUE != hFile) 
	{
		do
		{
			if (0 == (FILE_ATTRIBUTE_DIRECTORY & pfd->pRec[c].Info.dwFileAttributes) && 
				(0 == (FVS_IGNOREHIDDEN & fStyle)  || 0 == (FILE_ATTRIBUTE_HIDDEN & pfd->pRec[c].Info.dwFileAttributes)))
			{
				LPCWSTR pszExt = PathFindExtensionW(pfd->pRec[c].Info.cFileName);
				if (L'.' == *pszExt) pszExt++;
				else pszExt = NULL;

				pfd->pRec[c].extOffset = (pszExt && *pszExt) ? (pszExt - pfd->pRec[c].Info.cFileName) : 0;

				pfd->pRec[c].fileType = FileView_GetFileType(pszExt);
				pfd->pRec[c].pMeta = NULL;

				if ((FVFT_UNKNOWN == pfd->pRec[c].fileType && (FVS_SHOWUNKNOWN & fStyle)) ||
					(FVFT_AUDIO == pfd->pRec[c].fileType && (FVS_SHOWAUDIO & fStyle)) ||
					(FVFT_VIDEO == pfd->pRec[c].fileType && (FVS_SHOWVIDEO & fStyle)) ||
					(FVFT_PLAYLIST == pfd->pRec[c].fileType && (FVS_SHOWPLAYLIST & fStyle)))
				{
				
					if (NULL != pfd->pRec[c].extOffset && FVFT_UNKNOWN != pfd->pRec[c].fileType && (FVS_HIDEEXTENSION & fStyle)) 
						pfd->pRec[c].Info.cFileName[pfd->pRec[c].extOffset -1] = L'\0';
				
					pfd->pSort[c] = c;
					pfd->folderSize += (ULONGLONG)(((__int64)pfd->pRec[c].Info.nFileSizeHigh << 32) | pfd->pRec[c].Info.nFileSizeLow);
					c++;

					if (c == pfd->allocated)
					{
						void *pData;
						pData = realloc(pfd->pRec, sizeof(FILERECORD) * (pfd->allocated + FILEREC_ALLOCATION_STEP));
						if (!pData) { c = ((size_t)-1); break; }
						pfd->pRec = (FILERECORD*)pData;

						pData = realloc(pfd->pSort, sizeof(size_t) * (pfd->allocated + FILEREC_ALLOCATION_STEP));
						if (!pData) { c = ((size_t)-1); break; }
						pfd->pSort= (size_t*)pData;

						pfd->allocated += FILEREC_ALLOCATION_STEP;
						
					}
				}
			}

		} while (pfsi->fnFindNextFile(hFile, &pfd->pRec[c].Info));
		pfsi->fnFindClose(hFile);
	}
	else
	{
		DWORD e = GetLastError();
		if (0 != e) c = ((size_t)-1);
	}

	if (((size_t)-1) != c) pfd->count = c;
	return c;
}

LPCWSTR FileView_GetTypeFamily(LPCWSTR pszExtension)
{
	if (!pszExtension || 0 == supportedFilesList.size()) 
		return NULL;
	
	//FILETYPEREC *pftr = (FILETYPEREC*)bsearch(pszExtension, supportedFilesList.begin(), supportedFilesList.size(), sizeof(FILETYPEREC), QSort_StrCmpI);
	FILETYPEREC* pftr = nullptr;
	auto it = std::find_if(supportedFilesList.begin(), supportedFilesList.end(),
		[&](const FILETYPEREC& rec) -> bool
		{
			return CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, (LPCWSTR)(&rec), -1, (LPCWSTR)(pszExtension), -1);
		}
	);
	if (it != supportedFilesList.end())
	{
		pftr = &(*it);
	}

	return (pftr) ? pftr->szFamily : NULL;
}
