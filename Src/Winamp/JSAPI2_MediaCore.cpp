#include "JSAPI2_MediaCore.h"
#include "main.h"
#include "api.h"
#include "JSAPI.h"
#include "JSAPI2_Security.h"
#include "JSAPI2_CallbackManager.h"

JSAPI2::MediaCoreAPI::MediaCoreAPI(const wchar_t *_key, JSAPI::ifc_info *_info) : metadataGuard("MediaCoreAPI metadata Guard")
{
	info = _info;
	key = _key;
	refCount = 1;
}

JSAPI2::MediaCoreAPI::~MediaCoreAPI()
{
	JSAPI2::callbackManager.Deregister(this);
}

#define DISP_TABLE \
	CHECK_ID(IsRegisteredExtension)\
	CHECK_ID(GetMetadata)\
	CHECK_ID(AddMetadataHook)\
	CHECK_ID(RemoveMetadataHook)\


#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{ rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }

HRESULT JSAPI2::MediaCoreAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE

			rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::MediaCoreAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::MediaCoreAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::MediaCoreAPI::IsRegisteredExtension(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	const wchar_t *extension = JSAPI_PARAM(pdispparams, 1).bstrVal;
	int start_offs=0;
	wchar_t filename[MAX_PATH] = {0};
	StringCbPrintfW(filename, sizeof(filename), L"test.%s", extension);
	In_Module *i = in_setmod_noplay(filename, &start_offs);
	if (i)
		V_BOOL(pvarResult) = VARIANT_TRUE;
	else
		V_BOOL(pvarResult) = VARIANT_FALSE;

	return S_OK;
}

HRESULT JSAPI2::MediaCoreAPI::GetMetadata(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	if (security.GetActionAuthorization(L"mediacore", L"metadata", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{		
		wchar_t buffer[4096] = {0};
		extendedFileInfoStructW info;

		info.filename = JSAPI_PARAM(pdispparams, 1).bstrVal;
		info.metadata = JSAPI_PARAM(pdispparams, 2).bstrVal;
		info.ret = buffer;
		info.retlen = sizeof(buffer)/sizeof(*buffer);

		if (NULL != info.filename && 
			NULL != info.metadata)
		{
			if (0 == SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE))
				info.ret = NULL;

			JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(info.ret));
		}
		else
			JSAPI_EMPTY_RESULT(pvarResult);
	}
	else
	{
		JSAPI_EMPTY_RESULT(pvarResult);
	}

	return S_OK;
}
void JSAPI2::MediaCoreAPI::RemoveMetadataHook(const wchar_t *filename)
{
	Nullsoft::Utility::AutoLock metadataLock(metadataGuard);
RemoveMetadataHook_again:
	MetadataMap::iterator itr;
	for (itr=metadataMap.begin();itr!=metadataMap.end();itr++)
	{
		if (!_wcsicmp(filename, itr->url.c_str()))
		{
			metadataMap.erase(itr);
			goto RemoveMetadataHook_again;
		}
	}
}

void JSAPI2::MediaCoreAPI::RemoveMetadataHook(const wchar_t *filename, const wchar_t *tag)
{
	Nullsoft::Utility::AutoLock metadataLock(metadataGuard);
RemoveMetadataHook_again2:
	MetadataMap::iterator itr;
	for (itr=metadataMap.begin();itr!=metadataMap.end();itr++)
	{
		if (!_wcsicmp(filename, itr->url.c_str())
			&& !_wcsicmp(tag, itr->tag.c_str()))
		{
			metadataMap.erase(itr);
			goto RemoveMetadataHook_again2;
		}
	}
}

HRESULT JSAPI2::MediaCoreAPI::AddMetadataHook(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 3);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 3, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"mediacore", L"metadatahook", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		const wchar_t *filename = JSAPI_PARAM(pdispparams, 1).bstrVal;
		const wchar_t *tag = JSAPI_PARAM(pdispparams, 2).bstrVal;
		const wchar_t *value = JSAPI_PARAM(pdispparams, 3).bstrVal;

		if (NULL == filename || L'\0' == *filename ||
			NULL == tag || L'\0' == *tag)
		{
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
			return S_OK;
		}

		JSAPI2::callbackManager.Register(this);
		metadata_info info;
		info.url = filename;
		info.tag = tag;
		info.metadata= value;
		
		Nullsoft::Utility::AutoLock metadataLock(metadataGuard);
		
		RemoveMetadataHook(filename, tag);
		
		metadataMap.push_back(info);
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

HRESULT JSAPI2::MediaCoreAPI::RemoveMetadataHook(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	const wchar_t *filename = JSAPI_PARAM(pdispparams, 1).bstrVal;
	const wchar_t *tag = JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, 0);

	if (NULL == filename || L'\0' == *filename)
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
		return S_OK;
	}

	if (NULL != tag && L'\0' == *tag)
		tag = NULL;

	if (NULL != tag)
		RemoveMetadataHook(filename, tag);
	else
		RemoveMetadataHook(filename);
	

	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);

	return S_OK;
}

bool JSAPI2::MediaCoreAPI::OverrideMetadata(const wchar_t *filename, const wchar_t *tag, wchar_t *out, size_t outCch)
{
	Nullsoft::Utility::AutoLock metadataLock(metadataGuard);
	MetadataMap::iterator itr;
	for (itr=metadataMap.begin();itr!=metadataMap.end();itr++)
	{
		if (!_wcsicmp(filename, itr->url.c_str()) && !_wcsicmp(tag, itr->tag.c_str()))
		{
			StringCchCopyW(out, outCch, itr->metadata.c_str());
			return true;
		}
	}
	return false;
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::MediaCoreAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::MediaCoreAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::MediaCoreAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::MediaCoreAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
