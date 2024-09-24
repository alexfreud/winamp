/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "Main.h"
#include "ExternalCOM.h"
#include "BrowserCOM.h"
#include "CurrentSongCOM.h"
#include "SkinCOM.h"
#include "ApplicationCOM.h"
#include "BookmarksCOM.h"
#include "MediaCoreCOM.h"
#include "DataStoreCOM.h"
#include "JNetCOM.h"
#include "SecurityCOM.h"
#include "../nu/ConfigCOM.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#include "./Winamp/JSAPI.h"
#include "JSAPI2_ExternalObject.h"
#include "JSAPI2_Security.h"

static volatile LONG unique_dispid;

enum
{
	DISPID_CONFIG = 0,
	DISPID_JNET = 1,
	INITIAL_DISPID = 776,
};

static ExternalCOM *externalCOM = NULL;

HRESULT __cdecl JSAPI1_Initialize()
{
	if (NULL != externalCOM) 
		return S_FALSE;
	
	ExternalCOM *temp = new ExternalCOM();
	if (NULL == temp) 
		return E_OUTOFMEMORY;
	
	externalCOM  = temp;
	return S_OK;
}

HRESULT __cdecl JSAPI1_Uninitialize()
{
	if (NULL == externalCOM)
		return S_FALSE;

	ExternalCOM *temp = externalCOM;
	externalCOM = NULL;
	temp->Release();

	return S_OK;
}

HRESULT __cdecl JSAPI1_GetExternal(ExternalCOM **instance)
{		
	if (NULL == instance) 
		return E_POINTER;
	
	if (NULL == externalCOM) 
	{
		*instance = NULL;
		return E_UNEXPECTED;
	}
	
	externalCOM->AddRef();
	*instance = externalCOM;
		
	return S_OK;
}

HRESULT __cdecl JSAPI1_GetSkinCOM(SkinCOM **instance)
{	
	ExternalCOM *external = 0;
	HRESULT hr = JSAPI1_GetExternal(&external);
	if (SUCCEEDED(hr) && external)
	{
		hr = external->GetSkinCOM(instance);
		external->Release();
	}
	else
	{
		if (NULL == instance) hr = E_POINTER;
		else *instance = NULL;
	}
	return hr;
}

HRESULT __cdecl JSAPI1_GetMediaCoreCOM(MediaCoreCOM **instance)
{	
	ExternalCOM *external = 0;
	HRESULT hr = JSAPI1_GetExternal(&external);
	if (SUCCEEDED(hr) && external)
	{
		hr = external->GetMediaCoreCOM(instance);
		external->Release();
	}
	else
	{
		if (NULL == instance) hr = E_POINTER;
		else *instance = NULL;
	}
	return hr;
}

HRESULT __cdecl JSAPI1_GetCurrentSongCOM(CurrentSongCOM **instance)
{	
	ExternalCOM *external = 0;
	HRESULT hr = JSAPI1_GetExternal(&external);
	if (SUCCEEDED(hr) && external)
	{
		hr = external->GetCurrentSongCOM(instance);
		external->Release();
	}
	else
	{
		if (NULL == instance) hr = E_POINTER;
		else *instance = NULL;
	}
	return hr;
}

HRESULT __cdecl JSAPI1_SkinChanged()
{
	SkinCOM *skinCOM = 0;
	HRESULT hr = JSAPI1_GetSkinCOM(&skinCOM);
	if (SUCCEEDED(hr) && skinCOM)
	{
		skinCOM->SkinChanged();
		skinCOM->Release();
	}
	return hr;
}

HRESULT __cdecl JSAPI1_CurrentTitleChanged()
{
	CurrentSongCOM *songCOM = 0;
	HRESULT hr = JSAPI1_GetCurrentSongCOM(&songCOM);
	if (SUCCEEDED(hr) && songCOM)
	{
		songCOM->TitleChanged();
		songCOM->Release();
	}
	return hr;
}

DISPID __cdecl JSAPI1_GenerateUniqueDispatchId()
{
	return (DISPID)InterlockedIncrement(&unique_dispid);
}

#define REGISTER_DISPATCH_EX(__name, __creator, __dispId, __pDisp)\
	{	__pDisp = new __creator;\
		if (NULL != __pDisp) {\
			__dispId = AddDispatch(__name, __pDisp);\
			if (0 == __dispId) 	{ (__pDisp)->Release(); (__pDisp) = NULL;}\
		}\
	}

#define REGISTER_DISPATCH(__name, __creator)\
	{ DISPID __dispId; IDispatch *pDisp; \
		REGISTER_DISPATCH_EX(__name, __creator, __dispId, pDisp);\
		if (NULL != pDisp) pDisp->Release(); }

static const wchar_t *api1_api2_key = L"1";

ExternalCOM::ExternalCOM() : ref(1), mediaCoreCOM(NULL), skinCOM(NULL), songCOM(NULL), api2(0)
{
	InitializeCriticalSection(&tableLock);
	unique_dispid = INITIAL_DISPID; // we can't count on the CRT to have initialized this yet.
	configFilename[0]=0;

	DISPID dispId = 0;
	REGISTER_DISPATCH(L"Browser", BrowserCOM());
	REGISTER_DISPATCH_EX(L"CurrentSong", CurrentSongCOM(), dispId, songCOM);
	REGISTER_DISPATCH_EX(L"CurrentSkin", SkinCOM(), dispId, skinCOM);
	REGISTER_DISPATCH(L"Application", ApplicationCOM());
	REGISTER_DISPATCH(L"Bookmarks", BookmarksCOM());
	REGISTER_DISPATCH_EX(L"MediaCore", MediaCoreCOM(), dispId, mediaCoreCOM);
	REGISTER_DISPATCH(L"DataStore", DataStoreCOM());
	REGISTER_DISPATCH(L"Security", SecurityCOM());
	JSAPI2::security.SetBypass(api1_api2_key, true);
	REGISTER_DISPATCH_EX(L"API2", JSAPI2::ExternalObject(api1_api2_key), dispId, api2);
}

ExternalCOM::~ExternalCOM()
{
	EnterCriticalSection(&tableLock);

	for ( JSAPI::Dispatcher *l_dispatch_table : dispatchTable )
		delete l_dispatch_table;

	for ( ConfigCOM *l_config : configs )
		l_config->Release();

	LeaveCriticalSection(&tableLock);

	DeleteCriticalSection(&tableLock);

	if (NULL != mediaCoreCOM) mediaCoreCOM->Release();
	if (NULL != songCOM) songCOM->Release();
	if (NULL != skinCOM) skinCOM->Release();
	if (NULL != api2) api2->Release();
}

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

STDMETHODIMP ExternalCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	UINT unknowns = 0;

	EnterCriticalSection(&tableLock);
	size_t tableCount = dispatchTable.size();

	for (unsigned int i = 0;i != cNames; i++)
	{
		rgdispid[i]=DISPID_UNKNOWN;

		for (size_t entry = 0; entry < tableCount; entry++)
		{
			if (!wcscmp(rgszNames[i], dispatchTable[entry]->name))
			{
				rgdispid[i] = dispatchTable[entry]->id;
				break;
			}
		}

		if (rgdispid[i] == DISPID_UNKNOWN && !wcscmp(rgszNames[i], L"Config"))
			rgdispid[i] = DISPID_CONFIG;
		else if (rgdispid[i] == DISPID_UNKNOWN && !wcscmp(rgszNames[i], L"JNetLib"))
			rgdispid[i] = DISPID_JNET;
		else if (rgdispid[i] == DISPID_UNKNOWN)
			unknowns++;
	}

	LeaveCriticalSection(&tableLock);

	if (0 != unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

STDMETHODIMP ExternalCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP ExternalCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP ExternalCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch(dispid)
	{
		case DISPID_CONFIG:
			{
				JSAPI_VERIFY_METHOD(wFlags);
				JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

				LPCWSTR configName;
				JSAPI_GETSTRING(configName, pdispparams, 1, puArgErr); 

				if (NULL != pvarResult)
				{
					VariantInit(pvarResult);

					ConfigCOM *config;
					if (SUCCEEDED(GetConfig(configName, &config))) 
					{
						V_VT(pvarResult) = VT_DISPATCH;
						V_DISPATCH(pvarResult) = config;
					}
					else
					{
						V_VT(pvarResult) = VT_NULL;
					}
				}
			}
			return S_OK;

		case DISPID_JNET:
			{
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_DISPATCH;
				V_DISPATCH(pvarResult) = new JNetCOM(pdispparams->rgvarg[0].pdispVal);
				return S_OK;
			}
			break;
	}

	EnterCriticalSection(&tableLock);
	size_t index = dispatchTable.size();
	while(index--)
	{
		if (dispatchTable[index]->id == dispid)
		{
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_DISPATCH;
			V_DISPATCH(pvarResult) = dispatchTable[index]->object;
			dispatchTable[index]->object->AddRef();
			break;
		}
	}
	LeaveCriticalSection(&tableLock);
	if (((size_t)-1) != index) return S_OK;

	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP ExternalCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

STDMETHODIMP_(ULONG) ExternalCOM::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) ExternalCOM::Release(void)
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

STDMETHODIMP ExternalCOM::QueryDispatchable(REFIID riid, Dispatchable **ppDispatchable)
{
	if (IsEqualIID(riid, JSAPI::IID_JSAPI_ifc_info))
	{
		*ppDispatchable = (JSAPI::ifc_info *)this;
	}
	else
	{
		*ppDispatchable = NULL;
		return E_NOINTERFACE;
	}
	(*ppDispatchable)->AddRef();
	return S_OK;
}

const wchar_t *ExternalCOM::GetUserAgent()
{
	return L"JSAPI1";
}

HRESULT ExternalCOM::GetConfig(LPCWSTR configName, ConfigCOM **config)
{
	if (NULL == config) return E_POINTER;

	AutoChar nameAnsi(configName);
	if (NULL == (const char*)nameAnsi)
	{
		*config = NULL;
		return E_INVALIDARG;
	}

	EnterCriticalSection(&tableLock);

	// check if there's already an open config object
	size_t index = configs.size();
	while (index-- && FALSE != configs[index]->IsEqual(nameAnsi));

	if ((size_t)-1 == index)
	{
		if (L'\0' != configFilename[0] || 
			NULL != PathCombineW(configFilename, CONFIGDIR, L"jscfg.ini"))
		{
			if (SUCCEEDED(ConfigCOM::CreateInstanceA(nameAnsi, AutoCharFn(configFilename), config)))
				configs.push_back(*config);
		}
	}
	else
	{
		*config = configs[index];
	}

	HRESULT hr = S_OK;
	if (NULL != *config) 
	{
		(*config)->AddRef();
	}
	else
		hr = E_FAIL;

	LeaveCriticalSection(&tableLock);

	return hr;
}

HRESULT ExternalCOM::FindDispatch(DISPID dispId, IDispatch **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	EnterCriticalSection(&tableLock);
	size_t index = dispatchTable.size();
	while(index--)
	{
		if (dispatchTable[index]->id == dispId)
		{
			*instance = dispatchTable[index]->object;
			break;
		}
	}
	LeaveCriticalSection(&tableLock);

	if (((size_t)-1) != index)
		return S_OK;
	
	*instance = NULL;
	return S_FALSE;
}

DISPID ExternalCOM::AddDispatch(const wchar_t *name, IDispatch *object)
{
	if (NULL == object) 
		return 0;

	DISPID id = JSAPI1_GenerateUniqueDispatchId();

	JSAPI::Dispatcher *dispatcher = new JSAPI::Dispatcher(name, id, object);
	if (NULL == dispatcher) return 0;

	EnterCriticalSection(&tableLock);
	dispatchTable.push_back(dispatcher);
	LeaveCriticalSection(&tableLock);

	return id;
}

BOOL ExternalCOM::RemoveDispatch(DISPID dispatchId)
{
	EnterCriticalSection(&tableLock);
	size_t index = dispatchTable.size();
	while(index--)
	{
		if (dispatchTable[index]->id == dispatchId)
		{
			JSAPI::Dispatcher *dispatcher = dispatchTable[index];
			dispatchTable.erase(dispatchTable.begin() + index);
			delete dispatcher;
			break;
		}
	}
	LeaveCriticalSection(&tableLock);
	return (((size_t)-1) != index);
}

HRESULT ExternalCOM::GetSkinCOM(SkinCOM **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = skinCOM;
	if (NULL != *instance) 
		(*instance)->AddRef();

	return S_OK;
}

HRESULT ExternalCOM::GetMediaCoreCOM(MediaCoreCOM **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = mediaCoreCOM;
	if (NULL != *instance) 
		(*instance)->AddRef();

	return S_OK;
}

HRESULT ExternalCOM::GetCurrentSongCOM(CurrentSongCOM **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = songCOM;
	if (NULL != *instance) 
		(*instance)->AddRef();

	return S_OK;
}

#define CBCLASS ExternalCOM
START_DISPATCH;
CB(JSAPI_IFC_INFO_GETUSERAGENT, GetUserAgent)
END_DISPATCH;
#undef CBCLASS