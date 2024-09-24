#include "JSAPI2_ExternalObject.h"
#include "JSAPI2_svc_apicreator.h"
#include "JSAPI2_TransportAPI.h"
#include "JSAPI2_PlayerAPI.h"
#include "JSAPI2_Downloader.h"
#include "JSAPI2_SecurityAPI.h"
#include "JSAPI2_Security.h"
#include "JSAPI2_Bookmarks.h"
#include "JSAPI2_Application.h"
#include "JSAPI2_SkinAPI.h"
#include "JSAPI2_MediaCore.h"
#include "JSAPI2_AsyncDownloader.h"
#include "JSAPI.h"
#include "api.h"

#include "main.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#include <api/service/waservicefactory.h>
#include <shlwapi.h>
/* benski: basic thoughts

ExternalObject will be created "on demand" when ml_online loads a page
so ExternalObjects will have a lifetime (unlike ExternalCOM which is a singleton)

create wasabi services for creating window.external.* objects
when a window.external.* function is requested, services are enumerated
the key is passed to the object creator.  The returned object will be treated
as a singleton from ExternalObject's perspective.  It will be Release()'d when
the ExternalObject is destroyed.

the basic layout will be
window.external.API.method

e.g.
window.external.Transport.Play();

any time that non-singleton objects need to be created (Config object, 
playlist object, etc) they will be created from a method within the API object
e.g.
var new_playlist = window.external.Playlist.Create();
*/

JSAPI2::ExternalObject::ExternalObject(const wchar_t *_key)
{
	hwnd=0;

	if (_key)
		key = _wcsdup(_key);
	else
		key = 0;

	refCount = 1;
	// create Config API now.
	
	ConfigCOM *configCOM;
	wchar_t szPath[MAX_PATH] = {0};
	PathCombineW(szPath, CONFIGDIR, L"jscfg.ini");
	if (SUCCEEDED(ConfigCOM::CreateInstanceW(key, AutoCharFn(szPath), &configCOM)))
	{
		AddDispatch(L"Config", configCOM);
		configCOM->Release();
	}
}

JSAPI2::ExternalObject::~ExternalObject()
{
	for (JSAPI::DispatchTable::iterator itr = dispatchTable.begin(); itr != dispatchTable.end(); itr++)
	{
		//JSAPI::Dispatcher *entry = *itr;
		delete (*itr);
	}
	dispatchTable.clear();
	if (key) free(key);
}

DWORD JSAPI2::ExternalObject::AddDispatch(const wchar_t *name, IDispatch *object)
{
	int id = (int) dispatchTable.size();
	dispatchTable.push_back(new JSAPI::Dispatcher(name, id, object));

	return id;
}

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT JSAPI2::ExternalObject::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		if (GetDispID(rgszNames[i], fdexNameCaseSensitive, &rgdispid[i]) == DISPID_UNKNOWN)
			unknowns=true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::ExternalObject::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	return InvokeEx(dispid, lcid, wFlags, pdispparams, pvarResult, pexecinfo, 0);
}

STDMETHODIMP JSAPI2::ExternalObject::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_IWasabiDispatchable))
		*ppvObject = (IWasabiDispatchable *)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG JSAPI2::ExternalObject::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}

ULONG JSAPI2::ExternalObject::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}

HRESULT JSAPI2::ExternalObject::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
	*pid = DISP_E_UNKNOWNNAME;

	for (size_t entry=0;entry!=dispatchTable.size();entry++)
	{
		if (!wcscmp(bstrName, dispatchTable[entry]->name))
		{
			if (dispatchTable[entry]->object)
			{
				*pid = (DISPID)entry;
				return S_OK;
			}
			else
			{
				return DISPID_UNKNOWN;
			}
		}
	}
	// look it up in wasabi
	IDispatch *disp = 0;
	waServiceFactory *sf = 0;
	int n = 0;
	do
	{
		sf = WASABI_API_SVC->service_enumService(JSAPI2::svc_apicreator::getServiceType(), n++);
		if (!sf)
			break;

		if (sf)
		{
			JSAPI2::svc_apicreator *creator = (JSAPI2::svc_apicreator *)sf->getInterface();
			if (creator)
			{
				disp = creator->CreateAPI(bstrName, key, static_cast<JSAPI::ifc_info *>(this));
			}
			sf->releaseInterface(creator);
		}
	} while (sf && !disp);
	*pid = AddDispatch(bstrName, disp);
	if (!disp)
	{
		*pid = DISP_E_UNKNOWNNAME;
		return DISPID_UNKNOWN;
	}
	else
		return S_OK;
}

HRESULT JSAPI2::ExternalObject::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
{
	if (id >= 0 && (size_t)id < dispatchTable.size())
	{
		IDispatch *disp = dispatchTable[id]->object;
		if (disp)
			disp->AddRef(); // I assume we're supposed to do this, but I'm not 100% sure
		JSAPI_INIT_RESULT(pvarRes, VT_DISPATCH);
		JSAPI_SET_RESULT(pvarRes, pdispVal, disp);
		return S_OK;
	}

	return DISP_E_MEMBERNOTFOUND;

}

HRESULT JSAPI2::ExternalObject::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::DeleteMemberByDispID(DISPID id)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::GetMemberName(DISPID id, BSTR *pbstrName)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::GetNameSpaceParent(IUnknown **ppunk)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ExternalObject::QueryDispatchable(REFIID riid, Dispatchable **ppDispatchable)
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

const wchar_t *JSAPI2::ExternalObject::GetUserAgent()
{
	return L"JSAPI2";
}

void JSAPI2::ExternalObject::SetHWND(HWND hwnd)
{
	this->hwnd = hwnd;
}

HWND JSAPI2::ExternalObject::GetHWND()
{
	return hwnd;
}

void JSAPI2::ExternalObject::SetName(const wchar_t *name)
{
	JSAPI2::security.AssociateName(key, name);
}

const wchar_t *JSAPI2::ExternalObject::GetName()
{
	return JSAPI2::security.GetAssociatedName(key);
}

int JSAPI2::ExternalObject::AddAPI(const wchar_t *name, IDispatch *dispatch)
{
	if (dispatch)
	{
		dispatch->AddRef();
		AddDispatch(name, dispatch);
		return 0;
	}
	return 1;
}


#define CBCLASS JSAPI2::ExternalObject
START_DISPATCH;
CB(JSAPI_IFC_INFO_GETUSERAGENT, GetUserAgent)
VCB(JSAPI_IFC_INFO_SETHWND, SetHWND)
CB(JSAPI_IFC_INFO_GETHWND, GetHWND)
VCB(JSAPI_IFC_INFO_SETNAME, SetName)
CB(JSAPI_IFC_INFO_GETNAME, GetName)
CB(JSAPI_IFC_INFO_ADDAPI, AddAPI)
END_DISPATCH;
#undef CBCLASS
