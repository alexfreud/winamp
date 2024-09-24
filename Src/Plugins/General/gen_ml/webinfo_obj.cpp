#include "main.h"
#include "./webinfo_obj.h"
#include "../winamp/buildtype.h"
#include "./resource.h"
#include "api__gen_ml.h"
#include "../Winamp/buildtype.h"

#include <exdisp.h>
#include <mshtmdid.h>
#include <mshtml.h>

#include <strsafe.h>

#define WEBINFO_URL				L"http://client.winamp.com/nowplaying/mini"
#define WEBINFO_FUNCTION			L"SongInfoCallback"

#define WEBINFO_USERAGENT		L"Winamp File Info"	

#define WEBINFO_DOWNLOADFLAGS		(	DLCTL_DLIMAGES |					\
										/*DLCTL_NO_SCRIPTS |*/			\
										/*DLCTL_NO_JAVA | */			\
										DLCTL_NO_DLACTIVEXCTLS |	\
										/*DLCTL_NO_RUNACTIVEXCTLS |*/	\
										/*DLCTL_RESYNCHRONIZE	|*/		\
										DLCTL_NO_BEHAVIORS |		\
										0)

#define WEBINFO_HOSTINFODFLAGS		(	DOCHOSTUIFLAG_DIALOG |							\
										DOCHOSTUIFLAG_DISABLE_HELP_MENU |				\
										DOCHOSTUIFLAG_NO3DBORDER |						\
										DOCHOSTUIFLAG_SCROLL_NO |						\
										DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE |			\
										DOCHOSTUIFLAG_ACTIVATE_CLIENTHIT_ONLY |			\
										DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION |		\
										DOCHOSTUIFLAG_THEME |							\
										DOCHOSTUIFLAG_NOPICS |							\
										DOCHOSTUIFLAG_NO3DOUTERBORDER |					\
										DOCHOSTUIFLAG_DISABLE_UNTRUSTEDPROTOCOL |		\
										DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION |	\
										DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL |		\
										0)

#define WEBINFO_CONTAINERSTYLE			(CSTYLE_NAVIGATE2_NOCLICKSOUND | CSTYLE_NOCLICKSOUND)



static const wchar_t pszCSSTemplate[] = L"BODY { "
												L"background-color: #%06X;"
												L"color: #%06X;"
												L"scrollbar-face-color: #%06X;"
												L"scrollbar-track-color: #%06X;"
												L"scrollbar-3dlight-color: #%06X;"
												L"scrollbar-shadow-color: #%06X;"
												L"scrollbar-darkshadow-color: #%06X;"
												L"scrollbar-highlight-color: #%06X;"
												L"scrollbar-arrow-color: #%06X"
										L" }";

static const wchar_t pszHTMLTemplate[] =	L"<HTML><HEAD></HEAD><BODY>"
											L"<table height=\"100%%\" width=\"100%%\">"
											L"<tr><td align=center valign=middle height=\"96%%\" width=\"96%%\">"
											L"<font size=-1 face=\"Arial\">%s</font>"
											L"</td></tr></table>"
										L"</BODY></HTML>";

WebFileInfo *CreateWebFileInfo(HWND hwndParent, IDispatch *pDispWA)
{
	return new WebFileInfo(hwndParent, pDispWA);
}

static COLORREF GetHTMLColor(int nColorIndex);

WebFileInfo::WebFileInfo(HWND hwndParent, IDispatch *pDispWA) :
						 HTMLContainer2(plugin.hwndParent, hwndParent), nHomePage(HOMEPAGE_NOTLOADED),
					     bstrMessage(NULL), bstrFileName(NULL), nDragMode(DROPEFFECT_NONE)
{
	this->pDispWA = pDispWA;
	if (NULL != pDispWA)
		pDispWA->AddRef();
}

WebFileInfo::~WebFileInfo(void)
{
	if (bstrMessage) 
	{
		SysFreeString(bstrMessage);
		bstrMessage = NULL;
	}
	if (bstrFileName) 
	{
		SysFreeString(bstrFileName);
		bstrFileName = NULL;
	}
	if (NULL != pDispWA)
		pDispWA->Release();
}

HRESULT WebFileInfo::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (IsEqualIID(riid, IID_IDropTarget)) 
	{
		*ppvObject = (IDropTarget*)this;
		return S_OK;
	}
		
	return HTMLContainer2::QueryInterface(riid, ppvObject);
}

ULONG WebFileInfo::AddRef(void)
{
	return HTMLContainer2::AddRef();
}

ULONG WebFileInfo::Release(void)
{
	return HTMLContainer2::Release();
}

HRESULT WebFileInfo::TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
	if ((WM_KEYDOWN == lpMsg->message ||  WM_KEYUP == lpMsg->message) && lpMsg->wParam >= VK_F1  && lpMsg->wParam <= VK_F24) 
	{
		HWND hHost = GetParentHWND();
		if (hHost && IsWindow(hHost)) PostMessageW(hHost, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
		return S_OK;
	}
	return HTMLContainer2::TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
}

HRESULT WebFileInfo::ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
#ifdef WINAMP_FINAL_BUILD
	return S_OK; // block menu
#else
	return E_NOTIMPL;
#endif
}

HRESULT WebFileInfo::ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult)
{
	wchar_t szBuffer[256] = {0};
	lpstrCaption = WASABI_API_LNGSTRINGW_BUF(IDS_WEBINFO_MESSAGEBOX_TITLE, szBuffer,
											 sizeof(szBuffer)/sizeof(wchar_t));
	*plResult = MessageBoxW(hwnd, lpstrText, lpstrCaption, dwType);
	return S_OK;
}

COLORREF WebFileInfo::OnGetHostBkColor(void)
{
	return  WADlg_getColor(WADLG_ITEMBG);
}

DWORD WebFileInfo::OnGetHostInfoFlags(void)
{
	return WEBINFO_HOSTINFODFLAGS;
}

OLECHAR *WebFileInfo::OnGetHostCSS(void)
{
	LPWSTR pszCSS;
	pszCSS  = (LPWSTR)CoTaskMemAlloc(sizeof(wchar_t)*4096);
	if (pszCSS && S_OK != StringCchPrintfW(pszCSS, 4096, pszCSSTemplate, 
														GetHTMLColor(WADLG_ITEMBG), 
														GetHTMLColor(WADLG_ITEMFG), 
														GetHTMLColor(WADLG_LISTHEADER_BGCOLOR), 
														GetHTMLColor(WADLG_SCROLLBAR_BGCOLOR), 
														GetHTMLColor(WADLG_LISTHEADER_FRAME_TOPCOLOR), 
														GetHTMLColor(WADLG_LISTHEADER_BGCOLOR), 
														GetHTMLColor(WADLG_LISTHEADER_FRAME_BOTTOMCOLOR), 
														GetHTMLColor(WADLG_LISTHEADER_BGCOLOR), 
														GetHTMLColor(WADLG_BUTTONFG)))
	{
		CoTaskMemFree(pszCSS);
		pszCSS = NULL;
	}
	
	return pszCSS;
}

DWORD WebFileInfo::OnGetDownlodFlags(void)
{

	return WEBINFO_DOWNLOADFLAGS
#ifdef WINAMP_FINAL_BUILD
			|DLCTL_SILENT 
#endif
		;
}

LPCWSTR WebFileInfo::OnGetUserAgent(void)
{
	return WEBINFO_USERAGENT;
}

DWORD WebFileInfo::GetContainerStyle(void)
{
	return WEBINFO_CONTAINERSTYLE;
}
HRESULT WebFileInfo::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;
	
	if (NULL != pDispWA)
	{
		*ppDispatch = pDispWA;
		pDispWA->AddRef();
		return S_OK;
	}
	return HTMLContainer2::GetExternal(ppDispatch);
}

HRESULT WebFileInfo::GetDropTarget(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
	if (ppDropTarget)
	{
		*ppDropTarget = (IDropTarget*)this;
		AddRef();
		return S_OK;
	}
	return HTMLContainer2::GetDropTarget(pDropTarget, ppDropTarget);
}

void WebFileInfo::OnBeforeNavigate(IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
{
	HTMLContainer2::OnBeforeNavigate(pDispatch, URL, Flags, TargetFrameName, PostData, Headers, Cancel);
	if (HOMEPAGE_LOADED == nHomePage) nHomePage = HOMEPAGE_NOTLOADED;
	
}
void WebFileInfo::OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
	HTMLContainer2::OnNewWindow3(ppDisp, Cancel, dwFlags, bstrUrlContext, bstrUrl);
	if (bstrUrl) 
	{
		HWND hwndHost;
		hwndHost = GetHostHWND();
		ShellExecuteW(hwndHost, NULL, bstrUrl, NULL, L".", 0); // lets open all annoying popups in default browser
	}
	*ppDisp = NULL;
	*Cancel = VARIANT_TRUE;
}

void WebFileInfo::OnNavigateError(IDispatch *pDispatch, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel)
{
	HTMLContainer2::OnNavigateError(pDispatch, URL, TargetFrameName, StatusCode, Cancel);

	*Cancel = VARIANT_TRUE;

	if (bstrFileName) 
	{
		SysFreeString(bstrFileName);
		bstrFileName = NULL;
	}

	IWebBrowser2 *pWeb2;
	
	if (pDispatch && SUCCEEDED(pDispatch->QueryInterface(IID_IWebBrowser2, (void**)&pWeb2)))
	{
		pWeb2->Stop();
		pWeb2->Release();
	}

	wchar_t szErrorString[128] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_WEBINFO_NAVIGATE_ERROR, szErrorString, sizeof(szErrorString)/sizeof(wchar_t));
	DisplayMessage(szErrorString, TRUE);
	nHomePage = HOMEPAGE_FAILED;
}

void WebFileInfo::OnDocumentReady(IDispatch *pDispatch, VARIANT *URL)
{
	HTMLContainer2::OnDocumentReady(pDispatch, URL);

	if (bstrMessage)
	{
		if (URL && VT_BSTR == URL->vt && URL->bstrVal && 0 == lstrcmpW(URL->bstrVal, L"about:blank"))
		{
			wchar_t szHTML[4096] = {0};
			if (S_OK == StringCchPrintfW(szHTML, sizeof(szHTML)/sizeof(wchar_t), pszHTMLTemplate, bstrMessage))
			{
				WriteHTML(szHTML);
			}
		}
		SysFreeString(bstrMessage);
		bstrMessage = NULL;
	}

	if (HOMEPAGE_LOADING == nHomePage) 
	{
		nHomePage = HOMEPAGE_LOADED;
		if (bstrFileName)
		{
			InvokeFileInfo(bstrFileName);
			SysFreeString(bstrFileName);
			bstrFileName = NULL;
		}
	}
		
}

HRESULT WebFileInfo::InvokeFileInfo(LPCWSTR pszFileName)
{
    HRESULT hr;
	DISPPARAMS dispParams;
	LCID lcid;

	if (HOMEPAGE_NOTLOADED == nHomePage)
	{
		if (bstrFileName) 
		{
			SysFreeString(bstrFileName);
			bstrFileName = NULL;
		}
		bstrFileName = (pszFileName) ? SysAllocString(pszFileName) : NULL;
		return NavigateToPage();
	}
		
	lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	ZeroMemory(&dispParams, sizeof(DISPPARAMS));
	dispParams.cArgs = 1;
	dispParams.rgvarg = (VARIANTARG*)calloc(dispParams.cArgs, sizeof(VARIANTARG));
	if (!dispParams.rgvarg) hr = E_OUTOFMEMORY;
	else
	{
		VariantInit(&dispParams.rgvarg[0]);
		dispParams.rgvarg[0].vt			= VT_BSTR;
		dispParams.rgvarg[0].bstrVal		= SysAllocString(pszFileName);
		wTRACE_FMT(L"WebInfo: Requesting song info for '%s'.\n", pszFileName);
		hr = InvokeScriptFunction(WEBINFO_FUNCTION, lcid, &dispParams, NULL, NULL, NULL);
		if (S_OK != hr) TRACE_FMT(TEXT("Error sending webinfo (0x%08X)\n"), hr);
		VariantClear(&dispParams.rgvarg[0]);
		free(dispParams.rgvarg);
	}
	return hr;
}

HRESULT WebFileInfo::NavigateToPage(void)
{
	HRESULT hr;
	nHomePage = HOMEPAGE_LOADING;
	hr = NavigateToName(WEBINFO_URL, navNoHistory);
	if (FAILED(hr)) nHomePage = HOMEPAGE_FAILED;
	return hr;
}

HRESULT WebFileInfo::UpdateColors(void)
{
	HRESULT hr;
	IWebBrowser2 *pWeb2;

	if (HOMEPAGE_LOADED == nHomePage) return S_OK;
	
	hr = GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr))
	{
		hr = pWeb2->Refresh();
		pWeb2->Release();
	}
	return hr;
}

HRESULT WebFileInfo::DisplayMessage(LPCWSTR pszMessage, BOOL bPostIt)
{
	HRESULT hr;
	VARIANT Flags, URL;
	if (bstrMessage) 
	{
		SysFreeString(bstrMessage);
		bstrMessage = NULL;
	}
	bstrMessage = (pszMessage) ? SysAllocString(pszMessage) : NULL;

	VariantInit(&URL);
	VariantInit(&Flags);
	
	Flags.vt = VT_I4;
	V_I4(&Flags) = navNoHistory | navNoReadFromCache | navNoWriteToCache;
	URL.vt = VT_BSTR;
	V_BSTR(&URL) = SysAllocString(L"about:blank");
	
	nHomePage = HOMEPAGE_NOTLOADED;
	hr = (bPostIt) ? PostNavigate2(&URL, &Flags, NULL, NULL, NULL) : Navigate2(&URL, &Flags, NULL, NULL, NULL);
	
	VariantClear(&Flags);
	VariantClear(&URL);
	return hr;	
}

HRESULT WebFileInfo::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	HRESULT hr;
	FORMATETC format = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	hr = pDataObject->QueryGetData(&format);
	nDragMode = ((hr == S_OK) ? DROPEFFECT_COPY : DROPEFFECT_NONE);
	*pdwEffect = nDragMode;
	return S_OK;
}

HRESULT WebFileInfo::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	*pdwEffect = nDragMode;
	return S_OK;
}

HRESULT WebFileInfo::DragLeave(void)
{
	nDragMode = DROPEFFECT_NONE;
	return S_OK;
}

HRESULT WebFileInfo::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	STGMEDIUM medium;
	FORMATETC format = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	if (nDragMode)
	{
		HRESULT hr = pDataObject->QueryGetData(&format);
		if (S_OK == hr)
		{
			hr = pDataObject->GetData (&format, &medium);
			if (S_OK == hr)
			{
				wchar_t szFileName[4096] = {0};
				HDROP hdrop = (HDROP)medium.hGlobal;
				if (hdrop && DragQueryFileW(hdrop, 0, szFileName, sizeof(szFileName)/sizeof(wchar_t))) InvokeFileInfo(szFileName);
			}
		}
		nDragMode = DROPEFFECT_NONE;
	}
	return S_OK;
}

static COLORREF GetHTMLColor(int nColorIndex)
{
	COLORREF rgb = WADlg_getColor(nColorIndex);
	return ((rgb >> 16)&0xff|(rgb&0xff00)|((rgb<<16)&0xff0000));
}