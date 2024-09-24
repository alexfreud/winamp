// AudioPlugInPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AudioPlugInPropPage.h"

#include <MedParam.h>
#include "CakeMedParam.h"
#include "Parameters.h"

/////////////////////////////////////////////////////////////////////////////
// CAudioPlugInPropPage property page

extern HMODULE g_hInst;

SPSEffectContext *g_fucko_ctx;

CAudioPlugInPropPage::CAudioPlugInPropPage( IUnknown* pUnk, HRESULT* phr ) :
	CUnknown( "AudioPlugInPropPage", pUnk ),
	m_hWnd( NULL ),
	m_pMediaParams( NULL ),
	m_pUICallback( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
    SPS_initcontext(&m_ctx); //FUCKO
    g_fucko_ctx=&m_ctx;
}

/////////////////////////////////////////////////////////////////////////////

CUnknown * WINAPI CAudioPlugInPropPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	return new CAudioPlugInPropPage( lpunk, phr );
}

/////////////////////////////////////////////////////////////////////////////

CAudioPlugInPropPage::~CAudioPlugInPropPage()
{
	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CUnknown

HRESULT CAudioPlugInPropPage::NonDelegatingQueryInterface( REFIID riid, void** ppv )
{    
	if (IID_IUnknown == riid)
		return GetInterface( (IUnknown*)this, ppv );
	else if (IID_IPropertyPage == riid)
		return GetInterface( (IPropertyPage*)this, ppv );
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage

HRESULT CAudioPlugInPropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "AudioPlugIn";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = ::CreateDialogParam( g_hInst, MAKEINTRESOURCE( IDD_DIALOG1 ), GetDesktopWindow(), (DLGPROC)StaticDialogProc, 0 );
	if (hWnd)
	{
		// Get the dialog size and destroy the window
		RECT rc;
		GetWindowRect( hWnd, &rc );
		pPageInfo->size.cx = rc.right - rc.left;
		pPageInfo->size.cy = rc.bottom - rc.top;
		DestroyWindow( hWnd );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	// Release old interfaces
	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;

	// Look for a new IFilter
	ULONG cObj = 0;
	for (ULONG i = 0; i < cObjects; ++i)
	{
		if (S_OK == ppUnk[i]->QueryInterface( IID_IMediaParams, (void**)&m_pMediaParams ))
		{
			ppUnk[i]->QueryInterface( IID_IMediaParamsUICallback, (void**)&m_pUICallback );
			break;
		}
	}

	// Update controls if we've got a new object and we're activated
	if (m_pMediaParams && ::IsWindow( m_hWnd ))
		UpdateControls();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

extern BOOL CALLBACK SPS_configWindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

BOOL CALLBACK CAudioPlugInPropPage::StaticDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
#if 0
	CAudioPlugInPropPage* pPage;

	if (WM_INITDIALOG == uMsg)
	{
		SetWindowLong( hwnd, DWL_USER, lParam );
		pPage = reinterpret_cast<CAudioPlugInPropPage*>(lParam);
		if (!pPage)
			return TRUE;
	}

	pPage = reinterpret_cast<CAudioPlugInPropPage*>(GetWindowLong( hwnd, DWL_USER ));
	if (!pPage)
		return TRUE;

	return pPage->DialogProc( hwnd, uMsg, wParam, lParam );
#endif
  return SPS_configWindowProc(hwnd, uMsg, wParam,lParam);
}

////////////////////////////////////////////////////////////////////////////////

BOOL CAudioPlugInPropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  //return SPS_configWindowProc(hwnd, uMsg, wParam,lParam);
  return 0;

/*	switch( uMsg )
	{
		case WM_INITDIALOG:
			m_hWnd = hwnd;
			break;

		default:
			return FALSE;
	}

	return TRUE;*/
}

////////////////////////////////////////////////////////////////////////////////

void CAudioPlugInPropPage::UpdateControls()
{
	// TODO: update all UI elements to reflect new control state
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
{
	if (!pRect)
		return E_POINTER;
	if (NULL != m_hWnd)
		return E_UNEXPECTED;	// already active!

	m_hWnd = CreateDialogParam( g_hInst, MAKEINTRESOURCE( IDD_DIALOG1 ), hwndParent, (DLGPROC)StaticDialogProc, (LPARAM)&m_ctx );
  if (!m_hWnd)
		return E_OUTOFMEMORY;

	// Refresh the property page controls
	UpdateControls();

	// Move page into position and show it
	Move( pRect );
	Show( SW_SHOWNORMAL );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( m_hWnd, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::Show( UINT nCmdShow )
{
	if (NULL == m_hWnd)
		E_UNEXPECTED;
	// Ignore wrong show flags
	if (nCmdShow != SW_SHOW && nCmdShow != SW_SHOWNORMAL && nCmdShow != SW_HIDE)
		return E_INVALIDARG;

	ShowWindow( m_hWnd, nCmdShow );

	if (SW_SHOWNORMAL == nCmdShow || SW_SHOW == nCmdShow)
	{
		// TODO: set the focus to which control needs it
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	DestroyWindow( m_hWnd );
	m_hWnd = NULL;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
{
	if (pPageSite)
	{
		if (m_pPageSite)
			return E_UNEXPECTED;
		m_pPageSite = pPageSite;
		m_pPageSite->AddRef();
	}
	else
	{
		if (m_pPageSite == NULL)
			return E_UNEXPECTED;
		m_pPageSite->Release();
		m_pPageSite = NULL;
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the filter. I.e. there is no Cancel.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::IsPageDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::Help( LPCWSTR lpszHelpDir )
{
	// Get location of DLL
	char szDLL[ _MAX_PATH ];
	if (0 == ::GetModuleFileName( g_hInst, szDLL, sizeof szDLL ))
		return E_FAIL;
	
	// Convert to location of .HLP file
	char szHelp[ _MAX_PATH ];
	::strncpy( szHelp, szDLL, ::strlen( szDLL ) - 3 );
	::strcat( szHelp, "HLP" );

	// Call help 
	if (::WinHelp( m_hWnd, szHelp, HELP_CONTENTS, NULL ))
		return S_OK;

	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugInPropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
