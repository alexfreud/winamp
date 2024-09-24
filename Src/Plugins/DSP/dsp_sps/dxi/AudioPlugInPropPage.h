#ifndef _PLUGIN_PROP_PAGE_H_
#define _PLUGIN_PROP_PAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../sps_common.h"

struct IMediaParams;
struct IMediaParamsUICallback;

/////////////////////////////////////////////////////////////////////////////
// CAudioPlugInPropPage dialog

class CAudioPlugInPropPage :
	public CUnknown,
	public IPropertyPage
{
// Construction
public:
	CAudioPlugInPropPage( IUnknown* pUnk, HRESULT* phr );
	virtual ~CAudioPlugInPropPage();

	// CUnknown
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

	// *** IPropertyPage methods ***
	STDMETHODIMP_(HRESULT)	SetPageSite(LPPROPERTYPAGESITE pPageSite);
	STDMETHODIMP_(HRESULT)	Activate(HWND hwndParent, LPCRECT prect, BOOL fModal);
	STDMETHODIMP_(HRESULT)	Deactivate(void);
	STDMETHODIMP_(HRESULT)	GetPageInfo(LPPROPPAGEINFO pPageInfo);
	STDMETHODIMP_(HRESULT)	SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk);
	STDMETHODIMP_(HRESULT)	Show(UINT nCmdShow);
	STDMETHODIMP_(HRESULT)	Move(LPCRECT prect);
	STDMETHODIMP_(HRESULT)	IsPageDirty(void);
	STDMETHODIMP_(HRESULT)	Apply(void);
	STDMETHODIMP_(HRESULT)	Help(LPCWSTR lpszHelpDir);
	STDMETHODIMP_(HRESULT)	TranslateAccelerator(LPMSG lpMsg);

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

// Implementation
protected:

	void UpdateControls();
	BOOL DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	static BOOL CALLBACK StaticDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:

	HWND							m_hWnd;
	BOOL							m_bDirty;
	IPropertyPageSite*		m_pPageSite;
	IMediaParams*				m_pMediaParams;
	IMediaParamsUICallback*	m_pUICallback;

  SPSEffectContext m_ctx;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _PLUGIN_PROP_PAGE_H_
