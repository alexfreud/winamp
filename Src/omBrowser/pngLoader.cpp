#include "main.h"
#include "./pngLoader.h"
#include "./browserObject.h"
#include "./ifc_wasabihelper.h"

#include <shlwapi.h>
#include <strsafe.h>

#ifndef LOAD_LIBRARY_AS_IMAGE_RESOURCE
  #define LOAD_LIBRARY_AS_IMAGE_RESOURCE 0x000000020
#endif //LOAD_LIBRARY_AS_IMAGE_RESOURCE

PngLoader::PngLoader(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply) 
	: ref(1), instance(hInstance), name(NULL), flags(0)

{	
	name = Plugin_DuplicateResString(pszName);

	if (FALSE != fPremultiply)
		flags |= flagPremultiply;
}

PngLoader::~PngLoader()
{
	Plugin_FreeResString(name);
}

HRESULT PngLoader::CreateInstance(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader)
{
	if (NULL == imageLoader) return E_POINTER;
	*imageLoader = NULL;
	if (NULL == pszName) return E_INVALIDARG;

	*imageLoader = new PngLoader(hInstance, pszName, fPremultiply);
	if (NULL == *imageLoader) return E_OUTOFMEMORY;
	
	return S_OK;
}

size_t PngLoader::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t PngLoader::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int PngLoader::QueryInterface(GUID interface_guid, void **object)
{
	return E_NOINTERFACE;
}

static HRESULT PngLoader_LoadResourceData(const void *data, unsigned int size, void **dataOut, int *cx, int *cy, BOOL fPremultiply)
{
	if (NULL == data) return E_INVALIDARG;

	svc_imageLoader *loader = NULL;
	ifc_wasabihelper *wasabi = NULL;
	if (FAILED(Plugin_GetWasabiHelper(&wasabi))) return E_UNEXPECTED;

	HRESULT hr = wasabi->GetPngLoader(&loader);
	wasabi->Release();
	if (FAILED(hr))	return hr;


	*dataOut = (FALSE  == fPremultiply) ? 
				loader->loadImageData(data, size, cx, cy) :
				loader->loadImage(data, size, cx, cy);
		
	if (NULL == *dataOut)
		hr = E_OUTOFMEMORY;
	
	loader->Release();
	return hr;
}

static HRESULT PngLoader_LoadFromResource(HINSTANCE hInstance, LPCWSTR pszName, LPCWSTR pszType, void **dataOut, int *cx, int *cy, BOOL fPremultiply)
{
	UINT errorCode = 0;
	HRSRC res = FindResourceW(hInstance, pszName, pszType);
	if (NULL == res)
	{
		errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}
	
	HRESULT hr;
	HANDLE handle = LoadResource(hInstance, res);
	if (NULL == handle)
	{
		errorCode = GetLastError();
		hr = HRESULT_FROM_WIN32(errorCode); 
	}
	else
	{
		UINT resourceSize = SizeofResource(hInstance, res);
		if (0 == resourceSize)
		{
			errorCode = GetLastError();
			hr = HRESULT_FROM_WIN32(errorCode); 
		}
		else
		{
			void *resourceData = LockResource(handle);
			if (NULL == resourceData) 
				hr = E_OUTOFMEMORY;
			else
				hr = PngLoader_LoadResourceData(resourceData, resourceSize, dataOut, cx, cy, fPremultiply);
		}

		FreeResource(handle);
	}

	return hr;
}

static HRESULT PngLoader_ParseResProtocol(LPWSTR pszAddress, LPCWSTR defaultType, HINSTANCE *module, LPCWSTR *resourceName, LPCWSTR *resourceType)
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

	HINSTANCE hModule = LoadLibraryExW(pszAddress, NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
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
		INT typeId = 0;
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

static HRESULT PngLoader_LoadFromFile(LPWSTR pszPath, void **dataOut, int *cx, int *cy, BOOL fPremultiply)
{
	HINSTANCE resModule = NULL;
	LPCWSTR resName = NULL, resType = NULL;
	HRESULT hr = PngLoader_ParseResProtocol(pszPath, RT_RCDATA, &resModule, &resName, &resType);
	if (S_OK == hr)
		return PngLoader_LoadFromResource(resModule, resName, resType, dataOut, cx, cy, fPremultiply);

	if (FAILED(hr))
		return hr;

	UINT errorCode = 0;
	HANDLE hFile = CreateFileW(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	UINT resourceSize = GetFileSize(hFile, NULL);
	if (INVALID_FILE_SIZE == resourceSize)
	{
		errorCode = GetLastError();
		hr = HRESULT_FROM_WIN32(errorCode);
	}
	else
	{
		void *resourceData = malloc(resourceSize);
		if (NULL == resourceData)
			hr = E_OUTOFMEMORY;
		else
		{
			DWORD readed = 0;
			if (0 == ReadFile(hFile, resourceData, resourceSize, &readed, NULL) || resourceSize != readed)
			{
				errorCode = GetLastError();
				hr = HRESULT_FROM_WIN32(errorCode);
			}
			else
			{
				hr = PngLoader_LoadResourceData(resourceData, resourceSize, dataOut, cx, cy, fPremultiply);
			}
			free(resourceData);
		}
	}
	CloseHandle(hFile);
	return hr;
}

HRESULT PngLoader::LoadData(int *widthOut, int *heightOut, void** dataOut)
{
	if (NULL == dataOut) 
		return E_POINTER;
	*dataOut = NULL;

	return (NULL == instance && !IS_INTRESOURCE(name))?
		PngLoader_LoadFromFile(name, dataOut, widthOut, heightOut, (0 != (flagPremultiply & flags))) :
		PngLoader_LoadFromResource(instance, name, RT_RCDATA, dataOut, widthOut, heightOut, (0 != (flagPremultiply & flags)));
}

HRESULT  PngLoader::FreeData(void *data)
{
	if (NULL == data)
		return S_FALSE;

	ifc_wasabihelper *wasabi = NULL;
	HRESULT hr = Plugin_GetWasabiHelper(&wasabi);
	if (SUCCEEDED(hr) && wasabi != NULL)
	{
		api_memmgr *memoryManager = NULL;
		hr = wasabi->GetMemoryManager(&memoryManager);
		if (SUCCEEDED(hr) && memoryManager != NULL) 
		{
			memoryManager->sysFree(data);
			memoryManager->Release();
		}
		wasabi->Release();
	}
	return hr;
}

HRESULT PngLoader::LoadBitmapEx(HBITMAP *bitmapOut, BITMAPINFOHEADER *headerInfo, void **dataOut)
{
	INT imageCX, imageCY;
	
	if(NULL == bitmapOut) return E_POINTER;

	void *data = NULL;
	HRESULT hr = LoadData(&imageCX, &imageCY, &data);
	if (FAILED(hr)) return  hr;

	ZeroMemory(headerInfo, sizeof(BITMAPINFOHEADER));
	headerInfo->biSize = sizeof(BITMAPINFOHEADER);
	headerInfo->biCompression = BI_RGB;
	headerInfo->biBitCount = 32;
	headerInfo->biPlanes = 1;
	headerInfo->biWidth = imageCX;
	headerInfo->biHeight = -imageCY;

	*bitmapOut = CreateDIBSection(NULL, (LPBITMAPINFO)headerInfo, DIB_RGB_COLORS, dataOut, NULL, 0);
	if (NULL != (*bitmapOut))
	{
		CopyMemory((*dataOut), data, headerInfo->biWidth * abs(headerInfo->biHeight) * sizeof(DWORD));
	}
	else
	{
		*dataOut = NULL;
		hr = E_FAIL;
	}
	FreeData(data);
	return hr;
}

HRESULT  PngLoader::LoadBitmap(HBITMAP *bitmapOut, int *widthOut, int *heightOut)
{
	BITMAPINFOHEADER header = {0};
	void *pixelData = NULL;

	if(NULL == bitmapOut) return E_POINTER;

	HRESULT hr = LoadBitmapEx(bitmapOut, &header, &pixelData);
	if (SUCCEEDED(hr))
	{
		if (NULL != widthOut) *widthOut = header.biWidth;
		if (NULL != heightOut) *heightOut = header.biHeight;
	}
	else
	{
		if (NULL != widthOut) *widthOut = 0;
		if (NULL != heightOut) *heightOut = 0;
	}
	return hr;
}

#define CBCLASS PngLoader
START_DISPATCH;
CB(ADDREF, AddRef);
CB(RELEASE, Release);
CB(QUERYINTERFACE, QueryInterface);
CB(API_LOADDATA, LoadData);
CB(API_FREEDATA, FreeData);
CB(API_LOADBITMAP, LoadBitmap);
CB(API_LOADBITMAPEX, LoadBitmapEx);
END_DISPATCH;
#undef CBCLASS