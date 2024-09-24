#include "SWFContainer.h"

#include <strsafe.h>

// ---------------------------------------------------------------
IConnectionPoint *SWFContainer::GetConnectionPoint (REFIID riid)
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

void SWFContainer::SyncSizeToWindow(HWND hwnd)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	int height = (rect.bottom - rect.top);

	// if we get a null height then hide the html control (after limiting to 1px)
	// and also hide it's parent window - is mainly for ml_wire to prevent display
	// glitches when resizing the bottom segment all the way to the bottom
	//ShowWindow(m_hwnd,height?SW_SHOWNA:SW_HIDE);
	//ShowWindow(hwnd,height?SW_SHOWNA:SW_HIDE);
	setLocation(0, 0, rect.right - rect.left, height?height:1);
}

 SWFContainer::SWFContainer(HWND hwnd)
	 : flash (0), m_cRefs(1), m_hwnd(hwnd),  m_punk(NULL),
	 externalInterface(0)
 {
	
	bInitialized = (S_OK == CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) ? true : false;
	
	memset(&m_rect, 0, sizeof(m_rect));
	add(ShockwaveFlashObjects::CLSID_ShockwaveFlash);

	IUnknown *punk = getUnknown();
	if (punk)
	 {
		 
		 if (SUCCEEDED(punk->QueryInterface (ShockwaveFlashObjects::IID_IShockwaveFlash, (void **) & flash)))
		 {
			 IConnectionPoint *icp = GetConnectionPoint(ShockwaveFlashObjects::DIID__IShockwaveFlashEvents);
			 if (icp)
			 {
				 m_dwCookie = 0;
				 HRESULT hr = icp->Advise(static_cast<IDispatch *>(this), &m_dwCookie);
				 icp->Release();
			 }
		 }
		 else
			 flash=0;
		 punk->Release();
	 }
 }

SWFContainer::~SWFContainer()
{
	close();

	if (bInitialized) CoUninitialize();
}

void SWFContainer::close()
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

	if (flash)
	{
		flash->Stop();
		flash->Release();
		flash = 0;
	}
}

STDMETHODIMP SWFContainer::QueryInterface(REFIID riid, PVOID *ppvObject)
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
//	else if (IsEqualIID(riid, IID_IOleControlSite))
//		*ppvObject = (IOleControlSite *)this;
	else if (IsEqualIID(riid, IID_IOleWindow))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else if (IsEqualIID(riid, __uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents)))
		*ppvObject = (IDispatch *)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG SWFContainer::AddRef(void)
{
	return ++m_cRefs;
}

ULONG SWFContainer::Release(void)
{
	if (--m_cRefs)
		return m_cRefs;

//	PostQuitMessage(0);
	delete this;
	return 0;
}

HRESULT SWFContainer::SaveObject()
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppMk)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::GetContainer(LPOLECONTAINER * ppContainer)
{
	return E_NOINTERFACE;
}

HRESULT SWFContainer::ShowObject()
{
	return S_OK;
}

HRESULT SWFContainer::OnShowWindow(BOOL fShow)
{
	return S_OK;
}

HRESULT SWFContainer::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::GetWindow(HWND * lphwnd)
{
	if (!IsWindow(m_hwnd))
		return S_FALSE;

	*lphwnd = m_hwnd;
	return S_OK;
}

HRESULT SWFContainer::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::CanInPlaceActivate(void)
{
	return S_OK;
}

HRESULT SWFContainer::OnInPlaceActivate(void)
{
	return S_OK;
}

HRESULT SWFContainer::OnUIActivate(void)
{
	return S_OK;
}

HRESULT SWFContainer::GetWindowContext(IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppIIPUIWin,
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

HRESULT SWFContainer::Scroll(SIZE scrollExtent)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::OnUIDeactivate(BOOL fUndoable)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::OnInPlaceDeactivate(void)
{
	return S_OK;
}

HRESULT SWFContainer::DiscardUndoState(void)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::OnPosRectChange(LPCRECT lprcPosRect)
{
	return S_OK;
}

HRESULT SWFContainer::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::RemoveMenus(HMENU hmenuShared)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::SetStatusText(LPCOLESTR pszStatusText)
{
	return S_OK;
}

HRESULT SWFContainer::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
	return S_OK;
}

HRESULT SWFContainer::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::OnControlInfoChanged()
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::LockInPlaceActive(BOOL fLock)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::GetExtendedControl(IDispatch **ppDisp)
{
	if (ppDisp == NULL)
		return E_INVALIDARG;

	*ppDisp = (IDispatch *)this;
	(*ppDisp)->AddRef();

	return S_OK;
}

HRESULT SWFContainer::TransformCoords(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
	return S_FALSE;
}

HRESULT SWFContainer::OnFocus(BOOL fGotFocus)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::ShowPropertyFrame(void)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	*rgdispid = DISPID_UNKNOWN;
	return DISP_E_UNKNOWNNAME;
}

HRESULT SWFContainer::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

#define GET_SAFE_DISP_BSTR(_val) ((_val.pvarVal && VT_BSTR == _val.pvarVal->vt) ? _val.pvarVal->bstrVal : NULL)
#define GET_SAFE_DISP_I4(_val) ((_val.pvarVal && VT_I4 == _val.pvarVal->vt) ? _val.pvarVal->intVal : 0)



void SWFContainer::add(CLSID clsid)
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

void SWFContainer::remove()
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


void SWFContainer::setLocation(int x, int y, int width, int height)
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

HRESULT SWFContainer::GetBorder(LPRECT lprectBorder)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

HRESULT SWFContainer::SetActiveObject(IOleInPlaceActiveObject * pActiveObject, LPCOLESTR lpszObjName)
{
	return E_NOTIMPL;
}

void SWFContainer::setVisible(BOOL fVisible)
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

void SWFContainer::setFocus(BOOL fFocus)
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

bool SWFContainer::translateKey(LPMSG pMsg)
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
enum
{
	FLASH_DISPID_EXTERNALINTERFACE_CALL = 197,
};

HRESULT SWFContainer::Invoke(
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr)
{
	switch (dispIdMember)
	{
		case 0x000007a6 : // OnProgress
		break;

        case 0x00000096 : // FSCommand
		break;

        case FLASH_DISPID_EXTERNALINTERFACE_CALL : // ExternalInterface.call()
		{
			if (externalInterface)
				externalInterface->ExternalInterface_call(pDispParams->rgvarg[0].bstrVal);
		}
		break;
	}
	return DISP_E_MEMBERNOTFOUND;
}


/**************************************************************************
 
* adContainer::getUnknown()
 
**************************************************************************/

IUnknown * SWFContainer::getUnknown()
{
	if (!m_punk)
		return NULL;

	m_punk->AddRef();
	return m_punk;
}
