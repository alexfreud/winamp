#ifndef NULLSOFT_AUTH_LOGINDATA_HEADER
#define NULLSOFT_AUTH_LOGINDATA_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

// {69346B92-168E-452e-AA88-986AB3883920}
static const GUID IID_LoginData = 
{ 0x69346b92, 0x168e, 0x452e, { 0xaa, 0x88, 0x98, 0x6a, 0xb3, 0x88, 0x39, 0x20 } };


class LoginProvider;
class LoginStatus;

class LoginData
{

protected:
	LoginData(const GUID *pRealm, HWND hPage, HWND hLoginbox);
	virtual ~LoginData();

public: 
	static HRESULT CreateInstance(const GUID *pRealm, HWND hPage, HWND hLoginbox, LoginData **instance);

public:
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual HRESULT QueryInterface(REFIID riid, void** ppObject);
	
	virtual HWND GetLoginbox();
	virtual HWND GetPage();
	virtual HRESULT GetRealm(GUID *pRealm);
	virtual HRESULT GetProvider(LoginProvider **ppProvider);
	virtual HRESULT GetStatus(LoginStatus **ppStatus);
	virtual HRESULT SetStatus(LPCWSTR pszStatus);

protected:
	UINT ref;
	GUID realm;
	HWND hPage;
	HWND hLoginbox;
	LoginProvider *provider;
	LoginStatus *status;
	UINT statusCookie;
};

#endif //NULLSOFT_AUTH_LOGINDATA_HEADER