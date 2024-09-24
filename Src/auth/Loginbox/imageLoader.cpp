#include "./imageLoader.h"
#include "./common.h"
#include "../api.h"

#include <api/service/waservicefactory.h>

#include <shlwapi.h>

template <class api_T>
void ImageLoader_ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

typedef BOOL (CALLBACK *IMAGEDATAPROCESSOR)(const void * /*data*/, size_t /*dataSize*/, ULONG_PTR /*param*/);


typedef struct __LOADDATAPARAM
{
	BOOL premultiply;
	INT cx;
	INT cy;
	void *pixels;
} LOADDATAPARAM;

typedef struct __GETDIMENSIONPARAM
{
	INT cx;
	INT cy;
} GETDIMENSIONPARAM;

static BOOL CALLBACK ImageLoader_LoadDataCallback(const void *data, size_t size, ULONG_PTR param)
{
	LOADDATAPARAM *loadParam = (LOADDATAPARAM*)param;
	if (NULL == loadParam) return FALSE;

	if (NULL == WASABI_API_PNGLOADER)
	{
		ImageLoader_ServiceBuild(WASABI_API_PNGLOADER, pngLoaderGUID);
		if (NULL == WASABI_API_PNGLOADER) 
			return FALSE;
	}
	
	loadParam->pixels = (FALSE  == loadParam->premultiply) ? 
			WASABI_API_PNGLOADER->loadImageData(data, (INT)size, &loadParam->cx, &loadParam->cy) :
			WASABI_API_PNGLOADER->loadImage(data, (INT)size, &loadParam->cx, &loadParam->cy);

	return (NULL != loadParam->pixels);
}

static BOOL CALLBACK ImageLoader_GetDimensionsCallback(const void *data, size_t size, ULONG_PTR param)
{
	GETDIMENSIONPARAM *dimensionParam = (GETDIMENSIONPARAM*)param;
	if (NULL == dimensionParam) return FALSE;

	if (NULL == WASABI_API_PNGLOADER)
	{
		ImageLoader_ServiceBuild(WASABI_API_PNGLOADER, pngLoaderGUID);
		if (NULL == WASABI_API_PNGLOADER) 
			return FALSE;
	}
	
	return WASABI_API_PNGLOADER->getDimensions(data, (INT)size, &dimensionParam->cx, &dimensionParam->cy);
}

static BOOL ImageLoader_ProcessResource(HINSTANCE hInstance, LPCWSTR pszName, LPCWSTR pszType, IMAGEDATAPROCESSOR processor, ULONG_PTR param)
{
	HRSRC res = FindResourceW(hInstance, pszName, pszType);
	if (NULL == res) return FALSE;
		
	BOOL fSucceeded = FALSE;
	HANDLE handle = LoadResource(hInstance, res);
	if (NULL != handle)
	{
		UINT resourceSize = SizeofResource(hInstance, res);
		if (0 != resourceSize)
		{
			void *resourceData = LockResource(handle);
			if (NULL != resourceData)
				fSucceeded = processor(resourceData, resourceSize, param);
		}
		FreeResource(handle);
	}
	return fSucceeded;
}

static HRESULT ImageLoader_ParseResProtocol(LPWSTR pszAddress, LPCWSTR defaultType, HINSTANCE *module, LPCWSTR *resourceName, LPCWSTR *resourceType)
{
	if (NULL == module || NULL == resourceName || NULL == resourceType)
		return E_POINTER;

	if (NULL == pszAddress || L'\0' == *pszAddress) 
		return E_INVALIDARG;

	INT cchAddress = lstrlenW(pszAddress);
	const WCHAR szPrefix[] = L"res://";
	INT cchPrefix = ARRAYSIZE(szPrefix) - 1;
	if (cchAddress <= cchPrefix) 
		return S_FALSE;

	if (CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, pszAddress, cchPrefix, szPrefix, cchPrefix))
		return S_FALSE;

	pszAddress += cchPrefix;
	cchAddress -= cchPrefix;

	LPWSTR resType = NULL;
	LPWSTR resName = NULL;

	LPWSTR p = pszAddress + cchAddress;
	while (p != pszAddress && L'/' != *p) p--;
	if (p != pszAddress && p < (pszAddress + cchAddress))
	{
		resName = p + 1;
		*p = L'\0';
		p--;
	}
	
	if (NULL == resName || L'\0' == *resName)
		return E_FAIL;
	

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
			resType = p + 1;
			*p = L'\0';
			p--;
		}
	}

	HINSTANCE hModule;
	hModule = LoadLibraryExW(pszAddress, NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
	if (NULL == hModule)
	{
		UINT errorCode = GetLastError();
		if (NULL != resType)
		{
			*(resType - 1) = L'/';
			resType = NULL;
			hModule = LoadLibraryExW(pszAddress, NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
			if (NULL == hModule) errorCode = GetLastError();
		}

		if (ERROR_SUCCESS != errorCode)
			return HRESULT_FROM_WIN32(errorCode);
	}

	if (NULL == resType)
		resType = (LPWSTR)defaultType;

	if (NULL != resType && FALSE == IS_INTRESOURCE(resType) && L'#' == *resType)
	{
		INT typeId;
		if (FALSE != StrToIntExW(resType + 1, STIF_DEFAULT, &typeId))
			resType = MAKEINTRESOURCEW(typeId);
	}

	if (NULL != resName && FALSE == IS_INTRESOURCE(resName) && L'#' == *resName)
	{
		INT nameId;
		if (FALSE != StrToIntExW(resName + 1, STIF_DEFAULT, &nameId))
			resName = MAKEINTRESOURCEW(nameId);
	}

	*module = hModule;
	*resourceName = resName;
	*resourceType = resType;
	return S_OK;
}

static BOOL ImageLoader_ProcessFile(LPCWSTR pszPath, IMAGEDATAPROCESSOR processor, ULONG_PTR param)
{
	HINSTANCE resModule;
	LPCWSTR resName, resType;

	BOOL fSucceeded	= FALSE;

	LPWSTR name = LoginBox_CopyString(pszPath);
	HRESULT hr = ImageLoader_ParseResProtocol(name, RT_RCDATA, &resModule, &resName, &resType);
	if (S_OK == hr)
	{
		fSucceeded = ImageLoader_ProcessResource(resModule, resName, resType, processor, param);
		LoginBox_FreeString(name);
		return fSucceeded;
	}

	LoginBox_FreeString(name);

	if (FAILED(hr))
		return FALSE;

	HANDLE hFile = CreateFileW(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return FALSE;

	UINT resourceSize = GetFileSize(hFile, NULL);
	if (INVALID_FILE_SIZE != resourceSize)
	{
		void *resourceData = malloc(resourceSize);
		if (NULL != resourceData)
		{
			DWORD readed = 0;
			if (0 != ReadFile(hFile, resourceData, resourceSize, &readed, NULL) || resourceSize != readed)
				fSucceeded = processor(resourceData, resourceSize, param);
			free(resourceData);
		}
	}
	CloseHandle(hFile);

	return fSucceeded;
}

void *ImageLoader_LoadData(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, INT *widthOut, INT *heightOut)
{	
	BOOL fSucceeded;
	LOADDATAPARAM param;
	param.premultiply = fPremultiply;
	if (NULL == hInstance && !IS_INTRESOURCE(pszName))
		fSucceeded = ImageLoader_ProcessFile(pszName, ImageLoader_LoadDataCallback, (ULONG_PTR)&param);
	else
		fSucceeded = ImageLoader_ProcessResource(hInstance, pszName, RT_RCDATA, ImageLoader_LoadDataCallback, (ULONG_PTR)&param);

	if (FALSE == fSucceeded)
	{
		if (NULL != widthOut) *widthOut = 0;
		if (NULL != heightOut) *heightOut = 0;
		return NULL;
	}
	
	if (NULL != widthOut) *widthOut = param.cx;
	if (NULL != heightOut) *heightOut = param.cy;

	return param.pixels;
}

void ImageLoader_FreeData(void *data)
{
	if (NULL == data)
		return;

	if (NULL == WASABI_API_MEMMNGR)
	{
		ImageLoader_ServiceBuild(WASABI_API_MEMMNGR, memMgrApiServiceGuid);
		if (NULL == WASABI_API_MEMMNGR) return;
	}
	
	WASABI_API_MEMMNGR->sysFree(data);
}

HBITMAP ImageLoader_LoadBitmapEx(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, BITMAPINFOHEADER *headerInfo, void **dataOut)
{
	INT imageCX, imageCY;
	
	void *data = ImageLoader_LoadData(hInstance, pszName, fPremultiply, &imageCX, &imageCY);
	if (NULL == data) return NULL;
	
	ZeroMemory(headerInfo, sizeof(BITMAPINFOHEADER));
	headerInfo->biSize = sizeof(BITMAPINFOHEADER);
	headerInfo->biCompression = BI_RGB;
	headerInfo->biBitCount = 32;
	headerInfo->biPlanes = 1;
	headerInfo->biWidth = imageCX;
	headerInfo->biHeight = -imageCY;

	void *pixelData;
	HBITMAP bitmap = CreateDIBSection(NULL, (LPBITMAPINFO)headerInfo, DIB_RGB_COLORS, &pixelData, NULL, 0);
	if (NULL != bitmap)
	{
		if (NULL != dataOut) *dataOut = pixelData;
		CopyMemory(pixelData, data, headerInfo->biWidth * abs(headerInfo->biHeight) * sizeof(DWORD));
	}
	else
	{
		if (NULL != dataOut) *dataOut = NULL;
	}
	
	ImageLoader_FreeData(data);
	return bitmap;
}

HBITMAP ImageLoader_LoadBitmap(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, INT *widthOut, INT *heightOut)
{
	BITMAPINFOHEADER header;
	HBITMAP bitmap = ImageLoader_LoadBitmapEx(hInstance, pszName, fPremultiply, &header, NULL);
	if (NULL != bitmap)
	{
		if (NULL != widthOut) *widthOut = header.biWidth;
		if (NULL != heightOut) *heightOut = header.biHeight;
	}
	else
	{
		if (NULL != widthOut) *widthOut = 0;
		if (NULL != heightOut) *heightOut = 0;
	}
	return bitmap;
}

BOOL ImageLoader_GetDimensions(HINSTANCE hInstance, LPCWSTR pszName, INT *widthOut, INT *heightOut)
{
	BOOL fSucceeded;
	GETDIMENSIONPARAM param;
	if (NULL == hInstance && !IS_INTRESOURCE(pszName))
		fSucceeded = ImageLoader_ProcessFile(pszName, ImageLoader_GetDimensionsCallback, (ULONG_PTR)&param);
	else
		fSucceeded = ImageLoader_ProcessResource(hInstance, pszName, RT_RCDATA, ImageLoader_GetDimensionsCallback, (ULONG_PTR)&param);

	if (FALSE == fSucceeded)
	{
		if (NULL != widthOut) *widthOut = 0;
		if (NULL != heightOut) *heightOut = 0;
		return FALSE;
	}
	
	if (NULL != widthOut) *widthOut = param.cx;
	if (NULL != heightOut) *heightOut = param.cy;

	return TRUE;
}