#include "JSAPI2_Downloader.h"
#include "JSAPI2_Security.h"
#include "main.h"
#include "../Agave/Language/api_language.h"
#include "JSAPI.h"
#include "../nu/AutoChar.h"
#include "api.h"
#include "resource.h"
#include "../Plugins/General/gen_ml/ml.h"
#include <api/service/svcs/svc_imgload.h>

JSAPI2::DownloaderAPI::DownloaderAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

enum
{
	DISP_DOWNLOADERAPI_DOWNLOADMEDIA,
};

#define DISP_TABLE \
	CHECK_ID(DownloadMedia, DISP_DOWNLOADERAPI_DOWNLOADMEDIA)\

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT JSAPI2::DownloaderAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE;

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::DownloaderAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::DownloaderAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

bool IsImage(const wchar_t *filename);
bool IsPlaylist(const wchar_t *filename);
bool IsMedia(const wchar_t *filename);
bool GetOnlineDownloadPath(const wchar_t *key, const wchar_t *svcname, wchar_t path_to_store[MAX_PATH]);
HRESULT JSAPI2::DownloaderAPI::DownloadMedia(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 3);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 3, VT_BOOL, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"downloader", L"downloadmedia", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);

		const wchar_t *url = JSAPI_PARAM(pdispparams, 1).bstrVal;
		wchar_t *destFileSpec=JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, PathFindFileNameW(url));
		
		//filter reserved characters in file name
		CleanNameForPath(destFileSpec);

		// verify that passed-in URL is a valid media type
		if (!IsImage(destFileSpec) && !IsPlaylist(destFileSpec) && !IsMedia(destFileSpec))
		{
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
			return S_OK;
		}

		wchar_t path_to_store[MAX_PATH] = {0};
		if (GetOnlineDownloadPath(this->key, this->info->GetName(), path_to_store))
		{
			wchar_t dlgtitle[256] = {0};
			CreateDirectoryW(path_to_store, NULL);
			
			wchar_t destfile[MAX_PATH] = {0};
			PathCombineW(destfile, path_to_store, destFileSpec);

			httpRetrieveFileW(hMainWindow, AutoChar(url), destfile, getStringW(IDS_DOWNLOADING,dlgtitle,256));

			// TODO: optional adding to media library or not (param 3)
			LMDB_FILE_ADD_INFOW fi = {const_cast<wchar_t *>(destfile), -1, -1};
			sendMlIpc(ML_IPC_DB_ADDORUPDATEFILEW, (WPARAM)&fi);
			sendMlIpc(ML_IPC_DB_SYNCDB, 0);
		}
		else
		{
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
		}

	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}
	return S_OK;
}

#undef CHECK_ID
#define CHECK_ID(str, id) 		case id: return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::DownloaderAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::DownloaderAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::DownloaderAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::DownloaderAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
