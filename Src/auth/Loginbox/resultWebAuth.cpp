#include "./resultWebAuth.h"	
#include "./common.h"
#include "./commandWebAuth.h"
#include "./loginData.h"
#include "./dataAddress.h"
#include "./loginCredentials.h"
#include "./loginBox.h"
#include "./loginProvider.h"
#include "./loginStatus.h"
#include "./externalMngr.h"
#include "./browserWindow.h"

#include "../resource.h"
#include "../api.h"
#include "../api_auth.h"

#include "../../omBrowser/obj_ombrowser.h"
#include "../../omBrowser/ifc_ombrowserwndenum.h"
#include "../../omBrowser/ifc_ombrowserwndmngr.h"
#include "../../omBrowser/browserUiCommon.h"

#include "../../winamp/jsapi.h"
#include "../../winamp/jsapi_CallbackParameters.h"

#include <api/service/waservicefactory.h>

#include <strsafe.h>

#define SERVICE_ID			369

#define DISPTABLE_CLASS	 LoginResultWebAuth
DISPTABLE_BEGIN()
	DISPENTRY_ADD(DISPID_LOGINCOMPLETE, L"loginComplete", OnLoginComplete)
	DISPENTRY_ADD(DISPID_GETPAGERECT, L"getPageRect", OnGetPageRect)
	DISPENTRY_ADD(DISPID_GETBOXRECT, L"getBoxRect", OnGetBoxRect)
	DISPENTRY_ADD(DISPID_SETSTATUS, L"setStatus", OnSetStatus)
	DISPENTRY_ADD(DISPID_NAVIGATE, L"navigate", OnNavigate)
	DISPENTRY_ADD(DISPID_GETSTRING, L"getString", OnGetString)
DISPTABLE_END
#undef DISPTABLE_CLASS

LoginResultWebAuth::LoginResultWebAuth(obj_ombrowser *pManager, LPCWSTR pszTargetUrl, LoginData *pInput, Callback fnCallback, void *pUser)
	: ref(1), browserMngr(pManager), external(NULL), targetUrl(NULL), input(pInput), callback(fnCallback), user(pUser), 
		credentials(NULL), authCode(AUTH_NOT_AUTHORIZED), hView(NULL), completed(NULL), 
		dispId(DISPID_UNKNOWN), connectionVerified(FALSE), readyUrl(NULL)
{
	InitializeCriticalSection(&lock);

	targetUrl = LoginBox_CopyString(pszTargetUrl);
	input->AddRef();
	browserMngr->AddRef();
	
	if(SUCCEEDED(ExternalManager::CreateInstance(&external)))
		external->AddDispatch(L"loginBox", this, &dispId);
}

LoginResultWebAuth::~LoginResultWebAuth()
{
	hView = NULL;

	Finish();
	
	if (NULL != completed)
	{
		CloseHandle(completed);
	}

	LoginBox_FreeString(targetUrl);
	input->Release();
		
	if (NULL != credentials)
		credentials->Release();

	LoginBox_FreeString(readyUrl);



	DeleteCriticalSection(&lock);

}

HRESULT LoginResultWebAuth::InitBrowserManager(obj_ombrowser **browserMngr)
{
	HRESULT hr;
	
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(OBJ_OmBrowser);
	if (NULL == sf) return E_UNEXPECTED;
		
	*browserMngr = (obj_ombrowser*)sf->getInterface();
	if (NULL == browserMngr) 
		hr = E_UNEXPECTED;
	else
	{
		HWND hWinamp = (NULL != WASABI_API_WINAMP) ? WASABI_API_WINAMP->GetMainWindow() : NULL;
		if (NULL == hWinamp || FAILED((*browserMngr)->Initialize(NULL, hWinamp)))
		{
			(*browserMngr)->Release();
			hr =  E_FAIL;
		}
		else
		{
			hr = S_OK;
		}
	}

	sf->Release();
	return hr;
}

HRESULT LoginResultWebAuth::CreateInstance(LPCWSTR targetUrl, LoginData *input, Callback callback, void *user, LoginResultWebAuth **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;
	
	if (NULL == targetUrl || NULL == input)
		return E_INVALIDARG;

	obj_ombrowser *browserMngr;
	if (FAILED(InitBrowserManager(&browserMngr)))
		return E_FAIL;

	HRESULT hr;
		
	LoginResultWebAuth *result = new LoginResultWebAuth(browserMngr, targetUrl, input, callback, user);
	if (NULL == result) 
		hr = E_OUTOFMEMORY;
	else
	{
		if (DISPID_UNKNOWN == result->dispId)
			hr = E_FAIL;
		else
			hr = result->Start();

		if (FAILED(hr))
		{
			result->Release();
			result = NULL;
		}
		else
		{
			*instance = result;
		}
	}

	browserMngr->Release();
	return hr;
}

STDMETHODIMP_(ULONG) LoginResultWebAuth::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) LoginResultWebAuth::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP LoginResultWebAuth::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (NULL == ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IFC_OmService))
		*ppvObject = static_cast<ifc_omservice*>(this);
	else if (IsEqualIID(riid, IFC_OmServiceCommand))
		*ppvObject = static_cast<ifc_omservicecommand*>(this);
	else if (IsEqualIID(riid, LCUID_WEBAUTH))
		*ppvObject = static_cast<LoginResultWebAuth*>(this);
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = static_cast<IDispatch*>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = static_cast<IUnknown*>(reinterpret_cast<LoginResult*>(this));
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *ppvObject)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

size_t LoginResultWebAuth::Wasabi_AddRef()
{
	return AddRef();	
}

size_t LoginResultWebAuth::Wasabi_Release()
{
	return Release();
}

int LoginResultWebAuth::Wasabi_QueryInterface(GUID iid, void **object)
{
	return QueryInterface(iid, object);
}

HRESULT LoginResultWebAuth::Start()
{	
	input->SetStatus(MAKEINTRESOURCE(IDS_STATUS_INITIALIZING));

	HWND hParent = input->GetPage();
	UINT viewStyle = NBCS_DISABLECONTEXTMENU | NBCS_DIALOGMODE | NBCS_POPUPOWNER | 
		NBCS_DISABLEFULLSCREEN | NBCS_DISABLEHOSTCSS | NBCS_NOTOOLBAR | NBCS_NOSTATUSBAR;

	HRESULT hr = browserMngr->CreateView(this, hParent, NULL, viewStyle, &hView);
	if (FAILED(hr))
		return hr;

	if (FALSE == BrowserWindow_Attach(hView, this))
	{
		DestroyWindow(hView);
		hView = NULL;
		hr = E_FAIL;
	}
	else
	{
		SetWindowPos(hView, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE);
	}
	return hr;
}

HRESULT LoginResultWebAuth::Finish()
{
	EnterCriticalSection(&lock);

	if (NULL != external)
	{
		if (DISPID_UNKNOWN != dispId)
		{
			external->DeleteMemberByDispID(dispId);
			dispId = DISPID_UNKNOWN;
		}
		external->Release();
		external = NULL;
	}
	
	// try to destroy windows before browserMngr->Finish() (this will let them to lock less)

	if (NULL != hView)
	{
		DestroyWindow(hView);
		hView = NULL;
	}

	if (NULL != browserMngr)
	{
		ifc_ombrowserwndmngr *windowMngr;
		if (SUCCEEDED(browserMngr->QueryInterface(IFC_OmBrowserWindowManager, (void**)&windowMngr)))
		{
			ifc_ombrowserwndenum *windowEnum;
			if (SUCCEEDED(windowMngr->Enumerate(NULL, NULL, &windowEnum)))
			{
				HWND hBrowser;
				while(S_OK == windowEnum->Next(1, &hBrowser, NULL))
				{
					if (FALSE != DestroyWindow(hBrowser) && hBrowser == hView)
						hView = NULL;
				}
				windowEnum->Release();
			}
			windowMngr->Release();
		}
	}

	if (NULL != browserMngr)
	{
		browserMngr->Finish();
		browserMngr->Release();
		browserMngr = NULL;
	}

	LeaveCriticalSection(&lock);

	return S_OK;
}
void LoginResultWebAuth::NotifyComplete()
{
	Callback callbackCopy(NULL);
	HANDLE completedCopy(NULL);

	INT statusId;
	switch(authCode)
	{
			case AUTH_SUCCESS:	statusId = IDS_STATUS_SUCCEEDED; break;
			case AUTH_SECURID:	statusId = IDS_STATUS_PASSCODE_REQUIRED; break;
			case AUTH_ABORT:	statusId = IDS_STATUS_ABORTED; break;
			default:			statusId = IDS_STATUS_FAILED; break;
	}
	input->SetStatus(MAKEINTRESOURCE(statusId));

	EnterCriticalSection(&lock);

	callbackCopy = callback;
	if (NULL == completed || FALSE == DuplicateHandle(GetCurrentProcess(), completed, 
			GetCurrentProcess(), &completedCopy, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		completedCopy = NULL;
	}

	LeaveCriticalSection(&lock);

	if (NULL != completedCopy)
	{
		SetEvent(completedCopy);
		CloseHandle(completedCopy);
	}

	if (NULL != callbackCopy)
		callbackCopy(this);

}

HRESULT LoginResultWebAuth::GetWaitHandle(HANDLE *handle)
{
	if (NULL == handle)
		return E_POINTER;

	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	if (NULL == completed)
	{
		completed = CreateEvent(NULL, TRUE, (S_OK == IsCompleted()), NULL);
		if (NULL == completed)
		{
			*handle = NULL;
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32(error);
		}
	}

	if (SUCCEEDED(hr) && 
		FALSE == DuplicateHandle(GetCurrentProcess(), completed, 
					GetCurrentProcess(), handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		*handle = NULL;
		DWORD error = GetLastError();
		hr = HRESULT_FROM_WIN32(error);
	}
	
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT LoginResultWebAuth::GetUser(void **pUser)
{
	if (NULL == pUser) return E_POINTER;
	*pUser = user;
	return S_OK;
}

HRESULT	LoginResultWebAuth::RequestAbort(BOOL fDrop)
{
	HRESULT hr;
	EnterCriticalSection(&lock);

	authCode = AUTH_ABORT;

	if (NULL == browserMngr)
		hr = S_FALSE;
	else
	{
		input->SetStatus(MAKEINTRESOURCE(IDS_STATUS_ABORTING));
		Finish();
		hr = S_OK;
	}
		
	if (NULL != credentials)
	{
		credentials->Release();
		credentials = NULL;
	}

	LeaveCriticalSection(&lock);
	
	if (S_OK == hr)
		NotifyComplete();	

	return hr;
}

HRESULT LoginResultWebAuth::IsCompleted()
{
	return (NULL == browserMngr) ? S_OK : S_FALSE;
}

HRESULT LoginResultWebAuth::IsAborting()
{
	return E_NOTIMPL;
}

HRESULT LoginResultWebAuth::GetLoginData(LoginData **loginData)
{
	if (NULL == loginData) return E_POINTER;
	*loginData = input;
	if (NULL != input) 
		input->AddRef();
	return S_OK;
}

STDMETHODIMP_(void) LoginResultWebAuth::Event_BrowserReady(HWND hBrowser)
{
	input->SetStatus(MAKEINTRESOURCE(IDS_STATUS_CONNECTING));
}

STDMETHODIMP_(void) LoginResultWebAuth::Event_DocumentReady(HWND hBrowser)
{
	BOOL validateDocument = TRUE;

	EnterCriticalSection(&lock);
	if (NULL != readyUrl)
	{		
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, readyUrl, -1, L"loginbox:finish", -1))
		{			
			if (FAILED(BrowserWindow_QueueApc(hView, 0L)))
			{
				Finish();
				NotifyComplete();
			}
			return;
		}

		connectionVerified = FALSE;
		BrowserControl_Navigate(hBrowser, NAVIGATE_STOP, FALSE);
		if (FALSE != BrowserControl_Navigate(hBrowser, readyUrl, TRUE))
			validateDocument = FALSE;

		LoginBox_FreeString(readyUrl);
		readyUrl = NULL;

	}
	LeaveCriticalSection(&lock);

	if (FALSE != validateDocument && FALSE == connectionVerified)
	{
		// time to kill it 
		authCode = AUTH_404;
		Finish();
		NotifyComplete();
	}
}

STDMETHODIMP_(void) LoginResultWebAuth::Event_BrowserClosing(HWND hBrowser)
{
}

STDMETHODIMP_(void) LoginResultWebAuth::Event_InvokeApc(HWND hBrowser, LPARAM param)
{
	Finish();
	NotifyComplete();
}
unsigned int LoginResultWebAuth::GetId()
{
	return SERVICE_ID;
}

HRESULT LoginResultWebAuth::GetName(wchar_t *pszBuffer, int cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	WCHAR szName[128] = {0}, szTemplate[256] = {0};

	WASABI_API_LNGSTRINGW_BUF(IDS_WEBAUTH_CAPTION_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));

	LoginProvider *loginProvider;
	if (SUCCEEDED(input->GetProvider(&loginProvider)) && NULL != loginProvider)
	{
		loginProvider->GetName(szName, ARRAYSIZE(szName));
		loginProvider->Release();
	}

	return StringCchPrintf(pszBuffer, cchBufferMax, szTemplate, szName);
}

HRESULT LoginResultWebAuth::GetUrl(wchar_t *pszBuffer, int cchBufferMax)
{

	input->SetStatus(MAKEINTRESOURCE(IDS_STATUS_CONNECTING));

	LPWSTR cursor;
	size_t remaining;
	HRESULT hr = StringCchCopyEx(pszBuffer, cchBufferMax, targetUrl, &cursor, &remaining, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr)) return hr;

	LoginDataAddress *inputAddress;
	if (SUCCEEDED(input->QueryInterface(IID_LoginDataAddress, (void**)&inputAddress)))
	{
		LPCWSTR pszAddress = inputAddress->GetAddress();
		if (NULL != pszAddress && L'\0' != *pszAddress)
		{
			LPWSTR paramBegin = cursor;
			while (L'?' != *paramBegin && paramBegin != pszBuffer) 
				paramBegin = CharPrev(pszBuffer, paramBegin);

			WCHAR separator = (L'?' == *paramBegin) ? L'&' : L'?';
			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, 0, L"%cuserUrl=%s", separator, pszAddress);
		}
		inputAddress->Release();
	}
	return hr;
}

HRESULT LoginResultWebAuth::GetExternal(IDispatch **ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;
	
	*ppDispatch = external;
	if (NULL != external)
		external->AddRef();

	return S_OK;
}

HRESULT LoginResultWebAuth::QueryState(HWND hBrowser, const GUID *commandGroup, UINT commandId)
{
	if (NULL == commandGroup)
		return E_NOTIMPL;

	if (IsEqualGUID(*commandGroup, CMDGROUP_NAVIGATION))
	{
		switch(commandId)
		{
			case NAVCOMMAND_HOME:
				return CMDSTATE_DISABLED;
		}
	}

	return E_NOTIMPL;
}

HRESULT LoginResultWebAuth::Exec(HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg)
{
	return E_NOTIMPL;
}

HRESULT LoginResultWebAuth::DispParamsToCredentials(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, UINT FAR *puArgErr, HRESULT *errorEx, LPWSTR *finishUrl)
{
	if (NULL != finishUrl)
		*finishUrl = NULL;

	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 5, 6);
	
	BSTR username;
	BSTR token;
	BSTR sessionKey;
	__time64_t expire;

	JSAPI_GETUNSIGNED_AS_NUMBER(authCode, pdispparams, 1, puArgErr);
	JSAPI_GETSTRING(token, pdispparams, 2, puArgErr);
	JSAPI_GETSTRING(sessionKey, pdispparams, 3, puArgErr);
	JSAPI_GETUNSIGNED_AS_NUMBER(expire, pdispparams, 4, puArgErr);
	JSAPI_GETSTRING(username, pdispparams, 5, puArgErr);
	expire += _time64(0);

	if (NULL != finishUrl && pdispparams->cArgs > 5)
	{
		VARIANTARG *arg = &pdispparams->rgvarg[pdispparams->cArgs - 6];
		if (NULL != arg && VT_BSTR == arg->vt)
			*finishUrl = LoginBox_CopyString(arg->bstrVal);
	}

	if (NULL != credentials)
	{
		credentials->Release();
		credentials = NULL;
	}

	if (AUTH_SUCCESS == authCode)
	{
		GUID realm;
		HRESULT  hr = input->GetRealm(&realm);
		if (SUCCEEDED(hr))
		{
			LPSTR sessionAnsi(NULL), tokenAnsi(NULL);
			if (FAILED(LoginBox_WideCharToMultiByte(CP_UTF8, 0, sessionKey, -1, NULL, NULL, &sessionAnsi)) ||
				FAILED(LoginBox_WideCharToMultiByte(CP_UTF8, 0, token, -1, NULL, NULL, &tokenAnsi)))
			{
				hr = E_FAIL;
			}
			if (SUCCEEDED(hr))
				hr = LoginCredentials::CreateInstance(&realm, username, sessionAnsi, tokenAnsi, expire, &credentials);

			LoginBox_FreeAnsiString(sessionAnsi);
			LoginBox_FreeAnsiString(tokenAnsi);
		}

		if (FAILED(hr))
		{
			if (NULL != errorEx)
				*errorEx = hr;
		}
	}

	return S_OK;

}

HRESULT LoginResultWebAuth::OnLoginComplete(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	HRESULT hr, errorEx;
	BOOL finishLogin = TRUE;

	EnterCriticalSection(&lock);

	LPWSTR finishUrl;
	hr = DispParamsToCredentials(wFlags, pdispparams, pvarResult, puArgErr, &errorEx, &finishUrl);
	JSAPI_SET_RESULT(pvarResult, boolVal, (SUCCEEDED(hr) && SUCCEEDED(errorEx)) ? VARIANT_TRUE : VARIANT_FALSE);

	if (NULL != finishUrl && L'\0' != *finishUrl)
	{
		LoginBox_FreeString(readyUrl);
		readyUrl = LoginBox_CopyString(L"loginbox:finish");
		if (NULL != readyUrl)
		{
			if (FALSE == hView || FALSE == BrowserControl_Navigate(hView, finishUrl, FALSE))
			{
				LoginBox_FreeString(readyUrl);
				readyUrl = NULL;
			}
			else
				finishLogin = FALSE;
		}
	}

	LoginBox_FreeString(finishUrl);
	
	LeaveCriticalSection(&lock);

	if (FALSE != finishLogin)
	{
		if (FAILED(BrowserWindow_QueueApc(hView, 0L)))
		{
			Finish();
			NotifyComplete();
		}
	}
	
	return hr;
}

HRESULT LoginResultWebAuth::OnGetPageRect(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_INIT_RESULT(pvarResult, VT_DISPATCH);

	RECT rect;
	EnterCriticalSection(&lock);

	HWND hPage = input->GetPage();
	if (NULL == hPage || FALSE == GetWindowRect(hPage, &rect))
		SetRectEmpty(&rect);

	LeaveCriticalSection(&lock);

	JSAPI::CallbackParameters *params = new JSAPI::CallbackParameters;
	params->AddLong(L"x", rect.left);
	params->AddLong(L"y", rect.top);
	params->AddLong(L"cx", rect.right - rect.left);
	params->AddLong(L"cy", rect.bottom - rect.top);
	V_DISPATCH(pvarResult) = params;
	return S_OK;
}

HRESULT LoginResultWebAuth::OnGetBoxRect(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_INIT_RESULT(pvarResult, VT_DISPATCH);

	RECT rect;
	EnterCriticalSection(&lock);

	HWND hBox = input->GetLoginbox();
	if (NULL == hBox || FALSE == GetWindowRect(hBox, &rect))
		SetRectEmpty(&rect);

	LeaveCriticalSection(&lock);

	if (NULL != pvarResult)
	{
		JSAPI::CallbackParameters *params = new JSAPI::CallbackParameters;
		params->AddLong(L"x", rect.left);
		params->AddLong(L"y", rect.top);
		params->AddLong(L"cx", rect.right - rect.left);
		params->AddLong(L"cy", rect.bottom - rect.top);
		V_DISPATCH(pvarResult) = params;
	}
	return S_OK;
}

HRESULT LoginResultWebAuth::OnSetStatus(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	connectionVerified = TRUE;

	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	LPCWSTR pszStatus = NULL; 
	INT paramIndex = pdispparams->cArgs - 1;
	VARIANTARG *param = &pdispparams->rgvarg[paramIndex];
	switch(param->vt)
	{
		case VT_I4:
			pszStatus = (MAKEINTRESOURCE(param->lVal));
			break;
		case VT_BSTR:
			pszStatus = param->bstrVal;
			break;
		default:
			if (puArgErr) *puArgErr = paramIndex;
			return DISP_E_TYPEMISMATCH;
	}
	
	HRESULT hr = input->SetStatus(pszStatus);
	JSAPI_SET_RESULT(pvarResult, boolVal, (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE));
	
	return S_OK;
}

HRESULT LoginResultWebAuth::OnNavigate(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 3);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	BSTR targetUrl = JSAPI_PARAM_OPTIONAL(pdispparams, 1, bstrVal, NULL);
	BSTR callbackUrl = JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, NULL);
	BSTR message = JSAPI_PARAM_OPTIONAL(pdispparams, 3, bstrVal, NULL);
	
	HRESULT hr;
	
	if (NULL == targetUrl || L'\0' == *targetUrl || 
		NULL == callbackUrl || L'\0' == *callbackUrl)
	{
		hr = E_INVALIDARG;
	}
	else
	{
		EnterCriticalSection(&lock);
		LoginBox_FreeString(readyUrl);
		readyUrl = LoginBox_CopyString(callbackUrl);
		if (NULL == readyUrl)
			hr = E_OUTOFMEMORY;
		else
		{
			connectionVerified = FALSE;
			if (NULL == hView || FALSE == BrowserControl_Navigate(hView, targetUrl, FALSE))
			{
				LoginBox_FreeString(readyUrl);
				readyUrl = NULL;
				hr = E_FAIL;
			}
			else
				hr = S_OK;
		}
		LeaveCriticalSection(&lock);
	}
	
	JSAPI_SET_RESULT(pvarResult, boolVal, (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE));
	
	return S_OK;
}

HRESULT LoginResultWebAuth::OnGetString(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (0 == ((DISPATCH_METHOD |DISPATCH_PROPERTYGET) &wFlags))
		return DISP_E_MEMBERNOTFOUND;
		
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
	
	UINT stringId;
	JSAPI_GETUNSIGNED_AS_NUMBER(stringId, pdispparams, 1, puArgErr);
	
	WCHAR szBuffer[4096] = {0};
	BSTR string = SysAllocString(WASABI_API_LNGSTRINGW_BUF(stringId, szBuffer, ARRAYSIZE(szBuffer)));
	JSAPI_SET_RESULT(pvarResult, bstrVal, string);

	return S_OK;
}
HRESULT LoginResultWebAuth::GetResult(INT *pAuthCode, LoginCredentials **ppCredentials)
{
	if (S_OK != IsCompleted())
		return E_PENDING;

	EnterCriticalSection(&lock);

	if (NULL != pAuthCode)
		*pAuthCode = authCode;

	if (NULL != ppCredentials)
	{
		*ppCredentials = credentials;
		if (NULL != credentials)
			credentials->AddRef();
	}

	LeaveCriticalSection(&lock);
	
	return S_OK;
}


#define CBCLASS LoginResultWebAuth
START_MULTIPATCH;
 START_PATCH(MPIID_OMSVC)
  M_CB(MPIID_OMSVC, ifc_omservice, ADDREF, Wasabi_AddRef);
  M_CB(MPIID_OMSVC, ifc_omservice, RELEASE, Wasabi_Release);
  M_CB(MPIID_OMSVC, ifc_omservice, QUERYINTERFACE, Wasabi_QueryInterface);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETID, GetId);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETNAME, GetName);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETURL, GetUrl);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETEXTERNAL, GetExternal);
 NEXT_PATCH(MPIID_OMSVCCOMMAND)
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, ADDREF, Wasabi_AddRef);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, RELEASE, Wasabi_Release);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, QUERYINTERFACE, Wasabi_QueryInterface);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, API_QUERYSTATE, QueryState);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, API_EXEC, Exec);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS