/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "ApplicationCOM.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "api.h"
#include "../Plugins/General/gen_ml/ml.h"
#include <api/syscb/callbacks/browsercb.h>
#include "../Agave/Language/api_language.h"
HINSTANCE WASABI_API_LNG_HINST;
HINSTANCE WASABI_API_ORIG_HINST;
#include <shlwapi.h>
#include "TempFileCOM.h"
#include "resource.h"
#include "JSAPI.h"

#define CSTR_INVARIANT	MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

bool FilterUrl(const wchar_t *url)
{
	const wchar_t filterNowPlaying[] = L"http://client.winamp.com/nowplaying";
	size_t urlLength, filterLength;
	if (NULL == url)
		return false;

	urlLength = lstrlenW(url);
	filterLength =  ARRAYSIZE(filterNowPlaying) - 1;
	if (urlLength >= filterLength &&
		CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, 
							url, (int)filterLength, filterNowPlaying, (int)filterLength))
	{
		return true;
	}

	return false;
}

void myOpenURL(HWND hwnd, const wchar_t *loc)
{
	if (loc)
	{
		bool override=false;
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::BROWSER, BrowserCallback::ONOPENURL, reinterpret_cast<intptr_t>(loc), reinterpret_cast<intptr_t>(&override));
		if (!override && false == FilterUrl(loc))
			ShellExecuteW(hwnd, L"open", loc, NULL, NULL, SW_SHOWNORMAL);
	}
}

void myOpenURLWithFallback(HWND hwnd, wchar_t *loc, wchar_t *fallbackLoc)
{
	bool override=false;
	if (loc)
	{
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::BROWSER, BrowserCallback::ONOPENURL, reinterpret_cast<intptr_t>(loc), reinterpret_cast<intptr_t>(&override));
	}
	if (!override  && false == FilterUrl(loc) && fallbackLoc)
		ShellExecuteW(hwnd, L"open", fallbackLoc, NULL, NULL, SW_SHOWNORMAL);
}


enum
{
	DISP_APPLICATION_IDLE = 777,
	DISP_APPLICATION_GETLANGUAGE,
	DISP_APPLICATION_ISWINAMPPRO,
	DISP_APPLICATION_GETCOUNTRY,
	DISP_APPLICATION_GETLOCALE,
	DISP_APPLICATION_LAUNCHURL,
	DISP_APPLICATION_VERSION,
	DISP_APPLICATION_SPECIALBUILD,
	DISP_APPLICATION_DOWNLOADMEDIA,
	DISP_APPLICATION_CREATETEMPFILE,
};

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT ApplicationCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("Idle", DISP_APPLICATION_IDLE)
		CHECK_ID("GetLanguage", DISP_APPLICATION_GETLANGUAGE)
		CHECK_ID("IsWinampPro", DISP_APPLICATION_ISWINAMPPRO)
		CHECK_ID("GetCountry", DISP_APPLICATION_GETCOUNTRY)
		CHECK_ID("GetLocale", DISP_APPLICATION_GETLOCALE)
		CHECK_ID("LaunchURL", DISP_APPLICATION_LAUNCHURL)
		CHECK_ID("Version", DISP_APPLICATION_VERSION)
		CHECK_ID("GetSpecialBuildName", DISP_APPLICATION_SPECIALBUILD)
		CHECK_ID("DownloadMedia", DISP_APPLICATION_DOWNLOADMEDIA)
		CHECK_ID("CreateTempFile", DISP_APPLICATION_CREATETEMPFILE)
		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT ApplicationCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT ApplicationCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

void GetLanguage(wchar_t *language, size_t size)
{
	StringCchCopyW(language, size, WASABI_API_LNG->GetLanguageIdentifier(LANG_LANG_CODE));
}

void GetCountry(wchar_t *language, size_t size)
{
	// language packs aren't country-specific enough to use them for country info, yet
	int err = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME , language, (int)size);
	if (err == 0) // win95 doesn't support this flag, so we'll check for an error
		lstrcpynW(language, L"US", (int)size); // and default to english
}

void CALLBACK OpenURLAPC(ULONG_PTR param)
{
	wchar_t *url = (wchar_t *)param;
	myOpenURL(NULL, url);
	free(url);
}

void GetPathToStore(wchar_t path_to_store[MAX_PATH])
{
	if (FAILED(SHGetFolderPathW(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
	{
		if (FAILED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
		{
			// and if that all fails then do a reasonable default
			GetPrivateProfileStringW(L"gen_ml_config", L"extractpath", L"C:\\My Music", path_to_store, MAX_PATH, ML_INI_FILE);
		}
		// if there's no valid My Music folder (typically win2k) then default to %my_documents%\my music
		else
		{
			PathCombineW(path_to_store, path_to_store, L"My Music");
		}
	}
}

static void CALLBACK SendOpenUrl_Callback(HWND hwnd, UINT uMsg, ULONG_PTR dwData, LRESULT lResult)
{
	if (NULL != dwData)
		free((void*)dwData);
}

HRESULT ApplicationCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		case DISP_APPLICATION_CREATETEMPFILE:
		{
			IDispatch *tempFile =0;
			if (pdispparams->cArgs == 1)
				tempFile = new TempFileCOM(pdispparams->rgvarg[0].bstrVal);
			else
				tempFile = new TempFileCOM(0);

			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_DISPATCH;
			V_DISPATCH(pvarResult) = tempFile;
			return S_OK;
		}
		case DISP_APPLICATION_DOWNLOADMEDIA:
		{
			const wchar_t *url = pdispparams->rgvarg[0].bstrVal;
			if (url)
			{
				const wchar_t *destFileSpec=PathFindFileNameW(url);
				wchar_t path_to_store[MAX_PATH] = {0};
				GetPathToStore(path_to_store);
				wchar_t destfile[MAX_PATH] = {0};
				wchar_t dlgtitle[256] = {0};

				CreateDirectoryW(path_to_store, NULL);

				PathCombineW(destfile, path_to_store, destFileSpec);
				httpRetrieveFileW(hMainWindow, AutoChar(url), destfile, getStringW(IDS_DOWNLOADING, dlgtitle,256));
				LMDB_FILE_ADD_INFOW fi = {const_cast<wchar_t *>(destfile), -1, -1};
				sendMlIpc(ML_IPC_DB_ADDORUPDATEFILEW, (WPARAM)&fi);
				sendMlIpc(ML_IPC_DB_SYNCDB, 0);
			}
		}
		return S_OK;
		case DISP_APPLICATION_IDLE:
			//Idle();
			return S_OK;
		case DISP_APPLICATION_GETLANGUAGE:
		{
			wchar_t langName[8] = {0};
			GetLanguage(langName, 8);

			BSTR language = SysAllocString(langName);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = language;
			return S_OK;
		}
		case DISP_APPLICATION_GETCOUNTRY:
		{
			wchar_t countryName[8] = {0};
			GetCountry(countryName, 8);

			BSTR country = SysAllocString(countryName);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = country;
			return S_OK;
		}
		case DISP_APPLICATION_GETLOCALE:
		{
			wchar_t countryName[8] = {0};
			GetCountry(countryName, 8);

			wchar_t langName[8] = {0};
			GetLanguage(langName, 8);

			wchar_t language_country[16] = {0};
			StringCchPrintfW(language_country, 16, L"%s-%s", langName, countryName);

			BSTR languageAndCountryCode = SysAllocString(language_country);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = languageAndCountryCode;
			return S_OK;
		}
		case DISP_APPLICATION_ISWINAMPPRO:
		{
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = VARIANT_TRUE;
			return S_OK;
		}
		case DISP_APPLICATION_LAUNCHURL:
			if (pdispparams->cArgs == 1 || pdispparams->cArgs == 2)
			{
				if (JSAPI_PARAM_OPTIONAL(pdispparams, 2, boolVal, FALSE) == TRUE)
					ShellExecuteW(NULL, L"open", JSAPI_PARAM(pdispparams, 1).bstrVal, NULL, L".", 0);
				else
				{
					LPWSTR url = _wcsdup(JSAPI_PARAM(pdispparams, 1).bstrVal);
					if ( 0 == SendMessageCallback(hMainWindow, WM_WA_IPC, (WPARAM)url, (LPARAM)IPC_OPEN_URL, SendOpenUrl_Callback, (ULONG_PTR)url) &&
						0 == QueueUserAPC(OpenURLAPC, hMainThread, (ULONG_PTR)url))
					{
						free(url);
					}
				}
					
				return S_OK;
			}
			else
				return DISP_E_BADPARAMCOUNT;
		case DISP_APPLICATION_VERSION:
		{
			AutoWide versionW(APP_VERSION);
			BSTR versionB = SysAllocString(versionW);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = versionB;
			return S_OK;
		}
		case DISP_APPLICATION_SPECIALBUILD:
		{
			BSTR special = SysAllocString(SPECIAL_BUILD_NAME);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = special;
			return S_OK;
		}
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP ApplicationCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG ApplicationCOM::AddRef(void)
{
	return 0;
}

ULONG ApplicationCOM::Release(void)
{
	return 0;
}

void ApplicationCOM::Idle()
{
	WASABI_API_APP->app_messageLoopStep();
}