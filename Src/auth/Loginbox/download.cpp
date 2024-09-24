#include "./common.h"
#include "./download.h"
#include "./downloadResult.h"
#include "../api.h"
#include <api/service/waservicefactory.h>

#include "./providerLoader.h"
#include "./providerEnumerator.h"

#include <shlwapi.h>

LoginDownload::LoginDownload()
{
}

LoginDownload::~LoginDownload()
{

}

HRESULT LoginDownload::Begin(LPCWSTR pszUrl, UINT type, LoginDownloadResult::Callback callback, void *data, LoginStatus *pStatus, LoginDownloadResult **result)
{
	if (NULL == result) return E_POINTER;
	*result = NULL;

	if (NULL == pszUrl || L'\0' == *pszUrl)
		return E_INVALIDARG;

	HRESULT hr;

	LPSTR addressAnsi;
	hr = LoginBox_WideCharToMultiByte(CP_UTF8, 0, pszUrl, -1, NULL, NULL, &addressAnsi);
	if (SUCCEEDED(hr))
	{
		if (NULL != WASABI_API_SVC)
		{
			waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(DownloadManagerGUID);
			api_downloadManager *manager = (NULL != sf) ? (api_downloadManager *)sf->getInterface() : NULL;
			if (NULL == manager) 
				hr = E_UNEXPECTED;
			else
			{
				hr = LoginDownloadResult::CreateInstance(manager, type, callback, data, pStatus, result);
				if (SUCCEEDED(hr))
				{
					if (0 == manager->DownloadEx(addressAnsi, *result, api_downloadManager::DOWNLOADEX_TEMPFILE))
					{
						(*result)->Release();
						*result = NULL;
						hr = E_FAIL;
					}
				}
				else
				{
					sf->releaseInterface(manager);
				}
			}
		}
	}
		
	LoginBox_FreeAnsiString(addressAnsi);
	return hr;
}

HRESULT LoginDownload::End(LoginDownloadResult *result, BSTR *bstrFileName)
{
	if (NULL != bstrFileName)
		*bstrFileName = NULL;

	if (NULL == result)	return E_INVALIDARG;

	HRESULT hr;
	UINT state;
	if (FAILED(result->GetState(&state)) || LoginDownloadResult::stateCompleted != state)
	{
		HANDLE completed;
		hr = result->GetWaitHandle(&completed);
		if (FAILED(hr)) return hr;
		
		while(WAIT_OBJECT_0 != WaitForSingleObjectEx(completed, INFINITE, TRUE));
		CloseHandle(completed);
	}

	UINT type;
	hr = result->GetType(&type);
	if (FAILED(hr)) return hr;

	
	
	switch(type)
	{
		case LoginDownloadResult::typeProviderList:
			hr = SaveProviderList(result, bstrFileName);
			break;
		case LoginDownloadResult::typeImage:
			hr = SaveImage(result, bstrFileName);
			break;
		case LoginDownloadResult::typeUnknown:
			hr = E_NOTIMPL;
			break;
	}

	return hr;
}

HRESULT LoginDownload::SaveProviderList(LoginDownloadResult *result, BSTR *bstrFileName)
{
	if (NULL == result) 
		return E_INVALIDARG;

	HRESULT hr;
	LPCWSTR pszSource;
	hr = result->GetFile(&pszSource);
	if (FAILED(hr)) return hr;

	WCHAR szTarget[2048] = {0};
	hr = LoginBox_GetConfigPath(szTarget, TRUE);
	if (FAILED(hr)) return hr;

	if (FALSE == PathAppend(szTarget, L"loginProviders.xml"))
		return E_FAIL;
	
	if (S_OK == IsBinaryEqual(pszSource, szTarget))
		hr = S_FALSE;
	else
	{
		// validate source
		LoginProviderEnumerator *enumerator;
		LoginProviderLoader loader;
		if (FAILED(loader.ReadXml(pszSource, &enumerator, NULL)))
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		else
		{
			enumerator->Release();
						
			if (FALSE == CopyFile(pszSource, szTarget, FALSE))
			{
				DWORD errorCode = GetLastError();
				hr = HRESULT_FROM_WIN32(errorCode);
			}
			else
				hr = S_OK;
		}
	}
	
	if (NULL !=  bstrFileName)
		*bstrFileName = (SUCCEEDED(hr)) ? SysAllocString(szTarget) : NULL;
	
	return hr;
}

HRESULT LoginDownload::SaveImage(LoginDownloadResult *result, BSTR *bstrFileName)
{
	if (NULL == result) 
		return E_INVALIDARG;

	HRESULT hr;
	LPCWSTR pszSource;
	hr = result->GetFile(&pszSource);
	if (FAILED(hr)) return hr;

	WCHAR szTarget[2048] = {0};
	hr = LoginBox_GetConfigPath(szTarget, TRUE);
	if (FAILED(hr)) return hr;

	CHAR szFileAnsi[MAX_PATH] = {0}, szUrlAnsi[2096] = {0};	
	hr = result->CreateDownloadFileName(szFileAnsi, ARRAYSIZE(szFileAnsi));
	if (FAILED(hr)) return hr;

	hr = result->GetUrl(szUrlAnsi, ARRAYSIZE(szUrlAnsi));
	if (FAILED(hr)) return hr;
	
	LPWSTR pszFile;
	hr = LoginBox_MultiByteToWideChar(CP_UTF8, 0, szFileAnsi, -1, &pszFile);
	if (FAILED(hr)) return hr;

	if (FALSE == PathAppend(szTarget, pszFile))
		hr = E_FAIL;
	else if (FALSE == CopyFile(pszSource, szTarget, FALSE))
	{
		DWORD errorCode = GetLastError();
		hr = HRESULT_FROM_WIN32(errorCode);
	}
	if (NULL !=  bstrFileName)
		*bstrFileName = SysAllocString(szTarget);

	LoginBox_FreeString(pszFile);
	return hr;
}

HRESULT LoginDownload::IsBinaryEqual(LPCWSTR pszFile1, LPCWSTR pszFile2)
{
	HRESULT hr;
	HANDLE hFile1, hFile2;

	hFile1 = CreateFile(pszFile1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile1)
	{
		DWORD errorCode = GetLastError();
		hr = HRESULT_FROM_WIN32(errorCode);
	}
	else hr = S_OK;

	if (SUCCEEDED(hr))
	{
		hFile2 = CreateFile(pszFile2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE == hFile2)
		{
			DWORD errorCode = GetLastError();
			hr = HRESULT_FROM_WIN32(errorCode);
		}
	}
	else
		hFile2 = INVALID_HANDLE_VALUE;

	if (SUCCEEDED(hr))
	{
		// check sizes;
		LARGE_INTEGER size1, size2;
		if (FALSE == GetFileSizeEx(hFile1, &size1) || FALSE == GetFileSizeEx(hFile2, &size2))
		{
			DWORD errorCode = GetLastError();
			hr = HRESULT_FROM_WIN32(errorCode);
		}
		else
		{
			if (size1.QuadPart == size2.QuadPart)
			{
				// compare data
				BYTE szBuffer1[4096] = {0}, szBuffer2[4096] = {0};
				for(;;)
				{
					DWORD read1 = 0, read2 = 0;
					if (FALSE == ReadFile(hFile1, szBuffer1, ARRAYSIZE(szBuffer1), &read1, NULL) ||
						FALSE == ReadFile(hFile2, szBuffer2, ARRAYSIZE(szBuffer2), &read2, NULL))
					{
						DWORD errorCode = GetLastError();
						hr = HRESULT_FROM_WIN32(errorCode);
						break;

					}

					if (0 == read1 || 0 == read2)
					{
						hr = (read1 == read2) ? S_OK : S_FALSE;
						break;
					}

					if(read1 != read2 || 0 != memcmp(szBuffer1, szBuffer2, read1))
					{
						hr = S_FALSE;
						break;
					}
				}
			}
			else
				hr = S_FALSE;
		}
	}

	if (INVALID_HANDLE_VALUE != hFile1)
		CloseHandle(hFile1);

	if (INVALID_HANDLE_VALUE != hFile2)
		CloseHandle(hFile2);

	return hr;
}