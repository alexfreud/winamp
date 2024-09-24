#ifndef NULLSOFT_AUTH_LOGINRESULT_WINAMPAUTH_HEADER
#define NULLSOFT_AUTH_LOGINRESULT_WINAMPAUTH_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginResult.h"
#include "../auth/ifc_authcallback.h"
#include "../auth/api_auth.h"

class LoginData;
class LoginCredentials;
class LoginDataCredentials;

class LoginResultWinampAuth : public LoginResult,
								public ifc_authcallback
{
protected:
	LoginResultWinampAuth(api_auth *auth, LoginDataCredentials *pInput, Callback callback, void *user);
	~LoginResultWinampAuth();

public:
	static HRESULT CreateInstance(LoginData *input, Callback callback, void *user, LoginResultWinampAuth **instance);

public:
	/* IUnknown */
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* LoginResult */
	HRESULT GetWaitHandle(HANDLE *handle);
	HRESULT GetUser(void **pUser);
	HRESULT	RequestAbort(BOOL fDrop);
	HRESULT IsCompleted();
	HRESULT IsAborting();
	HRESULT GetLoginData(LoginData **loginData);

public:
	HRESULT GetResult(INT *authCode, LoginCredentials **credentials);

protected:
	/* Dispatchable */
	size_t Wasabi_AddRef();
	size_t Wasabi_Release();
	int Wasabi_QueryInterface(GUID iid, void **object);

	/*ifc_authcallback*/
	int Event_AuthConnecting();
	int Event_AuthSending();
	int Event_AuthReceiving();
	int Event_AuthIdle();

private:
	HRESULT Start();
	DWORD ThreadProc();
	HRESULT IsAbortingEx(UINT waitMs);
	LPWSTR MakeAuthParam(LPCWSTR pszInput, INT cchInput, INT min, INT max, BOOL removeSpaces, BOOL firstLetter, WORD typeMask, INT errorBase, INT *authError);
	LPWSTR GetUsername(LPCWSTR pszInput, INT *authError);
	LPWSTR GetPassword(LPCWSTR pszInput, INT *authError);
	LPWSTR GetPasscode(LPCWSTR pszInput, INT *authError);

	friend  static DWORD WINAPI WinampAuth_ThreadProcCallback(void *param);
	

protected:
	ULONG ref;
	LoginDataCredentials *input;
	Callback callback;
	void *user;
	api_auth *authApi;
	HANDLE thread;
	HANDLE abort;
	HANDLE completed;
	LoginCredentials *credentials;
	INT	 authCode;
	CRITICAL_SECTION lock;
	UINT statusCookie;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGINRESULT_WINAMPAUTH_HEADER