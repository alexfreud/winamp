#include "FLVCOM.h"
#include "AMFDispatch.h"
FLVCOM flvCOM;
extern bool mute;

static HANDLE DuplicateCurrentThread()
{
	HANDLE fakeHandle = GetCurrentThread();
	HANDLE copiedHandle = 0;
	HANDLE processHandle = GetCurrentProcess();
	DuplicateHandle(processHandle, fakeHandle, processHandle, &copiedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
	return copiedHandle;
}

enum
{
	DISP_FLV_REGISTER_CALLBACK,
	DISP_FLV_UNREGISTER_CALLBACK,
	DISP_FLV_SETMUTE,
};

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT FLVCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("RegisterCallback", DISP_FLV_REGISTER_CALLBACK)
		CHECK_ID("UnregisterCallback", DISP_FLV_UNREGISTER_CALLBACK)
		CHECK_ID("SetMute", DISP_FLV_SETMUTE)
		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT FLVCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT FLVCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT FLVCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	if (pvarResult)
		VariantInit(pvarResult);

	switch (dispid)
	{
	case DISP_FLV_SETMUTE:
		{
			if (pdispparams->rgvarg[0].boolVal)
				mute = true;
			else
				mute = false;
		}
		return S_OK;
	case DISP_FLV_REGISTER_CALLBACK:
	{
		IDispatch *callback = pdispparams->rgvarg[0].pdispVal;

		callbacks.push_back(DispatchCallbackInfo(callback, GetCurrentThreadId(), DuplicateCurrentThread()));
		return S_OK;
	}
	break;
	case DISP_FLV_UNREGISTER_CALLBACK:
	{
		IDispatch *callback = pdispparams->rgvarg[0].pdispVal;
		size_t numCallbacks = callbacks.size();
		while (numCallbacks--)
		{
			if (callbacks[numCallbacks].dispatch == callback)
			{
				CloseHandle(callbacks[numCallbacks].threadHandle);
				callbacks.erase(callbacks.begin() + numCallbacks);
			}
		}
		return S_OK;
	}
	break;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP FLVCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG FLVCOM::AddRef(void)
{
	return 0;
}

ULONG FLVCOM::Release(void)
{
	return 0;
}

static void CallDispatchMethod(IDispatch *dispatch, DISPPARAMS &params, OLECHAR *name)
{
	try
	{
		unsigned int ret;
		DISPID dispid;
		if (SUCCEEDED(dispatch->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
			dispatch->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
	}
	catch (...)
		{}
}

struct APCdata
{
	IDispatch *disp;
	wchar_t *name;
	AMFDispatch *amf;
};

static VOID CALLBACK MetadataAPC(ULONG_PTR param)
{
	APCdata *data = (APCdata *)param;

	DISPPARAMS params;
	VARIANT argument;
	params.cArgs = 1;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = 0;
	params.rgvarg = &argument;
	VariantInit(&argument);
	V_VT(&argument) = VT_DISPATCH;
	V_DISPATCH(&argument) = data->amf;

	CallDispatchMethod(data->disp, params, data->name);

	data->disp->Release();
	data->amf->Release();
	free(data->name);
	delete data;
}

void FLVCOM::MetadataCallback(FLVMetadata::Tag *tag)
{
	if (!callbacks.empty())
	{
		AMFDispatch *disp = new AMFDispatch(tag->parameters); // we're newing this we can refcount

		DWORD curThreadId = GetCurrentThreadId();
		for (size_t i = 0;i != callbacks.size();i++)
		{
			APCdata *data = new APCdata;
			data->disp = callbacks[i].dispatch;
			data->disp->AddRef();
			data->name = _wcsdup(tag->name.str);
			data->amf = disp;
			data->amf->AddRef();

			if (curThreadId == callbacks[i].threadId)
				MetadataAPC((ULONG_PTR)data);
			else
			{
				if (callbacks[i].threadHandle)
					QueueUserAPC(MetadataAPC, callbacks[i].threadHandle, (ULONG_PTR)data);
			}
		}
		disp->Release();
	}
}