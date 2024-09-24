/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "Main.h"
#include "CurrentSongCOM.h"
#include "../nu/AutoWide.h"
#include "Browser.h"
#include "JSAPI.h"
#include <malloc.h>

HANDLE DuplicateCurrentThread()
{
	HANDLE fakeHandle = GetCurrentThread();
	HANDLE copiedHandle = 0;
	HANDLE processHandle = GetCurrentProcess();
	DuplicateHandle(processHandle, fakeHandle, processHandle, &copiedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
	return copiedHandle;
}

enum
{
  DISP_CURRENTSONG_GETFILENAME = 777,
  DISP_CURRENTSONG_GETFILETITLE ,
  DISP_CURRENTSONG_GETFILELENGTH,
  DISP_CURRENTSONG_GETMETADATA ,
  DISP_CURRENTSONG_GETPLAYPOSITION ,
  DISP_CURRENTSONG_ISPLAYING ,
  DISP_CURRENTSONG_ISSTOPPED ,
  DISP_CURRENTSONG_ISPAUSED ,
	DISP_CURRENTSONG_PAUSE ,
	DISP_CURRENTSONG_RESUME,
  DISP_CURRENTSONG_REGISTERMETADATACALLBACK,
  DISP_CURRENTSONG_UNREGISTERMETADATACALLBACK,
  DISP_CURRENTSONG_REGISTERTITLECHANGECALLBACK,
  DISP_CURRENTSONG_UNREGISTERTITLECHANGECALLBACK,
	DISP_CURRENTSONG_REFRESHTITLE,
};


#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT CurrentSongCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	UNREFERENCED_PARAMETER(riid);

	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("GetFilename", DISP_CURRENTSONG_GETFILENAME) // July 27, 2005
		CHECK_ID("GetFileTitle", DISP_CURRENTSONG_GETFILETITLE) // July 27, 2005
		CHECK_ID("GetFileLength", DISP_CURRENTSONG_GETFILELENGTH) // July 27, 2005
		CHECK_ID("GetMetadata", DISP_CURRENTSONG_GETMETADATA) // July 27, 2005
		CHECK_ID("GetPlayPosition", DISP_CURRENTSONG_GETPLAYPOSITION) // July 27, 2005
		CHECK_ID("IsPlaying", DISP_CURRENTSONG_ISPLAYING) // July 27, 2005
		CHECK_ID("IsStopped", DISP_CURRENTSONG_ISSTOPPED) // July 27, 2005
		CHECK_ID("IsPaused", DISP_CURRENTSONG_ISPAUSED) // July 27, 2005
		CHECK_ID("Pause", DISP_CURRENTSONG_PAUSE)
		CHECK_ID("Resume", DISP_CURRENTSONG_RESUME)
		CHECK_ID("RegisterMetadataCallback", DISP_CURRENTSONG_REGISTERMETADATACALLBACK)
		CHECK_ID("UnregisterMetadataCallback", DISP_CURRENTSONG_UNREGISTERMETADATACALLBACK)
		CHECK_ID("RegisterTitleChangeCallback", DISP_CURRENTSONG_REGISTERTITLECHANGECALLBACK)
		CHECK_ID("UnregisterTitleChangeCallback", DISP_CURRENTSONG_UNREGISTERTITLECHANGECALLBACK)
		CHECK_ID("RefreshTitle", DISP_CURRENTSONG_REFRESHTITLE)

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT CurrentSongCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	UNREFERENCED_PARAMETER(itinfo);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pptinfo);

	return E_NOTIMPL;
}

HRESULT CurrentSongCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	UNREFERENCED_PARAMETER(pctinfo);

	return E_NOTIMPL;
}


HRESULT CurrentSongCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(wFlags);
	UNREFERENCED_PARAMETER(pexecinfo);

	switch (dispid)
	{
	case DISP_CURRENTSONG_REFRESHTITLE:
	
			SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
			return S_OK;
		
		case DISP_CURRENTSONG_REGISTERTITLECHANGECALLBACK:
			return titleChangeCallbacks.RegisterFromDispParam(pdispparams, 0, puArgErr);
		case DISP_CURRENTSONG_UNREGISTERTITLECHANGECALLBACK:
			return titleChangeCallbacks.UnregisterFromDispParam(pdispparams, 0, puArgErr);
		case DISP_CURRENTSONG_REGISTERMETADATACALLBACK:
			return metadataCallbacks.RegisterFromDispParam(pdispparams, 0, puArgErr);
		case DISP_CURRENTSONG_UNREGISTERMETADATACALLBACK:
			return metadataCallbacks.UnregisterFromDispParam(pdispparams, 0, puArgErr);
		case DISP_CURRENTSONG_GETMETADATA:
			JSAPI_VERIFY_METHOD(wFlags);
			JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
			JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
			JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
			{
				wchar_t buffer[4096] = {0};
				extendedFileInfoStructW info;

				info.filename = FileName;
				info.metadata = pdispparams->rgvarg[0].bstrVal;
				info.ret = buffer;
				info.retlen = sizeof(buffer)/sizeof(wchar_t);

				if (NULL != info.filename && 
					NULL != info.metadata)
				{
					if (0 == SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE))
						info.ret = NULL;

					JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(info.ret));
				}
				else
					JSAPI_EMPTY_RESULT(pvarResult);

				return S_OK;
			}
			break;

		case DISP_CURRENTSONG_GETFILENAME:
		{
			BSTR name = SysAllocString(FileName);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = name;
			return S_OK;
		}
		break;

		case DISP_CURRENTSONG_GETFILETITLE:
		{
			BSTR title = SysAllocString(FileTitle);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = title;
			return S_OK;
		}
		break;

		case DISP_CURRENTSONG_GETFILELENGTH:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_I4;
			V_I4(pvarResult) = in_getlength();
			return S_OK;

		case DISP_CURRENTSONG_GETPLAYPOSITION:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_I4;
			V_I4(pvarResult) = in_getouttime();
			return S_OK;

		case DISP_CURRENTSONG_ISPLAYING:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = (0 != playing) ? VARIANT_TRUE : VARIANT_FALSE;
			return S_OK;

		case DISP_CURRENTSONG_ISSTOPPED:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = (0 == playing) ? VARIANT_TRUE : VARIANT_FALSE;
			return S_OK;

		case DISP_CURRENTSONG_ISPAUSED:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = (0 != paused) ? VARIANT_TRUE : VARIANT_FALSE;
			return S_OK;
		case DISP_CURRENTSONG_PAUSE:
			PausePlaying();
			return S_OK;
		case DISP_CURRENTSONG_RESUME:
			UnPausePlaying();
			return S_OK;

	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP CurrentSongCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG CurrentSongCOM::AddRef(void)
{

	return 0;
}

ULONG CurrentSongCOM::Release(void)
{
	return 0;
}

static void TitleChanged_NotifyCb(IDispatch *dispatch, void *param)
{
	UNREFERENCED_PARAMETER(param);

	DISPPARAMS params;
	unsigned int ret;

	if (NULL == dispatch)
		return;

	params.cArgs = 0;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = 0;
	params.rgvarg = 0;
	
	if (!(config_no_visseh&8))
	{
		try
		{
			dispatch->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
		}
		catch (...)
		{}
	}
	else
		dispatch->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
}

void CurrentSongCOM::TitleChanged()
{
	titleChangeCallbacks.Notify(TitleChanged_NotifyCb, NULL, NULL);
}

static void MetadataChanged_NotifyCb(IDispatch *dispatch, void *param)
{
	VARIANT argument;
	DISPPARAMS params;

	if (NULL == dispatch)
		return;

	VariantInit(&argument);
	V_VT(&argument) = VT_BSTR;
	V_BSTR(&argument) = (BSTR)param;
	
	params.cArgs = 1;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;
	params.rgvarg = &argument;
	unsigned int ret;

	if (!(config_no_visseh&8))
	{
		try
		{
			dispatch->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
		}
		catch (...)
		{}
	}
	else
		dispatch->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);

}

static void MetadataChanged_FreeCb(void *param)
{
	BSTR bstr = (BSTR)param;
	SysFreeString(bstr);
}

void CurrentSongCOM::MetadataChanged(char *metadataString)
{
	AutoWide wideMetadata(metadataString);
	BSTR bstr = SysAllocString(wideMetadata);

	metadataCallbacks.Notify(MetadataChanged_NotifyCb, MetadataChanged_FreeCb, bstr);
}

