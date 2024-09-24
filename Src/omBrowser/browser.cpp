#include "main.h"
#include "./browser.h"
#include "./browserRegistry.h"
#include "./graphics.h"
#include "./resource.h"

#include "../winamp/wa_dlg.h"
#include "../Plugins/General/gen_ml/colors.h"
#include "../winamp/IWasabiDispatchable.h"
#include "../winamp/JSAPI_Info.h"

#include "./obj_ombrowser.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedbrowser.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_omservice.h"
#include "./ifc_omdebugconfig.h"
#include "./travelLogHelper.h"
#include "./menu.h"

#include <wininet.h>
#include <exdisp.h>
#include <exdispid.h>
#include <mshtmdid.h>
#include <mshtml.h>

#include <shlwapi.h>
#include <strsafe.h>

#define CONTROL_DOWNLOADFLAGS		(	DLCTL_DLIMAGES |					\
										DLCTL_VIDEOS |					\
										/*DLCTL_PRAGMA_NO_CACHE |*/		\
										/*DLCTL_NO_CLIENTPULL |	*/		\
										DLCTL_RESYNCHRONIZE |			\
										0)


#define CONTROL_HOSTINFOFLAGS		(	DOCHOSTUIFLAG_DISABLE_HELP_MENU |				\
										DOCHOSTUIFLAG_NO3DBORDER |						\
										DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE |			\
										DOCHOSTUIFLAG_ACTIVATE_CLIENTHIT_ONLY |			\
										DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION |		\
										DOCHOSTUIFLAG_THEME |							\
										DOCHOSTUIFLAG_NOPICS |							\
										DOCHOSTUIFLAG_NO3DOUTERBORDER |					\
										DOCHOSTUIFLAG_DISABLE_UNTRUSTEDPROTOCOL |		\
										DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION |	\
										DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL |	\
										DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE |		\
										DOCHOSTUIFLAG_IME_ENABLE_RECONVERSION |			\
										0)

#ifndef LOAD_LIBRARY_AS_IMAGE_RESOURCE
  #define LOAD_LIBRARY_AS_IMAGE_RESOURCE 0x000000020
#endif //LOAD_LIBRARY_AS_IMAGE_RESOURCE

	/*BHNAVCOMPLETECALLBACK EventDocumentReady;
	BHNAVCOMPLETECALLBACK EventNavigateComplete;
	BHCALLBACK EventDownloadBegin;
	BHCALLBACK EventDownloadComplete;
	BHCALLBACK EventContainerDestroyed;
	BHCMDSTATECALLBACK EventCommandStateChange;
	BHTEXTCALLBACK EventStatusChange;
	BHTEXTCALLBACK EventTitleChange;
    BHCALLBACK  EventSecureLockIconChange;
	BHCREATEPOPUPCALLBACK EventCreatePopup;
	BHBOOLCALLBACK EventVisible;
	BHBOOLCALLBACK EventSetResizable;
	BHCLOSECALLBACK EventWindowClosing;
	BHSHOWUICALLBACK EventShowUiElement;
	BHCLIENTTOHOSTCALLBACK EventClientToHost;
	BHWINDOWPOSCALLBACK EventSetWindowPos;
	BHFOCUSCHANGECALLBACK EventFocusChange;
	BHBOOLCALLBACK EventSetFullscreen;
	BHCALLBACK EventClosePopup;*/

Browser::Browser(obj_ombrowser *browserMngr, HWND winampWindow, HWND hParent) 
		: HTMLContainer2(winampWindow, hParent),
		EventDocumentReady(NULL),
		EventNavigateComplete(NULL),
		EventDownloadBegin(NULL), 
		EventDownloadComplete(NULL),
        EventContainerDestroyed(NULL), 
		EventCommandStateChange(NULL), 
		EventStatusChange(NULL),
		EventTitleChange(NULL),
		EventSecureLockIconChange(NULL), 
		EventCreatePopup(NULL),
		EventVisible(NULL),
		EventSetResizable(NULL),
		EventWindowClosing(NULL),
		EventShowUiElement(NULL),
		EventClientToHost(NULL),
		EventSetWindowPos(NULL),
		EventFocusChange(NULL),
		EventSetFullscreen(NULL),
		EventClosePopup(NULL),
		CallbackGetOmService(NULL),
		CallbackRedirectKey(NULL),
		browserManager(browserMngr), externalDisp(NULL),
		pDropTargetHerlper(NULL), navigationState(0),
		secureLockIcon(secureLockIconUnsecure),
		pszUserAgent(NULL), uiFlags(0)
{
	memset(szDone, 0, sizeof(szDone));

	if (NULL != externalDisp)
		externalDisp->AddRef();

	if (NULL != browserManager)
		browserManager->AddRef();
}

Browser::~Browser()
{
	if (NULL != EventContainerDestroyed)
		EventContainerDestroyed(this);

	if (NULL != externalDisp)
		externalDisp->Release();

	if (NULL != pszUserAgent)
		Plugin_FreeString(pszUserAgent);

	if (NULL != pDropTargetHerlper)
		pDropTargetHerlper->Release();

	if (NULL != browserManager)
		browserManager->Release();
}

HRESULT Browser::SetExternal(IDispatch *pDispatch)
{
	if (NULL != externalDisp)
		externalDisp->Release();

	externalDisp = pDispatch;

	if (NULL != externalDisp)
		externalDisp->AddRef();

	return S_OK;
}

Browser *Browser::CreateInstance(obj_ombrowser *browserManager, HWND winampWindow, HWND hParent)
{
	Browser *instance = new Browser(browserManager, winampWindow, hParent);
	return instance;
}

ULONG Browser::AddRef(void)
{
	return HTMLContainer2::AddRef();
}

ULONG Browser::Release(void)
{
	return HTMLContainer2::Release();
}

STDMETHODIMP Browser::Initialize(BOOL fRegisterAsBrowser)
{	
	HRESULT hr = __super::Initialize();
	if (SUCCEEDED(hr))
	{
		if (FALSE != fRegisterAsBrowser)
		{
			IWebBrowser2 *pWeb2 = NULL;
			if (SUCCEEDED(GetIWebBrowser2(&pWeb2)) && pWeb2 != NULL)
			{
				pWeb2->put_RegisterAsBrowser(VARIANT_TRUE);
				pWeb2->Release();
			}
		}

		szDone[0] = L'\0';
		HINSTANCE hModule;

		if (L'\0' == szDone[0] && NULL != (hModule = LoadLibraryExW(L"ieframe.dll.mui", NULL,
																	LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)))
		{			
			LoadString(hModule, 8169, szDone, ARRAYSIZE(szDone));
			FreeLibrary(hModule);
		}
		
		if (L'\0' == szDone[0] && NULL != (hModule = LoadLibraryExW(L"shdoclc.dll", NULL, 
																	LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)))
		{			
			LoadString(hModule, 8169, szDone, ARRAYSIZE(szDone));
			FreeLibrary(hModule);
		}

		if (L'\0' == szDone[0])
		StringCchPrintf(szDone, ARRAYSIZE(szDone), L"Done");
	}
	return hr;
}

STDMETHODIMP Browser::Finish(void)
{
	IWebBrowser2 *pWeb2 = NULL;
	if (SUCCEEDED(GetIWebBrowser2(&pWeb2)) && pWeb2 != NULL)
	{
		pWeb2->put_RegisterAsBrowser(VARIANT_FALSE);
		pWeb2->Release();
	}
	return HTMLContainer2::Finish();
}

STDMETHODIMP Browser::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	if (IsEqualIID(riid, IID_IDropTarget)) 
	{
		*ppvObject = (IDropTarget*)this;
		((IUnknown*)*ppvObject)->AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IProtectFocus))
	{
		*ppvObject = (IProtectFocus*)this;
		((IUnknown*)*ppvObject)->AddRef();
		return S_OK;
	}
			
	return HTMLContainer2::QueryInterface(riid, ppvObject);
}

STDMETHODIMP Browser::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));

	if (NULL == pDropTargetHerlper && 
		FAILED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (PVOID*)&pDropTargetHerlper)))
	{
		pDropTargetHerlper = NULL;
	}

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y};
		HWND hwnd = this->GetHostHWND();
		pDropTargetHerlper->DragEnter(hwnd, pDataObject, &pt, *pdwEffect);
	}
	return S_OK;
}

STDMETHODIMP Browser::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{	
	*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y};
		pDropTargetHerlper->DragOver(&pt, *pdwEffect);
	}
	return S_OK;
}

STDMETHODIMP Browser::DragLeave(void)
{
	if (NULL != pDropTargetHerlper) 
		pDropTargetHerlper->DragLeave();

	return S_OK;
}

STDMETHODIMP Browser::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y};
		pDropTargetHerlper->Drop(pDataObject, &pt, *pdwEffect);
	}
	return S_OK;
}

STDMETHODIMP Browser::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
	if (NULL == ppDropTarget)
		return E_POINTER;

	HRESULT hr = QueryInterface(IID_IDropTarget, (void**)ppDropTarget);
	if (SUCCEEDED(hr)) 
		return S_OK;

	return HTMLContainer2::GetDropTarget(pDropTarget, ppDropTarget);
}

STDMETHODIMP Browser::GetOverrideKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
	HRESULT hr;
	size_t cbBuffer = 0;
	ifc_ombrowserregistry *browserRegistry = NULL;
	if (NULL != browserManager && SUCCEEDED(browserManager->GetRegistry(&browserRegistry)) && browserRegistry != NULL)
	{
		WCHAR szBuffer[256] = {0};
		if (SUCCEEDED(browserRegistry->GetPath(szBuffer, ARRAYSIZE(szBuffer))) &&
			SUCCEEDED(StringCbLengthW(szBuffer, ARRAYSIZE(szBuffer), &cbBuffer)))
		{
			cbBuffer += sizeof(WCHAR);
			*pchKey = (LPOLESTR)CoTaskMemAlloc(cbBuffer);
			if (NULL != *pchKey)
			{
				hr = StringCbCopyW(*pchKey, cbBuffer, szBuffer);
				if (FAILED(hr))
				{
					CoTaskMemFree(*pchKey);
				}
			}
			else
			{
				hr = E_OUTOFMEMORY;
			}
		}
		else
		{
			hr = E_INVALIDARG;
		}

		browserRegistry->Release();
	}
	else
	{
		hr = E_INVALIDARG;
	}

	if (FAILED(hr))
	{
		*pchKey = NULL;
	}
	return hr;
}

STDMETHODIMP Browser::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	if (ppDispatch && NULL != externalDisp)
	{
		externalDisp->AddRef();
		*ppDispatch = externalDisp;
		return S_OK;
	}
	return HTMLContainer2::GetExternal(ppDispatch);
}

STDMETHODIMP Browser::ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
	if (0 != (flagUiDisableContextMenu & uiFlags))
		return S_OK;

	return S_FALSE;
}

STDMETHODIMP Browser::ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult)
{
	wchar_t szBuffer[256] = {0};
	lpstrCaption = (LPOLESTR)Plugin_LoadString(IDS_OMBROWSER_TITLE, szBuffer, ARRAYSIZE(szBuffer));
	*plResult = MessageBoxW(hwnd, lpstrText, lpstrCaption, dwType);
	return S_OK;
}

STDMETHODIMP Browser::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
	HRESULT hr = S_OK;

    if (NULL != pguidCmdGroup)
	{ 
		if (IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
		{
			switch (nCmdID) 
			{
				case OLECMDID_SHOWSCRIPTERROR:
					{
						ifc_omdebugconfig *debugConfig = NULL;
						HRESULT showError = GetDebugConfig(&debugConfig);
						if (SUCCEEDED(showError) && debugConfig != NULL)
						{
							showError = debugConfig->GetScriptErrorEnabled();
							debugConfig->Release();
						}

						if (S_FALSE == showError)
						{
							OutputDebugStringA("~~<<=== script error\r\n");
							(*pvaOut).vt = VT_BOOL;
							(*pvaOut).boolVal = VARIANT_TRUE;
						}
						else
						{
							hr = OLECMDERR_E_NOTSUPPORTED;
						}
					}
					break;
				default:
					hr = OLECMDERR_E_NOTSUPPORTED;
					break;
			}
		}
		else if (IsEqualGUID(*pguidCmdGroup, CGID_ShellDocView))
		{
			switch (nCmdID) 
			{
				case 53 /*SHDVID_ADDMENUEXTENSIONS*/:
					return S_OK;
			}
		}
		else
		{
			hr = OLECMDERR_E_UNKNOWNGROUP;
		}
	}
	else
	{
		 hr = OLECMDERR_E_UNKNOWNGROUP;
	}

	return hr;
}

STDMETHODIMP Browser::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
	if (IsEqualIID(riid, SID_SProtectFocus))
	{
		*ppv = (IProtectFocus*)this;
		this->AddRef();
		return S_OK;
	}
	else if (IsEqualIID(guidService, SID_SHTMLOMWindowServices))
	{
		if (IsEqualIID(riid, IID_IHTMLOMWindowServices))
		{
			*ppv = (IHTMLOMWindowServices*)this;
			this->AddRef();
			return S_OK;
		}
	}
	else if (IsEqualIID(riid, IID_INewWindowManager))
	{
		*ppv = (INewWindowManager*)this;
		this->AddRef();
		return S_OK;
	}
	return HTMLContainer2::QueryService(guidService, riid, ppv);
}

STDMETHODIMP Browser::AllowFocusChange(BOOL *pfAllow)
{
	if (NULL == pfAllow)
		return E_POINTER;

	if (NULL != EventFocusChange)
	{
		VARIANT_BOOL Allow = VARIANT_TRUE;
		EventFocusChange(this, &Allow);
		if (VARIANT_FALSE == Allow)
		{
			*pfAllow = FALSE;
		}
	}
	
	return S_OK;
}

STDMETHODIMP Browser::moveTo(LONG x, LONG y)
{
	if (NULL == EventSetWindowPos)
		return E_FAIL;

	EventSetWindowPos(this, HTMLContainer2::wndLeft | HTMLContainer2::wndTop, x, y, 0, 0);
	return S_OK;
}

STDMETHODIMP Browser::moveBy(LONG x, LONG y)
{
	if (NULL == EventSetWindowPos)
		return E_FAIL;

	EventSetWindowPos(this, HTMLContainer2::wndLeft | HTMLContainer2::wndTop | HTMLContainer2::wndRelative, x, y, 0, 0);
	return S_OK;
}

STDMETHODIMP Browser::resizeTo(LONG x, LONG y)
{
	if (NULL == EventSetWindowPos)
		return E_FAIL;

	EventSetWindowPos(this, HTMLContainer2::wndWidth | HTMLContainer2::wndHeight, 0, 0, x, y);
	return S_OK;
}

STDMETHODIMP Browser::resizeBy(LONG x, LONG y)
{
	if (NULL == EventSetWindowPos)
		return E_FAIL;

	EventSetWindowPos(this, HTMLContainer2::wndWidth | HTMLContainer2::wndHeight  | HTMLContainer2::wndRelative, 0, 0, x, y);
	return S_OK;
}

STDMETHODIMP Browser::EvaluateNewWindow(LPCWSTR pszUrl, LPCWSTR pszName, LPCWSTR pszUrlContext, LPCWSTR pszFeatures, BOOL fReplace, DWORD dwFlags, DWORD dwUserActionTime)
{
	#ifdef _DEBUG
	char szBuffer[2048] = {0}, szFlags[128] = {0};
	LPSTR cursor = szFlags;
	size_t remaining = ARRAYSIZE(szFlags);
	if (0 != (NWMF_UNLOADING & dwFlags)) StringCchCopyExA(cursor, remaining, " unloading |", &cursor, &remaining, 0);
	if (0 != (NWMF_USERINITED & dwFlags)) StringCchCopyExA(cursor, remaining, " userInited |", &cursor, &remaining, 0);
	if (0 != (0x0004/*NWMF_FIRST_USERINITED*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " first |", &cursor, &remaining, 0);
	if (0 != (NWMF_OVERRIDEKEY & dwFlags)) StringCchCopyExA(cursor, remaining, " overrideKey |", &cursor, &remaining, 0);
	if (0 != (NWMF_SHOWHELP & dwFlags)) StringCchCopyExA(cursor, remaining, " showHelp |", &cursor, &remaining, 0);
	if (0 != (NWMF_HTMLDIALOG & dwFlags)) StringCchCopyExA(cursor, remaining, " htmlDialog |", &cursor, &remaining, 0);
	if (0 != (0x80/*NWMF_USERREQUESTED*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " userRequested |", &cursor, &remaining, 0);
	if (0 != (0x100/*NWMF_USERALLOWED*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " userAllowed |", &cursor, &remaining, 0);
	if (0 != (0x10000/*NWMF_FORCEWINDOW*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " forceWindow |", &cursor, &remaining, 0);
	if (0 != (0x20000/*NWMF_FORCETAB*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " forceTab |", &cursor, &remaining, 0);
	if (0 != (0x40000/*NWMF_SUGGESTWINDOW*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " suggestWindow |", &cursor, &remaining, 0);
	if (0 != (0x80000/*NWMF_SUGGESTTAB*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " suggestTab |", &cursor, &remaining, 0);
	if (0 != (0x100000/*NWMF_INACTIVETAB*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " inactiveTab |", &cursor, &remaining, 0);

	if (cursor != szFlags)
	{	
		cursor -= 2;
		*cursor = '\0';
	}
	else
	{
		StringCchCopyExA(cursor, remaining, " <none>", &cursor, &remaining, 0);
	}

	StringCchPrintfA(szBuffer, ARRAYSIZE(szBuffer), "EvaluateNewWindow:\r\n\turlContext: %S,\r\n\turl: %S,\r\n\tflags:%s\r\n", 
		((NULL == pszUrlContext || L'\0' == *pszUrlContext) ? L"<empty>" : pszUrlContext),
		((NULL == pszUrl || L'\0' == *pszUrl) ? L"<empty>" : pszUrl), 
		szFlags);

	OutputDebugStringA(szBuffer);
#endif // _DEBUG

	if (0 != (NWMF_UNLOADING & dwFlags))
	{ 
		return S_FALSE;
	}

	return S_OK;
}

OLECHAR *Browser::OnGetHostCSS(void)
{
	if (0 != (flagUiDisableHostCss & uiFlags))
		return NULL;

	ifc_skinnedbrowser *skinnedBrowser = NULL;
	if (FAILED(Plugin_GetBrowserSkin(&skinnedBrowser)))
		return NULL;

	OLECHAR *hostCss = NULL;
	if (FAILED(skinnedBrowser->GetHostCss(&hostCss)))
		hostCss = NULL;

	skinnedBrowser->Release();
	return hostCss;
}

COLORREF Browser::OnGetHostBkColor(void)
{
	if (0 != (flagUiDisableHostCss & uiFlags))
		return GetSysColor(COLOR_WINDOW);

	COLORREF rgbBk;
	ifc_skinhelper *skin = NULL;
	if (SUCCEEDED(Plugin_GetSkinHelper(&skin)) && skin != NULL)
	{
		skin->GetColor(WADLG_ITEMBG, &rgbBk);
		skin->Release();
	}

	if (FAILED(rgbBk))
		rgbBk = GetSysColor(COLOR_WINDOW);

	return rgbBk;
}

DWORD Browser::OnGetHostInfoFlags(void)
{
	DWORD flags = CONTROL_HOSTINFOFLAGS;
	
	if (0 != (flagUiDisableScroll & uiFlags))
		flags |= DOCHOSTUIFLAG_SCROLL_NO;

	if (0 != (flagUiDialogMode & uiFlags))
		flags |= DOCHOSTUIFLAG_DIALOG;

	return flags;
}

DWORD Browser::OnGetDownlodFlags(void)
{
	return CONTROL_DOWNLOADFLAGS;
}

HRESULT Browser::GetExternalName(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	pszBuffer[0] = L'\0';
	
	IDocHostUIHandler *pHandler = NULL;
	HRESULT hr = QueryInterface(IID_IDocHostUIHandler, (void**)&pHandler);
	if (SUCCEEDED(hr) && pHandler != NULL)
	{
		IDispatch *pDispatch = NULL;
		hr = pHandler->GetExternal(&pDispatch);
		if (SUCCEEDED(hr) && NULL != pDispatch)
		{
			IWasabiDispatchable *pWasabi = NULL;
			hr = pDispatch->QueryInterface(IID_IWasabiDispatchable, (void**)&pWasabi);
			if (SUCCEEDED(hr) && pWasabi != NULL)
			{
				JSAPI::ifc_info *pInfo = NULL;
				hr = pWasabi->QueryDispatchable(JSAPI::IID_JSAPI_ifc_info, (Dispatchable**)&pInfo);
				if (SUCCEEDED(hr) && pInfo != NULL)
				{
					LPCWSTR p = pInfo->GetUserAgent();
					if (NULL != p && L'\0' != *p)
						StringCchCopy(pszBuffer, cchBufferMax, p);

					pInfo->Release();
				}
				pWasabi->Release();
			}
			pDispatch->Release();
		}
		pHandler->Release();
	}
	return S_OK;
}

LPCWSTR Browser::OnGetUserAgent(void)
{
	if (NULL == pszUserAgent)
	{
		BSTR version = NULL;

		if (SUCCEEDED(GetUserAgent(&version)) && NULL != version)
		{
			WCHAR szExternal[128] = {0};
			size_t cchExternal = 0;

			if (FAILED(GetExternalName(szExternal, ARRAYSIZE(szExternal)))
				|| FAILED(StringCchLength(szExternal, ARRAYSIZE(szExternal), &cchExternal)))
			{
				cchExternal = 0;
			}
			
			INT cchVersion = (NULL != version) ? SysStringLen(version) : 0;
			INT cchBufferMax = cchVersion + (INT)cchExternal;
			if (0 != cchExternal)
				cchBufferMax += 4; // for ", "

			if (cchBufferMax > 0)
			{
				pszUserAgent = Plugin_MallocString(cchBufferMax);
				if (NULL != pszUserAgent)
				{
					HRESULT hr = S_OK;
					LPWSTR cursor = pszUserAgent;
					size_t remaining = cchBufferMax;

					if (L'\0' != *version && SUCCEEDED(hr))
					{
						hr = StringCchCopyEx(cursor, remaining,
											 version, &cursor,
											 &remaining,
											 STRSAFE_NULL_ON_FAILURE);
					}

					if (SUCCEEDED(hr) && 0 != cchExternal)
					{
						BOOL needCloseBracket = FALSE;

						if (cursor > pszUserAgent) 
						{
							if (L')' == *(cursor - 1))
							{
								*(cursor - 1) = L',';
								needCloseBracket = TRUE;
							}

							hr = StringCchCopyEx(cursor, remaining, 
												 L" ", &cursor,
												 &remaining,
												 STRSAFE_NULL_ON_FAILURE);
						}

						if (SUCCEEDED(hr))
						{
							hr = StringCchCopyEx(cursor, remaining,
												 szExternal, &cursor,
												 &remaining,
												 STRSAFE_NULL_ON_FAILURE);
						}

						if (SUCCEEDED(hr) && FALSE != needCloseBracket)
						{
							hr = StringCchCopyEx(cursor, remaining, L")",
												 &cursor, &remaining,
												 STRSAFE_NULL_ON_FAILURE);
						}

						if (FAILED(hr))
						{
							hr = S_OK;
						}
					}

					if (FAILED(hr))
					{
						Plugin_FreeString(pszUserAgent);
						pszUserAgent = NULL;
					}
				}
			}

			if (NULL != version)
				SysFreeString(version);
		}
	}

	return pszUserAgent;
}

HRESULT Browser::SendCommand(INT commandId)
{
	IWebBrowser2 *pWeb2 = NULL;
	HRESULT hr = GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr) && pWeb2 != NULL)
	{
		switch(commandId)
		{
			case Browser::commandBack:
				hr = pWeb2->GoBack();
				break;
			case Browser::commandForward:
				hr = pWeb2->GoForward();
				break;
			case Browser::commandStop:
				hr = pWeb2->Stop();
				break;
			case Browser::commandRefresh:
				hr = pWeb2->Refresh();
				break;
			case Browser::commandRefreshCompletely:
				{
					VARIANT param;
					VariantInit(&param);
					V_VT(&param) = VT_I4;
					V_I4(&param) = REFRESH_COMPLETELY;
					hr = pWeb2->Refresh2(&param);
				}
				break;
			default:
				hr = E_INVALIDARG;
				break;
		}
		pWeb2->Release();
	}
	return hr;
}

#define POSTAPPCMD(/*HWND*/ __hwndTarget, /*HWND*/__hwndSource, /*INT*/__commandId)\
	(PostMessage((__hwndTarget), WM_APPCOMMAND, (WPARAM)(__hwndSource), MAKELPARAM(0, FAPPCOMMAND_KEY | (__commandId))))

BOOL Browser::TranslateKey(LPMSG pMsg)
{
	UINT vKey = (UINT)pMsg->wParam;
	BOOL redirectKey = TRUE;
    if (0 != (0x8000 & GetAsyncKeyState(VK_MENU)))
	{
		if (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
		{
			redirectKey = FALSE;
		}
		else
		{
			switch(vKey)
			{
				case VK_TAB:
				case VK_BACK:
				case VK_UP:
				case VK_DOWN:
				case VK_LEFT:
				case VK_RIGHT:
				case VK_NEXT:
				case VK_PRIOR:
				case VK_END:
				case VK_INSERT:
				case VK_DELETE:
					redirectKey = FALSE;
					break;
			}
		}
	}
	else if (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
	{
		switch(vKey)
		{
			case VK_BACK:
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_NEXT:
			case VK_PRIOR:
			case VK_HOME:
			case VK_END:
			case VK_INSERT:
			case VK_DELETE:
			case VK_F5:
			case 'R':
			case 'X':
			case 'C':
			case 'V':
			case 'A':
			case 'Z':
			case 'Y':
				redirectKey = FALSE;
				break;
		}
	}
	else
	{
		if (WM_KEYDOWN == pMsg->message && VK_ESCAPE == vKey)
		{
			POSTAPPCMD(hParent, pMsg->hwnd, APPCOMMAND_BROWSER_STOP);
			return TRUE;
		}

		redirectKey = FALSE;
	}

	if (FALSE != redirectKey && NULL != CallbackRedirectKey && FALSE != CallbackRedirectKey(this, pMsg))
	{
		return TRUE;
	}

	return __super::TranslateKey(pMsg);
}

STDMETHODIMP Browser::TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
	return  E_NOTIMPL; // override HtmlContainer2 cause we already filtered keys
}

static INT OLECMDFTOCOMMANDSTATE(OLECMDF cmdf)
{
	INT state = 0;
	if ( 0 != (OLECMDF_SUPPORTED & cmdf))
		state |= Browser::commandStateSupported;
	if ( 0 != (OLECMDF_ENABLED & cmdf))
		state |= Browser::commandStateEnabled;
	if ( 0 != (OLECMDF_LATCHED & cmdf))
		state |= Browser::commandStateLatched;

	return state;
}

HRESULT Browser::QueryCommandState(INT commandId, INT *commandState)
{	
	if (NULL == commandState)
		return E_INVALIDARG;

	HRESULT hr(S_OK);
	*commandState = 0;
	IWebBrowser2 *pWeb2 = NULL;
	OLECMDF cmdf = (OLECMDF)0;

	switch(commandId)
	{			
		case Browser::commandBack:
			if (0 != (Browser::navigationBackEnabled & navigationState))
				*commandState |= (Browser::commandStateSupported | Browser::commandStateEnabled);
			break;
		case Browser::commandForward:
			if (0 != (Browser::navigationForwardEnabled & navigationState))
				*commandState |= (Browser::commandStateSupported | Browser::commandStateEnabled);
			break;
		case Browser::commandStop:
			hr = GetIWebBrowser2(&pWeb2);
			if (SUCCEEDED(hr) && pWeb2 != NULL)
			{
				hr = pWeb2->QueryStatusWB(OLECMDID_STOP, &cmdf);
				if (SUCCEEDED(hr))
					*commandState = OLECMDFTOCOMMANDSTATE(cmdf);
				pWeb2->Release();
			}
			break;
		case Browser::commandRefresh:
			hr = GetIWebBrowser2(&pWeb2);
			if (SUCCEEDED(hr) && pWeb2 != NULL)
			{
				hr = pWeb2->QueryStatusWB(OLECMDID_REFRESH, &cmdf);
				if (SUCCEEDED(hr))
					*commandState = OLECMDFTOCOMMANDSTATE(cmdf);
				pWeb2->Release();
			}
			break;
	}

	return hr;
}

void Browser::OnBeforeNavigate(IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
{	
	HTMLContainer2::OnBeforeNavigate(pDispatch, URL, Flags, TargetFrameName, PostData, Headers, Cancel);
}

void Browser::OnDownloadBegin(void)
{
	HTMLContainer2::OnDownloadBegin();

	if (NULL != EventDownloadBegin)
		EventDownloadBegin(this);
}

void Browser::OnDownloadComplete(void)
{
	HTMLContainer2::OnDownloadComplete();

	if (NULL != EventDownloadComplete)
		EventDownloadComplete(this);
}

void Browser::OnNavigateComplete(IDispatch *pDispatch, VARIANT *URL)
{
	HTMLContainer2::OnNavigateComplete(pDispatch, URL);

	if (NULL != EventNavigateComplete)
		EventNavigateComplete(this, pDispatch, URL);
}

HRESULT Browser::GetErrorPageName(LPWSTR pszBuffer, HRESULT cchBufferMax, UINT errorCode, BOOL fCancel)
{
	WCHAR szPath[MAX_PATH] = {0}, szTemp[MAX_PATH] = {0};
	LPCWSTR pszFile = NULL;

	ifc_omdebugconfig *debugConfig = NULL;
	if (SUCCEEDED(GetDebugConfig(&debugConfig)) && debugConfig != NULL)
	{
		debugConfig->GetBrowserPath(szPath, ARRAYSIZE(szPath));
		debugConfig->Release();
	}
			
	if (FALSE == fCancel)
	{
		pszFile = (errorCode < 0x800C0000) ? L"httpError.htm" : L"dnsError.htm";
	}
	else
	{
		pszFile = (errorCode == -1 ? L"inetDisabled.htm" : L"navCancel.htm");
	}
		
	if (L'0' != szPath[0] && 
		FALSE != PathCombine(szTemp, szPath, pszFile) &&
		FALSE != PathFileExists(szTemp))
	{
		return StringCchCopy(pszBuffer, cchBufferMax, szTemp);
	}
		
	HINSTANCE hModule = Plugin_GetLangInstance();
	if (NULL != hModule && 
		NULL == FindResource(hModule, pszFile, /*RT_HTML*/MAKEINTRESOURCEW(23)))
	{
		hModule = NULL;
	}

	if (NULL == hModule)
	{
		hModule = Plugin_GetInstance();
		if (NULL != hModule && 
			NULL == FindResource(hModule, pszFile, /*RT_HTML*/MAKEINTRESOURCEW(23)))
		{
			hModule = NULL;
		}
	}

	if (NULL != hModule)
	{			
		if (0 == GetModuleFileName(hModule, szPath, ARRAYSIZE(szPath)))
			return E_FAIL;

		return StringCchPrintf(pszBuffer, cchBufferMax, L"res://%s/%s", szPath, pszFile);
	}

	return E_NOTIMPL;
}

static HRESULT Browser_FormatDefaultErrorMessage(LPWSTR pszBuffer, INT cchBufferMax, UINT errorCode, LPCWSTR pszUrl, BOOL fCancel)
{
	HRESULT hr;
	if (FALSE != fCancel)
	{
		hr = StringCchCopy(pszBuffer, cchBufferMax, 
						   L"about:<HEAD><title>Service page load cancelled</title></Head><Body><H3>Service page load canncelled</H3></Body>");
	}
	else
	{
		WCHAR szStatus[32] = {0};
		hr = StringCchPrintf(szStatus, ARRAYSIZE(szStatus), ((errorCode < 0x800C0000) ? L"HTTP-%d" : L"0x%08X"), errorCode);
		
		if (FAILED(hr))
			szStatus[0] = L'\0';

		hr = StringCchPrintf(pszBuffer, cchBufferMax, 
							 L"about:<HEAD><title>Service load error</title></Head><Body><H3>Online Media cannot load service page</H3><p>URL: %s<br>Code: %s</p></Body>",
							 ((NULL != pszUrl && L'\0' != *pszUrl) ? pszUrl : L"Unknown"), szStatus);
	}
	return hr;
}

HRESULT Browser::FormatErrorParam(LPWSTR pszBuffer, INT cchBufferMax, UINT errorCode, LPCWSTR pszUrl)
{
	LPWSTR cursor = pszBuffer;
	size_t remaining = cchBufferMax;

	StringCchCopyEx(cursor, remaining, L"#", &cursor,&remaining, STRSAFE_NULL_ON_FAILURE);

	if (((UINT)-1) != errorCode)
		StringCchPrintfEx(cursor, remaining, &cursor,&remaining, STRSAFE_NULL_ON_FAILURE, L"errorcode=%d&", errorCode);

	if (NULL != pszUrl && L'\0' != *pszUrl)
		StringCchPrintfEx(cursor, remaining, &cursor,&remaining, STRSAFE_NULL_ON_FAILURE, L"url=%s&", pszUrl);

	ifc_omservice *service = NULL;
	if (NULL == CallbackGetOmService || FAILED(CallbackGetOmService(this, &service)))
		service = NULL;

	if (NULL != service)
	{
		StringCchPrintfEx(cursor, remaining, &cursor,&remaining, STRSAFE_NULL_ON_FAILURE, L"svcid=%d&", service->GetId());

		WCHAR szName[256] = {0};
		if (SUCCEEDED(service->GetName(szName, ARRAYSIZE(szName))) && L'\0' != *szName)
		{
			WCHAR szNameEscaped[ARRAYSIZE(szName)] = {0};
			DWORD cchName = ARRAYSIZE(szNameEscaped);
			if (SUCCEEDED(UrlEscape(szName, szNameEscaped, &cchName, URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_PERCENT)) && 0 != cchName)
				StringCchPrintfEx(cursor, remaining, &cursor,&remaining, STRSAFE_NULL_ON_FAILURE, L"servicename=%s&", szNameEscaped);
		}

		service->Release();
	}

	WCHAR szClient[128] = {0};
	if (NULL != browserManager && SUCCEEDED(browserManager->GetClientId(szClient, ARRAYSIZE(szClient))))
		StringCchPrintfEx(cursor, remaining, &cursor,&remaining, STRSAFE_NULL_ON_FAILURE, L"uniqueid=%s&", szClient);

	if (cursor > pszBuffer)
		*(--cursor) = L'\0';

	return S_OK;
}

void Browser::OnNavigateError(IDispatch *pDispatch, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel)
{
	UINT errorCode = (NULL != StatusCode && VT_I4 == StatusCode->vt) ? StatusCode->lVal : 0;
	if (200 == errorCode || 0 == errorCode)
	{
		return;
	}

	HTMLContainer2::OnNavigateError(pDispatch, URL, TargetFrameName, StatusCode, Cancel);

	WCHAR szUrl[INTERNET_MAX_URL_LENGTH] = {0};
	WCHAR szBuffer[INTERNET_MAX_URL_LENGTH] = {0};

	DWORD cchUrl = ARRAYSIZE(szUrl);
	DWORD cchBuffer = ARRAYSIZE(szBuffer);

	if (FAILED(GetErrorPageName(szUrl, ARRAYSIZE(szUrl), errorCode, FALSE)) ||
		FAILED(UrlEscape(szUrl, szBuffer, &cchBuffer, URL_ESCAPE_SPACES_ONLY | URL_DONT_ESCAPE_EXTRA_INFO)))
	{
		szBuffer[0] = L'\0';
		cchBuffer = 0;
	}

	if (NULL == URL || VT_BSTR != URL->vt || NULL == URL->bstrVal || L'\0' == *URL->bstrVal || 
		FAILED(UrlEscape(URL->bstrVal, szUrl, &cchUrl, URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_PERCENT)))
	{
		szUrl[0] = L'\0';
	}

	if (0 != cchBuffer)
	{		
		FormatErrorParam(szBuffer + cchBuffer, ARRAYSIZE(szBuffer) - cchBuffer, errorCode, szUrl);
	}
	else
	{
		if(FAILED(Browser_FormatDefaultErrorMessage(szBuffer, ARRAYSIZE(szBuffer), errorCode, szUrl, FALSE)))
			return;
	}

	IWebBrowser2 *pWeb2 = NULL;
	if (FAILED(GetIWebBrowser2(&pWeb2)))
		return;

	INT frameCount = 0;
	if (FAILED(GetFramesCount(pWeb2, &frameCount)))
		frameCount = 0;
	
	BOOL invokeDlComplete = FALSE;

	if (0 == frameCount)
	{
        pWeb2->Stop();
		BSTR currentUrl;
		if (FAILED(pWeb2->get_LocationURL(&currentUrl)))
			currentUrl = NULL;

		if (NULL != currentUrl &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, currentUrl, -1, szBuffer, -1))
		{
			*Cancel = VARIANT_TRUE;
			invokeDlComplete = TRUE;
		}
		else
		{
			if (FAILED(PostNavigateToName(szBuffer, navNoHistory)))
				NavigateToName(szBuffer, navNoHistory);
		}

		if (NULL != currentUrl)
			SysFreeString(currentUrl);
	}
	else
	{
		*Cancel = VARIANT_TRUE;
		invokeDlComplete = TRUE;

		IWebBrowser2* pWebActive;
		if (NULL != pDispatch && 
			SUCCEEDED(pDispatch->QueryInterface(IID_IWebBrowser2, (void**)&pWebActive)))
		{			
			NavigateToNameEx(pWebActive, szBuffer, navNoHistory);
			invokeDlComplete = FALSE;
			pWebActive->Release();
		}
	}

	if (FALSE != invokeDlComplete)
	{
		DISPPARAMS params;
		ZeroMemory(&params, sizeof(DISPPARAMS));
		Invoke(DISPID_DOWNLOADCOMPLETE, GUID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, NULL);
	}

	pWeb2->Release();
}

void Browser::OnNavigateCancelled(LPCWSTR pszUrl, VARIANT_BOOL *Cancel)
{
	HTMLContainer2::OnNavigateCancelled(pszUrl, Cancel);

	WCHAR szBuffer[2048] = {0};
	if (SUCCEEDED(GetErrorPageName(szBuffer, ARRAYSIZE(szBuffer), (*Cancel == ((VARIANT_BOOL)-2) ? -1 : 0), TRUE)))
	{
		INT cchLen = lstrlen(szBuffer);
		FormatErrorParam(szBuffer + cchLen, ARRAYSIZE(szBuffer) - cchLen, 0, pszUrl);
	}
	else
	{
		if(FAILED(Browser_FormatDefaultErrorMessage(szBuffer, ARRAYSIZE(szBuffer), 0, pszUrl, TRUE)))
			return;
	}

	*Cancel = VARIANT_TRUE;
	NavigateToName(szBuffer, navNoHistory);
}

void Browser::OnDocumentReady(IDispatch *pDispatch, VARIANT *URL)
{
	HTMLContainer2::OnDocumentReady(pDispatch, URL);
	if (NULL != EventDocumentReady)
		EventDocumentReady(this, pDispatch, URL);
}

void Browser::OnCommandStateChange(LONG commandId, VARIANT_BOOL Enable)
{
	HTMLContainer2::OnCommandStateChange(commandId, Enable);

	switch(commandId)
	{
		case CSC_NAVIGATEBACK:
			if (VARIANT_TRUE == Enable)
				navigationState |= navigationBackEnabled;
			else
				navigationState &= ~3;
			
			if (NULL != EventCommandStateChange)
				EventCommandStateChange(this, commandBack, 0 != (navigationBackEnabled & navigationState));
			break;

		case CSC_NAVIGATEFORWARD:
			if (VARIANT_TRUE == Enable)
				navigationState |= navigationForwardEnabled;
			else
				navigationState &= ~navigationForwardEnabled;
			
			if (NULL != EventCommandStateChange)
				EventCommandStateChange(this, commandForward, 0 != (navigationForwardEnabled & navigationState));
			break;

		case CSC_UPDATECOMMANDS:
			if (NULL != EventCommandStateChange)
			{
				INT state = 0;

				if (SUCCEEDED(QueryCommandState(commandStop, &state)) && 0 != (commandStateSupported & state))
					EventCommandStateChange(this, commandStop, 0 != (commandStateEnabled & state));
				if (SUCCEEDED(QueryCommandState(commandRefresh, &state)) && 0 != (commandStateSupported & state))
					EventCommandStateChange(this, commandRefresh, 0 != (commandStateEnabled & state));
			}
			break;
	}
}

void Browser::OnStatusTextChange(LPCWSTR pszText)
{
	HTMLContainer2::OnStatusTextChange(pszText);

	if (NULL != EventStatusChange)
	{
		if (NULL != pszText &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, szDone, -1, pszText, -1))
		{
			pszText = NULL;
		}
		EventStatusChange(this, pszText);
	}
}

void Browser::OnSetSecureLockIcon(UINT secureLockIcon)
{
	HTMLContainer2::OnSetSecureLockIcon(secureLockIcon);

	this->secureLockIcon = secureLockIcon;

	if (NULL != EventSecureLockIconChange)
		EventSecureLockIconChange(this);
}

void Browser::OnTitleChange(BSTR pszText)
{
	if (NULL != EventTitleChange)
		EventTitleChange(this, pszText);
}

void Browser::OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
#ifdef _DEBUG
	char szBuffer[2048], szFlags[128] = {0};
	LPSTR cursor = szFlags;
	size_t remaining = ARRAYSIZE(szFlags);
	if (0 != (NWMF_UNLOADING & dwFlags)) StringCchCopyExA(cursor, remaining, " unloading |", &cursor, &remaining, 0);
	if (0 != (NWMF_USERINITED & dwFlags)) StringCchCopyExA(cursor, remaining, " userInited |", &cursor, &remaining, 0);
	if (0 != (0x0004/*NWMF_FIRST_USERINITED*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " first |", &cursor, &remaining, 0);
	if (0 != (NWMF_OVERRIDEKEY & dwFlags)) StringCchCopyExA(cursor, remaining, " overrideKey |", &cursor, &remaining, 0);
	if (0 != (NWMF_SHOWHELP & dwFlags)) StringCchCopyExA(cursor, remaining, " showHelp |", &cursor, &remaining, 0);
	if (0 != (NWMF_HTMLDIALOG & dwFlags)) StringCchCopyExA(cursor, remaining, " htmlDialog |", &cursor, &remaining, 0);
	if (0 != (0x80/*NWMF_USERREQUESTED*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " userRequested |", &cursor, &remaining, 0);
	if (0 != (0x100/*NWMF_USERALLOWED*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " userAllowed |", &cursor, &remaining, 0);
	if (0 != (0x10000/*NWMF_FORCEWINDOW*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " forceWindow |", &cursor, &remaining, 0);
	if (0 != (0x20000/*NWMF_FORCETAB*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " forceTab |", &cursor, &remaining, 0);
	if (0 != (0x40000/*NWMF_SUGGESTWINDOW*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " suggestWindow |", &cursor, &remaining, 0);
	if (0 != (0x80000/*NWMF_SUGGESTTAB*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " suggestTab |", &cursor, &remaining, 0);
	if (0 != (0x100000/*NWMF_INACTIVETAB*/ & dwFlags)) StringCchCopyExA(cursor, remaining, " inactiveTab |", &cursor, &remaining, 0);

	if (cursor != szFlags)
	{	
		cursor -= 2;
		*cursor = '\0';
	}
	else
	{
		StringCchCopyExA(cursor, remaining, " <none>", &cursor, &remaining, 0);
	}

	StringCchPrintfA(szBuffer, ARRAYSIZE(szBuffer), "NewWindow3:\r\n\turlContext: %S,\r\n\turl: %S,\r\n\tflags:%s\r\n", 
					 ((NULL == bstrUrlContext || L'\0' == *bstrUrlContext) ? L"<empty>" : bstrUrlContext),
					 ((NULL == bstrUrl || L'\0' == *bstrUrl) ? L"<empty>" : bstrUrl), szFlags);

	OutputDebugStringA(szBuffer);
#endif // _DEBUG
}

void Browser::OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel)
{
	if (NULL != EventCreatePopup)
	{
		EventCreatePopup(this, ppDisp, Cancel);
	}
	else if (NULL != Cancel)
	{
		*Cancel = VARIANT_TRUE;
	}
}

void Browser::OnVisibleChange(VARIANT_BOOL Visible)
{
	if (NULL != EventVisible)
	{
		EventVisible(this, Visible);
	}
}

void Browser::OnWindowClosing(VARIANT_BOOL IsChildWindow, VARIANT_BOOL *Cancel)
{
	if (NULL != EventWindowClosing)
	{
		EventWindowClosing(this, IsChildWindow, Cancel);
	}
}

void Browser::OnShowUiElement(UINT elementId, VARIANT_BOOL fShow)
{
	HTMLContainer2::OnShowUiElement(elementId, fShow);
	if (NULL != EventShowUiElement)
	{
		EventShowUiElement(this, elementId, fShow);
	}
}

void Browser::OnWindowSetResizable(VARIANT_BOOL Enable)
{
	if (NULL != EventSetResizable)
	{
		EventSetResizable(this, Enable);
	}
}

void Browser::OnEnableFullscreen(VARIANT_BOOL Enable)
{
	if (NULL != EventSetFullscreen)
	{
		EventSetFullscreen(this, Enable);
	}
}

void Browser::OnClientToHostWindow(LONG *CX, LONG *CY)
{
	if (NULL != EventClientToHost)
	{
		EventClientToHost(this, CX, CY);
	}
}

void Browser::OnSetWindowPos(UINT Flags, LONG X, LONG Y, LONG CX, LONG CY)
{
	if (NULL != EventSetWindowPos)
	{
		EventSetWindowPos(this, Flags, X, Y, CX, CY);
	}
}

HANDLE Browser::InitializePopupHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hHook = NULL;
	BSTR version = NULL;
	if (SUCCEEDED(GetAppVersion(&version)) && NULL != version)
	{			
	//	if (NULL == StrStrIW(version, L"Trident/4.0") /*IE8*/ && 
	//		NULL == StrStrIW(version, L"Trident/5.0") /*IE9*/)
		{
			hHook = Menu_InitializeHook(hwnd, NULL);
		}

		SysFreeString(version);
	}
	return hHook;
}

void Browser::DeletePopupHook(HANDLE hHook)
{
	Menu_RemoveHook(hHook);
}

void Browser::InitializeMenuPopup(HWND hwnd, HMENU hMenu, INT iPos, BOOL fWindowMenu)
{
	UINT szItems[] = {	2263 /*Save Background As...*/,
						2264 /*Set as Background*/,
						2265 /*Copy Background*/,
						2266 /*Create Shortcut*/,
						2261 /*Add to Favorite*/,
						2268 /*Save Target As...*/,
						2269 /*Show Picture*/,
						2270 /*Save Picture As...*/,
						2288 /*Email Picture*/,
						2289 /*Print Picture*/,
						2287 /*Goto My Picures*/,
						2278 /*Set as Desktop Item...*/,
						2435 /*Open in New Tab*/,
					};

	if (NULL == hMenu)
		return;

	ifc_omdebugconfig *debugConfig = NULL;
	if (SUCCEEDED(GetDebugConfig(&debugConfig)) && debugConfig != NULL)
	{
		HRESULT hr = debugConfig->GetMenuFilterEnabled();
		debugConfig->Release();
		if (S_FALSE == hr) return;
	}

	// remove known items
	INT i;
	for (i = 0; i < ARRAYSIZE(szItems); i++)
	{
		DeleteMenu(hMenu, szItems[i], MF_BYCOMMAND);
	}	

	// fix separators and remove extensions
	i = GetMenuItemCount(hMenu);

	MENUITEMINFOW mi = {0};
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_SUBMENU;

	INT separatorIndex = i;
	while(i--)
	{
		if (GetMenuItemInfoW(hMenu, i, TRUE, &mi))
		{
			if (0 != (MFT_SEPARATOR & mi.fType))
			{
				if ((i + 1) == separatorIndex)
				{
					DeleteMenu(hMenu, i, MF_BYPOSITION);
				}
				separatorIndex = i;
			}
			else if (mi.wID >= 3700 && NULL == mi.hSubMenu)
			{
				DeleteMenu(hMenu, i, MF_BYPOSITION);
				if (separatorIndex > i)
					separatorIndex--;
			}
		}
	}
}

void Browser::SetUiFlags(UINT flags, UINT mask)
{
	uiFlags = (uiFlags & ~mask) | (flags & mask);
}

UINT Browser::GetUiFlags(UINT mask)
{
	return (mask & uiFlags);
}

HRESULT Browser::ToggleFullscreen()
{
	IWebBrowser2 *pWeb2 = NULL;
	HRESULT hr = GetIWebBrowser2(&pWeb2);
	if (FAILED(hr)) return hr;

	VARIANT_BOOL fullscreenMode;
	
	hr = pWeb2->get_FullScreen(&fullscreenMode);
	if (SUCCEEDED(hr))
	{
		if (VARIANT_FALSE == fullscreenMode) fullscreenMode = VARIANT_TRUE;
		else if (VARIANT_TRUE == fullscreenMode) fullscreenMode = VARIANT_FALSE;
		else hr = E_UNEXPECTED;

		if (SUCCEEDED(hr))
		{
			hr = pWeb2->put_FullScreen(fullscreenMode);
			if (SUCCEEDED(hr))
			{
			}
		}
	}

	pWeb2->Release();
	return hr;
}

HRESULT Browser::GetDebugConfig(ifc_omdebugconfig **debugConfig)
{
	if (NULL == debugConfig) return E_POINTER;
	if (NULL == browserManager || FAILED(browserManager->GetConfig(&IFC_OmDebugConfig, (void**)debugConfig)))
	{
		*debugConfig = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

HRESULT Browser::GetTravelLog(ifc_travelloghelper **travelLog)
{
	if (NULL == travelLog) return E_POINTER;

	IWebBrowser2 *pWeb2 = NULL;
	HRESULT hr = GetIWebBrowser2(&pWeb2);
	if (FAILED(hr))
	{
		*travelLog = NULL;
		return hr;
	}

	hr = TravelLogHelper::CreateInstance(pWeb2, (TravelLogHelper**)travelLog);
	pWeb2->Release();

	return hr;
}

BOOL Browser::InputLangChangeRequest(HWND hwnd, UINT flags, HKL hkl)
{
	HWND hTarget = GetParent(hwnd);
	if (NULL != hTarget) 
	{
		SendMessage(hTarget, WM_INPUTLANGCHANGEREQUEST, (WPARAM)flags, (LPARAM)hkl);
		return TRUE;
	}
	return FALSE;
}

void Browser::InputLangChange(UINT charset, HKL hkl)
{
	ActivateKeyboardLayout(hkl, KLF_SETFORPROCESS);
}

void Browser::OnClosePopupInternal()
{
	if(NULL != EventClosePopup)
		EventClosePopup(this);
}

#ifdef _DEBUG
void BrowserDebug_PrintRefs(Browser *browser)
{
	if (NULL == browser)
	{
		aTRACE_LINE("browser object is NULL");
		return;
	}

	browser->AddRef();
	ULONG refBrowser, refUnknown, refWeb;

	IUnknown *pUnk = NULL;
	if (SUCCEEDED(browser->GetIUnknown(&pUnk)) && pUnk != NULL)
	{
		refUnknown = pUnk->Release();
	}
	else 
	{
		refUnknown = ((ULONG)(0 - 2));
	}

	IWebBrowser2 *pWeb2 = NULL;
	if (SUCCEEDED(browser->GetIWebBrowser2(&pWeb2)) && pWeb2 != NULL)
	{
		refWeb = pWeb2->Release();
	}
	else 
	{
		refWeb = ((ULONG)(0 - 2));
	}
	refBrowser = browser->Release();

	aTRACE_FMT("Browser Stats: Instance=%d, IUnknown=%d, IWebBrowser2=%d\r\n", refBrowser, refUnknown, refWeb);
}
#endif // _DEBUG