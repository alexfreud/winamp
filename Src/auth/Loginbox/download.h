#ifndef NULLSOFT_AUTH_LOGIN_DOWNLOAD_HEADER
#define NULLSOFT_AUTH_LOGIN_DOWNLOAD_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./downloadResult.h"



class LoginDownload
{

public:
	LoginDownload();
	~LoginDownload();

public:
	HRESULT Begin(LPCWSTR pszUrl, UINT type, LoginDownloadResult::Callback callback, void *data, LoginStatus *status, LoginDownloadResult **result);
	HRESULT End(LoginDownloadResult *result, BSTR *bstrFileName); // return S_FALSE if files binary indentical

private:
	HRESULT SaveProviderList(LoginDownloadResult *result, BSTR *bstrFileName);
	HRESULT SaveImage(LoginDownloadResult *result, BSTR *bstrFileName);
	HRESULT IsBinaryEqual(LPCWSTR pszFile1, LPCWSTR pszFile2);
};

#endif //NULLSOFT_AUTH_LOGIN_DOWNLOAD_HEADER