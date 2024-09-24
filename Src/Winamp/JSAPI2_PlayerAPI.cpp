#include "JSAPI2_PlayerAPI.h"
#include "JSAPI2_Security.h"
#include "main.h"
#include "JSAPI.h"
#include "ipc_pe.h"
#include <strsafe.h>

JSAPI2::PlayerAPI::PlayerAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

enum
{
	DISPID_PLAYERAPI_PLAY, // start playing a file immediately (and clear the old playlist)
	DISPID_PLAYERAPI_ENQUEUE, // insert a URL at the end of the playlist
	DISPID_PLAYERAPI_INSERT, // insert a URL at a particular position in the playlist
	DISPID_PLAYERAPI_CLEARQUEUE, // clear the playlist
	DISPID_PLAYERAPI_GETURL, // get the URL (or filename) for an item in the playlist
	DISPID_PLAYERAPI_GETTITLE, // get title for an item in the playlist
	DISPID_PLAYERAPI_GETMETADATA, // get a metadata string for an item in the playlist

	// properties
	DISPID_PLAYERAPI_LENGTH, // length of the playlist
	DISPID_PLAYERAPI_POSITION, // position in the playlist
};

#define DISP_TABLE \
	CHECK_ID(Play, DISPID_PLAYERAPI_PLAY)\
	CHECK_ID(Enqueue, DISPID_PLAYERAPI_ENQUEUE)\
	CHECK_ID(Insert, DISPID_PLAYERAPI_INSERT)\
	CHECK_ID(ClearQueue, DISPID_PLAYERAPI_CLEARQUEUE)\
	CHECK_ID(GetURL, DISPID_PLAYERAPI_GETURL)\
	CHECK_ID(GetTitle, DISPID_PLAYERAPI_GETTITLE)\
	CHECK_ID(GetMetadata, DISPID_PLAYERAPI_GETMETADATA)\
	CHECK_ID(length, DISPID_PLAYERAPI_LENGTH)\
	CHECK_ID(cursor, DISPID_PLAYERAPI_POSITION)\

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT JSAPI2::PlayerAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::PlayerAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PlayerAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PlayerAPI::Play(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 3);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 3, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"player", L"playlist", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{

		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		enqueueFileWithMetaStructW s = {0,0,0,0};
		SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_DELETE);

		//PlayList_delete();
		s.filename = JSAPI_PARAM(pdispparams, 1).bstrVal;
		s.title    = JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, 0);
		s.ext      = NULL;
		s.length   = JSAPI_PARAM_OPTIONAL(pdispparams, 3, lVal, 0);

		/*if (title) 
		PlayList_append_withinfo(filename, title, length);
		else 
		PlayList_appendthing(filename, 0);
		plEditRefresh();
		*/
		SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
		SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_STARTPLAY);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}
	return S_OK;
}

HRESULT JSAPI2::PlayerAPI::Enqueue(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 3);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 3, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"player", L"playlist", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		enqueueFileWithMetaStructW s = {0,0,0,0};

		//PlayList_delete();
		s.filename = JSAPI_PARAM(pdispparams, 1).bstrVal;
		s.title    = JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, 0);
		s.ext      = NULL;
		s.length   = JSAPI_PARAM_OPTIONAL(pdispparams, 3, lVal, 0);

		/*if (title) 
		PlayList_append_withinfo(filename, title, length);
		else 
		PlayList_appendthing(filename, 0);
		plEditRefresh();
		*/
		SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);

	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}
	return S_OK;
}


HRESULT JSAPI2::PlayerAPI::Insert(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 2, 4);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 3, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 4, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"player", L"playlist", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		fileinfoW info;
		COPYDATASTRUCT cds;
		cds.dwData = IPC_PE_INSERTFILENAMEW;
		cds.lpData = &info;
		cds.cbData = sizeof(info);
		StringCbCopyW(info.file, sizeof(info.file), JSAPI_PARAM(pdispparams, 2).bstrVal);
		info.index = JSAPI_PARAM(pdispparams, 1).lVal;
		// benski> TODO const wchar_t *title = JSAPI_PARAM_OPTIONAL(pdispparams, 3, bstrVal, 0);
		// benski> TODO int length = JSAPI_PARAM_OPTIONAL(pdispparams, 4, lVal, 0);

		SendMessageW(hPLWindow, WM_COPYDATA, 0, (LPARAM)&cds);

	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}
	return S_OK;
}

HRESULT JSAPI2::PlayerAPI::ClearQueue(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"player", L"playlist", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_DELETE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}
	return S_OK;
}

HRESULT JSAPI2::PlayerAPI::GetURL(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	if (security.GetActionAuthorization(L"player", L"metadata", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		int position = JSAPI_PARAM(pdispparams, 1).lVal;
		wchar_t filename[FILENAME_SIZE] = {0};
		if (PlayList_getitem2W(position, filename, 0) == 0)
		{
			JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(filename));
		}
		else
		{
			JSAPI_EMPTY_RESULT(pvarResult);
		}
	}
	else
	{
		JSAPI_EMPTY_RESULT(pvarResult);
	}
	return S_OK;
}

HRESULT JSAPI2::PlayerAPI::GetTitle(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	if (security.GetActionAuthorization(L"player", L"metadata", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		int position = JSAPI_PARAM(pdispparams, 1).lVal;
		wchar_t filetitle[FILETITLE_SIZE] = {0};
		if (PlayList_getitem2W(position, 0, filetitle) == 0)
		{
			JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(filetitle));
		}
		else
		{
			JSAPI_EMPTY_RESULT(pvarResult);
		}
	}
	else
	{
		JSAPI_EMPTY_RESULT(pvarResult);
	}
	return S_OK;
}

HRESULT JSAPI2::PlayerAPI::GetMetadata(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	if (security.GetActionAuthorization(L"player", L"metadata", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		int position = JSAPI_PARAM(pdispparams, 1).lVal;
		wchar_t filename[FILENAME_SIZE] = {0};
		if (PlayList_getitem2W(position, filename, 0) == 0)
		{
			wchar_t buffer[4096] = {0};
			extendedFileInfoStructW info;

			info.filename = filename;
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
	}
	else
	{
		JSAPI_EMPTY_RESULT(pvarResult);
	}
	return S_OK;
}

HRESULT JSAPI2::PlayerAPI::length(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_I4;
		V_I4(pvarResult) = PlayList_getlength();
		return S_OK;
	}
	
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::PlayerAPI::cursor(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYPUT)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
		if (security.GetActionAuthorization(L"player", L"playlist", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
		{
			PlayList_setposition(JSAPI_PARAM(pdispparams, 1).lVal);
			plEditRefresh();
		}
		return S_OK;
	}
	else if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_I4;
		V_I4(pvarResult) = PlayList_getPosition();
		return S_OK;
	}
	
	return DISP_E_MEMBERNOTFOUND;
}


#undef CHECK_ID
#define CHECK_ID(str, id) 		case id: return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::PlayerAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::PlayerAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::PlayerAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::PlayerAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
