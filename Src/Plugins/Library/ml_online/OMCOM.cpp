#include "main.h"
#include "./omcom.h"
#include "./resource.h"
#include "./api__ml_online.h"

#include "JnetCOM.h"

#include "./navigation.h"
#include "./preferences.h"
#include "./serviceHelper.h"
#include "./serviceHost.h"
#include "./config.h"

#include "../nu/ConfigCOM.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoUrl.h"


#include <ifc_omservice.h>
#include <ifc_omserviceeditor.h>
#include <ifc_omfilestorage.h>


#include "../Winamp/JSAPI.h" // IDispatch helper macros

#include <shlwapi.h>
#include <strsafe.h>

extern MediaLibraryInterface mediaLibrary;
extern C_Config *g_config;

#define CONFIG_SERIALNUMBER		"serialNumber"
#define SERIALNUMBER_INVALID	((INT)-1)
#define SERIALNUMBER_DEFAULT	((INT)0)

enum
{
	DISPATCH_ISSUBSCRIBED = 12312,
	DISPATCH_ADDSUBSCRIBED,
	DISPATCH_CLEARSUBSCRIBED,
	DISPATCH_SERIALNUMBER,
	DISPATCH_JNETCREATE,
	DISPATCH_GETWID,
	DISPATCH_GETSID,
	DISPATCH_PLAY,
	DISPATCH_CONFIG,
	DISPATCH_ENQUEUE,
	DISPATCH_PREF,
	DISPATCH_SETSIZE,
	DISPATCH_GETX,
	DISPATCH_GETY,
	DISPATCH_NAVDISPLAY,
	DISPATCH_FOCUSURL,
	DISPATCH_SETCURRENTGUID,
	DISPATCH_ADDTITLEHOOK,
	DISPATCH_REMOVETITLEHOOK,
	DISPATCH_ADDMETADATAHOOK,
	DISPATCH_REMOVEMETADATAHOOK,
	DISPATCH_SUBSCRIBE,
};



OMCOM::OMCOM()
	: config(NULL), serialNumber(SERIALNUMBER_INVALID), publishCookie(0)
{
	
}

OMCOM::~OMCOM()
{
	if (NULL != config)
		config->Release();

	if (0 != publishCookie)
	{
		SENDWAIPC(Plugin_GetWinamp(), IPC_REMOVE_DISPATCH_OBJECT, publishCookie);
		publishCookie = NULL;
	}
}

HRESULT OMCOM::Publish()
{
	if (0 != publishCookie)
		return E_FAIL;

	DispatchInfo dispatchInfo;
	ZeroMemory(&dispatchInfo, sizeof(dispatchInfo));

	dispatchInfo.name = L"OnMedia";
	dispatchInfo.dispatch = this;

	if (0 != SENDWAIPC(Plugin_GetWinamp(), IPC_ADD_DISPATCH_OBJECT, (WPARAM)&dispatchInfo))
		return E_FAIL;
	
	publishCookie = dispatchInfo.id;
	return S_OK;
}

HRESULT OMCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		if (wcscmp(rgszNames[i], L"IsOmSubscribed") == 0)
			rgdispid[i] = DISPATCH_ISSUBSCRIBED;
		else if (wcscmp(rgszNames[i], L"AddOmSubscribed") == 0)
			rgdispid[i] = DISPATCH_ADDSUBSCRIBED;
		else if (wcscmp(rgszNames[i], L"ClearOmSubscribed") == 0)
			rgdispid[i] = DISPATCH_CLEARSUBSCRIBED;
		else if (wcscmp(rgszNames[i], L"OmSerialNumber") == 0)
			rgdispid[i] = DISPATCH_SERIALNUMBER;
		else if (wcscmp(rgszNames[i], L"JnetCreate") == 0)
			rgdispid[i] = DISPATCH_JNETCREATE;
		else if (wcscmp(rgszNames[i], L"GetUniqueID") == 0)
			rgdispid[i] = DISPATCH_GETWID;
		else if (wcscmp(rgszNames[i], L"GetSessionID") == 0)
			rgdispid[i] = DISPATCH_GETSID;
		else if (wcscmp(rgszNames[i], L"PlayUrl") == 0)
			rgdispid[i] = DISPATCH_PLAY;
		else if (wcscmp(rgszNames[i], L"Config") == 0)
			rgdispid[i] = DISPATCH_CONFIG;
		else if (wcscmp(rgszNames[i], L"EnqueueUrl") == 0)
			rgdispid[i] = DISPATCH_ENQUEUE;
		else if (wcscmp(rgszNames[i], L"ShowPreferences") == 0)
			rgdispid[i] = DISPATCH_PREF;
		else if (wcscmp(rgszNames[i], L"SetSize") == 0)
			rgdispid[i] = DISPATCH_SETSIZE;
		else if (wcscmp(rgszNames[i], L"GetX") == 0)
			rgdispid[i] = DISPATCH_GETX;
		else if (wcscmp(rgszNames[i], L"GetY") == 0)
			rgdispid[i] = DISPATCH_GETY;
		else if (wcscmp(rgszNames[i], L"DisplayNav") == 0)
			rgdispid[i] = DISPATCH_NAVDISPLAY;
		else if (wcscmp(rgszNames[i], L"FocusUrl") == 0)
			rgdispid[i] = DISPATCH_FOCUSURL;
		else if (wcscmp(rgszNames[i], L"SetCurrentGUID") == 0)
			rgdispid[i] = DISPATCH_SETCURRENTGUID;
		else if (wcscmp(rgszNames[i], L"AddTitleHook") == 0)
			rgdispid[i] = DISPATCH_ADDTITLEHOOK;
		else if (wcscmp(rgszNames[i], L"RemoveTitleHook") == 0)
			rgdispid[i] = DISPATCH_REMOVETITLEHOOK;
		else if (wcscmp(rgszNames[i], L"AddMetadataHook") == 0)
			rgdispid[i] = DISPATCH_ADDMETADATAHOOK;
		else if (wcscmp(rgszNames[i], L"RemoveMetadataHook") == 0)
			rgdispid[i] = DISPATCH_REMOVEMETADATAHOOK;
		else if (wcscmp(rgszNames[i], L"Subscribe") == 0)
			rgdispid[i] = DISPATCH_SUBSCRIBE;
		else
		{
			rgdispid[i] = DISPID_UNKNOWN;
			unknowns = true;
		}
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}
HRESULT OMCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT OMCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

#define VIDEO_GENFF_SIZEREQUEST (WM_USER+2048)
static void RemoveTitleHook(const wchar_t *url)
{
	Nullsoft::Utility::AutoLock lock(urlMapGuard);
DISPATCH_REMOVETITLEHOOK_again:
	URLMap::iterator itr;
	for (itr=urlMap.begin();itr!=urlMap.end();itr++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, url, -1, itr->url.c_str(), - 1))
		{
			urlMap.erase(itr);
			goto DISPATCH_REMOVETITLEHOOK_again;
		}
	}
}

static void RemoveMetadataHook(const wchar_t *url)
{
	Nullsoft::Utility::AutoLock lock(urlMapGuard);
DISPATCH_REMOVEMETADATAHOOK_again:
	MetadataMap::iterator itr;
	for (itr=metadataMap.begin();itr!=metadataMap.end();itr++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, url, -1, itr->url.c_str(), - 1))
		{
			metadataMap.erase(itr);
			goto DISPATCH_REMOVEMETADATAHOOK_again;
		}
	}
}

static void RemoveMetadataHook(const wchar_t *url, const wchar_t *tag)
{
	Nullsoft::Utility::AutoLock lock(urlMapGuard);
DISPATCH_REMOVEMETADATAHOOK_again2:
	MetadataMap::iterator itr;
	for (itr=metadataMap.begin();itr!=metadataMap.end();itr++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, url, -1, itr->url.c_str(), - 1) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, tag, -1, itr->tag.c_str(), - 1))
		{
			metadataMap.erase(itr);
			goto DISPATCH_REMOVEMETADATAHOOK_again2;
		}
	}
}

HRESULT OMCOM::FindService(VARIANTARG *pArg, ifc_omservice **service)
{
	if (NULL == service)
		return E_POINTER;
	
	*service = NULL;
	if (NULL == pArg) 
		return E_INVALIDARG;

	HRESULT hr = E_INVALIDARG;
	UINT serviceId;

	if (VT_BSTR == pArg->vt)
	{
		if (FALSE != StrToIntEx(pArg->bstrVal, STIF_SUPPORT_HEX, (INT*)&serviceId))
			hr = S_OK;
	}
	else if (VT_I4 == pArg->vt)
	{
		serviceId = pArg->lVal;
		hr = S_OK;
	}
	
	if (SUCCEEDED(hr))
		hr = ServiceHelper_Find(serviceId, service);
			
	return hr;
}
static HRESULT OmCom_AddServiceToNavigation(ifc_omservice *service)
{
	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
    if (SUCCEEDED(hr))
	{
		if (NULL != navigation->CreateItem(service))
		{
			hr = S_OK;
		}
		else
		{
			hr = E_FAIL;
		}
		navigation->Release();
	}
	return hr;
}

static HRESULT OmCom_SubscribeToService(UINT serviceId, LPCWSTR pszName, LPCWSTR pszUrl, INT iconId)
{
	ifc_omservice *service = NULL;
	HRESULT hr = ServiceHelper_Find(serviceId, &service);
	if (FAILED(hr)) return hr;
	
	if (S_FALSE == hr)
	{
		ServiceHost *serviceHost;
		if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
			serviceHost = NULL;

		if (NULL != OMSERVICEMNGR)
		{
            hr = OMSERVICEMNGR->CreateService(serviceId, serviceHost, &service);
		}
		else
			hr = E_FAIL;
	}
	else if (S_OK == ServiceHelper_IsSubscribed(service))
	{
		hr = S_FALSE;
	}

	if (SUCCEEDED(hr))
	{
		if (S_OK == hr)
		{
			ifc_omserviceeditor *editor;
			hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
			if (SUCCEEDED(hr))
			{
				if (NULL != pszName && L'\0' != *pszName)
					editor->SetName(pszName, FALSE);
			
				if (NULL != pszUrl && L'\0' != *pszUrl)
					editor->SetUrl(pszUrl, FALSE);
		
				WCHAR szIcon[256] = {0};
				if (SUCCEEDED(StringCchPrintf(szIcon, ARRAYSIZE(szIcon), L"%u", iconId)))
				{
					editor->SetIcon(szIcon, FALSE);
				}
				
				hr = editor->SetFlags(SVCF_SUBSCRIBED, SVCF_SUBSCRIBED);
				if (SUCCEEDED(hr))
					ServiceHelper_Save(service);
				editor->Release();
			}

			if (SUCCEEDED(hr))
				OmCom_AddServiceToNavigation(service);
		}
		service->Release();
	}

	return hr;
}

HRESULT OMCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	if (dispid == DISPATCH_ADDTITLEHOOK)
	{
		if (pdispparams->cArgs == 3)
		{
			Nullsoft::Utility::AutoLock lock(urlMapGuard);
			RemoveTitleHook(pdispparams->rgvarg[2].bstrVal); // ensure no duplicates
			url_info info;
			info.url = pdispparams->rgvarg[2].bstrVal;
			info.url_wcslen = wcslen(info.url.c_str());
			info.title=pdispparams->rgvarg[1].bstrVal;
			info.length= pdispparams->rgvarg[0].lVal;
			urlMap.push_back(info);
			return S_OK;
		}
	}
	if (dispid == DISPATCH_REMOVETITLEHOOK)
	{
		if (pdispparams->cArgs == 1)
		{
			RemoveTitleHook(pdispparams->rgvarg[0].bstrVal);
			return S_OK;
		}
		else
			return DISP_E_BADPARAMCOUNT;
	}

	if (dispid == DISPATCH_REMOVEMETADATAHOOK)
	{
		if (pdispparams->cArgs == 1)
		{
			RemoveMetadataHook(pdispparams->rgvarg[0].bstrVal);
			return S_OK;
		}
		else if (pdispparams->cArgs == 2)
		{
			RemoveMetadataHook(pdispparams->rgvarg[1].bstrVal, pdispparams->rgvarg[0].bstrVal);
			return S_OK;
		}
		else
			return DISP_E_BADPARAMCOUNT;
	}

	if (dispid == DISPATCH_ADDMETADATAHOOK)
	{
		if (pdispparams->cArgs == 3)
		{
			Nullsoft::Utility::AutoLock lock(urlMapGuard);
			RemoveMetadataHook(pdispparams->rgvarg[2].bstrVal, pdispparams->rgvarg[1].bstrVal); // ensure no duplicates
			metadata_info info;
			info.url = pdispparams->rgvarg[2].bstrVal;
			info.tag = pdispparams->rgvarg[1].bstrVal;
			info.metadata= pdispparams->rgvarg[0].bstrVal;
			metadataMap.push_back(info);
			return S_OK;
		}
	}

	if (dispid == DISPATCH_SERIALNUMBER)
	{		
		int serial = GetSerialNumber(FALSE);
		if (pdispparams->cArgs == 1)
		{
			SetSerialNumber(pdispparams->rgvarg[0].lVal);
			serial = GetSerialNumber(FALSE);
		}

		JSAPI_INIT_RESULT(pvarResult, VT_I4);
		JSAPI_SET_RESULT(pvarResult, lVal, serial);
		return S_OK;
	}

	if (dispid == DISPATCH_ADDSUBSCRIBED)
	{				
		return AddOmSubscribed(wFlags, pdispparams, pvarResult, puArgErr);
	}

	if (dispid == DISPATCH_SUBSCRIBE)
	{
		return Subscribe(wFlags, pdispparams, pvarResult, puArgErr);
	}

	if (dispid == DISPATCH_CLEARSUBSCRIBED && pdispparams->cArgs == 1)
	{
		JSAPI_VERIFY_METHOD(wFlags);
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
		JSAPI_INIT_RESULT(pvarResult, VT_I4);

		BOOL result = FALSE;
		ifc_omservice *service;
		if (S_OK == FindService(&JSAPI_PARAM(pdispparams, 1), &service))
		{
			if (SUCCEEDED(ServiceHelper_Subscribe(service, FALSE, SHF_SAVE /* | SHF_NOTIFY*/)))
				result = TRUE;
			
			service->Release();
		}
		
		JSAPI_SET_RESULT(pvarResult, lVal, result);
		return S_OK;
	}

	if (dispid == DISPATCH_ISSUBSCRIBED)
	{
		return IsOmSubscribed(wFlags, pdispparams, pvarResult, puArgErr);
	}

	if (dispid == DISPATCH_JNETCREATE)
	{
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_DISPATCH;
		V_DISPATCH(pvarResult) = new JnetCOM();
		return S_OK;
	}

	if (dispid == DISPATCH_GETWID)
	{
		WCHAR szBuffer[512] = {0};
		if (NULL == OMBROWSERMNGR || 
			FAILED(OMBROWSERMNGR->GetClientId(szBuffer, ARRAYSIZE(szBuffer))))
		{
			szBuffer[0] = L'\0';
		}
				
		BSTR tag = SysAllocString(szBuffer);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = tag;

		return S_OK;
	}

	if (dispid == DISPATCH_GETSID)
	{
		WCHAR szBuffer[512] = {0};
		if (NULL == OMBROWSERMNGR || 
			FAILED(OMBROWSERMNGR->GetSessionId(szBuffer, ARRAYSIZE(szBuffer))))
		{
			szBuffer[0] = L'\0';
		}

		BSTR tag = SysAllocString(szBuffer);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = tag;

		return S_OK;
	}

	if (dispid == DISPATCH_PLAY && pdispparams->cArgs == 1)
	{
		if (pdispparams->rgvarg[0].bstrVal)
			mediaLibrary.PlayStream(pdispparams->rgvarg[0].bstrVal);
		return S_OK;
	}

	if (dispid == DISPATCH_PREF)
	{
		Preferences_Show();
		return S_OK;
	}

	if (dispid == DISPATCH_CONFIG)
	{
		VariantInit(pvarResult);
		if(NULL == config && FAILED(ConfigCOM::CreateInstanceA("ml_online_config", g_config->GetPath(), &config)))
		{
			V_VT(pvarResult) = VT_NULL;
		}
		else
		{		
			V_VT(pvarResult) = VT_DISPATCH;
			V_DISPATCH(pvarResult) = config;
			config->AddRef();
		}
		return S_OK;
	}

	if (dispid == DISPATCH_ENQUEUE && pdispparams->cArgs == 1)
	{
		if (pdispparams->rgvarg[0].bstrVal)
			mediaLibrary.EnqueueStream(pdispparams->rgvarg[0].bstrVal);
		return S_OK;
	}

	if (dispid == DISPATCH_SETSIZE && pdispparams->cArgs == 2)
	{
		HWND hView = NULL;
		Navigation *navigation;
		if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
		{
			hView = navigation->GetActiveView(NULL);
			navigation->Release();
		}

		if (NULL != hView && mediaLibrary.library && 
			GetParent(mediaLibrary.library) &&
			g_config->ReadInt("AutoSize",1))
		{
			HWND hWnd;
			bool GenFF = false;
			if (GetParent(GetParent(mediaLibrary.library)))
			{
				hWnd = GetParent(GetParent(mediaLibrary.library));
				GenFF = true;
			}
			else
				hWnd = GetParent(mediaLibrary.library);

			int width   = pdispparams->rgvarg[1].lVal;
			int height  = pdispparams->rgvarg[0].lVal;

			RECT rc;
			GetWindowRect(hView, &rc);   // Our Html page
			int WWidth = rc.right - rc.left;
			int WHeight = rc.bottom - rc.top;

			GetWindowRect(hWnd, &rc);   // Gen ML Size
			int PWidth = rc.right - rc.left;
			int PHeight = rc.bottom - rc.top;

			// Subtract the original window size from the parent(base) size
			PWidth -= WWidth;
			PHeight -= WHeight;

			// Add the target size to the parent(base) size
			PWidth += width;
			PHeight += height;

			if (GenFF)
			{
				SendMessage(hWnd, VIDEO_GENFF_SIZEREQUEST, PWidth, PHeight);
			}
			else
			{
				SetWindowPos(hWnd, 0, 0, 0, PWidth, PHeight, SWP_NOMOVE|SWP_ASYNCWINDOWPOS);
				// weird? sometimes height isnt set if called once...
				SetWindowPos(hWnd, 0, 0, 0, PWidth, PHeight, SWP_NOMOVE|SWP_ASYNCWINDOWPOS);
			}
		}
		return S_OK;
	}

	if (dispid == DISPATCH_GETX)
	{
		RECT rc;
		HWND hView = NULL;
		Navigation *navigation;
		if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
		{
			hView = navigation->GetActiveView(NULL);
			navigation->Release();
		}

		if (NULL != hView)
		{
			GetWindowRect(hView, &rc);   // Our Html page
			int WWidth = rc.right - rc.left;

			if (pvarResult)
			{
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_I4;
				V_I4(pvarResult) = WWidth;
			}
		}
		return S_OK;
	}

	if (dispid == DISPATCH_GETY)
	{
		RECT rc;
		HWND hView = NULL;
		Navigation *navigation;
		if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
		{
			hView = navigation->GetActiveView(NULL);
			navigation->Release();
		}

		if (NULL != hView)
		{
			GetWindowRect(hView, &rc);   // Our Html page
			int WHeight = rc.bottom - rc.top;
			if (pvarResult)
			{
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_I4;
				V_I4(pvarResult) = WHeight;
			}
		}
		return S_OK;
	}

	if (dispid == DISPATCH_NAVDISPLAY && pdispparams->cArgs == 1)
	{
		//int visible = pdispparams->rgvarg[0].lVal;
		return E_NOTIMPL;
	}

	if (dispid == DISPATCH_FOCUSURL && pdispparams->cArgs == 2)
	{
		ifc_omservice *service;
		if (S_OK == FindService(&pdispparams->rgvarg[1], &service))
		{
			Navigation *navigation;
			if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
			{
				navigation->ShowService(service->GetId(), pdispparams->rgvarg[0].bstrVal);
				navigation->Release();
			}
			
			service->Release();
		}
		return S_OK;
	}

	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP OMCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG OMCOM::AddRef(void)
{
	return 0;
}

ULONG OMCOM::Release(void)
{
	return 0;
}

HRESULT OMCOM::IsOmSubscribed(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_INIT_RESULT(pvarResult, VT_I4);
	// 0 No knowledge , 1 Knowledge and Disabled, 2 Knowledge and Enabled

	ifc_omservice *service;
	if (S_OK == FindService(&JSAPI_PARAM(pdispparams, 1), &service))
	{
		JSAPI_SET_RESULT(pvarResult, lVal, (S_OK == ServiceHelper_IsSubscribed(service)) ? 2 : 1);
		service->Release();
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, lVal, 0);
	}

	return S_OK;
}

HRESULT OMCOM::AddOmSubscribed(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 3, 4);

	JSAPI_INIT_RESULT(pvarResult, VT_I4);

	LPCWSTR pszUrl, pszName;
	UINT baseArg, serviceId, iconId;

	baseArg = 0;
	
	if (pdispparams->cArgs >= 4)
	{
		JSAPI_GETUNSIGNED_AS_NUMBER(iconId, pdispparams, (baseArg + 1), puArgErr);
		baseArg++;
	}
	else
		iconId = 0;
	
	JSAPI_GETUNSIGNED_AS_NUMBER(serviceId, pdispparams, (baseArg + 1), puArgErr);
	JSAPI_GETSTRING(pszName, pdispparams, (baseArg + 2), puArgErr);
	JSAPI_GETSTRING(pszUrl, pdispparams, (baseArg + 3), puArgErr);
	

	
	
	INT result = OmCom_SubscribeToService(serviceId, pszName, pszUrl, iconId);
	JSAPI_SET_RESULT(pvarResult, lVal, result);
	
	return S_OK;
}

typedef int (*HTTPRETRIEVEFILEW)(HWND hwnd, char *url, wchar_t *file, wchar_t *dlgtitle);
HRESULT OMCOM::Subscribe(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	// window.external.OnMedia.Subscribe(String name, String url, String id, String icon, String version);
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 5);
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	WCHAR szBuffer[4096] = {0};
	LPCWSTR pszName, pszUrl, pszIcon;
	UINT serviceId, version;

	JSAPI_GETSTRING(pszName, pdispparams, 1, puArgErr);
	JSAPI_GETSTRING(pszUrl, pdispparams, 2, puArgErr);	
	JSAPI_GETUNSIGNED_AS_NUMBER(serviceId, pdispparams, 3, puArgErr);
	JSAPI_GETNUMBER_AS_STRING(pszIcon, szBuffer, pdispparams, 4, puArgErr);
	JSAPI_GETUNSIGNED_AS_NUMBER(version, pdispparams, 5, puArgErr);
	
	HRESULT hr;
	ifc_omservice *service;
	hr = ServiceHelper_Find(serviceId, &service);
	if (S_OK != hr)
	{
		hr = ServiceHelper_Create(serviceId, pszName, pszIcon, pszUrl, SVCF_SUBSCRIBED | SVCF_PREAUTHORIZED, 2, TRUE, &service);
		if (SUCCEEDED(hr))
		{
			OmCom_AddServiceToNavigation(service);
			service->Release();
		}
	}
	else
	{
		hr = ServiceHelper_Subscribe(service, TRUE, SHF_SAVE /*| SHF_NOTIFY*/); // do not call SHF_NOTIFY - or it will adjust stats
		if (S_OK == hr) 
			OmCom_AddServiceToNavigation(service);

		service->Release();
	}
	
	JSAPI_SET_RESULT(pvarResult, boolVal, (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE));
	return S_OK;
}

HRESULT OMCOM::SetSerialNumber(INT sn)
{
	if (SERIALNUMBER_INVALID == sn)
		return E_INVALIDARG;

	if (serialNumber == sn)
		return S_FALSE;	

	serialNumber = sn;

	CHAR szBuffer[64] = {0};
	HRESULT hr = StringCchPrintfA(szBuffer, ARRAYSIZE(szBuffer), "%d", serialNumber);
	if (SUCCEEDED(hr))
	{
		hr = Config_WriteStr(NULL, CONFIG_SERIALNUMBER, szBuffer);
	}

	return hr;
}

INT OMCOM::GetSerialNumber(BOOL fForceRead)
{
	if (FALSE != fForceRead || SERIALNUMBER_INVALID == serialNumber)
	{
		serialNumber = Config_ReadInt(NULL, CONFIG_SERIALNUMBER, SERIALNUMBER_DEFAULT);
	}
	return serialNumber; 
}

/*
HRESULT OMCOM::Login(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	// window.external.OnMedia.Login(String url);
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	const wchar_t *url = JSAPI_PARAM(pdispparams, 1).bstrVal;
	HWND active_view = OmView_GetActive();
	if (active_view)
	{
		OmService *service = OmView_GetService(active_view);
		if (service)
		{
			OMNAVIGATION->SelectService(service, url);
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		}
	}
	return S_OK;
}
*/