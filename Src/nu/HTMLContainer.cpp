#include "HTMLContainer.h"

#include <exdisp.h>
#include <mshtmdid.h>
#include <mshtml.h>
#include <exdispid.h>
#include <strsafe.h>

#ifndef DISPID_NEWWINDOW3
#define DISPID_NEWWINDOW3 273 
#endif 

// ---------------------------------------------------------------
IConnectionPoint *HTMLContainer::GetConnectionPoint (REFIID riid)
{
	IUnknown *punk = getUnknown ();
	if (!punk)
		return 0; 

	IConnectionPointContainer *pcpc;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface (IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED (hr))
	{
		pcpc->FindConnectionPoint (riid, &pcp);
		pcpc->Release();
	}
	punk->Release();
	return pcp;
}

void HTMLContainer::SyncSizeToWindow(HWND hwnd)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int height = (rect.bottom - rect.top);

	// if we get a null height then hide the html control (after limiting to 1px)
	// and also hide it's parent window - is mainly for ml_wire to prevent display
	// glitches when resizing the bottom segment all the way to the bottom
	ShowWindow(m_hwnd,height?SW_SHOWNA:SW_HIDE);
	ShowWindow(hwnd,height?SW_SHOWNA:SW_HIDE);
	setLocation(0, 0, rect.right - rect.left, height?height:1);
}

// uncomment if you ever want to use mozilla instead of IE
// change the CLSID_WebBrowser in the constructor below to CLSID_MozillaBrowser
// but window.external from javascript doesn't work :(

static const CLSID CLSID_MozillaBrowser=
    { 0x1339B54C, 0x3453, 0x11D2, { 0x93, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

 HTMLContainer::HTMLContainer(HWND hwnd)
	 : m_pweb (0), pszHostCSS(NULL), m_cRefs(1), m_hwnd(hwnd),  m_punk(NULL)
 {
	bInitialized = (S_OK == CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) ? true : false;
	dwHostInfoFlags = DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION | DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIDBLCLK_DEFAULT;
	dwDownloadFlags = DLCTL_DLIMAGES | DLCTL_VIDEOS  | DLCTL_PRAGMA_NO_CACHE;

	memset(&m_rect, 0, sizeof(m_rect));
	add(CLSID_WebBrowser);

	IUnknown *punk = getUnknown();
	if (punk)
	{
		if (SUCCEEDED(punk->QueryInterface (IID_IWebBrowser2, (void **) & m_pweb))
			 || SUCCEEDED(punk->QueryInterface (IID_IWebBrowser, (void **) & m_pweb)))
		{
			IConnectionPoint *icp = GetConnectionPoint(DIID_DWebBrowserEvents2);
			if (icp)
			{
				m_dwCookie = 0;
				icp->Advise(static_cast<IDispatch *>(this), &m_dwCookie);
				icp->Release();
			}
		}
		else
			m_pweb=0;
		punk->Release();
	}
}

HTMLContainer::HTMLContainer()
		: m_pweb (0), pszHostCSS(NULL), 	m_cRefs(1), m_hwnd(NULL), m_punk(NULL)
{
	bInitialized = (S_OK == CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) ? true : false;
	dwHostInfoFlags = DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION | DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIDBLCLK_DEFAULT;
	dwDownloadFlags = DLCTL_DLIMAGES | DLCTL_VIDEOS  | DLCTL_PRAGMA_NO_CACHE;

	memset(&m_rect, 0, sizeof(m_rect));
	add(CLSID_WebBrowser);

	IUnknown *punk = getUnknown();
	if (punk)
	{
		if (SUCCEEDED(punk->QueryInterface (IID_IWebBrowser2, (void **) & m_pweb))
			|| SUCCEEDED(punk->QueryInterface (IID_IWebBrowser, (void **) & m_pweb)))
		{
			IConnectionPoint *icp = GetConnectionPoint(DIID_DWebBrowserEvents2);
			if (icp)
			{
				m_dwCookie = 0;
				icp->Advise(static_cast<IDispatch *>(this), &m_dwCookie);
				icp->Release();
			}
		}
		else
			m_pweb=0;
		punk->Release();
	}
}

HTMLContainer::~HTMLContainer()
{
	close();
	if (pszHostCSS) { free(pszHostCSS); pszHostCSS = NULL; }

	if (bInitialized) CoUninitialize();
}

void HTMLContainer::close()
{
	IOleObject *pioo;
	if ( m_punk )
	{
		HRESULT hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
		if (SUCCEEDED(hr))
		{
		    pioo->Close(OLECLOSE_NOSAVE);
		    pioo->Release();
		}
	}

	if (m_punk)
	{
		m_punk->Release();
		m_punk = NULL;
	}

	if (m_pweb)
	{
		m_pweb->Quit();
		m_pweb->Release();
		m_pweb = 0;
	}
}

STDMETHODIMP HTMLContainer::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	if (IsEqualIID(riid, IID_IOleClientSite))
		*ppvObject = (IOleClientSite *)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceSite))
		*ppvObject = (IOleInPlaceSite *)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
		*ppvObject = (IOleInPlaceFrame *)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
		*ppvObject = (IOleInPlaceUIWindow *)this;
	else if (IsEqualIID(riid, IID_IOleControlSite))
		*ppvObject = (IOleControlSite *)this;
	else if (IsEqualIID(riid, IID_IOleWindow))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_IDocHostUIHandler))
		*ppvObject = (IDocHostUIHandler *)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG HTMLContainer::AddRef(void)
{
	return ++m_cRefs;
}

ULONG HTMLContainer::Release(void)
{
	if (--m_cRefs)
		return m_cRefs;
	return 0;
}

HRESULT HTMLContainer::SaveObject()
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppMk)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetContainer(LPOLECONTAINER * ppContainer)
{
	return E_NOINTERFACE;
}

HRESULT HTMLContainer::ShowObject()
{
	return S_OK;
}

HRESULT HTMLContainer::OnShowWindow(BOOL fShow)
{
	return S_OK;
}

HRESULT HTMLContainer::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetWindow(HWND * lphwnd)
{
	if (!IsWindow(m_hwnd))
		return S_FALSE;

	*lphwnd = m_hwnd;
	return S_OK;
}

HRESULT HTMLContainer::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::CanInPlaceActivate(void)
{
	return S_OK;
}

HRESULT HTMLContainer::OnInPlaceActivate(void)
{
	return S_OK;
}

HRESULT HTMLContainer::OnUIActivate(void)
{
	return S_OK;
}

HRESULT HTMLContainer::GetWindowContext(IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppIIPUIWin,
        LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	*ppFrame = (IOleInPlaceFrame *)this;
	*ppIIPUIWin = NULL;

	RECT rect;
	GetClientRect(m_hwnd, &rect);
	lprcPosRect->left = 0;
	lprcPosRect->top = 0;
	lprcPosRect->right = rect.right;
	lprcPosRect->bottom = rect.bottom;

	CopyRect(lprcClipRect, lprcPosRect);

	lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = m_hwnd;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	(*ppFrame)->AddRef();
	return S_OK;
}

HRESULT HTMLContainer::Scroll(SIZE scrollExtent)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnUIDeactivate(BOOL fUndoable)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnInPlaceDeactivate(void)
{
	return S_OK;
}

HRESULT HTMLContainer::DiscardUndoState(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnPosRectChange(LPCRECT lprcPosRect)
{
	return S_OK;
}

HRESULT HTMLContainer::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::RemoveMenus(HMENU hmenuShared)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::SetStatusText(LPCOLESTR pszStatusText)
{
	return S_OK;
}

HRESULT HTMLContainer::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
	return S_OK;
}

HRESULT HTMLContainer::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnControlInfoChanged()
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::LockInPlaceActive(BOOL fLock)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetExtendedControl(IDispatch **ppDisp)
{
	if (ppDisp == NULL)
		return E_INVALIDARG;

	*ppDisp = (IDispatch *)this;
	(*ppDisp)->AddRef();

	return S_OK;
}

HRESULT HTMLContainer::TransformCoords(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
	return S_FALSE;
}

HRESULT HTMLContainer::OnFocus(BOOL fGotFocus)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::ShowPropertyFrame(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	*rgdispid = DISPID_UNKNOWN;
	return DISP_E_UNKNOWNNAME;
}

HRESULT HTMLContainer::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

void HTMLContainer::OnBeforeNavigate(IDispatch *pDispatch, LPCWSTR pszURL, DWORD dwFlags, LPCWSTR pszTargetFrameName, VARIANT *vtPostData, LPCWSTR pszHeaders, VARIANT_BOOL *Cancel)
{
	
}

void HTMLContainer::OnNavigateError(IDispatch *pDispatch, LPCWSTR pszURL, LPCWSTR pszTargetFrameName, INT nStatusCode, VARIANT_BOOL *Cancel)
{
}

void HTMLContainer::OnNavigateComplete(IDispatch *pDispatch, LPCWSTR pszURL)
{
}

void HTMLContainer::OnDocumentComplete(IDispatch *pDisp, LPCWSTR pszURL)
{
}

void HTMLContainer::OnDownloadBegin(void)
{
}

void HTMLContainer::OnDownloadComplete(void)
{
}

void HTMLContainer::OnFileDownload(VARIANT_BOOL *ActiveDocument, VARIANT_BOOL *Cancel)
{
}

void HTMLContainer::OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel)
{
}

void HTMLContainer::OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, LPCWSTR pszUrlContext, LPCWSTR pszUrl)
{
}

void HTMLContainer::OnProgressChange(long Progress, long ProgressMax)
{
}

void HTMLContainer::OnStatusTextChange(LPCWSTR pszText)
{
}


#define GET_SAFE_DISP_BSTR(_val) ((_val.pvarVal && VT_BSTR == _val.pvarVal->vt) ? _val.pvarVal->bstrVal : NULL)
#define GET_SAFE_DISP_I4(_val) ((_val.pvarVal && VT_I4 == _val.pvarVal->vt) ? _val.pvarVal->intVal : 0)

HRESULT HTMLContainer::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISPID_BEFORENAVIGATE2:
		OnBeforeNavigate();
		OnBeforeNavigate( pdispparams->rgvarg[6].pdispVal, GET_SAFE_DISP_BSTR(pdispparams->rgvarg[5]),
							GET_SAFE_DISP_I4(pdispparams->rgvarg[4]), GET_SAFE_DISP_BSTR(pdispparams->rgvarg[3]),
							pdispparams->rgvarg[2].pvarVal, GET_SAFE_DISP_BSTR(pdispparams->rgvarg[1]), pdispparams->rgvarg[0].pboolVal);
		break;
	case DISPID_NAVIGATEERROR:
		{			
			VARIANT * vt_statuscode = pdispparams->rgvarg[1].pvarVal;
			DWORD  dwStatusCode =  vt_statuscode->lVal;
			if (dwStatusCode == 200)  
			{
				*pdispparams->rgvarg[0].pboolVal = VARIANT_TRUE;
				break;
			}
			OnNavigateError();
			OnNavigateError(pdispparams->rgvarg[4].pdispVal, GET_SAFE_DISP_BSTR(pdispparams->rgvarg[3]), 
					GET_SAFE_DISP_BSTR(pdispparams->rgvarg[2]), GET_SAFE_DISP_I4(pdispparams->rgvarg[1]), pdispparams->rgvarg[0].pboolVal);
		}
		break;
	case DISPID_NAVIGATECOMPLETE2:
		OnNavigateComplete();
		OnNavigateComplete(pdispparams->rgvarg[1].pdispVal, GET_SAFE_DISP_BSTR(pdispparams->rgvarg[0]));
		break;
	case DISPID_DOCUMENTCOMPLETE:
		OnDocumentComplete(pdispparams->rgvarg[1].pdispVal, GET_SAFE_DISP_BSTR(pdispparams->rgvarg[0]));
		break;
	case DISPID_DOWNLOADBEGIN:
		OnDownloadBegin();
		break;
	case DISPID_DOWNLOADCOMPLETE:
		OnDownloadComplete();
		break;
	case DISPID_FILEDOWNLOAD:
		OnFileDownload(pdispparams->rgvarg[1].pboolVal, 	pdispparams->rgvarg[0].pboolVal);
		break;
	case DISPID_NEWWINDOW2:
		OnNewWindow2(pdispparams->rgvarg[1].ppdispVal, pdispparams->rgvarg[0].pboolVal);
		break;
	case DISPID_NEWWINDOW3:
		OnNewWindow3(pdispparams->rgvarg[4].ppdispVal, pdispparams->rgvarg[3].pboolVal, 
					 pdispparams->rgvarg[2].intVal, pdispparams->rgvarg[1].bstrVal, pdispparams->rgvarg[0].bstrVal);
		break;
	case DISPID_PROGRESSCHANGE:
		OnProgressChange(pdispparams->rgvarg[1].lVal, pdispparams->rgvarg[0].lVal);
		break;
	case DISPID_STATUSTEXTCHANGE:
		OnStatusTextChange(GET_SAFE_DISP_BSTR(pdispparams->rgvarg[0]));
		break;
	case DISPID_AMBIENT_USERAGENT:
		/* TODO:
	      pvar->vt = VT_BSTR;
      pvar->bstrVal = SysAllocString("...");
      return S_OK;
			*/
		break;
	case DISPID_AMBIENT_DLCONTROL:
		pvarResult->vt = VT_I4;
		pvarResult->lVal = dwDownloadFlags;
		return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}

void HTMLContainer::add(CLSID clsid)
{
	HRESULT hr;             // return code
	CoCreateInstance(clsid,
	                 NULL,
	                 CLSCTX_INPROC_SERVER/* | CLSCTX_LOCAL_SERVER*/,
	                 IID_IUnknown,
	                 (PVOID *)&m_punk);

	if (!m_punk)
		return ;

	IOleObject *pioo;
	hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
	if (FAILED(hr))
		return ;

	pioo->SetClientSite(this);
	pioo->Release();

	IPersistStreamInit *ppsi;
	hr = m_punk->QueryInterface(IID_IPersistStreamInit, (PVOID *) & ppsi);
	if (SUCCEEDED(hr))
	{
		ppsi->InitNew();
		ppsi->Release();
	}
}

void HTMLContainer::remove()
{
	if (!m_punk)
		return ;

	HRESULT hr;
	IOleObject *pioo;
	IOleInPlaceObject *pipo;

	/*
	benski> enabling this makes everything lock up!
			IConnectionPoint *icp = GetConnectionPoint(DIID_DWebBrowserEvents2);
			if (icp)
			{
//			 m_dwCookie = 0;
			 HRESULT hr = icp->Unadvise(m_dwCookie);
			 icp->Release();
			}
	*/

	hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
	if (SUCCEEDED(hr))
	{
		pioo->Close(OLECLOSE_NOSAVE);
		pioo->SetClientSite(NULL);
		pioo->Release();
	}

	hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *) & pipo);
	if (SUCCEEDED(hr))
	{
		pipo->UIDeactivate();
		pipo->InPlaceDeactivate();
		pipo->Release();
	}

	m_punk->Release();
	m_punk = NULL;
}

void HTMLContainer::setLocation(int x, int y, int width, int height)
{
	m_rect.left = x;
	m_rect.top = y;
	m_rect.right = x + width;
	m_rect.bottom = y + height;

	if (!m_punk)
		return ;

	HRESULT hr;
	IOleInPlaceObject *pipo;

	hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *) & pipo);
	if (FAILED(hr))
		return ;

	pipo->SetObjectRects(&m_rect, &m_rect);
	pipo->Release();
}

HRESULT HTMLContainer::GetBorder(LPRECT lprectBorder)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::SetActiveObject(IOleInPlaceActiveObject * pActiveObject, LPCOLESTR lpszObjName)
{
	return E_NOTIMPL;
}

void HTMLContainer::setVisible(BOOL fVisible)
{
	if (!m_punk)
		return ;

	HRESULT hr;
	IOleObject *pioo;

	hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
	if (FAILED(hr))
		return ;

	if (fVisible)
	{
		pioo->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_hwnd, &m_rect);
		pioo->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hwnd, &m_rect);
	}
	else
		pioo->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_hwnd, NULL);

	pioo->Release();
}

void HTMLContainer::setFocus(BOOL fFocus)
{
	if (!m_punk)
		return ;

	HRESULT hr;
	IOleObject *pioo;

	if (fFocus)
	{
		hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
		if (FAILED(hr))
			return ;

		pioo->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_hwnd, &m_rect);
		pioo->Release();
	}
}

bool HTMLContainer::translateKey(LPMSG pMsg)
{
	if (!m_punk)
		return false;

	HRESULT hr;
	IOleInPlaceActiveObject *pao;

	hr = m_punk->QueryInterface(IID_IOleInPlaceActiveObject, (PVOID *) & pao);
	if (FAILED(hr))
		return false;

	HRESULT res = pao->TranslateAccelerator(pMsg);
	pao->Release();
	return res == S_OK;
}

/**************************************************************************
 
* adContainer::getDispatch()
 
**************************************************************************/

IDispatch * HTMLContainer::getDispatch()
{
	if (!m_punk)
		return NULL;

	IDispatch *pdisp = NULL;
	m_punk->QueryInterface(IID_IDispatch, (PVOID *) & pdisp);
	return pdisp;
}

/**************************************************************************
 
* adContainer::getUnknown()
 
**************************************************************************/

IUnknown * HTMLContainer::getUnknown()
{
	if (!m_punk)
		return NULL;

	m_punk->AddRef();
	return m_punk;
}

// ***********************************************************************
//  IDocHostUIHandler
// ***********************************************************************

HRESULT HTMLContainer::ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo)
{
	pInfo->cbSize = sizeof(DOCHOSTUIINFO);
	pInfo->dwFlags = dwHostInfoFlags;

	if (pszHostCSS)
	{
		INT strlen;
		OLECHAR *pocCSS;
		strlen = lstrlenW(pszHostCSS);
		if (strlen)
		{
			strlen++;
			pocCSS = (OLECHAR*)CoTaskMemAlloc(strlen * sizeof(OLECHAR));
			if (pocCSS && S_OK== StringCchCopyW(pocCSS, strlen, pszHostCSS))  pInfo->pchHostCss = pocCSS;
		}
	}
	return S_OK;
}

HRESULT HTMLContainer::ShowUI(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::HideUI(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::UpdateUI(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnDocWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnFrameWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetOptionKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetDropTarget(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::TranslateUrl(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::FilterDataObject(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
	return E_NOTIMPL;
}

BOOL HTMLContainer::SetHostCSS(LPCWSTR pszHostCSS)
{
	if (this->pszHostCSS) { free(this->pszHostCSS); this->pszHostCSS = NULL; }
	if (pszHostCSS && *pszHostCSS) this->pszHostCSS = _wcsdup(pszHostCSS);
	return TRUE;
}

HWND HTMLContainer::GetHostHWND(void)
{	
	if (m_punk) 
	{
		IOleInPlaceObject   *pipo;
		m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
		if (pipo)
		{
			HWND hwndHost;
			pipo->GetWindow(&hwndHost);
			pipo->Release();
			return hwndHost;
		}
	}
	return NULL;
}

DWORD HTMLContainer::SetDownloadFlags(DWORD dwFlags)
{
	DWORD temp;
	temp = dwDownloadFlags;
	dwDownloadFlags = dwFlags;
	return temp;
}

DWORD HTMLContainer::SetHostInfoFlags(DWORD dwFlags)
{
	DWORD temp;
	temp = dwHostInfoFlags;
	dwHostInfoFlags = dwFlags;
	return temp;
}