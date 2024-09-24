#include "JSAPI2_TransportAPI.h"
#include "JSAPI2_Security.h"
#include "JSAPI2_CallbackManager.h"
#include "main.h"
#include "JSAPI.h"
#include "JSAPI_CallbackParameters.h"

#define SCRIPT_E_REPORTED 	0x80020101

JSAPI2::TransportAPI::TransportAPI(const wchar_t *_key, JSAPI::ifc_info *info)
{
	this->info = info;
	key = _key;
	refCount = 1;
}

JSAPI2::TransportAPI::~TransportAPI()
{
	// just in case someone forgot
	JSAPI2::callbackManager.Deregister(this);

	size_t index = events.size();
	while(index--)
	{
		IDispatch *pEvent = events[index];
		if (NULL != pEvent)
			pEvent->Release();
	}
}

#define DISP_TABLE \
	CHECK_ID(Prev)\
	CHECK_ID(Play)\
	CHECK_ID(Pause)\
	CHECK_ID(Stop)\
	CHECK_ID(Next)\
	CHECK_ID(shuffle)\
	CHECK_ID(repeat)\
	CHECK_ID(position)\
	CHECK_ID(length)\
	CHECK_ID(url)\
	CHECK_ID(title)\
	CHECK_ID(playing)\
	CHECK_ID(paused)\
	CHECK_ID(GetMetadata)\
	CHECK_ID(RegisterForEvents)\
	CHECK_ID(UnregisterFromEvents)\


#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{ rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }

HRESULT JSAPI2::TransportAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::TransportAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::TransportAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::TransportAPI::ButtonPress(WPARAM button, const wchar_t *action, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	switch (security.GetActionAuthorization(L"transport", action, key, info, JSAPI2::api_security::ACTION_PROMPT))
	{
	case JSAPI2::api_security::ACTION_DISALLOWED:
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
		break;
	case JSAPI2::api_security::ACTION_ALLOWED:
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		SendMessageW(hMainWindow, WM_COMMAND, button, 0);
		break;
	}
	return S_OK;
}

HRESULT JSAPI2::TransportAPI::Prev(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	return ButtonPress(WINAMP_BUTTON1, L"controls", wFlags, pdispparams, pvarResult, puArgErr);
}

HRESULT JSAPI2::TransportAPI::Play(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	return ButtonPress(WINAMP_BUTTON2, L"controls", wFlags, pdispparams, pvarResult, puArgErr);
}

HRESULT JSAPI2::TransportAPI::Pause(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	return ButtonPress(WINAMP_BUTTON3, L"controls", wFlags, pdispparams, pvarResult, puArgErr);
}

HRESULT JSAPI2::TransportAPI::Stop(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	return ButtonPress(WINAMP_BUTTON4, L"controls", wFlags, pdispparams, pvarResult, puArgErr);
}

HRESULT JSAPI2::TransportAPI::Next(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	return ButtonPress(WINAMP_BUTTON5, L"controls", wFlags, pdispparams, pvarResult, puArgErr);
}

HRESULT JSAPI2::TransportAPI::shuffle(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYPUT)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BOOL, puArgErr);
		switch (security.GetActionAuthorization(L"transport", L"controls", key, info, JSAPI2::api_security::ACTION_PROMPT))
		{
		case JSAPI2::api_security::ACTION_DISALLOWED:
			return S_OK;
		case JSAPI2::api_security::ACTION_ALLOWED:
			if (!!config_shuffle != JSAPI_PARAM(pdispparams, 1).lVal)
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_FILE_SHUFFLE, 0);

			return S_OK;
		}
	}
	else if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = (0 != config_shuffle) ? VARIANT_TRUE : VARIANT_FALSE;//(LONG)SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_GET_SHUFFLE);
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

	return E_FAIL;		
}


HRESULT JSAPI2::TransportAPI::repeat(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYPUT)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BOOL, puArgErr);
		switch (security.GetActionAuthorization(L"transport", L"controls", key, info, JSAPI2::api_security::ACTION_PROMPT))
		{
		case JSAPI2::api_security::ACTION_DISALLOWED:
			return S_OK;
		case JSAPI2::api_security::ACTION_ALLOWED:
			if (!!config_repeat != !!JSAPI_PARAM(pdispparams, 1).boolVal)
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_FILE_REPEAT, 0);

			return S_OK;
		}
	}
	else if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = (0 != config_repeat) ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

	return E_FAIL;		
}

HRESULT JSAPI2::TransportAPI::position(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYPUT)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
		if (security.GetActionAuthorization(L"transport", L"controls", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
		{
			SendMessageW(hMainWindow, WM_WA_IPC, JSAPI_PARAM(pdispparams, 1).lVal, IPC_JUMPTOTIME);
		}
		return S_OK;
	}
	else if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_I4;
		V_I4(pvarResult) = (LONG)SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_GETOUTPUTTIME);
		return S_OK;
	}
	
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::TransportAPI::length(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_I4;
		V_I4(pvarResult) = (LONG)SendMessageW(hMainWindow, WM_WA_IPC, 2, IPC_GETOUTPUTTIME);
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::TransportAPI::url(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
		if (security.GetActionAuthorization(L"transport", L"metadata", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
		{
			JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(FileName));
		}
		else
		{
			JSAPI_EMPTY_RESULT(pvarResult);
		}

		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

}

HRESULT JSAPI2::TransportAPI::title(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

		if (security.GetActionAuthorization(L"transport", L"metadata", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
		{
			JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(FileTitle));
		}
		else
		{
			JSAPI_EMPTY_RESULT(pvarResult);
		}

		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::TransportAPI::playing(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		switch (security.GetActionAuthorization(L"transport", L"controls", key, info, JSAPI2::api_security::ACTION_PROMPT))
		{
		case JSAPI2::api_security::ACTION_DISALLOWED:
			V_BOOL(pvarResult) = VARIANT_FALSE;
			break;
		case JSAPI2::api_security::ACTION_ALLOWED:
			V_BOOL(pvarResult) = ::playing?VARIANT_TRUE:VARIANT_FALSE;
			break;
		}

		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::TransportAPI::paused(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		switch (security.GetActionAuthorization(L"transport", L"controls", key, info, JSAPI2::api_security::ACTION_PROMPT))
		{
		case JSAPI2::api_security::ACTION_DISALLOWED:
			V_BOOL(pvarResult) = VARIANT_FALSE;
			break;
		case JSAPI2::api_security::ACTION_ALLOWED:
			V_BOOL(pvarResult) = ::paused?VARIANT_TRUE:VARIANT_FALSE;
			break;
		}

		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::TransportAPI::GetMetadata(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	if (security.GetActionAuthorization(L"transport", L"metadata", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		wchar_t buffer[4096] = {0};
		extendedFileInfoStructW info;

		info.filename = FileName;
		info.metadata = JSAPI_PARAM(pdispparams, 1).bstrVal;
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

HRESULT JSAPI2::TransportAPI::RegisterForEvents(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_DISPATCH, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	switch (security.GetActionAuthorization(L"transport", L"events", key, info, JSAPI2::api_security::ACTION_PROMPT))
	{
	case JSAPI2::api_security::ACTION_DISALLOWED:
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
		break;
	case JSAPI2::api_security::ACTION_ALLOWED:
		{
			/** if this is the first time someone is registering an event
			** add ourselves to the callback manager
			*/
			if (events.empty())
				JSAPI2::callbackManager.Register(this);

			IDispatch *event = JSAPI_PARAM(pdispparams, 1).pdispVal;
			event->AddRef();
			// TODO: benski> not sure, but we might need to: event->AddRef(); 
			events.push_back(event);
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		}
		break;
	}
	return S_OK;
}

HRESULT JSAPI2::TransportAPI::UnregisterFromEvents(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_DISPATCH, puArgErr);

	IDispatch *event = JSAPI_PARAM(pdispparams, 1).pdispVal;
	// TODO: benski> not sure, but we might need to: event->Release(); 

	size_t index = events.size();
	while(index--)
	{
		if (events[index] == event)
		{
			events.erase(events.begin() + index);
			event->Release();
		}
	}
	
	/** if we don't have any more event listeners
	** remove ourselves from the callback manager
	*/
	if (events.empty())
		JSAPI2::callbackManager.Deregister(this);

	return S_OK;
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::TransportAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::TransportAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::TransportAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::TransportAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}

void JSAPI2::TransportAPI::InvokeEvent(const wchar_t *eventName, JSAPI::CallbackParameters::PropertyTemplate *parameters, size_t parametersCount)
{
	size_t index  = events.size();
	if (0 == index) 
	{
		JSAPI2::callbackManager.Deregister(this);
		return;
	}

	JSAPI::CallbackParameters *eventData= new JSAPI::CallbackParameters;
	if (NULL == eventData) return;
		
	eventData->AddString(L"event", eventName);

	if (NULL != parameters && 0 != parametersCount)
		eventData->AddPropertyIndirect(parameters, parametersCount);
	
	HRESULT hr;
	while (index--)
	{
		IDispatch *pEvent = events[index];
		if (NULL != pEvent)
		{
			hr = JSAPI::InvokeEvent(eventData, pEvent);
			if (FAILED(hr) && SCRIPT_E_REPORTED != hr)
			{				
				events.erase(events.begin() + index);
				pEvent->Release();
			}
		}
	}

	if (events.empty())
		JSAPI2::callbackManager.Deregister(this);

	eventData->Release();
}

void JSAPI2::TransportAPI::OnStop(int position, int is_full_stop)
{
	if (!is_full_stop)
	{
		JSAPI::CallbackParameters::PropertyTemplate parameter = 
			{JSAPI::CallbackParameters::typeLong, L"position", (ULONG_PTR)position};
		
		InvokeEvent(L"OnStop", &parameter, 1);
	}
	else
	{
		InvokeEvent(L"OnEndOfFile", NULL, 0);
	}		
}

void JSAPI2::TransportAPI::OnPlay(const wchar_t *filename)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter = 
			{JSAPI::CallbackParameters::typeString, L"filename", (ULONG_PTR)filename};
		
	InvokeEvent(L"OnPlay", &parameter, 1);
}


void JSAPI2::TransportAPI::OnPause(bool pause_state)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter = 
			{JSAPI::CallbackParameters::typeBool, L"paused", (ULONG_PTR)pause_state};
		
	InvokeEvent(L"OnPause", &parameter, 1);
}