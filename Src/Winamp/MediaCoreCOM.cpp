/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename:
** Project:
** Description:
** Author: Ben Allison benski@nullsoft.com
** Created:
**/
#include "main.h"
#include "MediaCoreCOM.h"
#include "jsapi.h"

enum
{
	DISP_MEDIACORE_ISREGISTEREDEXTENSION = 777,
	DISP_MEDIACORE_GETMETADATA,
	DISP_MEDIACORE_REGISTER_CALLBACK,
	DISP_MEDIACORE_UNREGISTER_CALLBACK,
	DISP_MEDIACORE_PLAY,
	DISP_MEDIACORE_ENQUEUE,
	DISP_MEDIACORE_PAUSE,
	DISP_MEDIACORE_RESUME,
	DISP_MEDIACORE_VOLUME,
};

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT MediaCoreCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	UNREFERENCED_PARAMETER(riid);

	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("IsRegisteredExtension", DISP_MEDIACORE_ISREGISTEREDEXTENSION)
		CHECK_ID("GetMetadata", DISP_MEDIACORE_GETMETADATA)
		CHECK_ID("RegisterCallback", DISP_MEDIACORE_REGISTER_CALLBACK)
		CHECK_ID("UnregisterCallback", DISP_MEDIACORE_UNREGISTER_CALLBACK)
		CHECK_ID("Play", DISP_MEDIACORE_PLAY)
		CHECK_ID("Enqueue", DISP_MEDIACORE_ENQUEUE)
		CHECK_ID("Pause", DISP_MEDIACORE_PAUSE)
		CHECK_ID("Resume", DISP_MEDIACORE_RESUME)
		CHECK_ID("volume", DISP_MEDIACORE_VOLUME)
		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT MediaCoreCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	UNREFERENCED_PARAMETER(itinfo);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pptinfo);
	return E_NOTIMPL;
}

HRESULT MediaCoreCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	UNREFERENCED_PARAMETER(pctinfo);
	return E_NOTIMPL;
}

HRESULT MediaCoreCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pexecinfo);

	switch (dispid)
	{
	case DISP_MEDIACORE_PLAY:
		{
			if (pdispparams->cArgs < 1 || pdispparams->cArgs > 3)
				return DISP_E_BADPARAMCOUNT;

			/* we're probably not on the main thread, so we'll have to get onto the main thread */
			// TODO: APC this instead of SendMessageW
			enqueueFileWithMetaStructW s = {0,0,0,0};
			SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_DELETE);

			switch(pdispparams->cArgs)
			{
			case 1:
				s.filename = pdispparams->rgvarg[0].bstrVal;
				break;
			case 2:
				s.filename = pdispparams->rgvarg[1].bstrVal;
				s.title    = pdispparams->rgvarg[0].bstrVal;
				break;
			case 3:
				s.filename = pdispparams->rgvarg[2].bstrVal;
				s.title    = pdispparams->rgvarg[1].bstrVal;
				s.length   = pdispparams->rgvarg[0].lVal;
				break;
			}

			s.ext = NULL;

			SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
			SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_STARTPLAY);
			return S_OK;
		}
	case DISP_MEDIACORE_ENQUEUE:
			{
			if (pdispparams->cArgs < 1 || pdispparams->cArgs > 3)
				return DISP_E_BADPARAMCOUNT;

			/* we're probably not on the main thread, so we'll have to get onto the main thread */
			// TODO: APC this instead of SendMessageW
			enqueueFileWithMetaStructW s = {0,0,0,0};

			switch(pdispparams->cArgs)
			{
			case 1:
				s.filename = pdispparams->rgvarg[0].bstrVal;
				break;
			case 2:
				s.filename = pdispparams->rgvarg[1].bstrVal;
				s.title    = pdispparams->rgvarg[0].bstrVal;
				break;
			case 3:
				s.filename = pdispparams->rgvarg[2].bstrVal;
				s.title    = pdispparams->rgvarg[1].bstrVal;
				s.length   = pdispparams->rgvarg[0].lVal;
				break;
			}

			s.ext = NULL;

			SendMessageW( hMainWindow, WM_WA_IPC, (WPARAM)&s, IPC_ENQUEUEFILEW );

			return S_OK;
		}
	case DISP_MEDIACORE_ISREGISTEREDEXTENSION:
	{
		bool isReg = false;
		if (pdispparams->cArgs == 1)
		{
			const wchar_t *filename = pdispparams->rgvarg[0].bstrVal;
			int start_offs=0;
			In_Module *i = in_setmod_noplay(filename, &start_offs);
			if (i)
				isReg = true;
		}
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = (false != isReg) ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	break;

	case DISP_MEDIACORE_GETMETADATA:
		JSAPI_VERIFY_METHOD(wFlags);
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);

		JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
	
		{		
			wchar_t buffer[4096] = {0};
			extendedFileInfoStructW info = {0};

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
		return S_OK;

	case DISP_MEDIACORE_REGISTER_CALLBACK:
		return coreCallbacks.RegisterFromDispParam(pdispparams, 0, puArgErr);
		
	case DISP_MEDIACORE_UNREGISTER_CALLBACK:
		return coreCallbacks.UnregisterFromDispParam(pdispparams, 0, puArgErr);

	case DISP_MEDIACORE_PAUSE:
		PausePlaying();
		return S_OK;
	case DISP_MEDIACORE_RESUME:
		UnPausePlaying();
		return S_OK;
	case DISP_MEDIACORE_VOLUME:
		{
			if (wFlags & DISPATCH_PROPERTYPUT)
			{
				JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
				JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

				SendMessageW(hMainWindow, WM_WA_IPC, JSAPI_PARAM(pdispparams, 1).lVal, IPC_SETVOLUME);

				return S_OK;
			}
			else if (wFlags & DISPATCH_PROPERTYGET)
			{
				JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_I4;
				V_I4(pvarResult) = SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)-666, IPC_SETVOLUME);
				return S_OK;
			}
		}
		break;

	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP MediaCoreCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG MediaCoreCOM::AddRef(void)
{
	return 0;
}

ULONG MediaCoreCOM::Release(void)
{
	return 0;
}

void CallDispatchMethod(IDispatch *dispatch, DISPPARAMS &params, OLECHAR *name)
{
	unsigned int ret;
	DISPID dispid;

	if (NULL == dispatch)
		return;

	if (!(config_no_visseh&8))
	{
		__try
		{
			if (SUCCEEDED(dispatch->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
				dispatch->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
		} 
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	else
	{
		if (SUCCEEDED(dispatch->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
			dispatch->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
	}
}

static void Play_NotifyCb(IDispatch *dispatch, void *param)
{
	UNREFERENCED_PARAMETER(param);

	DISPPARAMS params;

	if (NULL == dispatch)
		return;
	
	params.cArgs = 0;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = 0;
	params.rgvarg = 0;
	
	CallDispatchMethod(dispatch, params, L"OnPlay");
}

struct StopNotifyParam
{
	DISPPARAMS params;
	VARIANT arguments[2];

	StopNotifyParam(int last_time, int fullstop)
	{
		VariantInit(&arguments[0]);
		V_VT(&arguments[0]) = VT_BOOL;
		V_BOOL(&arguments[0]) = (0 != fullstop) ? VARIANT_TRUE : VARIANT_FALSE;

		VariantInit(&arguments[1]);
		V_VT(&arguments[1]) = VT_I4;
		V_I4(&arguments[1]) = last_time;

		params.cArgs = ARRAYSIZE(arguments);
		params.cNamedArgs = 0;
		params.rgdispidNamedArgs = NULL;
		params.rgvarg = arguments;
	}
};

static void Stop_NotifyCb(IDispatch *dispatch, void *param)
{
	StopNotifyParam *stopParams = (StopNotifyParam*)param;

	if (NULL == dispatch ||
		NULL == stopParams)
	{
		return;
	}
	
	CallDispatchMethod(dispatch, stopParams->params, L"OnStop");
}

static void Stop_FreeCb(void *param)
{
	StopNotifyParam *stopParam;
	stopParam = (StopNotifyParam*)param;

	if (NULL != stopParam)
		delete(stopParam);
}

static void Pause_NotifyCb(IDispatch *dispatch, void *param)
{
	UNREFERENCED_PARAMETER(param);

	DISPPARAMS params;

	if (NULL == dispatch)
		return;
	
	params.cArgs = 0;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = 0;
	params.rgvarg = 0;
	
	CallDispatchMethod(dispatch, params, L"OnPause");
}

static void Resume_NotifyCb(IDispatch *dispatch, void *param)
{
	UNREFERENCED_PARAMETER(param);

	DISPPARAMS params;

	if (NULL == dispatch)
		return;
	
	params.cArgs = 0;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = 0;
	params.rgvarg = 0;
	
	CallDispatchMethod(dispatch, params, L"OnResume");
}

void MediaCoreCOM::OnPlay()
{
	coreCallbacks.Notify(Play_NotifyCb, NULL, NULL);
}

void MediaCoreCOM::OnStop(int last_time, int fullstop)
{
	StopNotifyParam *param;

	param = new StopNotifyParam(last_time, fullstop);
	if (NULL == param)
		return;

	coreCallbacks.Notify(Stop_NotifyCb, Stop_FreeCb, param);
}

void MediaCoreCOM::OnPause()
{
	coreCallbacks.Notify(Pause_NotifyCb, NULL, NULL);
}
void MediaCoreCOM::OnResume()
{
	coreCallbacks.Notify(Resume_NotifyCb, NULL, NULL);
}
