#include "main.h"
#include "./ml_imageloader.h"
#include "api__gen_ml.h"
#include "./ml_ipc_0313.h"
#include <api/service/waServiceFactory.h>
#include <api/service/svcs/svc_imgload.h>
#include <api/memmgr/api_memmgr.h>

#include <commctrl.h>
#include <shlwapi.h>

static const GUID pngGUID = { 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };

#ifndef LOAD_LIBRARY_AS_IMAGE_RESOURCE
  #define LOAD_LIBRARY_AS_IMAGE_RESOURCE 0x000000020
#endif //LOAD_LIBRARY_AS_IMAGE_RESOURCE

static svc_imageLoader	*wasabiPNGLoader = NULL;
static api_memmgr		*wasabiMemMgr = NULL;

static BOOL InitializePNGService(void)
{
	waServiceFactory *sf;
	if (!wasabiMemMgr)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
		if (sf) wasabiMemMgr = reinterpret_cast<api_memmgr*>(sf->getInterface());
	}
	if (wasabiMemMgr && !wasabiPNGLoader)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(pngGUID);
		if (sf) wasabiPNGLoader = reinterpret_cast<svc_imageLoader*>(sf->getInterface());
	}
	return (wasabiMemMgr && wasabiPNGLoader);
}

static HBITMAP LoadImage_LoadPngData(void *pngData, UINT pngSize, BOOL fPremultiply)
{
	INT cx, cy;
	HBITMAP bitmap;

	if (NULL == wasabiPNGLoader)
	{
		if (FALSE == InitializePNGService() || 
			NULL == wasabiPNGLoader)
		{
			return NULL;
		}
	}

	pngData = (FALSE != fPremultiply) ?
				wasabiPNGLoader->loadImage(pngData, pngSize, &cx, &cy) : 
				wasabiPNGLoader->loadImageData(pngData, pngSize, &cx, &cy);
	
	
	if (NULL != pngData)
	{
		BITMAPINFOHEADER header;
		void *pixelData;

		ZeroMemory(&header, sizeof(BITMAPINFOHEADER));
		header.biSize = sizeof(BITMAPINFOHEADER);
		header.biBitCount = 32;
		header.biPlanes = 1;
		header.biWidth = cx;
		header.biHeight = -cy;

		bitmap = CreateDIBSection(NULL, (LPBITMAPINFO)&header, DIB_RGB_COLORS, &pixelData, NULL, 0);
		if (NULL != bitmap)
			CopyMemory(pixelData, pngData, cx * cy * sizeof(DWORD));
		
		wasabiMemMgr->sysFree(pngData);
	}
	else
		bitmap = NULL;
	
	return bitmap;
}

static HBITMAP LoadImage_PngResource(HINSTANCE hInstance, HINSTANCE langModule, 
									 LPCWSTR pszName, LPCWSTR pszType, BOOL fPremultiply)
{
	HRSRC res;
	
	res = (NULL != langModule) ? 
				FindResourceW(langModule, pszName, pszType) : 
				NULL;
	
	if (NULL == res)
	{
		res = FindResourceW(hInstance, pszName, pszType);
	}
	else
	{
		hInstance = langModule;
	}
	
	if (NULL == res) 
		return NULL;

	HANDLE handle = LoadResource(hInstance, res);
	if (NULL == handle)
		return NULL;
	
	UINT pngSize = SizeofResource(hInstance, res);
	if (0 == pngSize)
		return NULL;

	
	void *pngData = LockResource(handle);
	if (NULL == pngData)
		return NULL;

	return LoadImage_LoadPngData(pngData, pngSize, fPremultiply); 
	
}

static HBITMAP LoadImage_BmpResource(HINSTANCE hInstace, HINSTANCE langModule, LPCWSTR pszName, LPCWSTR pszType, BOOL fPremultiply)
{
	UINT flags = LR_DEFAULTCOLOR | LR_CREATEDIBSECTION;

	if (NULL != langModule)
	{
		HBITMAP bitmap;

		bitmap = (HBITMAP)LoadImageW(langModule, pszName, IMAGE_BITMAP, 0, 0, flags);
		if (NULL != bitmap)
			return bitmap;
	}
	
	return (HBITMAP)LoadImageW(hInstace, pszName, IMAGE_BITMAP, 0, 0, flags);
}

static BOOL LoadImage_IsValidGuidChar(wchar_t c)
{
	if ((c >= L'0' && c <= L'9') || 
		(c >= L'A' && c <= L'F') || 
		(c >= L'a' && c <= L'f') || 
		(L'-' == c))
	{
		return TRUE;
	}
	return FALSE;
}

static HRESULT LoadImage_CrackResProtocol(LPCWSTR pszAddress, LPCWSTR pszDefaultType, HINSTANCE *module, 
										  HINSTANCE *langModule, LPCWSTR *name, wchar_t **type)
{
	if (NULL == module) 	return E_POINTER;
	if (NULL == pszAddress || L'\0' == *pszAddress)
		return E_INVALIDARG;

	INT cchAddress = lstrlenW(pszAddress);
	const WCHAR szPrefix[] = L"res://";
	INT cchPrefix = ARRAYSIZE(szPrefix) - 1;
	
	if (cchAddress <= cchPrefix || 
		CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, pszAddress, cchPrefix, szPrefix, cchPrefix))
	{
		return S_FALSE;
	}
	
	pszAddress += cchPrefix;
	cchAddress -= cchPrefix;

	LPCWSTR resType = NULL;
	LPCWSTR resName = NULL;

	LPCWSTR p = pszAddress + cchAddress;
	while (p != pszAddress && L'/' != *p) p--;
	if (p != pszAddress && p < (pszAddress + cchAddress))
	{
		resName = p + 1;
		p--;
	}
	
	if (NULL == resName || L'\0' == *resName)
		return E_FAIL;
			
	WCHAR szFile[MAX_PATH + 128] = {0};
	if (FAILED(StringCchCopyNW(szFile, ARRAYSIZE(szFile), pszAddress, (resName - pszAddress - 1))))
		return E_OUTOFMEMORY;

	while (p != pszAddress && L'/' != *p) p--;
	if (p != pszAddress && p < resName)
	{
		resType = p + 1;
		if (L'\0' == *resType)
		{
			resType = NULL;
		}
		else
		{
			size_t pos = (resType - pszAddress);
			szFile[pos - 1] = L'\0';
			resType = &szFile[pos];
		}
	}

	HINSTANCE hModule = LoadLibraryExW(szFile, NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

	if (NULL == hModule && NULL != resType)
	{
		DWORD errorCode = GetLastError();
		if (ERROR_FILE_NOT_FOUND == errorCode)
		{
			*((LPWSTR)(resType - 1)) = L'/';
			resType = NULL;
			hModule = LoadLibraryExW(szFile, NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
		}
	}
		
	if (NULL == hModule)
		return E_FAIL;

	if (NULL != type)
	{
		if (NULL == resType)
			resType = pszDefaultType;

		if (FALSE == IS_INTRESOURCE(resType))
		{
			if (L'#' == *resType)
			{
				INT typeId;
				if (FALSE != StrToIntExW(resType + 1, STIF_DEFAULT, &typeId))
					resType = MAKEINTRESOURCEW(typeId);
				else
					resType = NULL;
			}
			else
			{
				int length = lstrlenW(resType);
				wchar_t *str = (wchar_t*)malloc((length  + 1) * sizeof(wchar_t));
				if (NULL != str)
					CopyMemory(str, resType, sizeof(wchar_t) * (length + 1));
				resType = str;
			}
		}
		
		*type = (wchar_t*)resType;
	}

	if (NULL != langModule)
	{
		*langModule = NULL;

		if (NULL != WASABI_API_LNG && 
			FALSE == SENDWAIPC(plugin.hwndParent, IPC_GETLANGUAGEPACKINSTANCE, 1))
		{
			wchar_t buffer[64], *lang_str;
			GUID lang_id;

			if (0 != LoadStringW(hModule, LANG_DLL_GUID_STRING_ID, buffer, ARRAYSIZE(buffer)))
			{
				lang_str = buffer;
				while(FALSE == LoadImage_IsValidGuidChar(*lang_str))
				{
					if (L'0' == *lang_str)
						break;
					lang_str++;
				}
				
				int len = lstrlenW(lang_str);
				if (len > 0)
				{
					wchar_t *cursor = lang_str + (len - 1);
					while(FALSE == LoadImage_IsValidGuidChar(*cursor))
					{
						if (lang_str == cursor)
							break;
						cursor--;
					}
					*(cursor+1) = L'\0';
				}
			
				if (RPC_S_OK == UuidFromStringW((RPC_WSTR)lang_str, &lang_id))
				{
					*langModule = WASABI_API_LNG->FindDllHandleByGUID(lang_id);
					if (*langModule == hModule)
						*langModule = NULL;
				}
			}
		}
	}

	*module = hModule;
	
	if (NULL != name) 
		*name = resName;
	
	return S_OK;		
}

static HBITMAP 
LoadImage_CrackHBitmapProtocol(const wchar_t *address)
{	
	INT addressLen, prefixLen;
	const WCHAR prefix[] = L"hbitmap://";
	LONGLONG value;
	
	if (NULL == address || L'\0' == *address)
		return NULL;

	addressLen = lstrlenW(address);
	prefixLen = ARRAYSIZE(prefix) - 1;
		
	if (addressLen <= prefixLen || 
		CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, 
						address, prefixLen, prefix, prefixLen))
	{
		return NULL;
	}

	address += prefixLen;
	while(L'\0' != *address && L' ' == *address)
		address++;

	if (FALSE == StrToInt64ExW(address, STIF_SUPPORT_HEX, &value))
		return NULL;

	return (HBITMAP)value;
}
	
static HBITMAP LoadImage_ResProtocol(LPCWSTR pszAddress, LPCWSTR pszDefaultType, 
									 BOOL fPremultiply, BOOL ignoreLocalized)
{
	HRESULT hr;
	HINSTANCE hModule, langModule;
	LPCWSTR resName;
	wchar_t *resType;
	HBITMAP bitmap;
	BOOL loaded = FALSE;

	hr = LoadImage_CrackResProtocol(pszAddress, pszDefaultType, &hModule, 
							(FALSE == ignoreLocalized) ? &langModule : NULL, 
							&resName, &resType);
	
	if (FAILED(hr) || S_FALSE == hr) 
		return NULL;

	if (FALSE != ignoreLocalized)
		langModule = NULL;

	if (NULL != resType)
	{
		if (IS_INTRESOURCE(resType))
		{
			switch((INT)(INT_PTR)resType)
			{
				case (INT)(INT_PTR)RT_BITMAP: 
					bitmap = LoadImage_BmpResource(hModule, langModule, resName, resType, fPremultiply);
					loaded = TRUE;
					break;
			}
		}
	}

	if (FALSE == loaded)
	{
		bitmap = LoadImage_PngResource(hModule, langModule, resName, resType, fPremultiply);
	}

	if (NULL != langModule)
	{
		// do not call FreeLibrary for langModule (it was never loaded)
		// FreeLibrary(langModule);
	}

	FreeLibrary(hModule);

	if (FALSE == IS_INTRESOURCE(resType))
		free(resType);

	return bitmap;
}

static HBITMAP LoadImage_PngFile(LPCWSTR pszPath, BOOL fPremultiply, BOOL ignoreLocalized)
{
	HANDLE hFile = CreateFileW(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return NULL;
			
	HBITMAP bitmap = NULL;
	UINT pngSize = GetFileSize(hFile, NULL);
	if (INVALID_FILE_SIZE != pngSize)
	{
		void *pngData = malloc(pngSize);
		if (NULL != pngData)
		{
			DWORD readed = 0;
			if (0 != ReadFile(hFile, pngData, pngSize, &readed, NULL) || pngSize != readed)
			{
				bitmap = LoadImage_LoadPngData(pngData, pngSize, fPremultiply);
			}
			free(pngData);
		}
	}
	
	CloseHandle(hFile);
	return bitmap;
}

static HBITMAP LoadImage_Png(const MLIMAGESOURCE_I *pImgSource)
{
	HBITMAP bitmap;
	const wchar_t *path;
	BOOL premultiply, ignoreLocalized;

	path = pImgSource->lpszName;
	premultiply = (0 != (ISF_PREMULTIPLY_I & pImgSource->flags));
	ignoreLocalized = (0 != (ISF_NOLOCALIZED_LOAD_I & pImgSource->flags));

	
	if (FALSE == IS_INTRESOURCE(path))
	{
		bitmap = LoadImage_CrackHBitmapProtocol(path);
		if (NULL != bitmap)
		{
			bitmap = (HBITMAP)CopyImage(bitmap,IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			return bitmap;
		}

		bitmap = LoadImage_ResProtocol(path, (LPCWSTR)RT_RCDATA, premultiply, ignoreLocalized);
		if (NULL != bitmap)
			return bitmap;

		if (0 != (ISF_LOADFROMFILE_I & pImgSource->flags))
			return LoadImage_PngFile(path, premultiply, ignoreLocalized);
	}
	
	bitmap = LoadImage_PngResource(pImgSource->hInst, NULL, path, (LPCWSTR)RT_RCDATA, premultiply);
	if (NULL != bitmap)
		return bitmap;

	bitmap = LoadImage_PngResource(pImgSource->hInst, NULL, path, (LPCWSTR)L"PNG", premultiply);
	return bitmap;
}

static HBITMAP LoadImage_Bmp(const MLIMAGESOURCE_I *pImgSource)
{
	HBITMAP bitmap;
	const wchar_t *path;
	BOOL premultiply, ignoreLocalized;

	path = pImgSource->lpszName;
	premultiply = (0 != (ISF_PREMULTIPLY_I & pImgSource->flags));
	ignoreLocalized = (0 != (ISF_NOLOCALIZED_LOAD_I & pImgSource->flags));

	if (FALSE == IS_INTRESOURCE(path))
	{
		bitmap = LoadImage_CrackHBitmapProtocol(path);
		if (NULL != bitmap)
		{
			bitmap = (HBITMAP)CopyImage(bitmap,IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			return bitmap;
		}

		bitmap = LoadImage_ResProtocol(path, (LPCWSTR)RT_BITMAP, premultiply, ignoreLocalized);
		if (NULL != bitmap)
			return bitmap;
	}

	UINT flags = LR_DEFAULTCOLOR | LR_CREATEDIBSECTION;
	if (0 != (ISF_LOADFROMFILE_I & pImgSource->flags))
		flags |= LR_LOADFROMFILE;

	bitmap = (HBITMAP)LoadImageW(pImgSource->hInst, path, IMAGE_BITMAP, 0, 0, flags);

	return bitmap;
}

static HBITMAP LoadImage_HIMAGELIST(const MLIMAGESOURCE_I *pImgSource)
{
//	IMAGEINFO ii;
//	if (!pImgSource) return NULL;
//	return (ImageList_GetImageInfo((HIMAGELIST)pImgSource->hInst, (INT)(INT_PTR)pImgSource->lpszName, &ii)) ? ii.hbmImage : NULL;
	return NULL; // not supported;
}

static HBITMAP DuplicateStretchedDib(HBITMAP hbmpSrc, INT xSrc, INT ySrc, INT cxSrc, INT cySrc, INT cxDst, INT cyDst, INT bppDst, INT bppSrc, BOOL fStretch)
{
	HDC hdcSrc, hdcDst;
	HBITMAP hbmpDst;
	LPVOID dib;
	BITMAPINFOHEADER bi;

	hdcSrc = CreateCompatibleDC(0);
	if (!hdcSrc) return NULL;
    hdcDst = CreateCompatibleDC(0);
	if (!hdcDst) { DeleteDC(hdcSrc); return NULL; }
 
	ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
	bi.biSize		= sizeof (BITMAPINFOHEADER);
	bi.biWidth		= cxDst;
	bi.biHeight		= -ABS(cyDst);
	bi.biPlanes		= 1;
	bi.biBitCount	= bppDst;

	hbmpDst = CreateDIBSection(hdcDst, (BITMAPINFO *)&bi, DIB_RGB_COLORS, &dib, NULL, NULL);
	if (hbmpDst)
	{
		HGDIOBJ hgdiDst = SelectObject(hdcDst, hbmpDst);
		HGDIOBJ hgdiSrc = SelectObject(hdcSrc, hbmpSrc);

		if (fStretch && (cxDst != cxSrc || cyDst != cySrc)) 
		{
			if (32 == bppDst || 32 == bppSrc)
			{
				BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
				GdiAlphaBlend(hdcDst, 0, 0, cxDst, cyDst, hdcSrc, xSrc, ySrc, cxSrc, cySrc, bf); 
			}
			else
			{
				INT stretchModeOld = SetStretchBltMode(hdcDst, HALFTONE); 
				StretchBlt(hdcDst, 0, 0, cxDst, ABS(cyDst), hdcSrc, xSrc, ySrc, cxSrc, cySrc, SRCCOPY);
				SetStretchBltMode(hdcDst, stretchModeOld);
			}
		}
		else BitBlt(hdcDst, 0, 0, cxDst, ABS(cyDst), hdcSrc, xSrc, ySrc, SRCCOPY);

		SelectObject(hdcSrc, hgdiSrc);
		SelectObject(hdcDst, hgdiDst);
	}

	DeleteDC(hdcSrc);
	DeleteDC(hdcDst);

	return hbmpDst;
}

HBITMAP MLImageLoaderI_LoadDib(const MLIMAGESOURCE_I *pImgSource)
{
	HBITMAP hbmp;

	if (NULL == pImgSource) 
		return NULL;

	switch(pImgSource->type)
	{
		case SRC_TYPE_BMP_I:			hbmp = LoadImage_Bmp(pImgSource); break;
		case SRC_TYPE_PNG_I:			hbmp = LoadImage_Png(pImgSource); break;
		case SRC_TYPE_HBITMAP_I:		hbmp = (HBITMAP)CopyImage((HANDLE)pImgSource->lpszName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION); break;
		case SRC_TYPE_HIMAGELIST_I:	hbmp = LoadImage_HIMAGELIST(pImgSource); break;
		default:					hbmp = NULL; break;
	}

	if (NULL != hbmp) // 
	{
		// get bitmap info
		BITMAP bm;

		if (sizeof(bm) == GetObjectW(hbmp, sizeof(bm), &bm))
		{
			if (((ISF_USE_OFFSET_I & pImgSource->flags) && (pImgSource->xSrc || pImgSource->ySrc)) ||
				((ISF_USE_SIZE_I & pImgSource->flags) && (pImgSource->cxSrc != bm.bmWidth || pImgSource->cySrc != bm.bmHeight)) ||
				((ISF_FORCE_SIZE_I & pImgSource->flags) && (pImgSource->cxDst != bm.bmWidth || pImgSource->cyDst != bm.bmHeight)) ||
				((ISF_FORCE_BPP_I & pImgSource->flags) && (pImgSource->bpp != bm.bmBitsPixel)))
			{
				HBITMAP  hOldBmp;
				hOldBmp = hbmp;
				hbmp  = DuplicateStretchedDib(	hbmp, 
												(ISF_USE_OFFSET_I & pImgSource->flags) ? pImgSource->xSrc : 0,
												(ISF_USE_OFFSET_I & pImgSource->flags) ? pImgSource->ySrc : 0,
												(ISF_USE_SIZE_I & pImgSource->flags) ? pImgSource->cxSrc : 	bm.bmWidth,
												(ISF_USE_SIZE_I & pImgSource->flags) ? pImgSource->cySrc : 	bm.bmHeight,
												(ISF_FORCE_SIZE_I & pImgSource->flags) ? pImgSource->cxDst: bm.bmWidth,
												(ISF_FORCE_SIZE_I & pImgSource->flags) ? pImgSource->cyDst: bm.bmHeight,
												(ISF_FORCE_BPP_I & pImgSource->flags) ? pImgSource->bpp : bm.bmBitsPixel,
												bm.bmBitsPixel,
												(ISF_SCALE_I & pImgSource->flags));
				DeleteObject(hOldBmp);
			}
		}
		else
		{
			DeleteObject(hbmp);
			hbmp = NULL;
		}
	}

	return hbmp;
}

BOOL MLImageLoaderI_CopyData(MLIMAGESOURCE_I *pisDst, const MLIMAGESOURCE_I *pisSrc)
{
	if (!pisDst || !pisSrc)  return FALSE;
	CopyMemory(pisDst, pisSrc, sizeof(MLIMAGESOURCE_I));
	if (SRC_TYPE_HBITMAP_I != pisSrc->type && SRC_TYPE_HIMAGELIST_I != pisSrc->type &&
		pisSrc->lpszName && !IS_INTRESOURCE(pisSrc->lpszName)) pisDst->lpszName = _wcsdup(pisSrc->lpszName);
	return TRUE;
}

BOOL MLImageLoaderI_FreeData(MLIMAGESOURCE_I *pis)
{
	if (!pis) return FALSE;
	if (SRC_TYPE_HBITMAP_I != pis->type && SRC_TYPE_HIMAGELIST_I != pis->type && 
		pis->lpszName && !IS_INTRESOURCE(pis->lpszName)) free((LPWSTR)pis->lpszName);
	return TRUE;
}

BOOL MLImageLoaderI_CheckExist(const MLIMAGESOURCE_I *pis)
{
	const wchar_t *resType;

	if (NULL == pis) 
		return FALSE;

	switch(pis->type)
	{
		case SRC_TYPE_HBITMAP_I:		return (NULL != pis->lpszName);
		case SRC_TYPE_HIMAGELIST_I:		return FALSE;
		case SRC_TYPE_PNG_I:			resType = (LPCWSTR)RT_RCDATA; break;
		case SRC_TYPE_BMP_I:			resType = (LPCWSTR)RT_BITMAP; break;
		default:						resType = NULL; break;
	}

	if (FALSE == IS_INTRESOURCE(pis->lpszName))
	{
		BOOL result;
		HINSTANCE module, langModule;
		LPCWSTR name;
		wchar_t *type;
		HRESULT hr;

		if (NULL != LoadImage_CrackHBitmapProtocol(pis->lpszName))
			return TRUE;

		result = FALSE;

		langModule = NULL;

		hr = LoadImage_CrackResProtocol(pis->lpszName, resType, &module, 
						(0 == (ISF_NOLOCALIZED_LOAD_I & pis->flags)) ? &langModule : NULL, 
						&name, &type);

		if (0 != (ISF_NOLOCALIZED_LOAD_I & pis->flags))
			langModule = NULL;

		if (S_OK == hr)
		{
			if (NULL != langModule)
			{
				result = (NULL != FindResourceW(langModule, name, type));
				// do not call FreeLibrary for langModule (it was never loaded)
				//FreeLibrary(langModule);
			}

			if (FALSE == result)
				result = (NULL != FindResourceW(module, name, type));

			FreeLibrary(module);

			if (FALSE == IS_INTRESOURCE(type))
				free(type);

			return result;
		}

		if (0 != (ISF_LOADFROMFILE_I & pis->flags))
		{
			HANDLE hFile = CreateFileW(pis->lpszName, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (INVALID_HANDLE_VALUE != hFile)
			{
				switch(pis->type)
				{		
					case SRC_TYPE_PNG_I:
						if (NULL != wasabiPNGLoader || 
							InitializePNGService())
						{
							BYTE data[8] = {0};  // png signature len
							DWORD dataRead = 0;
							if(ReadFile(hFile, data, sizeof(data), &dataRead, NULL) && 0 != dataRead)
								result = wasabiPNGLoader->testData(data, dataRead);
						}
						break;
					default: 
						result = TRUE; 
						break;
				}
				CloseHandle(hFile);
			}
			return result;
		}
	}

	return (NULL != resType && 
			NULL != FindResourceW(pis->hInst, pis->lpszName, resType));
}