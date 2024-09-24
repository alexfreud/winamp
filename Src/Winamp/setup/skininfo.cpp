#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include "./skininfo.h"
#include "./loadimage.h"
#include "./langutil.h"
#include "./setup_resource.h"
#include "./api.h"
#include <../xml/obj_xml.h>
#include <api/service/waServiceFactory.h>
#include "../xml/ifc_xmlreadercallbacki.h"
#include "minizip/unzip.h"
#include "../nu/AutoChar.h"

#define ZIP_BUFFER_SIZE 2048

static wchar_t szClassic[64] = {0, };
#define CLASSIC_NAME() ((!*szClassic) ? getStringW(IDS_CLASSIC_SKIN_NAME, szClassic, sizeof(szClassic)/sizeof(wchar_t)) : szClassic)

static BOOL ExtractCurrentFile(unzFile f, LPCWSTR pszPath);
static INT ZipProcessSkinInfo(unzFile f, LPCWSTR pszTemp, SKININFO *psi, LPSTR szSearch, INT cchSearch); // returns length of search

class SkinXMLCallback : public ifc_xmlreadercallbackI
{
public:
	SkinXMLCallback(SKININFO *psi, LPCWSTR pszSkinXML, LPWSTR pszImage, INT cchImage)  // if pszIamge != NULL iamge path will be returned instead of loading image
	{ 
		this->psi = psi; 
		this->pszImage = pszImage;
		this->cchImage = cchImage;
		if (this->pszImage) *this->pszImage = 0x00;
		StringCchCopyW(szPath, sizeof(szPath)/sizeof(wchar_t), pszSkinXML);
		PathRemoveFileSpecW(szPath);
	};
	void xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *s);
	void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);

protected:
	SKININFO *psi;
	wchar_t szPath[MAX_PATH*2];
	LPWSTR pszImage;
	INT cchImage;
};

static BOOL LoadXMLFile(obj_xml *parser, LPCWSTR pszFileName)
{
	HANDLE hFile = CreateFileW(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) return FALSE;
	
	BOOL br (TRUE);
	DWORD bytesRead=0;
	do
	{
		BYTE buffer[1024] = {0};
		bytesRead=0;
		if (ReadFile(hFile, buffer, 1024, &bytesRead, NULL) && bytesRead)
		{
			if (parser->xmlreader_feed(buffer, bytesRead)!=API_XML_SUCCESS)  { br = FALSE; break; }
		}
		else
		{
			if (parser->xmlreader_feed(0, 0) != API_XML_SUCCESS) { br = FALSE; break; }
			bytesRead=0;
		}		
	} while (bytesRead);
	
	CloseHandle(hFile);
	return br;
}

static BOOL ReadSkinInfo(LPCWSTR pszSkinXML, SKININFO *psi, LPWSTR pszImage, INT cchImage)
{
	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		obj_xml *parser = (obj_xml *)parserFactory->getInterface();
		if (parser)
		{
			SkinXMLCallback cb(psi, pszSkinXML, pszImage, cchImage);
			parser->xmlreader_registerCallback(L"WinampAbstractionLayer", &cb);
			parser->xmlreader_registerCallback(L"WinampAbstractionLayer\fSkinInfo\f*", &cb);
			parser->xmlreader_registerCallback(L"WasabiXML", &cb);
            parser->xmlreader_registerCallback(L"WasabiXML\fSkinInfo\f*", &cb);
			parser->xmlreader_registerCallback(L"SkinInfo*", &cb);

			parser->xmlreader_open();
			LoadXMLFile(parser, pszSkinXML);
			parser->xmlreader_unregisterCallback(&cb);
			parser->xmlreader_close();
			parserFactory->releaseInterface(parser);
		}
	}

	return TRUE;
}

void SkinXMLCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, L"WinampAbstractionLayer", -1, xmltag, -1) ||
		CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, L"WasabiXML", -1, xmltag, -1))
	{
		const wchar_t *version = params->getItemValue(L"version");
		if (version) StringCchCopyW(psi->szWasabiVer, SI_VERMAX, version);
		else psi->szWasabiVer[0] = 0x00;
	}
}

void SkinXMLCallback::xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *s)
{
	static const wchar_t *tags[] = { L"name", L"version", L"comment", L"author", L"email", L"homepage", L"screenshot" };
	int cch, i, count = sizeof(tags)/sizeof(wchar_t*);
	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	LPWSTR p;

	for (i = 0; i < count; i++)
	{
		if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, tags[i], -1, xmltag, -1)) break;
	}

	switch(i)
	{
		case 0:  p = psi->szName; cch = SI_NAMEMAX; break;
		case 1:  p = psi->szVersion; cch = SI_VERMAX; break;
		case 2:  p = (SIF_COMMENT & psi->fMask) ? psi->pszComment : NULL; cch = psi->cchComment; break;
		case 3:  p = psi->szAuthor; cch = SI_AUTHORMAX; break;
		case 4:  p = psi->szEmail; cch = SI_EMAILMAX; break;
		case 5:  p = psi->szHomePage; cch = SI_HOMEPAGEMAX; break;
		case 6:	 
			if ((SIF_PREVIEW & psi->fMask) && *s)
			{
				if (pszImage) StringCchCopyW(pszImage, cchImage, s);
				else
				{
					wchar_t szPic[MAX_PATH*2] = {0};
					psi->hPreview = WALoadImage(NULL, NULL, (PathIsRootW(s)) ? s : PathCombineW(szPic, szPath, s), FALSE); 
				}
			}
			return;
		default: return;
	}	

	if (p) StringCchCopyW(p, cch, s);
}

BOOL GetSkinInfo(LPCWSTR pszSkinPath, SKININFO *psi)
{	
	wchar_t szBuffer[MAX_PATH*2] = {0};
	if (!psi || sizeof(SKININFO) != psi->cbSize) return FALSE;
	psi->type = SKIN_TYPE_UNKNOWN;
	
	if (SIF_COMMENT & psi->fMask) 
	{
		if (!psi->pszComment || !psi->cchComment) return FALSE;
		psi->pszComment[0] = 0x00;
	}
	if (SIF_PREVIEW & psi->fMask) psi->hPreview = NULL;

	if (!pszSkinPath || !*pszSkinPath || CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, 
												pszSkinPath, -1, CLASSIC_NAME(), -1))
	{
		// default classic
		psi->type = SKIN_TYPE_CLASSIC;
		StringCchCopyW(psi->szName, SI_NAMEMAX, CLASSIC_NAME());
				
		if (SIF_COMMENT & psi->fMask) StringCchCopyW(psi->pszComment, psi->cchComment, L"Winamp base skin v5.5");
		if (SIF_PREVIEW & psi->fMask) 
			psi->hPreview = WALoadImage2(L"PNG", MAKEINTRESOURCEW(IDB_PREVIEW_CLASSIC), FALSE);
		StringCchCopyW(psi->szAuthor, SI_AUTHORMAX, L"Steve Gedikian");
		StringCchCopyW(psi->szVersion, SI_VERMAX, L"2.0");
		return TRUE;
	}
	if (PathIsDirectoryW(pszSkinPath))
	{
		if (PathFileExistsW(PathCombineW(szBuffer, pszSkinPath, L"skin.xml"))) 
		{
			psi->type = SKIN_TYPE_MODERN;
			ReadSkinInfo(szBuffer, psi, NULL, 0); 
		}
		else if (PathFileExistsW(PathCombineW(szBuffer, pszSkinPath, L"main.bmp")))
		{
			psi->type = SKIN_TYPE_CLASSIC;
			ReadSkinInfo(PathCombineW(szBuffer, pszSkinPath, L"skininfo.xml"), psi, NULL, 0); 
		}
	}
	else
	{
		int len, start, lenSearch;
		DWORD lcid;
		wchar_t szTemp[MAX_PATH] = {0};
		char filename[MAX_PATH] = {0}, search[MAX_PATH] = {0};
		unzFile f;

		if (!GetTempPathW(MAX_PATH, szBuffer) || !GetTempFileNameW(szBuffer, L"WAS", 0, szTemp)) return FALSE;
	
		f = unzOpen(AutoChar(pszSkinPath)); 
		if (!f) return FALSE;
		
		lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		search[0] = 1;
		lenSearch = 0;
		start = 0;
		do
		{
			int pos = -1, nr = unzGoToFirstFile(f);
			while (UNZ_OK == nr && *search)
			{
				pos++;
				if (start && pos == start) {  start = 0; break; }
				if (UNZ_OK == unzGetCurrentFileInfo(f, NULL, filename, sizeof(filename), NULL, 0, NULL, 0) && *filename)
				{
					len = lstrlenA(filename);
					if (!len || filename[len - 1] == '/')  // empty or folder
					{
						nr = unzGoToNextFile(f);
						continue;
					}
					if (SKIN_TYPE_UNKNOWN == psi->type) 
					{
						if ((len == 8 || (len > 8 && '/' == filename[len - 8 - 1])) &&
							CSTR_EQUAL == CompareStringA(lcid, NORM_IGNORECASE, "main.bmp", -1, (filename + (len - 8)), -1))
						{
							psi->type = SKIN_TYPE_CLASSIC;
							StringCchCopyA(search, MAX_PATH, "skininfo.xml");
							lenSearch = lstrlenA(search);
							start = pos;
						}
						else if ((len == 8 || (len > 8 && '/' == filename[len - 8 - 1])) &&
							CSTR_EQUAL == CompareStringA(lcid, NORM_IGNORECASE, "skin.xml", -1, (filename + (len - 8)), -1))
						{
							psi->type = SKIN_TYPE_MODERN;
							lenSearch = ZipProcessSkinInfo(f, szTemp, psi, search, MAX_PATH);
							if (lenSearch) start = pos;
						}
					}
					else if ((len == lenSearch || (len > lenSearch && '/' == filename[len - lenSearch - 1])) &&
							CSTR_EQUAL == CompareStringA(lcid, NORM_IGNORECASE, search, -1, (filename + (len - lenSearch)), -1))
					{
						if (CSTR_EQUAL == CompareStringA(lcid, NORM_IGNORECASE, search, -1, "skininfo.xml", -1))
						{
							lenSearch = ZipProcessSkinInfo(f, szTemp, psi, search, MAX_PATH);
							if (lenSearch) start = pos;
						}
						else // image
						{
							if (ExtractCurrentFile(f, szTemp)) psi->hPreview = WALoadImage(NULL, NULL, szTemp, FALSE);
							else psi->hPreview = NULL;
							*search = 0x00;
						}
					}
				}
				nr = unzGoToNextFile(f);
			}
		} while(search && *search && start);
		unzClose(f);
		
		// delete tmp file
		DeleteFileW(szTemp);
	}

	return TRUE;
}

static BOOL ExtractCurrentFile(unzFile f, LPCWSTR pszPath)
{
	HANDLE hFile;
	OVERLAPPED asyncIO;
	static BOOL isNT = -1;
	BOOL bSuccess(TRUE);

	int l(1), pos(0), bufNum(0);
	char buf[ZIP_BUFFER_SIZE*2] = {0};
	
	if (UNZ_OK != unzOpenCurrentFile(f)) return FALSE;

	if (-1 == isNT) isNT = (GetVersion() < 0x80000000);
	ZeroMemory(&asyncIO, sizeof(OVERLAPPED));
	if (isNT) asyncIO.hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

    hFile = CreateFileW(pszPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
							FILE_ATTRIBUTE_NORMAL | ((isNT) ? FILE_FLAG_OVERLAPPED : 0), NULL);
	if (INVALID_HANDLE_VALUE == hFile) { bSuccess = FALSE; hFile = NULL; }

	while (bSuccess && l > 0)
	{
		DWORD written = 0;
		bufNum = !bufNum; 
		l = unzReadCurrentFile(f, buf+ZIP_BUFFER_SIZE*bufNum, ZIP_BUFFER_SIZE);
		if (l < 0) bSuccess = FALSE;
		if (isNT)
		{
			WaitForSingleObject(asyncIO.hEvent, INFINITE);
			if (l > 0)
			{
				asyncIO.Offset = pos;
				if (!WriteFile(hFile, buf+ZIP_BUFFER_SIZE*bufNum, l, NULL, &asyncIO) && ERROR_IO_PENDING != GetLastError())
				{
					bSuccess = FALSE;
				}
				pos += l;
			}
		}
		else
		{
			if (l > 0)
			{
				if (!WriteFile(hFile, buf+ZIP_BUFFER_SIZE*bufNum, l, &written, NULL)) bSuccess = FALSE;
			}
		}
	} 

	if (hFile) CloseHandle(hFile);
	if (asyncIO.hEvent) CloseHandle(asyncIO.hEvent);
	unzCloseCurrentFile(f);
	return bSuccess;
}

static INT ZipProcessSkinInfo(unzFile f, LPCWSTR pszTemp, SKININFO *psi, LPSTR szSearch, INT cchSearch) // returns length of search
{
	*szSearch = 0x00;
	if (ExtractCurrentFile(f, pszTemp))
	{
		wchar_t szImage[MAX_PATH] = {0};
		if (ReadSkinInfo(pszTemp, psi, szImage, MAX_PATH) && *szImage)
		{
			if(!PathIsRelativeW(szImage))  psi->hPreview = WALoadImage(NULL, NULL, szImage, FALSE);
			else StringCchCopyA(szSearch, cchSearch, AutoChar(szImage));
		}
	}
	return lstrlenA(szSearch);
}