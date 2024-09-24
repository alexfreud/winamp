#include "./cddbevnt.h"
#include "cddbinterface.h"
#include <strsafe.h>
#include <ocidl.h>

#include "cddbcontrolwinamp.tlh"

CDBBEventManager::CDBBEventManager(void) : ref(1), cookie(0), user(0),
											fnCmdCompleted(NULL), fnCmdProgress(NULL),
											fnLogMessage(NULL), fnServerMessage(NULL)
{
}

CDBBEventManager::~CDBBEventManager(void)
{
}

HRESULT CDBBEventManager::Advise(IUnknown *pCDDBCtrl)
{
	HRESULT hr;
	IConnectionPoint *pcp;
	IConnectionPointContainer *pcpc;
	
	if (cookie) return E_FAIL; 
    if (!pCDDBCtrl) return E_INVALIDARG;

	hr = pCDDBCtrl->QueryInterface(IID_IConnectionPointContainer, (PVOID*)&pcpc);
	if (SUCCEEDED (hr))
	{
		hr = pcpc->FindConnectionPoint(DIID_DCDDBEvents, &pcp);
		if (SUCCEEDED(hr))
		{
			hr = pcp->Advise(static_cast<IDispatch*>(this), &cookie);
			if (FAILED(hr)) cookie = 0;
			pcp->Release();
		}
		pcpc->Release();
	}
	return hr;
}

HRESULT CDBBEventManager::Unadvise(IUnknown *pCDDBCtrl)
{
	HRESULT hr;
	IConnectionPoint *pcp = nullptr;
	IConnectionPointContainer *pcpc = nullptr;
	
	if (!cookie) return S_OK;
	if (!pCDDBCtrl) return E_INVALIDARG;

	hr = pCDDBCtrl->QueryInterface(IID_IConnectionPointContainer, (PVOID*)&pcpc);
	if (SUCCEEDED (hr))
	{
		hr = pcpc->FindConnectionPoint(DIID_DCDDBEvents, &pcp);
		if (SUCCEEDED(hr))
		{
			hr = pcp->Unadvise(cookie);
			pcp->Release();
		}
		pcpc->Release();
	}
	return hr;
}

HRESULT CDBBEventManager::RegisterCallback(UINT nType, void *fnCallback)
{
	switch(nType)
	{
		case CDDB_CB_CMDCOMPLETED:	fnCmdCompleted = (CDDB_CMDCOMPLETED)fnCallback; break;
		case CDDB_CB_CMDPROGRESS:	fnCmdProgress = (CDDB_CMDPROGRESS)fnCallback; break;
		case CDDB_CB_LOGMSG:			fnLogMessage = (CDDB_LOGMSG)fnCallback; break;
		case CDDB_CB_SRVMSG:			fnServerMessage = (CDDB_SRVMSG)fnCallback; break;
		default:					return E_INVALIDARG;
	}
	return S_OK;
}

ULONG_PTR CDBBEventManager::SetUserParam(ULONG_PTR userParam)
{
	ULONG_PTR tmp = user;
	user = userParam;
	return tmp;
}

STDMETHODIMP CDBBEventManager::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject) return E_POINTER;

	if (IsEqualIID(riid, DIID_DCDDBEvents))
		*ppvObject = dynamic_cast<CDBBEventManager*>(this);
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = dynamic_cast<IDispatch*>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = dynamic_cast<IUnknown*>(this);
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG CDBBEventManager::AddRef(void)
{
	return InterlockedIncrement(&ref);
}

ULONG CDBBEventManager::Release(void)
{
	if (ref && 0 == InterlockedDecrement(&ref))
	{
		delete this;
		return 0;
	}
	return ref;
}

STDMETHODIMP CDBBEventManager::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	return  DISP_E_UNKNOWNNAME;
}

STDMETHODIMP CDBBEventManager::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDBBEventManager::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDBBEventManager::Invoke(DISPID dispId, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch(dispId)
	{
		case EVENT_COMMAND_COMPLETED: 
			if (3 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT; 
			OnCommandCompleted(pDispParams->rgvarg[2].lVal, pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].pvarVal);
			return S_OK;
		case EVENT_LOG_MESSAGE:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT; 
			OnLogMessage(pDispParams->rgvarg[0].bstrVal);
			return S_OK;
		case EVENT_SERVER_MESSAGE:
			if (3 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT; 
			OnServerMessage(pDispParams->rgvarg[2].lVal, pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].bstrVal);
			return S_OK;
		case EVENT_COMMAND_PROGRESS:
			if (4 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT; 
			OnCommandProgress(pDispParams->rgvarg[3].lVal, pDispParams->rgvarg[2].lVal, pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].lVal);
			return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}

void CDBBEventManager::OnCommandCompleted(LONG lCommandCode, HRESULT hCommandResult, VARIANT *pCommandData)
{
	if(fnCmdCompleted) fnCmdCompleted(lCommandCode, hCommandResult, pCommandData, user);
}

void CDBBEventManager::OnLogMessage(BSTR bstrMessage)
{
	if(fnLogMessage) fnLogMessage(bstrMessage, user);
}

void CDBBEventManager::OnServerMessage(LONG lMessageCode, LONG lMessageAction, BSTR bstrMessageData)
{
	if(fnServerMessage) fnServerMessage(lMessageCode, lMessageAction,bstrMessageData, user);
}

void CDBBEventManager::OnCommandProgress(LONG lCommandCode, LONG lProgressCode, LONG lBytesDone, LONG lBytesTotal)
{
	if(fnCmdProgress) fnCmdProgress(lCommandCode, lProgressCode, lBytesDone, lBytesTotal, user);

}