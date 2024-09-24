#include "./HTMLContainer2.h"
#include "../winamp/wa_ipc.h"
#include "CGlobalAtom.h"

#include <exdisp.h>
#include <mshtmdid.h>
#include <mshtml.h>
#include <exdispid.h>
#include <urlmon.h>
#include <Wininet.h>
#include <shlwapi.h>
#include <strsafe.h>

#ifndef GetWindowStyle
#define GetWindowStyle(__hwnd) ((UINT)GetWindowLongPtrW((__hwnd), GWL_STYLE))
#endif //GetWindowStyle

#ifndef GetWindowStyleEx
#define GetWindowStyleEx(__hwnd) ((UINT)GetWindowLongPtrW((__hwnd), GWL_EXSTYLE))
#endif // GetWindowStyleEx

static UINT WM_REDIRECTNAVIGATE	= 0;

#ifndef DISPID_NEWWINDOW3
#define DISPID_NEWWINDOW3 273 
#endif 

static CGlobalAtom WNDPROP_SCCTRLW(L"SCCTRL");

static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;

#define REGISTRY_FEATURE_CONTROL		L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl"
#define REGISTRY_FEATURE_USE_LEGACY_JSCRIPT (REGISTRY_FEATURE_CONTROL L"\\FEATURE_USE_LEGACY_JSCRIPT")

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#ifndef CSTR_INVARIANT
#define CSTR_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)
#endif //CSTR_INVARIANT

#define IS_EQUALCLASS(__class1, __class2)\
	(CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, __class1, -1, __class2, -1))

#define DEFAULT_HOSTBKCOLOR		0x00FFFFFF
#define DEFAULT_DOCHOSTUIFLAGS	(DOCHOSTUIFLAG_NO3DOUTERBORDER |			\
								DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION |	\
								DOCHOSTUIFLAG_NO3DBORDER |					\
								DOCHOSTUIDBLCLK_DEFAULT |					\
								0)

#define DEFAULT_DOWNLOADFLAGS	(DLCTL_DLIMAGES |							\
								DLCTL_VIDEOS |								\
								DLCTL_PRAGMA_NO_CACHE |						\
								0)

#define CF_DISABLEBEFORENAVFILTER	0x00000002

typedef struct __RMNAVIGATE2
{
	VARIANT URL;
	VARIANT Flags;
	VARIANT TargetFrameName;
	VARIANT PostData;
	VARIANT Headers;
}RMNAVIGATE2;

typedef struct __RMNAVIGATETONAME
{
	BSTR url;
	UINT flags;
} RMNAVIGATETONAME;

typedef struct _IECURSOR
{
	INT			nSysId;
	HCURSOR		hSys;
	HCURSOR		hUser;
} IECURSOR;

static INT bIsVistaOrHigher = -1;

typedef struct __LIBHDR LIBHDR;

typedef BOOL (WINAPI *LIBRARYLOADER)(LIBHDR* /*header*/);

typedef struct __LIBHDR
{
	HMODULE hMod;
	LPCWSTR name;
	LIBRARYLOADER loader;
	size_t	size;
} LIBHDR;

typedef struct __LIBWININET
{
	LIBHDR hdr;

	typedef BOOL (WINAPI *INTERNETCRACKURL)(LPCWSTR /*lpszUrl*/, DWORD /*dwUrlLength*/, DWORD /*dwFlags*/, URL_COMPONENTSW* /*lpUrlComponents*/);
	
	INTERNETCRACKURL InternetCrackUrl;

	static BOOL CALLBACK Loader(LIBHDR *pHdr)
	{
		LIBWININET *lib = (LIBWININET*)pHdr;
		lib->InternetCrackUrl = (LIBWININET::INTERNETCRACKURL)GetProcAddress(pHdr->hMod, "InternetCrackUrlW");
		return TRUE;
	}
} LIBWININET;

typedef struct __LIBURLMON
{	
	LIBHDR hdr;

	typedef HRESULT (WINAPI *COINTERNETSETFEATUREENABLED)(INTERNETFEATURELIST /*FeatureEntry*/, DWORD /*dwFlags*/, BOOL /*fEnable*/);
	typedef HRESULT (WINAPI *COINTERNETISFEATUREENABLED)(INTERNETFEATURELIST /*FeatureEntry*/, DWORD /*dwFlags*/);

	COINTERNETSETFEATUREENABLED CoInternetSetFeatureEnabled;
	COINTERNETISFEATUREENABLED CoInternetIsFeatureEnabled;
	
	static BOOL CALLBACK Loader(LIBHDR *pHdr)
	{
		LIBURLMON *lib = (LIBURLMON*)pHdr;
		lib->CoInternetSetFeatureEnabled = (LIBURLMON::COINTERNETSETFEATUREENABLED)GetProcAddress(pHdr->hMod, "CoInternetSetFeatureEnabled");
		lib->CoInternetIsFeatureEnabled = (LIBURLMON::COINTERNETISFEATUREENABLED)GetProcAddress(pHdr->hMod, "CoInternetIsFeatureEnabled");
		return TRUE;
	}
} LIBURLMON;

static LIBWININET libWininet = { { NULL, L"WinInet.dll", LIBWININET::Loader, sizeof(LIBWININET)}, NULL, };
static LIBURLMON libUrlmon = { { NULL, L"UrlMon.dll", LIBURLMON::Loader, sizeof(LIBURLMON)}, NULL, };
static LIBHDR* szLibrary[] = { (LIBHDR*)&libWininet, (LIBHDR*)&libUrlmon };

BOOL HTMLContainer2_Initialize()
{	
	for(size_t i = 0; i < ARRAYSIZE(szLibrary); i++)
	{
		if (NULL == szLibrary[i]->hMod)
		{
			size_t size = szLibrary[i]->size - sizeof(LIBHDR);
			ZeroMemory((BYTE*)szLibrary[i] + sizeof(LIBHDR), size);
		}
	}
	return TRUE;
}

BOOL HTMLContainer2_Uninitialize()
{
	for(size_t i = 0; i < ARRAYSIZE(szLibrary); i++)
	{
		if (NULL != szLibrary[i]->hMod)
		{
			FreeLibrary(szLibrary[i]->hMod);
			szLibrary[i]->hMod = NULL;
		}
		size_t size = szLibrary[i]->size - sizeof(LIBHDR);
		ZeroMemory((BYTE*)szLibrary[i] + sizeof(LIBHDR), size);
	}
	return TRUE;
}

BOOL HTMLContainer2_LoadLibray(LIBHDR *libraryHeader)
{
	if (NULL == libraryHeader)
		return FALSE;

	if (NULL != libraryHeader->hMod)
		return TRUE;

	libraryHeader->hMod = LoadLibraryW(libraryHeader->name);
	if (NULL == libraryHeader->hMod)
		return FALSE;

	return (NULL != libraryHeader->loader) ? 
			libraryHeader->loader(libraryHeader) :
			TRUE;
}

static BOOL SublassControl_IsAttached(HWND hwnd);
static BOOL SubclassControl_Attach(HWND hControl, HTMLContainer2 *pContainer);

// uncomment if you ever want to use mozilla instead of IE
// change the CLSID_WebBrowser in the constructor below to CLSID_MozillaBrowser
// but window.external from javascript doesn't work :(

static const CLSID CLSID_MozillaBrowser=
    { 0x1339B54C, 0x3453, 0x11D2, { 0x93, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

HTMLContainer2::HTMLContainer2(HWND waWindow, HWND hwndParent) :
							   ref(1), pUnk(NULL), hParent(hwndParent), dwCookie(0),
							   dwFlags(0), fnBrwoserCB(NULL) ,userData(NULL),
							   bNavigating(FALSE), hCursors(NULL), nCursors(0),
							   ensureChakraLoaded(TRUE), winampWindow(waWindow)
{
	if (NULL == hParent || FALSE == GetClientRect(hParent, &rect))
		SetRectEmpty(&rect);

	if (-1 == bIsVistaOrHigher)
	{
		OSVERSIONINFO os;
		os.dwOSVersionInfoSize  = sizeof(OSVERSIONINFO);
		bIsVistaOrHigher = (GetVersionEx(&os) && os.dwMajorVersion >= 6);
	}
}

HTMLContainer2::~HTMLContainer2(void)
{	
	if (NULL != fnBrwoserCB)
		fnBrwoserCB(this, DISPID_DESTRUCTOR, NULL, userData);

	if (hCursors)
	{
		HCURSOR hc;
		for(int i = 0; i < nCursors; i++)
		{
			hc = ((IECURSOR*)hCursors)[i].hUser;
			if (hc) DestroyCursor(hc);
		}
		free(hCursors);
	}
}

STDMETHODIMP HTMLContainer2::Initialize(void)
{
	IOleObject *pioo = NULL;
	IConnectionPoint *pcp = NULL;
	IConnectionPointContainer *pcpc = NULL;

    HRESULT hr = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC, IID_IUnknown, (PVOID*)&pUnk);
	if (FAILED(hr)) 
		return hr;

	hr = pUnk->QueryInterface(IID_IOleObject, (PVOID*)&pioo);
	if (SUCCEEDED(hr))
	{
		DWORD dwFlags = 0;
		if (FAILED(pioo->GetMiscStatus(DVASPECT_CONTENT, &dwFlags)))
			dwFlags = 0;

		if (0 != (OLEMISC_SETCLIENTSITEFIRST & dwFlags))
		{
			IOleClientSite *pClientSite = NULL;
			hr = QueryInterface(IID_IOleClientSite, (void**)&pClientSite);
			if (SUCCEEDED(hr))
			{
				pioo->SetClientSite(pClientSite);
				pioo->SetHostNames(L"NullsoftHtmlContainer2", NULL);

				if(NULL == hParent || FALSE == GetClientRect(hParent, &rect))
					SetRectEmpty(&rect);

				pioo->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, pClientSite, 0, hParent, &rect);

				pClientSite->Release();
			}
		}
		pioo->Release();	

		HWND hHost = GetHostHWND();
		if (IsWindow(hHost)) 
		{
			SubclassControl_Attach(hHost, this);
		}
	}

	if (FAILED(hr)) return hr;

	hr = pUnk->QueryInterface(IID_IConnectionPointContainer, (PVOID*)&pcpc);
	if (SUCCEEDED (hr))
	{
		hr = pcpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &pcp);
		if (SUCCEEDED(hr))
		{
			hr = pcp->Advise(static_cast<IDispatch*>(this), &dwCookie);
			if (FAILED(hr)) dwCookie = 0;
			pcp->Release();
		}
		pcpc->Release();
	}

	return hr;
}

STDMETHODIMP HTMLContainer2::Finish(void)
{
	if (!pUnk) return S_OK;

	AddRef();

	UnadviseBrowserEvents();

	IOleObject *pioo = NULL;
    IOleInPlaceObject *pipo = NULL;
	HRESULT hr = pUnk->QueryInterface(IID_IOleInPlaceObject, (PVOID*)&pipo);
	if (SUCCEEDED(hr))
	{
		pipo->InPlaceDeactivate();
		pipo->UIDeactivate();
		pipo->Release();
	}

	hr = pUnk->QueryInterface(IID_IOleObject, (PVOID*)&pioo);
	if (SUCCEEDED(hr))
	{
		pioo->Close(OLECLOSE_NOSAVE);
		pioo->SetClientSite(NULL);
		pioo->Release();
	}

	ULONG r = pUnk->Release();
	pUnk = NULL;

	Release();

	return hr;
}

STDMETHODIMP HTMLContainer2::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	if (IsEqualIID(riid, IID_IOleClientSite))
		*ppvObject = (IOleClientSite*)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceSite))
		*ppvObject = (IOleInPlaceSite*)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
		*ppvObject = (IOleInPlaceFrame*)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
		*ppvObject = (IOleInPlaceUIWindow*)this;
	else if (IsEqualIID(riid, IID_IOleControlSite))
		*ppvObject = (IOleControlSite*)this;
	else if (IsEqualIID(riid, IID_IOleWindow))
		*ppvObject = reinterpret_cast<IOleWindow*>(this);
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = reinterpret_cast<IUnknown*>(this);
	else if (IsEqualIID(riid, IID_IDocHostUIHandler))
		*ppvObject = (IDocHostUIHandler*)this;
	else if (IsEqualIID(riid, IID_IDocHostUIHandler2))
		*ppvObject = (IDocHostUIHandler2*)this;
	else if (IsEqualIID(riid, IID_IDocHostShowUI))
		*ppvObject = (IDocHostShowUI*)this;
	else if (IsEqualIID(riid, IID_IOleCommandTarget))
		*ppvObject = (IOleCommandTarget*)this;
	else if (IsEqualIID(riid, IID_IServiceProvider))
		*ppvObject = (IServiceProvider*)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG HTMLContainer2::AddRef(void)
{
	return InterlockedIncrement(&ref);
}

ULONG HTMLContainer2::Release(void)
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement(&ref);
	if (0 == r)
		delete(this);

	return r;
}

STDMETHODIMP HTMLContainer2::SaveObject()
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppMk)
{
	*ppMk = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetContainer(LPOLECONTAINER * ppContainer)
{
	*ppContainer = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP HTMLContainer2::ShowObject()
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::OnShowWindow(BOOL fShow)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetWindow(HWND * lphwnd)
{
	if (NULL != hParent && IsWindow(hParent))
	{
		*lphwnd = hParent;
		return S_OK;
	}
	else
	{
		*lphwnd = NULL;
		return E_FAIL;
	}
}

STDMETHODIMP HTMLContainer2::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::CanInPlaceActivate(void)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::OnInPlaceActivate(void)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::OnUIActivate(void)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::GetWindowContext(IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppIIPUIWin,
											  LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	if (FAILED(QueryInterface(IID_IOleInPlaceFrame, (void**)ppFrame))) 
		*ppFrame = NULL;

	*ppIIPUIWin = NULL;

	CopyRect(lprcPosRect, &rect);
	CopyRect(lprcClipRect, &rect);

	lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = hParent;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

STDMETHODIMP HTMLContainer2::Scroll(SIZE scrollExtent)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::OnUIDeactivate(BOOL fUndoable)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::OnInPlaceDeactivate(void)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::DiscardUndoState(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::OnPosRectChange(LPCRECT lprcPosRect)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::RemoveMenus(HMENU hmenuShared)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::SetStatusText(LPCOLESTR pszStatusText)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::OnControlInfoChanged()
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::LockInPlaceActive(BOOL fLock)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetExtendedControl(IDispatch **ppDisp)
{
	if (ppDisp == NULL)
		return E_INVALIDARG;

	*ppDisp = (IDispatch *)this;
	(*ppDisp)->AddRef();

	return S_OK;
}

STDMETHODIMP HTMLContainer2::TransformCoords(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
	return S_FALSE;
}

STDMETHODIMP HTMLContainer2::OnFocus(BOOL fGotFocus)
{
	return S_OK;
}

STDMETHODIMP HTMLContainer2::ShowPropertyFrame(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	*rgdispid = DISPID_UNKNOWN;
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP HTMLContainer2::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

static HRESULT HTMLContainer2_OnShowUiElementHelper(HTMLContainer2 *instance, UINT elementId, DISPPARAMS *pDispParams)
{
	if (NULL == pDispParams) return E_UNEXPECTED;
	if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
	if (VT_BOOL != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;

	instance->OnShowUiElement(elementId, pDispParams->rgvarg[0].boolVal);
	return S_OK;
}

STDMETHODIMP HTMLContainer2::Invoke(DISPID dispId, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	if (fnBrwoserCB && fnBrwoserCB(this, dispId, pDispParams, userData)) 
	{
		return DISP_E_MEMBERNOTFOUND;
	}

	switch (dispId)
	{
		case DISPID_BEFORENAVIGATE2:
			if(7 == pDispParams->cArgs) 
			{
				OnBeforeNavigate(pDispParams->rgvarg[6].pdispVal, pDispParams->rgvarg[5].pvarVal,
								pDispParams->rgvarg[4].pvarVal, pDispParams->rgvarg[3].pvarVal,
								pDispParams->rgvarg[2].pvarVal, pDispParams->rgvarg[1].pvarVal,
								pDispParams->rgvarg[0].pboolVal);
				return S_OK;
			}
			break;

		case DISPID_NAVIGATEERROR:
			if (200 == pDispParams->rgvarg[1].pvarVal->lVal)  
			{
				*pDispParams->rgvarg[0].pboolVal = VARIANT_TRUE;
			}
			if (5 == pDispParams->cArgs) 
			{
				OnNavigateError(pDispParams->rgvarg[4].pdispVal, pDispParams->rgvarg[3].pvarVal,
								pDispParams->rgvarg[2].pvarVal, pDispParams->rgvarg[1].pvarVal,
								pDispParams->rgvarg[0].pboolVal);
				return S_OK;
			}
			break;

		case DISPID_NAVIGATECOMPLETE2:
			if (2 == pDispParams->cArgs)
			{
				OnNavigateComplete(pDispParams->rgvarg[1].pdispVal, pDispParams->rgvarg[0].pvarVal);
				return S_OK;
			}
			break;

		case DISPID_DOCUMENTCOMPLETE:
			if (2 == pDispParams->cArgs) 
			{
				OnDocumentComplete(pDispParams->rgvarg[1].pdispVal, pDispParams->rgvarg[0].pvarVal);
			}

			if (NULL != pUnk)
			{
				IWebBrowser2 *pWeb1 = NULL, *pWeb2 = NULL;

				if (FAILED(GetIWebBrowser2(&pWeb1)))
					pWeb1 = NULL;

				if (pDispParams->cArgs < 2 || 
					NULL == pDispParams->rgvarg[1].pdispVal ||
					FAILED(pDispParams->rgvarg[1].pdispVal->QueryInterface(IID_IWebBrowser2, (void**)&pWeb2)))
				{
					pWeb2 = NULL;
				}

				if (NULL != pWeb1) pWeb1->Release();
				if (NULL != pWeb2) pWeb2->Release();

				if (NULL != pWeb1 && pWeb1 == pWeb2) 
					OnDocumentReady(pDispParams->rgvarg[1].pdispVal, pDispParams->rgvarg[0].pvarVal);
			}
			return S_OK;

		case DISPID_DOWNLOADBEGIN:
			OnDownloadBegin();
			return S_OK;

		case DISPID_DOWNLOADCOMPLETE:
			OnDownloadComplete();
			return S_OK;

		case DISPID_FILEDOWNLOAD:
			if (2 == pDispParams->cArgs)
			{
				OnFileDownload(pDispParams->rgvarg[1].pboolVal, 	pDispParams->rgvarg[0].pboolVal);
				return S_OK;
			}
			break;

		case DISPID_NEWWINDOW2:
			if (2 == pDispParams->cArgs)
			{
				OnNewWindow2(pDispParams->rgvarg[1].ppdispVal, pDispParams->rgvarg[0].pboolVal);
				return S_OK;
			}
			break;

		case DISPID_NEWWINDOW3:
			if (5 != pDispParams->cArgs) 
				return DISP_E_BADPARAMCOUNT;

			if ((VT_DISPATCH | VT_BYREF) != pDispParams->rgvarg[4].vt || 
				(VT_BOOL | VT_BYREF) != pDispParams->rgvarg[3].vt || 
				VT_I4 != pDispParams->rgvarg[2].vt || 
				VT_BSTR != pDispParams->rgvarg[1].vt || 
				VT_BSTR != pDispParams->rgvarg[0].vt)
			{
				return DISP_E_BADVARTYPE;
			}

			OnNewWindow3(pDispParams->rgvarg[4].ppdispVal, pDispParams->rgvarg[3].pboolVal, 
						pDispParams->rgvarg[2].intVal, pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].bstrVal);
			return S_OK;

		case DISPID_PROGRESSCHANGE:
			if (2 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_I4 != pDispParams->rgvarg[1].vt || 
				VT_I4 != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnProgressChange(pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].lVal);
			return S_OK;

		case DISPID_STATUSTEXTCHANGE:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_BSTR != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnStatusTextChange(pDispParams->rgvarg[0].bstrVal);
			return S_OK;

		case DISPID_AMBIENT_USERAGENT:
			pvarResult->vt = VT_BSTR;
			pvarResult->bstrVal = SysAllocString(OnGetUserAgent());
			if (NULL != pvarResult->bstrVal) 
				return S_OK;
			break;

		case DISPID_AMBIENT_DLCONTROL:
			pvarResult->vt = VT_I4;
			pvarResult->lVal = OnGetDownlodFlags();
			return S_OK;

		case DISPID_COMMANDSTATECHANGE:
			if (2 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_I4 != pDispParams->rgvarg[1].vt ||
				VT_BOOL != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnCommandStateChange(pDispParams->rgvarg[1].lVal, pDispParams->rgvarg[0].boolVal);
			return S_OK;

		case DISPID_SETSECURELOCKICON:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_I4 != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnSetSecureLockIcon(pDispParams->rgvarg[0].lVal);
			return S_OK;

		case DISPID_TITLECHANGE:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_BSTR != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnTitleChange(pDispParams->rgvarg[0].bstrVal);
			return S_OK;

		case DISPID_ONVISIBLE:
			if(1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_BOOL != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnVisibleChange(pDispParams->rgvarg[0].boolVal);
			return S_OK;

		case DISPID_WINDOWCLOSING:
			if (2 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_BOOL != pDispParams->rgvarg[1].vt ||
				(VT_BOOL|VT_BYREF) != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnWindowClosing(pDispParams->rgvarg[1].boolVal, pDispParams->rgvarg[0].pboolVal);
			return S_OK;

		case DISPID_ONTOOLBAR:			return HTMLContainer2_OnShowUiElementHelper(this, uiToolbar, pDispParams);
		case DISPID_ONMENUBAR:			return HTMLContainer2_OnShowUiElementHelper(this, uiMenubar, pDispParams);
		case DISPID_ONSTATUSBAR:		return HTMLContainer2_OnShowUiElementHelper(this, uiStatusbar, pDispParams);
		case DISPID_ONADDRESSBAR:		return HTMLContainer2_OnShowUiElementHelper(this, uiAddressbar, pDispParams);

		case DISPID_ONFULLSCREEN:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_BOOL != pDispParams->rgvarg[0].vt) DISP_E_BADVARTYPE;
			OnEnableFullscreen(pDispParams->rgvarg[0].boolVal);
			return S_OK;

		case DISPID_WINDOWSETRESIZABLE:	
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_BOOL != pDispParams->rgvarg[0].vt) DISP_E_BADVARTYPE;
			OnWindowSetResizable(pDispParams->rgvarg[0].boolVal);
			return S_OK;

		case DISPID_CLIENTTOHOSTWINDOW:
			if (2 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if ((VT_BYREF|VT_I4) != pDispParams->rgvarg[1].vt ||
				(VT_BYREF|VT_I4) != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnClientToHostWindow(pDispParams->rgvarg[1].plVal, pDispParams->rgvarg[0].plVal);
			return S_OK;

		case DISPID_WINDOWSETLEFT:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_I4 != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnSetWindowPos(wndLeft, pDispParams->rgvarg[0].lVal, 0, 0, 0);
			return S_OK;

		case DISPID_WINDOWSETTOP:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_I4 != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnSetWindowPos(wndTop, 0, pDispParams->rgvarg[0].lVal, 0, 0);
			return S_OK;

		case DISPID_WINDOWSETWIDTH:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_I4 != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnSetWindowPos(wndWidth, 0, 0, pDispParams->rgvarg[0].lVal, 0);
			return S_OK;

		case DISPID_WINDOWSETHEIGHT:
			if (1 != pDispParams->cArgs) return DISP_E_BADPARAMCOUNT;
			if (VT_I4 != pDispParams->rgvarg[0].vt) return DISP_E_BADVARTYPE;
			OnSetWindowPos(wndHeight, 0, 0, 0, pDispParams->rgvarg[0].lVal);
			return S_OK;
	}

	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP HTMLContainer2::SetLocation(int x, int y, int width, int height)
{
	if (!pUnk) return E_NOINTERFACE;

	SetRect(&rect, x, y, x + width, y + height);

	IOleInPlaceObject *pipo = NULL;
	HRESULT hr = pUnk->QueryInterface(IID_IOleInPlaceObject, (PVOID*)&pipo);
	if (SUCCEEDED(hr))
	{			
		hr = pipo->SetObjectRects(&rect, &rect);
		pipo->Release();
	}
	return hr;
}

STDMETHODIMP HTMLContainer2::GetBorder(LPRECT lprectBorder)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::SetActiveObject(IOleInPlaceActiveObject * pActiveObject, LPCOLESTR lpszObjName)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::SetFocus(BOOL fFocused)
{	
	if (NULL == pUnk) 
		return E_NOINTERFACE;

	if (FALSE != fFocused)
	{
		IOleObject *pioo = NULL;
		if (SUCCEEDED(pUnk->QueryInterface(IID_IOleObject, (PVOID*)&pioo)))
		{
			pioo->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, hParent, &rect);
			pioo->Release();
		}
	}
	else
	{
		IOleInPlaceObject *pipo = NULL;
		if (SUCCEEDED(pUnk->QueryInterface(IID_IOleInPlaceObject, (void**)&pipo)))
		{
			pipo->UIDeactivate();
			pipo->Release();
		}
	}

	return S_OK;
}

BOOL HTMLContainer2::TranslateKey(LPMSG pMsg)
{
	if (!pUnk) return FALSE;

	IOleInPlaceActiveObject *pao = NULL;
	HRESULT hr = pUnk->QueryInterface(IID_IOleInPlaceActiveObject, (PVOID*)&pao);
	if (SUCCEEDED(hr))
	{
		hr = pao->TranslateAccelerator(pMsg);
		pao->Release();
	}

	return (S_OK == hr);
}

STDMETHODIMP HTMLContainer2::ShowHelp(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit)
{
	return S_FALSE;
}

STDMETHODIMP HTMLContainer2::ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult)
{
	return S_FALSE;
}

// ***********************************************************************
//  IDocHostUIHandler
// ***********************************************************************

STDMETHODIMP HTMLContainer2::ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
	return S_FALSE;
}

STDMETHODIMP HTMLContainer2::GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo)
{
	pInfo->cbSize = sizeof(DOCHOSTUIINFO);
	pInfo->dwFlags = OnGetHostInfoFlags();

	pInfo->pchHostCss = OnGetHostCSS();
	pInfo->pchHostNS = OnGetHostNamespace();

	return S_OK;
}

STDMETHODIMP HTMLContainer2::ShowUI(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc)
{
	return S_FALSE;
}

STDMETHODIMP HTMLContainer2::HideUI(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::UpdateUI(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::OnDocWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::OnFrameWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
	if ((0x8000 & GetAsyncKeyState(VK_MENU)) || (0x8000 & GetAsyncKeyState(VK_CONTROL)))
	{
		if (NULL != hParent && IsWindow(hParent) && 
			PostMessageW(hParent, lpMsg->message, lpMsg->wParam, lpMsg->lParam))
		{
			return S_OK;
		}
	}	
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetOptionKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
	*pchKey = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetDropTarget(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	*ppDispatch = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::TranslateUrl(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	*ppchURLOut = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP HTMLContainer2::FilterDataObject(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
	*ppDORet = NULL;
	return S_FALSE;
}

STDMETHODIMP HTMLContainer2::GetOverrideKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
	*pchKey = NULL;
	return S_FALSE;
}

STDMETHODIMP HTMLContainer2::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD *prgCmds, OLECMDTEXT *pCmdText)
{
	return OLECMDERR_E_NOTSUPPORTED;
}

STDMETHODIMP HTMLContainer2::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{	
	return OLECMDERR_E_UNKNOWNGROUP;
}

STDMETHODIMP HTMLContainer2::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
	return E_NOINTERFACE;
}

HRESULT HTMLContainer2::GetIUnknown(IUnknown **ppUnk)
{
	if (!pUnk)
	{
		*ppUnk = NULL;
		return E_NOINTERFACE;
	}
	pUnk->AddRef();
	*ppUnk = pUnk;
	return S_OK;
}

HRESULT HTMLContainer2::GetIDispatch(IDispatch **ppDisp)
{
	*ppDisp = NULL;
	return (pUnk) ? pUnk->QueryInterface(IID_IDispatch, (PVOID*)ppDisp) : E_NOINTERFACE;
}

HRESULT HTMLContainer2::GetIWebBrowser2(IWebBrowser2 **ppWeb2)
{
	*ppWeb2 = NULL;
	return (pUnk) ? pUnk->QueryInterface(IID_IWebBrowser2, (PVOID*)ppWeb2) : E_NOINTERFACE;
}

HWND HTMLContainer2::GetHostHWND(void)
{	
	if (NULL == pUnk) return NULL;

	HWND hwndHost = NULL;
	IOleInPlaceObject *pipo = NULL;
	HRESULT hr = pUnk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
	if (SUCCEEDED(hr))
	{			
		hr = pipo->GetWindow(&hwndHost);
		if (FAILED(hr)) hwndHost = NULL;
		pipo->Release();
	}

	return hwndHost;
}

HWND HTMLContainer2::GetParentHWND(void)
{
	return hParent;
}

STDMETHODIMP HTMLContainer2::UnadviseBrowserEvents()
{
	if (0 == dwCookie)
		return S_OK;

	IConnectionPoint *pcp = NULL;
	IConnectionPointContainer *pcpc = NULL;
	HRESULT hr = pUnk->QueryInterface(IID_IConnectionPointContainer, (PVOID*)&pcpc);
	if (SUCCEEDED (hr))
	{
		hr = pcpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &pcp);
		if (SUCCEEDED(hr))
		{
			hr = pcp->Unadvise(dwCookie);
			if (SUCCEEDED(hr))
			{
				dwCookie = 0;
			}
			pcp->Release();
		}
		pcpc->Release();
	}
	return hr;
}

HRESULT HTMLContainer2::PostRedirectMessage(UINT messageId, LPARAM param)
{
	HWND hHost = GetHostHWND();
	if (NULL == hHost) return E_HANDLE;
	if (FALSE == SublassControl_IsAttached(hHost)) return E_NOTIMPL;

	if (0 == WM_REDIRECTNAVIGATE)
	{
		WM_REDIRECTNAVIGATE = RegisterWindowMessageW(L"htmlContainterRedirectNavigate");
		if (0 == WM_REDIRECTNAVIGATE) return E_UNEXPECTED;
	}

	if (FALSE == PostMessageW(hHost, WM_REDIRECTNAVIGATE, (WPARAM)messageId, param))
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	return S_OK;
}

HRESULT HTMLContainer2::NavigateEx(IWebBrowser2 *pWeb2, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers)
{
	HRESULT	hr;
	if (NULL == pWeb2) 
		return E_NOINTERFACE;

	pWeb2->AddRef();

	VARIANT vtHeader;
	BOOL performNavigate = TRUE;

	VariantInit(&vtHeader);

	LPCWSTR pszUserAgent = OnGetUserAgent();
	if (!Headers && NULL != pszUserAgent && L'\0' != *pszUserAgent) 
	{			
		wchar_t szBuffer[256] = {0};
		if (SUCCEEDED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"User-Agent: %s", pszUserAgent)))
		{
			V_VT(&vtHeader) = VT_BSTR;
			vtHeader.bstrVal = SysAllocString(szBuffer);
			Headers = &vtHeader;
		}
	}

	if (FALSE != ensureChakraLoaded)
	{
		if (NULL == GetModuleHandleW(L"mshtml.dll"))
		{
			HKEY hKey = NULL;
			unsigned long disposition = 0;
			long createResult = 0;
			wchar_t processName[2*MAX_PATH] = {0};
			HANDLE lock = CreateMutexW(NULL, FALSE, L"HTMLContainer2::ChakraEnabler");
			if (NULL != lock)
				WaitForSingleObject(lock, INFINITE);

			createResult = RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_FEATURE_USE_LEGACY_JSCRIPT,
										   NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE,
										   NULL, &hKey, &disposition);

			if (ERROR_SUCCESS == createResult)
			{	
				unsigned long featureEnabled = 0;
				if (0 != GetModuleFileNameW(NULL, processName, ARRAYSIZE(processName)))
				{
					featureEnabled = 0;
					PathStripPathW(processName);

					RegSetValueExW(hKey, processName, 0, REG_DWORD, (const BYTE*)&featureEnabled, sizeof(unsigned long));

					hr = pWeb2->Navigate2(URL, Flags, TargetFrameName, PostData, Headers);
					performNavigate = FALSE;

					RegDeleteValueW(hKey, processName);
				}

				RegCloseKey(hKey);
			}

			if (NULL != lock)
			{
				ReleaseMutex(lock);
				CloseHandle(lock);
			}
		}
		ensureChakraLoaded = FALSE;
	}

	if (FALSE != performNavigate)
		hr = pWeb2->Navigate2(URL, Flags, TargetFrameName, PostData, Headers);

	VariantClear(&vtHeader);
	pWeb2->Release();
	return hr;
}

HRESULT HTMLContainer2::Navigate2(VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers)
{
	if (NULL == pUnk) 
		return E_NOINTERFACE;

	IWebBrowser2 *pWeb2 = NULL;
	HRESULT hr = pUnk->QueryInterface(IID_IWebBrowser2, (LPVOID*)&pWeb2);

	if(SUCCEEDED(hr))
	{
		hr = NavigateEx(pWeb2, URL, Flags, TargetFrameName, PostData, Headers);
		pWeb2->Release();
	}
	return hr;
}

HRESULT HTMLContainer2::PostNavigate2(VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers)
{		
	RMNAVIGATE2 *param = (RMNAVIGATE2*)calloc(1, sizeof(RMNAVIGATE2));
	if (NULL == param) return E_OUTOFMEMORY;

	VariantInit(&param->URL);
	VariantInit(&param->Flags);
	VariantInit(&param->TargetFrameName);
	VariantInit(&param->PostData);
	VariantInit(&param->Headers);

	if (NULL != URL) VariantCopyInd(&param->URL, URL);
	if (NULL != Flags) VariantCopyInd(&param->Flags, Flags);
	if (NULL != TargetFrameName) VariantCopyInd(&param->TargetFrameName, TargetFrameName);
	if (NULL != PostData) VariantCopyInd(&param->PostData, PostData);
	if (NULL != Headers) VariantCopyInd(&param->Headers, Headers);

	HRESULT hr = PostRedirectMessage(HTMLContainer2::msgNavigate2, (LPARAM)param);
	if (FAILED(hr)) 
	{
		VariantClear(&param->URL);
		VariantClear(&param->Flags);
		VariantClear(&param->TargetFrameName);
		VariantClear(&param->PostData);
		VariantClear(&param->Headers);
		free(param);
	}
	return hr;
}

HRESULT HTMLContainer2::NavigateToNameEx(IWebBrowser2 *pWeb2, LPCWSTR pszUrl, UINT fFlags)
{
	VARIANT	Flags;
	VARIANT	URL;

	if (NULL == pszUrl || L'\0' == *pszUrl)
	{
		pszUrl = L"about:blank";
	}

	VariantInit(&Flags);
	V_VT(&Flags) = VT_I4;
	V_I4(&Flags) = fFlags;

	VariantInit(&URL);
	V_VT(&URL) = VT_BSTR;
	URL.bstrVal = SysAllocString(pszUrl);

	HRESULT hr = (NULL != URL.bstrVal) ? NavigateEx(pWeb2, &URL, &Flags, NULL, NULL, NULL) : E_FAIL;

	VariantClear(&URL);
	VariantClear(&Flags);
	return hr;
}

HRESULT HTMLContainer2::NavigateToName(LPCWSTR pszUrl, UINT fFlags)
{
	if (NULL == pUnk) 
		return E_NOINTERFACE;

	IWebBrowser2 *pWeb2 = NULL;
	HRESULT	hr = pUnk->QueryInterface(IID_IWebBrowser2, (LPVOID*)&pWeb2);

	if(SUCCEEDED(hr))
	{
		hr = NavigateToNameEx(pWeb2, pszUrl, fFlags);
		pWeb2->Release();
	}
	return hr;
}

HRESULT HTMLContainer2::PostNavigateToName(LPCWSTR pszUrl, UINT fFlags)
{
	RMNAVIGATETONAME *param = (RMNAVIGATETONAME*)calloc(1, sizeof(RMNAVIGATETONAME));
	if (NULL == param) return E_OUTOFMEMORY;

	param->url = SysAllocString(pszUrl);
	param->flags = fFlags;

	HRESULT hr = PostRedirectMessage(HTMLContainer2::msgNavigateToName, (LPARAM)param);
	if (FAILED(hr))
	{
		SysFreeString(param->url);
		free(param);
	}
	return hr;
}

HRESULT HTMLContainer2::WriteHTML(LPCWSTR pszHTML)
{
	BSTR data = SysAllocString(pszHTML);
	if (NULL == data && NULL != pszHTML)
		return E_OUTOFMEMORY;

	HRESULT hr = WriteDocument(data);
	if (FAILED(hr)) 
	{
		SysFreeString(data);
	}

	return hr;
}

HRESULT HTMLContainer2::WriteDocument(BSTR data)
{
	IWebBrowser2 *pWeb2 = NULL;
	HRESULT hr = GetIWebBrowser2(&pWeb2);
	if (FAILED(hr)) return hr;

	IDispatch *pDisp = NULL;
	hr = pWeb2->get_Document(&pDisp);
	if (S_OK == hr)
	{
		IHTMLDocument2 *pDoc = NULL;
		hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
		if (SUCCEEDED(hr))
		{	
			SAFEARRAY *array = SafeArrayCreateVector(VT_VARIANT, 0, 1);
			if (NULL != array)
			{
				VARIANT *item = NULL;
				hr = SafeArrayAccessData(array, (void**)&item);
				if (SUCCEEDED(hr))
				{
					VariantInit(item);
					V_VT(item) = VT_BSTR;
					V_BSTR(item) = data;

					hr = pDoc->write(array);
					pDoc->close();

					if (FAILED(hr))
                        V_BSTR(item) = NULL;

					SafeArrayUnaccessData(array);
				}
				SafeArrayDestroy(array);
			}
			else 
			{
				hr = E_OUTOFMEMORY;
			}

			pDoc->Release();
		}
		pDisp->Release();
	}
	pWeb2->Release();
	return hr;
}

COLORREF HTMLContainer2::OnGetHostBkColor(void)
{
	return DEFAULT_HOSTBKCOLOR;
}

DWORD HTMLContainer2::OnGetHostInfoFlags(void)
{
	return DEFAULT_DOCHOSTUIFLAGS;
}

OLECHAR *HTMLContainer2::OnGetHostCSS(void)
{
	return NULL;
}

OLECHAR *HTMLContainer2::OnGetHostNamespace(void)
{
	return NULL;
}

DWORD HTMLContainer2::OnGetDownlodFlags(void)
{
	return DEFAULT_DOWNLOADFLAGS;
}

LPCWSTR HTMLContainer2::OnGetUserAgent(void)
{
	return NULL;
}

HRESULT HTMLContainer2::InvokeScriptFunction(LPCWSTR pszFuncName, LCID lcid, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, UINT FAR *puArgErr)
{
	IWebBrowser2 *pWeb2 = NULL;
	IDispatch *pDisp = NULL;
	IDispatch *pDispScript = NULL;
	IHTMLDocument2 *pDoc = NULL;
	DISPID dispid;

	HRESULT hr = GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr))
	{
		hr = pWeb2->get_Document(&pDisp);
		if (SUCCEEDED(hr))
		{
			hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
			if (SUCCEEDED(hr))
			{
				hr = pDoc->get_Script(&pDispScript);
				if (SUCCEEDED(hr))
				{
					hr = pDispScript->GetIDsOfNames(IID_NULL, (LPWSTR*)&pszFuncName, 1, lcid, &dispid);
					if (SUCCEEDED(hr))
					{
						hr = pDispScript->Invoke(dispid, IID_NULL, lcid, DISPATCH_METHOD, pDispParams, pVarResult, pExcepInfo, puArgErr);
					}
					pDispScript->Release();
				}
				pDoc->Release();
			}
			pDisp->Release();
		}
		pWeb2->Release();
	}
	return hr;
}

BROWSERCB HTMLContainer2::RegisterBrowserEventCB(BROWSERCB fnBrowserCB, LPVOID pUserData)
{
	BROWSERCB fnTemp = this->fnBrwoserCB;
	this->fnBrwoserCB = fnBrowserCB;
	this->userData = pUserData;
	return fnTemp;
}

DWORD HTMLContainer2::GetContainerStyle(void)
{
	return CSTYLE_NORMAL;
}

HRESULT HTMLContainer2::IsFrameset(IWebBrowser2 *pWeb2)
{
	if (NULL == pWeb2)
		return E_INVALIDARG;

	IDispatch *pDocDisp = NULL;
	HRESULT hr = pWeb2->get_Document(&pDocDisp);
	if (SUCCEEDED(hr) && NULL != pDocDisp)
	{
		IHTMLDocument2 *pDoc = NULL;
		hr = pDocDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
		if (SUCCEEDED(hr) && NULL != pDoc)
		{
			IHTMLElement *pElement = NULL;
			if (SUCCEEDED(pDoc->get_body(&pElement)) && NULL != pElement)
			{
				IHTMLBodyElement* pBodyElement = NULL;
				if (SUCCEEDED(pElement->QueryInterface(IID_IHTMLBodyElement, (void**)&pBodyElement)))
				{
					hr = S_FALSE;
				}
				pElement->Release();
			}
			pDoc->Release();
		}
		pDocDisp->Release();
	}
	return hr;
}

HRESULT HTMLContainer2::GetFramesCount(IWebBrowser2 *pWeb2, INT *frameCount)
{
	if (NULL == frameCount)
		return E_POINTER;

	*frameCount = 0;

	if(NULL == pWeb2)
		return E_INVALIDARG;

	IDispatch *pDocDisp = NULL;
	HRESULT hr = pWeb2->get_Document(&pDocDisp);
	if (SUCCEEDED(hr) && NULL != pDocDisp)
	{
		IOleContainer *pContainer = NULL;
		hr = pDocDisp->QueryInterface(IID_IOleContainer, (void**)&pContainer);
		if (SUCCEEDED(hr))
		{
			IEnumUnknown* pEnumerator = NULL;
			hr = pContainer->EnumObjects(OLECONTF_EMBEDDINGS, &pEnumerator);
			if (SUCCEEDED(hr))
			{
				IUnknown* pUnknown = NULL;
				ULONG uFetched = 0;
				for (UINT i = 0; S_OK == pEnumerator->Next(1, &pUnknown, &uFetched); i++)
				{
					IWebBrowser2* pBrowser = NULL;
					if (SUCCEEDED(pUnknown->QueryInterface(IID_IWebBrowser2, (void**)&pBrowser)))
					{
						if (S_OK == IsFrameset(pBrowser))
						{
							INT f = 0;
							if (SUCCEEDED(GetFramesCount(pBrowser, &f)))
								*frameCount += f;
						}
						else
						{
							(*frameCount)++;
						}
						pBrowser->Release();
					}
					pUnknown->Release();
				}
				pEnumerator->Release();
			}
			pContainer->Release();
		}
		pDocDisp->Release();
	}
	return hr;
}

BOOL HTMLContainer2::ValidateURLHost(LPCWSTR pszUrl)
{
	if (NULL == libWininet.InternetCrackUrl)
	{
		HTMLContainer2_LoadLibray((LIBHDR*)&libWininet);
		if (NULL == libWininet.InternetCrackUrl)
		{			
			return TRUE;
		}
	}

	if (NULL == pszUrl || L'\0' == *pszUrl)
		return FALSE;

	URL_COMPONENTSW uc;
	WCHAR szHost[2048] = {0};

	ZeroMemory(&uc, sizeof(URL_COMPONENTSW));
	uc.dwStructSize = sizeof(URL_COMPONENTSW);
	uc.lpszHostName = szHost;
	uc.dwHostNameLength = ARRAYSIZE(szHost);

	if (FALSE == libWininet.InternetCrackUrl(pszUrl, 0, NULL, &uc))
	{
		// ERROR_INTERNET_INVALID_URL
		DWORD error = GetLastError();

		BSTR version = NULL;
		if (SUCCEEDED(GetAppVersion(&version)) && NULL != version)
		{				
			if (NULL != StrStrIW(version, L"MSIE 6.0"))
			{
				LPCWSTR c = pszUrl;
				while(c && L'\0' != *c)
				{
					if (L'?' == *c)
					{
						WCHAR szTemp[2048] = {0};
						if (SUCCEEDED(StringCchCopyNW(szTemp, ARRAYSIZE(szTemp), pszUrl, (c - pszUrl))) &&
							FALSE != libWininet.InternetCrackUrl(szTemp, 0, NULL, &uc))
						{
							error = ERROR_SUCCESS;
						}
						break;
					}
					c++;
				}
			}

			SysFreeString(version);
		}
		if (ERROR_SUCCESS != error)
			return FALSE;
	}

	if (0 == uc.dwHostNameLength)
		return TRUE;

	switch(uc.nScheme)
	{
		case INTERNET_SCHEME_PARTIAL:
		case INTERNET_SCHEME_DEFAULT:
		case INTERNET_SCHEME_FTP:
		case INTERNET_SCHEME_GOPHER:
		case INTERNET_SCHEME_HTTP:
		case INTERNET_SCHEME_HTTPS:
			{
				LPCWSTR block = uc.lpszHostName;
				LPCWSTR end = block + uc.dwHostNameLength;
				LPCWSTR cursor = block;

				for(;;)
				{
					if (cursor == end || L'.' == *cursor)
					{
						if (block == cursor && end != cursor)
							return FALSE;

						if (end == cursor)
							return TRUE;
					}
					cursor++;
				}
			}
			break;
		case INTERNET_SCHEME_RES:
		case INTERNET_SCHEME_FILE:
			if (CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, 0, L"shdoclc", -1, uc.lpszHostName, -1))
			{
				if (NULL != StrStrIW(pszUrl, L"syntax.htm"))
				{
					return FALSE;
				}
			}
			break;
	}
	return TRUE;
}

HRESULT HTMLContainer2::GetAppVersion(BSTR *p)
{
	if (NULL == p)
		return E_POINTER;

	IWebBrowser2 *pWeb2 = NULL;
	HRESULT hr = GetIWebBrowser2(&pWeb2);
	if (FAILED(hr)) return hr;

	IDispatch *pDispatch = NULL;
	hr = pWeb2->get_Document(&pDispatch);
	if (NULL == pDispatch) hr = E_FAIL;
	if (SUCCEEDED(hr))
	{
		IHTMLDocument2 *pDoc = NULL;
		hr = pDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
		if (SUCCEEDED(hr))
		{
			IHTMLWindow2 *pWnd = NULL;
			hr = pDoc->get_parentWindow(&pWnd);
			if (NULL == pWnd) hr = E_FAIL;
			if (SUCCEEDED(hr))
			{
				IOmNavigator *pNavigator = NULL;
				hr = pWnd->get_navigator(&pNavigator);
				if (NULL == pNavigator) hr = E_FAIL;
				if (SUCCEEDED(hr))
				{
					hr = pNavigator->get_appVersion(p);
					pNavigator->Release();
				}
				pWnd->Release();
			}
			pDoc->Release();
		}
		pDispatch->Release();
	}

	pWeb2->Release();
	return hr;
}

HRESULT HTMLContainer2::GetUserAgent(BSTR *p)
{
	if (NULL == p)
		return E_POINTER;

	IWebBrowser2 *pWeb2 = NULL;
	HRESULT hr = GetIWebBrowser2(&pWeb2);
	if (FAILED(hr)) return hr;

	IDispatch *pDispatch = NULL;
	hr = pWeb2->get_Document(&pDispatch);
	if (NULL == pDispatch) hr = E_FAIL;
	if (SUCCEEDED(hr))
	{
		IHTMLDocument2 *pDoc = NULL;
		hr = pDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
		if (SUCCEEDED(hr))
		{
			IHTMLWindow2 *pWnd = NULL;
			hr = pDoc->get_parentWindow(&pWnd);
			if (NULL == pWnd) hr = E_FAIL;
			if (SUCCEEDED(hr))
			{
				IOmNavigator *pNavigator = NULL;
				hr = pWnd->get_navigator(&pNavigator);
				if (NULL == pNavigator) hr = E_FAIL;
				if (SUCCEEDED(hr))
				{
					hr = pNavigator->get_userAgent(p);
					pNavigator->Release();
				}
				pWnd->Release();
			}
			pDoc->Release();
		}
		pDispatch->Release();
	}

	pWeb2->Release();
	return hr;
}

void HTMLContainer2::OnBeforeNavigate(IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
{
	if (0 == (CF_DISABLEBEFORENAVFILTER & dwFlags))
	{
		// follow the Winamp internet preferences options
		if (!SendMessageW(winampWindow, WM_WA_IPC, 0, IPC_INETAVAILABLE))
		{
			LPCWSTR pszTemplate = L"res://";
			LPCWSTR pszEntry = StrStrIW(URL->bstrVal, pszTemplate);
			if (NULL == pszEntry)
			{
				if (StrCmpNIW(URL->bstrVal, L"about:blank", 11) && StrCmpNIW(URL->bstrVal, L"javascript:", 11))
				{
					*Cancel = ((VARIANT_BOOL)-2);
					dwFlags |= CF_DISABLEBEFORENAVFILTER;
					OnNavigateCancelled(NULL, Cancel);
					dwFlags &= ~CF_DISABLEBEFORENAVFILTER;
					if (NULL != Cancel && VARIANT_TRUE == *Cancel)
						return;
				}
			}
		}

		if (FALSE == ValidateURLHost(URL->bstrVal))
		{
			VARIANT StatusCode;
			VariantInit(&StatusCode);
			V_VT(&StatusCode) = VT_I4;
			V_I4(&StatusCode) = INET_E_INVALID_URL;

			dwFlags |= CF_DISABLEBEFORENAVFILTER;

			*Cancel = VARIANT_TRUE;
			OnNavigateError(pDispatch, URL, TargetFrameName, &StatusCode, Cancel);
			Navigate2(URL, NULL, TargetFrameName, NULL, NULL);

			dwFlags &= ~CF_DISABLEBEFORENAVFILTER;
			if (NULL != Cancel && VARIANT_TRUE == *Cancel)
				return;
		}

		IWebBrowser2 *pWebMine = NULL, *pWebFrame = NULL;
		if (FAILED(GetIWebBrowser2(&pWebMine)))
			pWebMine = NULL;

		if (NULL == pDispatch ||
			FAILED(pDispatch->QueryInterface(IID_IWebBrowser2, (void**)&pWebFrame)))
		{
			pWebFrame = NULL;
		}

		if (NULL != pWebMine) pWebMine->Release();
		if (NULL != pWebFrame) pWebFrame->Release();

		if (NULL != pWebMine && pWebMine == pWebFrame && NULL != URL && VT_BSTR == URL->vt && NULL != URL->bstrVal)
		{
			LPCWSTR pszTemplate = L"navcancl.htm#";
			LPCWSTR pszEntry = StrStrIW(URL->bstrVal, pszTemplate);
			if (NULL != pszEntry)
			{
				dwFlags |= CF_DISABLEBEFORENAVFILTER;
				OnNavigateCancelled(pszEntry + lstrlenW(pszTemplate), Cancel);
				dwFlags &= ~CF_DISABLEBEFORENAVFILTER;

				if (NULL != Cancel && VARIANT_TRUE == *Cancel)
					return;
			}
		}
	}
}

STDMETHODIMP HTMLContainer2::RegisterBrowserCursor(INT nSysCurID, HCURSOR hCurToUse)
{
	IECURSOR *pCursor = (IECURSOR*)hCursors;
	if (pCursor)
	{
		for (int i =0; i < nCursors; i++)
		{
			if (pCursor[i].nSysId == nSysCurID)
			{
				if (pCursor[i].hUser) DestroyCursor(pCursor[i].hUser);
				pCursor[i].hSys = NULL;
				if (hCurToUse) pCursor[i].hUser = hCurToUse;
				else
				{
					MoveMemory(&pCursor[i], &pCursor[i + 1], sizeof(IECURSOR)*(nCursors - i - 1));
					nCursors--;
					if (!nCursors)
					{
						free(hCursors);
						hCursors = NULL;
					}

					VOID* data = realloc(hCursors, sizeof(IECURSOR)*nCursors);
					if (data) hCursors = data;
					else return E_OUTOFMEMORY;
				}
				return S_OK;	
			}
		}
	}

	if (hCurToUse)
	{
		VOID *data = realloc(hCursors, sizeof(IECURSOR)*(nCursors + 1));
		if (!data) return E_OUTOFMEMORY;
		hCursors = data;
		pCursor = (IECURSOR*)hCursors;
		pCursor[nCursors].nSysId = nSysCurID;
		pCursor[nCursors].hSys = NULL;
		pCursor[nCursors].hUser = hCurToUse;
		nCursors++;
	}
	return S_OK;
}

HRESULT HTMLContainer2::InternetSetFeatureEnabled(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags, BOOL fEnable)
{
	if (NULL == libUrlmon.CoInternetSetFeatureEnabled)
	{
		HTMLContainer2_LoadLibray((LIBHDR*)&libUrlmon);
		if (NULL == libUrlmon.CoInternetSetFeatureEnabled)
		{			
			return E_FAIL;
		}
	}
	return libUrlmon.CoInternetSetFeatureEnabled(FeatureEntry, dwFlags, fEnable);
}

HRESULT HTMLContainer2::InternetIsFeatureEnabled(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags)
{
	if (NULL == libUrlmon.CoInternetIsFeatureEnabled)
	{
		HTMLContainer2_LoadLibray((LIBHDR*)&libUrlmon);
		if (NULL == libUrlmon.CoInternetIsFeatureEnabled)
		{			
			return E_FAIL;
		}
	}
	return libUrlmon.CoInternetIsFeatureEnabled(FeatureEntry, dwFlags);
}

void HTMLContainer2::ProcessRedirectedMessage(HWND hwnd, UINT messageId, LPARAM param)
{
	switch(messageId)
	{
		case HTMLContainer2::msgNavigate2:
			{
				RMNAVIGATE2 *pnp = (RMNAVIGATE2*)param;
				if (NULL != pnp)
				{
					Navigate2(&pnp->URL, &pnp->Flags, NULL, NULL, &pnp->Headers);
					VariantClear(&pnp->URL);
					VariantClear(&pnp->Flags);
					VariantClear(&pnp->TargetFrameName);
					VariantClear(&pnp->PostData);
					VariantClear(&pnp->Headers);
					free(pnp);
				}
			}
			break;

		case HTMLContainer2::msgNavigateToName:
			{
				RMNAVIGATETONAME *pntn = (RMNAVIGATETONAME*)param;
				if (NULL != pntn)
				{
					NavigateToName(pntn->url, pntn->flags);
					free(pntn);
				}
			}
			break;
	}
}

#define SUBCLASS_UNKNOWN		0
#define SUBCLASS_EMBED			1
#define SUBCLASS_DOCOBJECT		2
#define SUBCLASS_IESERVER		3

typedef struct __SUBCLASSCTRL
{
	UINT	type;
	WNDPROC originalProc;
	HTMLContainer2 *container;
} SUBCLASSCTRL;

static LRESULT SubclassControl_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SUBCLASSCTRL *psc = (SUBCLASSCTRL*)GetPropW(hwnd, WNDPROP_SCCTRLW);
	if (NULL == psc || NULL == psc->originalProc)
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_NCDESTROY:
		case WM_DESTROY: // detach
			RemovePropW(hwnd, WNDPROP_SCCTRLW);
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)psc->originalProc);
			CallWindowProcW(psc->originalProc, hwnd, uMsg, wParam, lParam);
			free(psc);
			return 0;

		case WM_ERASEBKGND: 
			if(	NULL != wParam &&
				NULL != psc->container &&
				(SUBCLASS_EMBED == psc->type || SUBCLASS_DOCOBJECT == psc->type))
			{				
				HWND hChild = GetWindow(hwnd, GW_CHILD);
				if (NULL == hChild || 0 == (WS_VISIBLE & GetWindowLongPtrW(hChild, GWL_STYLE)))
				{						
					RECT paintRect;
					if (FALSE != GetUpdateRect(hwnd, &paintRect, FALSE) ||
						FALSE != GetClientRect(hwnd, &paintRect))
					{
						SetBkColor((HDC)wParam, psc->container->OnGetHostBkColor());
						ExtTextOutW((HDC)wParam, 0, 0, ETO_OPAQUE, &paintRect, NULL, 0, NULL);
					}
				}
			}
			return 1;

		case WM_SETFOCUS:
			if (SUBCLASS_IESERVER == psc->type)
			{	
				if (NULL != psc->container)
					psc->container->SetFocus(TRUE);
			}
			else
			{
				HWND hFocus = (HWND)wParam;
				if (NULL != hFocus && IsWindowEnabled(hFocus))
				{
					HWND hRoot = GetAncestor(hwnd, GA_ROOT);
					if (hFocus == hRoot || IsChild(hRoot, hFocus))
					{
						SetFocus(hFocus);
						return 0;
					}
				}
			}
			break;

		case WM_KILLFOCUS:
			if (SUBCLASS_IESERVER == psc->type && NULL != psc->container)
			{	
				psc->container->SetFocus(FALSE);
			}
			break;

		case WM_PARENTNOTIFY:
			if ((SUBCLASS_EMBED == psc->type || SUBCLASS_DOCOBJECT == psc->type) &&
				WM_CREATE == LOWORD(wParam) && 0L != lParam)
			{				
				SubclassControl_Attach((HWND)lParam,  psc->container);
			}

			if (SUBCLASS_EMBED == psc->type && WM_DESTROY == LOWORD(wParam) && NULL != lParam)
			{
				HWND hChild = (HWND)lParam;
				WCHAR szClass[64] = {0};
				if (0 != GetClassNameW(hChild, szClass, ARRAYSIZE(szClass)) &&
					IS_EQUALCLASS(szClass, L"Shell DocObject View"))
				{
					HWND hPrimary = FindWindowExW(hwnd, NULL, L"Shell DocObject View", NULL);
					if (hPrimary != NULL && hPrimary != hChild)
					{
						if(NULL != psc->container)
							psc->container->OnClosePopupInternal();
					}
				}
			}
			break;

		case WM_SETCURSOR:
			if (SUBCLASS_IESERVER == psc->type && FALSE == bIsVistaOrHigher && 
				NULL != psc->container && psc->container->nCursors)
			{
				ShowCursor(FALSE);
				CallWindowProcW(psc->originalProc, hwnd, uMsg, wParam, lParam);

				HCURSOR hCur = GetCursor();
				IECURSOR *pCur = (IECURSOR*)(psc->container->hCursors);
				for( int i= psc->container->nCursors -1; i > -1 ; i--)
				{	
					if (NULL == pCur[i].hSys)
						pCur[i].hSys = LoadCursor(NULL, MAKEINTRESOURCE(pCur[i].nSysId));

					if (pCur[i].hSys == hCur)
					{
						if (NULL != pCur[i].hUser)
							SetCursor(pCur[i].hUser);
						break;
					}
				}

				ShowCursor(TRUE);

				return TRUE;
			}
			break;
		
		case WM_CONTEXTMENU:
			if (NULL != psc->container)
			{
				HANDLE hook = psc->container->InitializePopupHook(hwnd, uMsg, wParam, lParam);
				if (NULL != hook)
				{
					LRESULT result = CallWindowProcW(psc->originalProc, hwnd, uMsg, wParam, lParam);
					psc->container->DeletePopupHook(hook);
					return result;
				}
			}
			break;

		case WM_INITMENUPOPUP:
			if (NULL != psc->container)
			{
				psc->container->InitializeMenuPopup(hwnd, (HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
			}
			break;

		case WM_INPUTLANGCHANGEREQUEST:
			if (SUBCLASS_IESERVER == psc->type && 
				NULL != psc->container &&
				FALSE != psc->container->InputLangChangeRequest(hwnd, (UINT)wParam, (HKL)lParam))
			{
				return 0;
			}
			break;

		case WM_INPUTLANGCHANGE:
			if (SUBCLASS_IESERVER == psc->type && NULL != psc->container)
				psc->container->InputLangChange((UINT)wParam, (HKL)lParam);
			break;
	}

	if (0 != WM_REDIRECTNAVIGATE && WM_REDIRECTNAVIGATE == uMsg)
	{
		if(SUBCLASS_EMBED == psc->type && NULL != psc->container)
			psc->container->ProcessRedirectedMessage(hwnd, (UINT)wParam, lParam);
		return TRUE;
	}

	if (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg && 
		WM_NULL != WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{
		ReplyMessage(TRUE);
		SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	return CallWindowProcW(psc->originalProc, hwnd, uMsg, wParam, lParam);
}

static BOOL SublassControl_IsAttached(HWND hwnd)
{
	SUBCLASSCTRL *psc = (SUBCLASSCTRL*)GetPropW(hwnd, WNDPROP_SCCTRLW);
	return (NULL != psc);
}

static BOOL SubclassControl_Attach(HWND hControl, HTMLContainer2 *pContainer)
{
	if (NULL == hControl || FALSE == IsWindow(hControl))
		return FALSE;

	SUBCLASSCTRL *psc = (SUBCLASSCTRL*)GetPropW(hControl, WNDPROP_SCCTRLW);
	if (NULL != psc) return TRUE;

	UINT controlType = SUBCLASS_UNKNOWN;

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");

	WCHAR szClass[128] = {0};
	if (!GetClassNameW(hControl, szClass, ARRAYSIZE(szClass)))
		return FALSE;

	if (IS_EQUALCLASS(szClass, L"Shell Embedding")) controlType = SUBCLASS_EMBED;
	else if (IS_EQUALCLASS(szClass, L"Shell DocObject View")) controlType = SUBCLASS_DOCOBJECT;
	else if (IS_EQUALCLASS(szClass, L"Internet Explorer_Server")) controlType = SUBCLASS_IESERVER;

	if (SUBCLASS_UNKNOWN == controlType)
		return FALSE;

	if (SUBCLASS_IESERVER != controlType)
	{
		DWORD styleEx = GetWindowStyleEx(hControl);
		SetWindowLongPtrW(hControl, GWL_EXSTYLE, styleEx | WS_EX_CONTROLPARENT);
	}
	else
	{
		DWORD style = GetWindowStyle(hControl);
		SetWindowLongPtrW(hControl, GWL_STYLE, style | WS_TABSTOP);
	}

	psc = (SUBCLASSCTRL*)calloc(1, sizeof(SUBCLASSCTRL));
	if (NULL == psc) return FALSE;

	psc->container = pContainer;
	psc->type = controlType;
	psc->originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hControl, GWLP_WNDPROC, (LONGX86)(LONG_PTR)SubclassControl_WindowProc);

	if (NULL == psc->originalProc ||
		FALSE == SetPropW(hControl, WNDPROP_SCCTRLW, psc))
	{
		if (NULL != psc->originalProc) 
			SetWindowLongPtrW(hControl, GWLP_WNDPROC, (LONGX86)(LONG_PTR)psc->originalProc);
		free(psc);
		return FALSE;
	}

	return TRUE;
}