/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1998 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          evntsink.cpp

   Description:   This file contains the implementation of the event sink.

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include "main.h"
#include <windows.h>
#include "evntsink.h"

/**************************************************************************
   function prototypes
**************************************************************************/

/**************************************************************************
   global variables and definitions
**************************************************************************/

/**************************************************************************

   CEventSink::CEventSink()

**************************************************************************/

CEventSink::CEventSink()
{
    m_cRefs     = 1;
}

/**************************************************************************

   CEventSink::QueryInterface()

**************************************************************************/

STDMETHODIMP CEventSink::QueryInterface(REFIID riid, PVOID *ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    if (IsEqualIID(riid, IID_IDispatch))
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

/**************************************************************************

   CEventSink::AddRef()

**************************************************************************/

ULONG CEventSink::AddRef(void)
{
    return ++m_cRefs;
}

/**************************************************************************

   CEventSink::Release()

**************************************************************************/

ULONG CEventSink::Release(void)
{
    if (--m_cRefs)
        return m_cRefs;

    delete this;
    return 0;
}

/**************************************************************************

   CEventSink::GetIDsOfNames()

**************************************************************************/

HRESULT CEventSink::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
    *rgdispid = DISPID_UNKNOWN;
    return DISP_E_UNKNOWNNAME;
}

/**************************************************************************

   CEventSink::GetTypeInfo()

**************************************************************************/

HRESULT CEventSink::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CEventSink::GetTypeInfoCount()

**************************************************************************/

HRESULT CEventSink::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CEventSink::Invoke()

**************************************************************************/

void main_setStatusText(LPCWSTR txt)
{
  char dest[512];
  dest[0]=0;
  WideCharToMultiByte(CP_ACP,0,txt,-1,dest,sizeof(dest),NULL,NULL);
  //SetDlgItemText(m_hwnd,IDC_STATUS,dest);
}

void main_beforeNavigate(LPCWSTR txt)
{
  VARIANT *blah=(VARIANT *)txt;
  char dest[512];
  dest[0]=0;
  WideCharToMultiByte(CP_ACP,0,blah->bstrVal,-1,dest,sizeof(dest),NULL,NULL);
  //SetDlgItemText(m_hwnd,IDC_QUICKSEARCH,dest);
}

HRESULT CEventSink::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
    switch (dispid)
    {
        // void StatusTextChange([in] BSTR Text);
        case 0x66:
            main_setStatusText(pdispparams->rgvarg[0].bstrVal);
            //m_pApp->eventStatusTextChange(pdispparams->rgvarg[0].bstrVal);
            break;

        // void ProgressChange([in] long Progress, [in] long ProgressMax);
        case 0x6c:
            break;

        // void CommandStateChange([in] long Command, [in] VARIANT_BOOL Enable);
        case 0x69:
            //m_pApp->eventCommandStateChange(pdispparams->rgvarg[1].lVal, pdispparams->rgvarg[0].boolVal);
            break;

        // void DownloadBegin();
        case 0x6a:
            //m_pApp->eventDownloadBegin();
            break;

        // void DownloadComplete();
        case 0x68:
            //m_pApp->eventDownloadComplete();
            break;
            
        // void TitleChange([in] BSTR Text);
        case 0x071:            
            //m_pApp->eventTitleChange(pdispparams->rgvarg[0].bstrVal);
            break;

        // void PropertyChange([in] BSTR szProperty);
        case 0x70:
            //m_pApp->eventPropertyChange(pdispparams->rgvarg[0].bstrVal);
            break;

        // void BeforeNavigate2([in] IDispatch* pDisp, [in] VARIANT* URL, [in] VARIANT* Flags, [in] VARIANT* TargetFrameName, [in] VARIANT* PostData, [in] VARIANT* Headers, [in, out] VARIANT_BOOL* Cancel);
        case 0xfa:
          main_beforeNavigate(pdispparams->rgvarg[5].bstrVal);
            break;

        // void NewWindow2([in, out] IDispatch** ppDisp, [in, out] VARIANT_BOOL* Cancel);
        case 0xfb:
            break;
        
        // void NavigateComplete2([in] IDispatch* pDisp, [in] VARIANT* URL);
        case 0xfc:
            break;

        // void DocumentComplete([in] IDispatch* pDisp, [in] VARIANT* URL);
        case 0x0103:
            break;

        // void OnQuit();
        case 0xfd:
            break;

        // void OnVisible([in] VARIANT_BOOL Visible);
        case 0xfe:
            break;

        // void OnToolBar([in] VARIANT_BOOL ToolBar);
        case 0xff:
            break;

        // void OnMenuBar([in] VARIANT_BOOL MenuBar);
        case 0x0100:
            break;

        // void OnStatusBar([in] VARIANT_BOOL StatusBar);
        case 0x0101:
            break;
            
        // void OnFullScreen([in] VARIANT_BOOL FullScreen);
        case 0x0102:
            break;
            
        // void OnTheaterMode([in] VARIANT_BOOL TheaterMode);
        case 0x0104:
            break;
    }

    return S_OK;
}
