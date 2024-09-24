#include "./update.h"
#include "./download.h"
#include "./downloadResult.h"
#include "./loginBox.h"
#include "./loginStatus.h"
#include "./providerLoader.h"
#include "./providerEnumerator.h"

#include "../resource.h"
#include "../api.h"


LoginUpdate::LoginUpdate(HWND hLoginbox)
: ref(1), hwnd(hLoginbox)
{
	InitializeCriticalSection(&lock);
}

LoginUpdate::~LoginUpdate()
{
	EnterCriticalSection(&lock);

	size_t index = downloads.size();
	while(index--)
	{
		LoginDownloadResult *result = downloads[index];
		if (NULL != result)
		{
			result->RequestAbort(TRUE);
			result->Release();
		}
	}
	downloads.clear();

	LeaveCriticalSection(&lock);
	DeleteCriticalSection(&lock);
}

HRESULT LoginUpdate::CreateInstance(HWND hLoginbox, LoginUpdate **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new LoginUpdate(hLoginbox);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginUpdate::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginUpdate::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginUpdate::Start()
{	
	EnterCriticalSection(&lock);

	HRESULT hr;
	if (0 != downloads.size())
		hr = E_PENDING;
	else
	{
		LoginDownload download;
		
		downloads.push_back(NULL);
		LoginDownloadResult **ppResult =  (downloads.end() - 1);
		
		hr = download.Begin(L"http://dl.getdropbox.com/u/1994752/loginProviders.xml", 
						LoginDownloadResult::typeProviderList, LoginUpdate_DownloadCompleted, this, NULL, ppResult);
		if (FAILED(hr))
		{
			downloads.pop_back();
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT LoginUpdate::Abort()
{
	EnterCriticalSection(&lock);
	size_t index = downloads.size();
	while(index--)
	{
		LoginDownloadResult *result = downloads[index];
		if (NULL != result)
		{
			result->RequestAbort(TRUE);
			result->Release();
		}
	}
	downloads.clear();

	LeaveCriticalSection(&lock);
	return S_OK;
}

void LoginUpdate::DownloadCompleted(LoginDownloadResult *result)
{
	
//	BSTR fileName = NULL;
//	LoginDownload download;
//	HRESULT hr = download.End(result, &fileName);
//	
//	SleepEx(10000, FALSE);
//
//	UINT type;
//	if (FAILED(result->GetType(&type)))
//		type = LoginDownloadResult::typeUnknown;
//	
//	switch(type)
//	{
//		case LoginDownloadResult::typeProviderList:
//			{
//				LoginProviderEnumerator *enumerator;
//				if (S_FALSE != hr)
//				{
//					LoginProviderLoader loader;
//					hr = loader.ReadXml(fileName, &enumerator);
//					if (FAILED(hr))
//						enumerator = NULL;
//				}
//				else
//					enumerator = NULL;
//				
//				if (SUCCEEDED(hr))
//				{
//					LoginBox_ProvidersUpdated(hwnd, this, (S_FALSE == hr), enumerator);
//				}
//
//				if (NULL != enumerator)
//					enumerator->Release();
//			}
//			break;
//		case LoginDownloadResult::typeImage:
//			break;
//	}
//	
//	EnterCriticalSection(&lock);
//	
//	size_t index = downloads.size();
//	while(index--)
//	{
//		if (downloads[index] == result)
//		{
//			downloads.eraseindex(index);
//			result->Release();
//			break;
//		}
//	}
//
//	LeaveCriticalSection(&lock);
//
//	SysFreeString(fileName);
}

static void CALLBACK LoginUpdate_DownloadCompleted(LoginDownloadResult *result, void *data)
{
	if (NULL == result) return;
	
	LoginUpdate *update = (LoginUpdate*)data;
	if (NULL != update)
	{
		update->AddRef();
		update->DownloadCompleted(result);
		update->Release();
	}
}
