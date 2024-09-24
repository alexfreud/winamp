#include "./imageCache.h"
#include "./imageLoader.h"
#include "./graphics.h"
#include "./loginBox.h"

#include <api/service/waservicefactory.h>
#include "../../ombrowser/ifc_omutility.h"
#include "../../ombrowser/ifc_omcachemanager.h"
#include "../../ombrowser/ifc_omcachegroup.h"
#include "../../ombrowser/ifc_omcacherecord.h"

#include "../api.h"

LoginImageCache::LoginImageCache(HWND hLoginbox)
	: ref(1), hwnd(hLoginbox), group(NULL)
{
	InitializeCriticalSection(&lock);
}

LoginImageCache::~LoginImageCache()
{
	if (NULL != group)
	{		
		group->Release();
	}

	DeleteCriticalSection(&lock);
}

HRESULT LoginImageCache::CreateInstance(HWND hLoginbox, LoginImageCache **instance)
{
	if (NULL == instance)
		return E_POINTER;
	
	*instance = NULL;
	if (NULL == hLoginbox) return E_INVALIDARG;

	*instance = new LoginImageCache(hLoginbox);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t LoginImageCache::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t LoginImageCache::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int LoginImageCache::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmCacheCallback))
		*object = static_cast<ifc_omcachecallback*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

void LoginImageCache::PathChanged(ifc_omcacherecord *record)
{
	HWND hLoginbox;

	EnterCriticalSection(&lock);
	
	hLoginbox = hwnd;
	
	LeaveCriticalSection(&lock);

	if (NULL != hLoginbox && FALSE != IsWindow(hLoginbox))
	{
		IMAGECACHERESULT result;
		result.imageCache = this;
		result.cacheRecord = record;
		SendMessage(hLoginbox, NLBM_IMAGECACHED, 0, (LPARAM)&result);
	}

}

void LoginImageCache::Finish()
{
	EnterCriticalSection(&lock);
	
	ifc_omcachegroup *groupCopy = group;
	if (NULL != groupCopy) 
		groupCopy->AddRef();

	LeaveCriticalSection(&lock);

	if (NULL != groupCopy)
	{
		groupCopy->Clear();

		WCHAR szGroup[64] = {0};
		if (SUCCEEDED(groupCopy->GetName(szGroup, ARRAYSIZE(szGroup))))
		{
			waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(IFC_OmUtility);
			if (NULL != serviceFactory)
			{
				ifc_omutility *omUtility = (ifc_omutility*)serviceFactory->getInterface();
				if (NULL != omUtility)
				{
					ifc_omcachemanager *cacheManager;
					if (SUCCEEDED(omUtility->GetCacheManager(&cacheManager)))
					{
						cacheManager->Delete(szGroup);
						cacheManager->Release();	
					}
					omUtility->Release();
				}
			}
		}
		
		groupCopy->Release();
	}
}

HRESULT LoginImageCache::InitGroup()
{
	HRESULT hr;
	EnterCriticalSection(&lock);
	if (NULL != group)
	{
		hr = S_FALSE;
	}
	else
	{
		hr  = S_OK;
		waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(IFC_OmUtility);
		if (NULL != serviceFactory)
		{
			ifc_omutility *omUtility = (ifc_omutility*)serviceFactory->getInterface();
			if (NULL != omUtility)
			{
				ifc_omcachemanager *cacheManager;
				if (SUCCEEDED(omUtility->GetCacheManager(&cacheManager)))
				{
					if (FAILED(cacheManager->Find(L"loginBox", TRUE, &group, NULL)))
						group = NULL;
					cacheManager->Release();	
				}
				omUtility->Release();
			}
		}

		if (NULL == group)
			hr = E_NOINTERFACE;
	}
	
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT LoginImageCache::GetImageListIndex(LPCWSTR pszPath, HIMAGELIST himl, UINT *index, UINT *indexActive, UINT *indexDisabled)
{
	if (NULL == pszPath || L'\0' == *pszPath)
		return E_INVALIDARG;
	
	HRESULT hr;
	WCHAR szBuffer[2048] = {0};

	EnterCriticalSection(&lock);

	BOOL fCreated = FALSE;

	hr = InitGroup();
	if (SUCCEEDED(hr))
	{
			
		ifc_omcacherecord *record;
		hr = group->Find(pszPath, TRUE, &record, &fCreated);
		if (S_OK != hr)
		{
			if (S_FALSE == hr) 
				hr = E_PENDING;
		}
		else
		{
			record->RegisterCallback(this);

			hr = record->GetPath(szBuffer, ARRAYSIZE(szBuffer));
			record->Release();
		}
	}
	
	LeaveCriticalSection(&lock);

	if (FAILED(hr)) 
		return hr;
	
	return GetImageListIndexLocal(szBuffer, himl, index, indexActive, indexDisabled);
}

HRESULT LoginImageCache::GetImageListIndexLocal(LPCWSTR pszPath, HIMAGELIST himl, UINT *index, UINT *indexActive, UINT *indexDisabled)
{
	if (NULL == himl || NULL == pszPath || L'\0' == *pszPath)
		return E_INVALIDARG;

	if (NULL == index && NULL == indexActive && NULL == indexDisabled)
		return E_INVALIDARG;

	INT destWidth, destHeight;
	if (0 == ImageList_GetIconSize(himl, &destWidth, &destHeight))
		return E_FAIL;
			
	INT imageWidth, imageHeight;
	HBITMAP hbmp = ImageLoader_LoadBitmap(NULL, pszPath, FALSE, &imageWidth, &imageHeight);
	if (NULL == hbmp) 
		return E_FAIL;
	
	
	HRESULT hr = S_OK;

	RECT imageRect;
	SetRect(&imageRect, 0, 0, imageWidth, imageHeight); 

	if (NULL != indexActive)
	{
		*indexActive = ImageList_Add(himl, hbmp, NULL);
		if (((UINT)-1) == *indexActive) hr = E_FAIL;
	}
	
	if (NULL != index)
	{
		Image_AdjustSaturationAlpha(hbmp, &imageRect, -150, -100);
		
		*index = ImageList_Add(himl, hbmp, NULL);
		if (((UINT)-1) == *index) hr = E_FAIL;
	}

	if (NULL != indexDisabled)
	{
		Image_AdjustSaturationAlpha(hbmp, &imageRect, -600, -600);

		*indexDisabled = ImageList_Add(himl, hbmp, NULL);
		if (((UINT)-1) == *indexDisabled) hr = E_FAIL;
	}
	
	if (NULL != hbmp)
		DeleteObject(hbmp);

	return hr;

}

HBITMAP LoginImageCache::AdjustBitmapSize(HBITMAP hBitmap, INT forceWidth, INT forceHeight)
{
	BITMAP bm;
	if (sizeof(BITMAP) != GetObject(hBitmap, sizeof(bm), &bm))
		return NULL;
	if (bm.bmHeight < 0) bm.bmHeight = -bm.bmHeight;
	
	if (bm.bmWidth == forceWidth && bm.bmHeight == forceHeight)
		return hBitmap;

	HDC hdc, hdcSrc, hdcDst;
	hdc = GetDCEx(NULL, NULL, DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
	if (NULL == hdc) return NULL;

	hdcSrc = CreateCompatibleDC(hdc);
	hdcDst = CreateCompatibleDC(hdc);

	BITMAPINFOHEADER bhi;
	ZeroMemory(&bhi, sizeof(bhi));
	bhi.biSize = sizeof(bhi);
	bhi.biCompression = BI_RGB;
	bhi.biBitCount = 32;
	bhi.biPlanes = 1;
	bhi.biWidth = forceWidth;
	bhi.biHeight = -forceHeight;

	UINT *pixelData;
	HBITMAP hbmpDst = CreateDIBSection(NULL, (LPBITMAPINFO)&bhi, DIB_RGB_COLORS, (void**)&pixelData, NULL, 0);
	if (NULL != hbmpDst)
	{
		for (INT i = 0; i < forceWidth * forceHeight; i++)
			pixelData[i] = 0x00FFFFFF; 
	}
	
	ReleaseDC(NULL, hdc);

	if (NULL != hdcSrc && NULL != hdcDst && NULL != hbmpDst)
	{
		HBITMAP hbmpSrcOrig = (HBITMAP)SelectObject(hdcSrc, hBitmap);
		HBITMAP hbmpDstOrig = (HBITMAP)SelectObject(hdcDst, hbmpDst);
		
		BOOL result;
		if (bm.bmWidth <= forceWidth && bm.bmHeight <= forceHeight)
		{
			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;
			
			result = GdiAlphaBlend(hdcDst, (forceWidth - bm.bmWidth)/2, (forceHeight - bm.bmHeight)/2, 
							bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, bf);
		}
		else
		{
			SetStretchBltMode(hdcDst, HALFTONE);
			result = StretchBlt(hdcDst, 0, 0, forceWidth, forceHeight, 
						hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		}


		if(FALSE != result)
		{
			hBitmap = hbmpDst;
			hbmpDst = NULL;
		}
		else
			hBitmap = NULL;

		SelectObject(hdcSrc, hbmpSrcOrig);
		SelectObject(hdcDst, hbmpDstOrig);
	}
	else
	{
		hBitmap = NULL;
	}
	
	if (NULL != hdcDst) DeleteDC(hdcDst);
	if (NULL != hdcSrc) DeleteDC(hdcSrc);
	if (NULL != hbmpDst) DeleteObject(hbmpDst);

	return hBitmap;
}

#define CBCLASS LoginImageCache
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_PATHCHANGED, PathChanged)
END_DISPATCH;
#undef CBCLASS