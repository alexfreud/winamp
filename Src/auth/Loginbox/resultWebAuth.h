#ifndef NULLSOFT_AUTH_LOGINRESULT_WEBAUTH_HEADER
#define NULLSOFT_AUTH_LOGINRESULT_WEBAUTH_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginResult.h"
#include "./browserEvent.h"
#include "../omBrowser/ifc_omservice.h"
#include "../omBrowser/ifc_omservicecommand.h"
#include "../../nu/dispatchTable.h"

#include <bfc/multipatch.h>

class LoginData;
class LoginCredentials;
class ExternalManager;
class obj_ombrowser;


#define MPIID_OMSVC				10
#define MPIID_OMSVCCOMMAND		20

class LoginResultWebAuth :	public LoginResult, 
							public IDispatch,
							public BrowserEvent,
							public MultiPatch<MPIID_OMSVC, ifc_omservice>,
							public MultiPatch<MPIID_OMSVCCOMMAND, ifc_omservicecommand>
							
{

public:
	typedef enum
	{
		DISPID_LOGINCOMPLETE	= 700,
		DISPID_GETPAGERECT		= 701,
		DISPID_GETBOXRECT		= 702,
		DISPID_SETSTATUS		= 703,
		DISPID_NAVIGATE			= 704,
		DISPID_GETSTRING		= 705,
	} DispatchCodes;

protected:
	LoginResultWebAuth(obj_ombrowser *pManager, LPCWSTR pszTargetUrl, LoginData *pInput, Callback fnCallback, void *pUser);
	~LoginResultWebAuth();

public:
	static HRESULT CreateInstance(LPCWSTR targetUrl, LoginData *input, Callback callback, void *user, LoginResultWebAuth **instance);

public:
	/* IUnknown */
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	
	/* IDispatchable */
	DISPTABLE_INCLUDE();
	DISPHANDLER_REGISTER(OnLoginComplete);
	DISPHANDLER_REGISTER(OnGetPageRect);
	DISPHANDLER_REGISTER(OnGetBoxRect);
	DISPHANDLER_REGISTER(OnSetStatus);
	DISPHANDLER_REGISTER(OnNavigate);
	DISPHANDLER_REGISTER(OnGetString);

	/* LoignResult */
	HRESULT GetWaitHandle(HANDLE *handle);
	HRESULT GetUser(void **pUser);
	HRESULT	RequestAbort(BOOL fDrop);
	HRESULT IsCompleted();
	HRESULT IsAborting();
	HRESULT GetLoginData(LoginData **loginData);

	/* BrowserEvent */
	STDMETHOD_(void, Event_BrowserReady)(HWND hBrowser);
	STDMETHOD_(void, Event_DocumentReady)(HWND hBrowser);
	STDMETHOD_(void, Event_BrowserClosing)(HWND hBrowser);
	STDMETHOD_(void, Event_InvokeApc)(HWND hBrowser, LPARAM param);

protected:
	/* Dispatchable */
	size_t Wasabi_AddRef();
	size_t Wasabi_Release();
	int Wasabi_QueryInterface(GUID iid, void **object);

	/* ifc_omservice */
	unsigned int GetId();
	HRESULT GetName(wchar_t *pszBuffer, int cchBufferMax);
	HRESULT GetUrl(wchar_t *pszBuffer, int cchBufferMax);
	HRESULT GetExternal(IDispatch **ppDispatch);

	/* ifc_omservicecommand */
	HRESULT QueryState(HWND hBrowser, const GUID *commandGroup, UINT commandId);
	HRESULT Exec(HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg);
	
public:
	HRESULT GetResult(INT *authCode, LoginCredentials **credentials);

private:
	static HRESULT InitBrowserManager(obj_ombrowser **browserMngr);
	HRESULT Start();
	HRESULT Finish();
	void NotifyComplete();
	HRESULT DispParamsToCredentials(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, UINT FAR *puArgErr, HRESULT *errorEx, LPWSTR *finishUrl);


protected:
	ULONG ref;
	LoginData *input;
	Callback callback;
	void *user;
	LoginCredentials *credentials;
	INT	 authCode;
	obj_ombrowser *browserMngr; 
	LPWSTR targetUrl;
	HWND hView;
	CRITICAL_SECTION lock;
	HANDLE completed;
	ExternalManager *external;
	DISPID	dispId;
	BOOL connectionVerified;
	LPWSTR readyUrl;
	

protected:
	RECVS_MULTIPATCH;
};

#endif //NULLSOFT_AUTH_LOGINRESULT_WEBAUTH_HEADER