#include <precomp.h>
#include "iebrowser.h"
#include "mbsvc.h"
#include "main.h"
#include "../nu/ns_wc.h"
#include "../Winamp/buildtype.h"
#include <api/config/items/cfgitem.h>
#include <wa2frontend.h>
#include <windows.h>
#include <Mshtml.h>
#include "minibrowserCOM.h"

class BrowserMsgProc: public ifc_messageprocessor
{
public: 	
	BrowserMsgProc(BrowserWnd *pBrowser) : pTarget(pBrowser) {} 
	~BrowserMsgProc(void)  {} 

public:
	bool ProcessMessage(MSG *pMsg)
	{
		if (WM_KEYFIRST <= pMsg->message && WM_KEYLAST >= pMsg->message)
		{
			HWND hwndHost;
			hwndHost = pTarget->gethWnd();
			if ((hwndHost == pMsg->hwnd || IsChild(hwndHost, pMsg->hwnd)) && IsWindowVisible(pMsg->hwnd))
			{
				if (!(GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_MENU)&0x8000))
				{
					if (pTarget->TranslateKey(pMsg)) return true;
				}
				switch(pMsg->message)
				{
					case WM_KEYDOWN:
					case WM_SYSKEYDOWN:
					case WM_KEYUP:
					case WM_SYSKEYUP:
						switch(pMsg->wParam)
						{
							case VK_F1:
								if (!(GetAsyncKeyState(VK_SHIFT)&0x8000)) pMsg->hwnd = plugin.hwndParent;
								break;

							case VK_F4:
								if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && (GetAsyncKeyState(VK_MENU)&0x8000))
										SendMessageW(plugin.hwndParent, WM_CLOSE, 0, 0);
								pMsg->message = WM_NULL;
								break;
							case 'P':
							case 'K':
							case 'H':
							case VK_TAB:
								if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_MENU)&0x8000)) pMsg->hwnd = plugin.hwndParent;
								break;
							case '3':
							case VK_UP:
							case VK_DOWN:
								break;
							default:
								if ((GetAsyncKeyState(VK_MENU)&0x8000) && !(GetAsyncKeyState(VK_CONTROL)&0x8000)) pMsg->hwnd = plugin.hwndParent;
								break;
						}
						break;
				}
				
			}

		}
		return /*(IsDialogMessageW(pTarget->gethWnd(), pMsg)) ? true :*/ false;
	}

protected:
	BrowserWnd *pTarget;
	RECVS_DISPATCH;

};

extern HINSTANCE hInstance;
STDAPI WriteBSTR(BSTR *pstrDest, LPCWSTR szSrc)
{
	*pstrDest = SysAllocString( szSrc );
	if ( !(*pstrDest) ) return E_OUTOFMEMORY;
	return NOERROR;
}

STDAPI FreeBSTR(BSTR* pstr)
{
	if ( *pstr == NULL ) return S_FALSE;
	SysFreeString( *pstr );
	return NOERROR;
}

HRESULT writeBString(BSTR* psz, const char *str)
{
	WCHAR WideStr[WA_MAX_PATH] = {0};
	String s = str;
	if (s.isempty()) s = "";
	MultiByteToWideCharSZ(CP_ACP, 0, s, -1, WideStr, WA_MAX_PATH);
	return WriteBSTR(psz, WideStr);
}

BrowserWnd::BrowserWnd() : HTMLContainer2(NULL, NULL), processor(NULL)
{
	setVirtual(0);

	oleOk = FALSE;
	homepage = L"about:blank";
	timerset1 = 0;
	timerset2 = 0;
	cancelIEErrorPage = false;
	scrollbarsflag = BROWSER_SCROLLBARS_DEFAULT;
}

BrowserWnd::~BrowserWnd()
{
	if (processor)
	{
		if (WASABI_API_APP) WASABI_API_APP->app_removeMessageProcessor(processor);
		free(processor);
		processor = NULL;
	}
	if (timerset1)
	{
		killTimer(MB_TIMERID1);
		timerset1 = 0;
	}
	if (timerset2)
	{
		killTimer(MB_TIMERID2);
		timerset2 = 0;
	}

	freeBrowserStuff();
}

bool BrowserWnd::InitializeLibrary()
{
	return (FALSE != HTMLContainer2_Initialize());
}

void BrowserWnd::UninitializeLibrary()
{
	HTMLContainer2_Uninitialize();
}

int BrowserWnd::onInit()
{
	BROWSER_PARENT::onInit();
	if (isVisible())
		onSetVisible(1);
	updateScrollbars();
	return 1;
}

void BrowserWnd::onSetVisible(int show)
{
	if (show) initBrowserStuff();
}

int BrowserWnd::initBrowserStuff()
{
	if (pUnk) return 1;
	
	if (SUCCEEDED(OleInitialize(NULL))) oleOk = TRUE;
	if (!oleOk) return 1;

	// {280876CF-48C0-40bc-8E86-73CE6BB462E5}
	const GUID options_guid =
	    { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
	hParent = gethWnd();

	int usemozilla = 0;

#ifdef WASABI_COMPILE_CONFIG
	usemozilla = _intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid), L"Use Mozilla instead of IE for minibrowser");
#endif

	if (SUCCEEDED(Initialize()))
	{
		HRESULT hr;
		IWebBrowser2 *pWeb2;
		
		hr = GetIWebBrowser2(&pWeb2);
		if (SUCCEEDED(hr))
		{
			pWeb2->put_RegisterAsBrowser(VARIANT_TRUE);
			
			if (deferednavigate.isempty())
			{
				if (!homepage.isempty())
					minibrowser_navigateUrl(homepage);
				else
					minibrowser_navigateUrl(L"about:blank");
			}
			else minibrowser_navigateUrl(deferednavigate);
		}
	}
	if (!processor && WASABI_API_APP)
	{
		processor = new BrowserMsgProc(this);
		WASABI_API_APP->app_addMessageProcessor(processor);
		
	}

	ShowWindow(hParent, SW_SHOWNA);
	
	return 1;
}

void BrowserWnd::freeBrowserStuff()
{
	if (oleOk)
	{
		Finish();
		OleUninitialize();
		oleOk = FALSE;
#ifndef WASABINOMAINAPI
		api->hint_garbageCollect();
#endif

	}
}

int BrowserWnd::minibrowser_navigateUrl(const wchar_t *url)
{
	HRESULT hr;
	
	curpage = url;

	hr = NavigateToName(url, 0);
	if (FAILED(hr))
	{
		deferednavigate = url;
		return 0;
	}
	return 1;
}

int BrowserWnd::minibrowser_back()
{
	HRESULT hr;
	IWebBrowser2 *pWeb2;
	hr = GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr))
	{
		pWeb2->GoBack();
		pWeb2->Release();
	}
	return 1;
}

int BrowserWnd::minibrowser_forward()
{
	HRESULT hr;
	IWebBrowser2 *pWeb2;
	hr = GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr))
	{
		pWeb2->GoForward();
		pWeb2->Release();
	}
	return 1;
}

int BrowserWnd::minibrowser_refresh()
{
	HRESULT hr;
	IWebBrowser2 *pWeb2;
	hr = GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr))
	{
		pWeb2->Refresh();
		pWeb2->Release();
	}
	return 1;
}

int BrowserWnd::minibrowser_home()
{
	minibrowser_navigateUrl(homepage);
	return 1;
}

int BrowserWnd::minibrowser_stop()
{
	HRESULT hr;
	IWebBrowser2 *pWeb2;
	hr = GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr))
	{
		pWeb2->Stop();
		pWeb2->Release();
	}
	return 1;
}

HWND BrowserWnd::getOSHandle()
{
	return ::GetWindow(gethWnd(), GW_CHILD); // assumes setVirtual(0) in constructor
}

void BrowserWnd::onTargetNameTimer()
{
	updateTargetName();
}

void BrowserWnd::onScrollbarsFlagTimer()
{
	updateScrollbars();
}

void BrowserWnd::timerCallback(int id)
{
	switch (id)
	{
	case MB_TIMERID1:
		onTargetNameTimer();
		return ;
	case MB_TIMERID2:
		onScrollbarsFlagTimer();
		return ;
	}
	BROWSER_PARENT::timerCallback(id);
}

void BrowserWnd::minibrowser_setTargetName(const wchar_t *name)
{
	targetname = name;
	updateTargetName();
}

void BrowserWnd::updateTargetName()
{
	if (!doSetTargetName(targetname))
	{
		if (!timerset1) { setTimer(MB_TIMERID1, 100); timerset1 = 1; }
		return ;
	}
	else
	{
		if (timerset1) { killTimer(MB_TIMERID1); timerset1 = 0; }
	}
}

int BrowserWnd::doSetTargetName(const wchar_t *name)
{
	HRESULT hr;
	IWebBrowser2 *pWeb2;
	IDispatch *id;

	hr = GetIWebBrowser2(&pWeb2);

	if (FAILED(hr)) return FALSE;

	if (SUCCEEDED(pWeb2->get_Document(&id)) && id)
	{
		IHTMLDocument2 *doc;
		if (SUCCEEDED(id->QueryInterface(IID_IHTMLDocument2, (void **)&doc)) && doc)
		{
			IHTMLWindow2 *w;
			if (SUCCEEDED(doc->get_parentWindow(&w)) && w)
			{
				w->put_name(SysAllocString(targetname.getValue()));
				w->Release();
				doc->Release();
				id->Release();
				pWeb2->Release();
				return 1;
			}
			doc->Release();
		}
		id->Release();
	}
	pWeb2->Release();
	return 0;
}

const wchar_t *BrowserWnd::minibrowser_getTargetName()
{
	return targetname;
}

void BrowserWnd::OnBeforeNavigate(IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
{
	int i = 0;
	foreach(callbacks)
	int r = callbacks.getfor()->minibrowsercb_onBeforeNavigate(URL->bstrVal, Flags->intVal, TargetFrameName->bstrVal);
	if (i++ == 0) *Cancel = (r) ? VARIANT_TRUE : VARIANT_FALSE;
	endfor;
	updateScrollbars();

}

void BrowserWnd::minibrowser_setScrollbarsFlag(int a)
{
	scrollbarsflag = a;
	updateScrollbars();
}

void BrowserWnd::updateScrollbars()
{
	if (!doSetScrollbars())
	{
		if (!timerset2) { setTimer(MB_TIMERID2, 100); timerset2 = 1; }
		return ;
	}
	else
	{
		if (timerset2) { killTimer(MB_TIMERID2); timerset2 = 0; }
	}
}

void BrowserWnd::OnDocumentComplete(IDispatch *pDispatch, VARIANT *URL)
{
	if (!targetname.isempty())
		minibrowser_setTargetName(targetname);
	foreach(callbacks)
	callbacks.getfor()->minibrowsercb_onDocumentComplete(URL->bstrVal);
	endfor;
	updateScrollbars();
}

void BrowserWnd::OnDocumentReady(IDispatch *pDispatch, VARIANT *URL)
{
	if (!targetname.isempty())
		minibrowser_setTargetName(targetname);
	foreach(callbacks)
	callbacks.getfor()->minibrowsercb_onDocumentReady(URL->bstrVal);
	endfor;
	updateScrollbars();
}

void BrowserWnd::OnNavigateError(IDispatch *pDispatch, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel)
{
	if (TargetFrameName->bstrVal != NULL)
		return; //TODO: send targetframe via api to script
	foreach(callbacks)
	callbacks.getfor()->minibrowsercb_onNavigateError(URL->bstrVal, StatusCode->intVal);
	endfor;
	if (cancelIEErrorPage) *Cancel = -1;
}

const wchar_t* BrowserWnd::messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3)
{
	const wchar_t* ret = 0;
	foreach(callbacks)
		ret = callbacks.getfor()->minibrowsercb_messageToMaki(str1, str2, i1, i2, i3);
		if (ret) break;
	endfor;
	return ret;
}

const wchar_t* BrowserWnd::minibrowser_messageToJS(const wchar_t* str1, const wchar_t* str2, int i1, int i2, int i3)
{
	// TODO feed JS w/ this info
	return 0;
}

void BrowserWnd::minibrowser_scrape()
{
	IWebBrowser2 *browser=0;
	GetIWebBrowser2(&browser);
	IDispatch *docDisp=0;
	IHTMLDocument2 *document = 0;
	if (browser)
	{
		browser->get_Document(&docDisp);
		if (docDisp)
		{
			docDisp->QueryInterface(&document);
			docDisp->Release();
		}
		browser->Release();
	}

	if (document)
	{
		IHTMLElementCollection *links=0;
		document->get_all(&links);
		
		if (links)
		{
			IDispatch *anchorDisp=0;
			VARIANT index;

			VariantInit(&index);
			index.vt = VT_I4;
			index.intVal = 0;

			links->item(index, index, &anchorDisp);
			while (anchorDisp)
			{
				IHTMLAnchorElement *anchor=0;
				anchorDisp->QueryInterface(&anchor);
				if (anchor)
				{
					BSTR href=0;
					anchor->get_href(&href);
					if (href && (wa2.CanPlay(href) || wa2.IsPlaylist(href)))
					{
						foreach(callbacks)
						callbacks.getfor()->minibrowsercb_onMediaLink(href);
						endfor;
					}
					SysFreeString(href);
					anchor->Release();
				}

				index.intVal++;
				anchorDisp->Release();
				links->item(index, index, &anchorDisp);
			}

			links->Release();
		}
		document->Release();
	}

}

void BrowserWnd::minibrowser_getDocumentTitle(wchar_t *str, size_t len)
{
	IWebBrowser2 *browser=0;
	GetIWebBrowser2(&browser);
	IDispatch *docDisp=0;
	IHTMLDocument2 *document = 0;
	if (browser)
	{
		browser->get_Document(&docDisp);
		if (docDisp)
		{
			docDisp->QueryInterface(&document);
			docDisp->Release();
		}
		browser->Release();
	}

	if (document)
	{
		BSTR title_bstr;
		document->get_title(&title_bstr);
		document->Release();

		WCSCPYN(str, title_bstr, len);
		// the COM object SysAllocString'd this for us, so we need to free it via COM also
		SysFreeString(title_bstr);
	}
	else
		str[0]=0;
}

void BrowserWnd::minibrowser_setCancelIEErrorPage (bool cancel)
{
	cancelIEErrorPage = cancel;
}

int BrowserWnd::doSetScrollbars()
{
	HRESULT hr;
	IWebBrowser2 *pWeb2;
	IDispatch *id;

	hr = GetIWebBrowser2(&pWeb2);

	if (FAILED(hr)) return 0;

	if (scrollbarsflag == BROWSER_SCROLLBARS_DEFAULT) return 1;

	if (SUCCEEDED(pWeb2->get_Document(&id)) && id)
	{
		IHTMLDocument2 *doc;
		if (SUCCEEDED(id->QueryInterface(IID_IHTMLDocument2, (void **)&doc)) && doc)
		{
			IHTMLElement *e;
			if (SUCCEEDED(doc->get_body(&e)))
			{
				IHTMLStyle *s;
				if (SUCCEEDED(e->get_style(&s)))
				{
					BSTR a;
					switch (scrollbarsflag)
					{
					case BROWSER_SCROLLBARS_ALWAYS:
						writeBString(&a, "scroll");
						break;
					case BROWSER_SCROLLBARS_AUTO:
						writeBString(&a, "auto");
						break;
					case BROWSER_SCROLLBARS_NEVER:
						writeBString(&a, "hidden");
						break;
					default: 
						a = NULL;
						break;
					}
					if (a) s->put_overflow(a);
					FreeBSTR(&a);
					s->Release();
					pWeb2->Release();
					return 1;
				}
				e->Release();
			}
			doc->Release();
		}
		id->Release();
	}
	pWeb2->Release();
	return 0;
}

const wchar_t *BrowserWnd::minibrowser_getCurrentUrl()
{
	return curpage;
}

STDMETHODIMP BrowserWnd::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	*ppDispatch = (IDispatch*) new MinibrowserCOM(this); //TODO we might need to delete this as well!
	return S_OK;
}

DWORD BrowserWnd::OnGetDownlodFlags(void)
{
	return DLCTL_DLIMAGES | DLCTL_VIDEOS |															DLCTL_PRAGMA_NO_CACHE
#ifdef WINAMP_FINAL_BUILD
		|DLCTL_SILENT
#endif
	;
}


#define CBCLASS BrowserMsgProc
START_DISPATCH;
CB(IFC_MESSAGEPROCESSOR_PROCESS_MESSAGE, ProcessMessage)
END_DISPATCH;
#undef CBCLASS
