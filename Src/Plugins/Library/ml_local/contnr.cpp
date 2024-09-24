/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1998 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          contnr.cpp

   Description:   This file contains the complete implementation of an 
                  ActiveX control container. This purpose of this container 
                  is to test a single control being hosted.

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/
#include "main.h"
#include <windows.h>
#include <commctrl.h>
#include "contnr.h"

#include <mshtmdid.h>
#include <shlobj.h>
#include "ml_local.h"

/**************************************************************************

   CContainer::CContainer()

**************************************************************************/
extern IDispatch *winampExternal;

CContainer::CContainer()
{
    m_cRefs     = 1;
    m_hwnd      = NULL;
    m_punk      = NULL;
    m_scrollbars= 1;
    m_allowScripts=1;
    m_restrictAlreadySet=0;
	m_hwndStatus = 0;
   
}

/**************************************************************************

   CContainer::~CContainer()

**************************************************************************/

CContainer::~CContainer()
{
    if (m_punk)
    {
        m_punk->Release();
        m_punk=NULL;
    }
}

/**************************************************************************

   CContainer::QueryInterface()

**************************************************************************/

STDMETHODIMP CContainer::QueryInterface(REFIID riid, PVOID *ppvObject)
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

/**************************************************************************

   CContainer::AddRef()

**************************************************************************/

ULONG CContainer::AddRef(void)
{
    return ++m_cRefs;
}

/**************************************************************************

   CContainer::Release()

**************************************************************************/

ULONG CContainer::Release(void)
{
    if (--m_cRefs)
        return m_cRefs;

    delete this;
    return 0;
}

/**************************************************************************

   CContainer::SaveObject()

**************************************************************************/

HRESULT CContainer::SaveObject()
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::GetMoniker()

**************************************************************************/

HRESULT CContainer::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppMk)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::GetContainer()

**************************************************************************/

HRESULT CContainer::GetContainer(LPOLECONTAINER * ppContainer)
{
    return E_NOINTERFACE;
}

/**************************************************************************

   CContainer::ShowObject()

**************************************************************************/

HRESULT CContainer::ShowObject()
{
    return S_OK;
}

/**************************************************************************

   CContainer::OnShowWindow()

**************************************************************************/

HRESULT CContainer::OnShowWindow(BOOL fShow)
{
    return S_OK;
}

/**************************************************************************

   CContainer::RequestNewObjectLayout()

**************************************************************************/

HRESULT CContainer::RequestNewObjectLayout()
{
    return E_NOTIMPL;
}

// ***********************************************************************
//  IOleWindow
// ***********************************************************************

/**************************************************************************

   CContainer::GetWindow()

**************************************************************************/

HRESULT CContainer::GetWindow(HWND * lphwnd)
{
    if (!IsWindow(m_hwnd))
        return S_FALSE;

    *lphwnd = m_hwnd;
    return S_OK;
}

/**************************************************************************

   CContainer::ContextSensitiveHelp()

**************************************************************************/

HRESULT CContainer::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}

// ***********************************************************************
//  IOleInPlaceSite
// ***********************************************************************

/**************************************************************************

   CContainer::CanInPlaceActivate()

**************************************************************************/

HRESULT CContainer::CanInPlaceActivate(void)
{
    return S_OK;
}

/**************************************************************************

   CContainer::OnInPlaceActivate()

**************************************************************************/

HRESULT CContainer::OnInPlaceActivate(void)
{
    return S_OK;
}

/**************************************************************************

   CContainer::OnUIActivate()

**************************************************************************/

HRESULT CContainer::OnUIActivate(void)
{
    return S_OK;
}

/**************************************************************************

   CContainer::GetWindowContext()

**************************************************************************/

HRESULT CContainer::GetWindowContext (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppIIPUIWin,
                                  LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    *ppFrame = (IOleInPlaceFrame *)this;
    *ppIIPUIWin = NULL;

    RECT rect;
    GetClientRect(m_hwnd, &rect);
    lprcPosRect->left       = 0;
    lprcPosRect->top        = 0;
    lprcPosRect->right      = rect.right;
    lprcPosRect->bottom     = rect.bottom;

    CopyRect(lprcClipRect, lprcPosRect);

    lpFrameInfo->cb             = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp        = FALSE;
    lpFrameInfo->hwndFrame      = m_hwnd;
    lpFrameInfo->haccel         = 0;
    lpFrameInfo->cAccelEntries  = 0;

    (*ppFrame)->AddRef();
    return S_OK;
}

/**************************************************************************

   CContainer::Scroll()

**************************************************************************/

HRESULT CContainer::Scroll(SIZE scrollExtent)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::OnUIDeactivate()

**************************************************************************/

HRESULT CContainer::OnUIDeactivate(BOOL fUndoable)
{
    return S_OK;
}

/**************************************************************************

   CContainer::OnInPlaceDeactivate()

**************************************************************************/

HRESULT CContainer::OnInPlaceDeactivate(void)
{
    return S_OK;
}

/**************************************************************************

   CContainer::DiscardUndoState()

**************************************************************************/

HRESULT CContainer::DiscardUndoState(void)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::DeactivateAndUndo()

**************************************************************************/

HRESULT CContainer::DeactivateAndUndo(void)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::OnPosRectChange()

**************************************************************************/

HRESULT CContainer::OnPosRectChange(LPCRECT lprcPosRect)
{
	HRESULT hr;
    IOleInPlaceObject   *pipo;

    hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
    if (SUCCEEDED(hr))
    {
        pipo->SetObjectRects(lprcPosRect, lprcPosRect);
        pipo->Release();
	}
	return(S_OK);
}

// ***********************************************************************
//  IOleInPlaceUIWindow
// ***********************************************************************

/**************************************************************************

   CContainer::GetBorder()

**************************************************************************/

HRESULT CContainer::GetBorder(LPRECT lprectBorder)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::RequestBorderSpace()

**************************************************************************/

HRESULT CContainer::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::SetBorderSpace()

**************************************************************************/

HRESULT CContainer::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::SetActiveObject()

**************************************************************************/

HRESULT CContainer::SetActiveObject(IOleInPlaceActiveObject * pActiveObject, LPCOLESTR lpszObjName)
{
    return S_OK;
}

// ***********************************************************************
//  IOleInPlaceFrame
// ***********************************************************************

/**************************************************************************

   CContainer::InsertMenus()

**************************************************************************/

HRESULT CContainer::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::SetMenu()

**************************************************************************/

HRESULT CContainer::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    return S_OK;
}

/**************************************************************************

   CContainer::RemoveMenus()

**************************************************************************/

HRESULT CContainer::RemoveMenus(HMENU hmenuShared)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::SetStatusText()

**************************************************************************/

HRESULT CContainer::SetStatusText(LPCOLESTR pszStatusText)
{
    char status[MAX_PATH];              // ansi version of status text

    if (NULL == pszStatusText)
        return E_POINTER;

    WideCharToMultiByte(CP_ACP, 0, pszStatusText, -1, status, MAX_PATH, NULL, NULL);

    if (IsWindow(m_hwndStatus))
        SendMessage(m_hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)status);

    return (S_OK);
}

/**************************************************************************

   CContainer::EnableModeless()

**************************************************************************/

HRESULT CContainer::EnableModeless(BOOL fEnable)
{
    return S_OK;
}

/**************************************************************************

   CContainer::TranslateAccelerator()

**************************************************************************/

HRESULT CContainer::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
    return E_NOTIMPL;
}

// ***********************************************************************
//  IOleControlSite
// ***********************************************************************

/**************************************************************************

   CContainer::OnControlInfoChanged()

**************************************************************************/

HRESULT CContainer::OnControlInfoChanged()
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::LockInPlaceActive()

**************************************************************************/

HRESULT CContainer::LockInPlaceActive(BOOL fLock)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::GetExtendedControl()

**************************************************************************/

HRESULT CContainer::GetExtendedControl(IDispatch **ppDisp)
{
    if (ppDisp == NULL)
        return E_INVALIDARG;

    *ppDisp = (IDispatch *)this;
    (*ppDisp)->AddRef();

    return S_OK;
}

/**************************************************************************

   CContainer::TransformCoords()

**************************************************************************/

HRESULT CContainer::TransformCoords(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::TranslateAccelerator()

**************************************************************************/

HRESULT CContainer::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
    return S_FALSE;
}

/**************************************************************************

   CContainer::OnFocus()

**************************************************************************/

HRESULT CContainer::OnFocus(BOOL fGotFocus)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::ShowPropertyFrame()

**************************************************************************/

HRESULT CContainer::ShowPropertyFrame(void)
{
    return E_NOTIMPL;
}

// ***********************************************************************
//  IDispatch
// ***********************************************************************

/**************************************************************************

   CContainer::GetIDsOfNames()

**************************************************************************/

HRESULT CContainer::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
    *rgdispid = DISPID_UNKNOWN;
    return DISP_E_UNKNOWNNAME;
}

/**************************************************************************

   CContainer::GetTypeInfo()

**************************************************************************/

HRESULT CContainer::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::GetTypeInfoCount()

**************************************************************************/

HRESULT CContainer::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CContainer::Invoke()

**************************************************************************/

HRESULT CContainer::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
    if(dispid==DISPID_AMBIENT_DLCONTROL)
    {
      m_restrictAlreadySet=1;
      if(!m_allowScripts)
      {
        pvarResult->vt = VT_I4;
        pvarResult->lVal = DLCTL_DLIMAGES|DLCTL_NO_SCRIPTS|DLCTL_NO_DLACTIVEXCTLS|DLCTL_NO_RUNACTIVEXCTLS|DLCTL_NO_JAVA;
        return S_OK;
      }
    }
    return DISP_E_MEMBERNOTFOUND;
}

// ***********************************************************************
//  Public (non-interface) Methods
// ***********************************************************************

/**************************************************************************

   CContainer::add()

**************************************************************************/

void CContainer::add(BSTR bstrClsid)
{
    CLSID   clsid;          // CLSID of the control object
    HRESULT hr;             // return code

    CLSIDFromString(bstrClsid, &clsid);
    CoCreateInstance(clsid, 
                     NULL, 
                     CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, 
                     IID_IUnknown,
                     (PVOID *)&m_punk);

    if (!m_punk)
        return;

    IOleObject *pioo;
    hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
    if (FAILED(hr))
        return;

    pioo->SetClientSite(this);
    pioo->Release();

    IPersistStreamInit  *ppsi;
    hr = m_punk->QueryInterface(IID_IPersistStreamInit, (PVOID *)&ppsi);
    if (SUCCEEDED(hr))
    {
        ppsi->InitNew();
        ppsi->Release();
    }
}

/**************************************************************************

   CContainer::remove()

**************************************************************************/

void CContainer::remove()
{
    if (!m_punk)
        return;

    HRESULT             hr;
    IOleObject          *pioo;
    IOleInPlaceObject   *pipo;

    hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
    if (SUCCEEDED(hr))
    {
        pioo->Close(OLECLOSE_NOSAVE);
        pioo->SetClientSite(NULL);
        pioo->Release();
    }

    hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
    if (SUCCEEDED(hr))
    {
        pipo->UIDeactivate();
        pipo->InPlaceDeactivate();
        pipo->Release();
    }

    m_punk->Release();
    m_punk = NULL;
}

/**************************************************************************

   CContainer::setParent()

**************************************************************************/

void CContainer::setParent(HWND hwndParent)
{
    m_hwnd = hwndParent;
}

/**************************************************************************

   CContainer::setLocation()

**************************************************************************/

void CContainer::setLocation(int x, int y, int width, int height)
{
	RECT rc;
	::SetRect(&rc, x, y, x + width, y + height);
    
    if (!m_punk)     return;

    HRESULT             hr;
    IOleInPlaceObject   *pipo;

    hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
    if (FAILED(hr))
        return;

    pipo->SetObjectRects(&rc, &rc);
    pipo->Release();
}

BOOL CContainer::SetRect(RECT *prc)
{	
	HRESULT             hr;
    IOleInPlaceObject   *pipo;
	

	if (!m_punk)	 return FALSE;

    hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
    if (SUCCEEDED(hr))
	{
		hr = pipo->SetObjectRects(prc, prc);
		pipo->Release();
	}
	return SUCCEEDED(hr);
}
/**************************************************************************

   CContainer::setVisible()

**************************************************************************/

void CContainer::setVisible(BOOL fVisible)
{
    if (!m_punk)
        return;

    HRESULT     hr;
    IOleObject  *pioo;

    hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
    if (FAILED(hr))
        return;
    
    if (fVisible)
    {
	
		pioo->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_hwnd, NULL);
        pioo->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hwnd, NULL);
    }
    else
        pioo->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_hwnd, NULL);

    pioo->Release();
}

/**************************************************************************

   CContainer::setFocus()

**************************************************************************/

void CContainer::setFocus(BOOL fFocus)
{
    if (!m_punk)
        return;

    if (fFocus)
    {
        IOleObject *pioo;
        HRESULT hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
        if (FAILED(hr))
            return;

        pioo->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_hwnd, NULL);
        pioo->Release();
    }
}

/**************************************************************************

   CContainer::setStatusWindow()

**************************************************************************/

void CContainer::setStatusWindow(HWND hwndStatus)
{
    m_hwndStatus = hwndStatus;
}

/**************************************************************************

   CContainer::translateKey()

**************************************************************************/

void CContainer::translateKey(MSG msg)
{
    if (!m_punk)
        return;

    HRESULT                 hr;
    IOleInPlaceActiveObject *pao;

    hr = m_punk->QueryInterface(IID_IOleInPlaceActiveObject, (PVOID *)&pao);
    if (FAILED(hr))
        return;

    pao->TranslateAccelerator(&msg);
    pao->Release();
}

/**************************************************************************

   * CContainer::getDispatch()

**************************************************************************/

IDispatch * CContainer::getDispatch()
{
    if (!m_punk)
        return NULL;

    IDispatch *pdisp;
    m_punk->QueryInterface(IID_IDispatch, (PVOID *)&pdisp);
    return pdisp;
}

/**************************************************************************

   * CContainer::getUnknown()

**************************************************************************/

IUnknown * CContainer::getUnknown()
{
    if (!m_punk)
        return NULL;

    m_punk->AddRef();
    return m_punk;
}

void CContainer::setScrollbars(int scroll)
{
  m_scrollbars=scroll;
}

// ***********************************************************************
//  IDocHostUIHandler
// ***********************************************************************

HRESULT CContainer::ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
    return E_NOTIMPL;
}
        
HRESULT CContainer::GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo)
{
  pInfo->dwFlags=0x00200000|(m_scrollbars?0:8);
  return S_OK;
}
        
HRESULT CContainer::ShowUI(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc)
{
    return E_NOTIMPL;
}
        
HRESULT CContainer::HideUI(void)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::UpdateUI(void)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::OnDocWindowActivate(BOOL fActivate)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::OnFrameWindowActivate(BOOL fActivate)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::GetOptionKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::GetDropTarget(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
    *ppDispatch = winampExternal;
	return S_OK; //E_NOTIMPL;
}

        
HRESULT CContainer::TranslateUrl(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
    return E_NOTIMPL;
}

        
HRESULT CContainer::FilterDataObject(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
    return E_NOTIMPL;
}

