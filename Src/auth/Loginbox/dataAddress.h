#ifndef NULLSOFT_AUTH_LOGINDATA_ADDRESS_HEADER
#define NULLSOFT_AUTH_LOGINDATA_ADDRESS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginData.h"

// {830B9FCD-3A09-4485-8BA6-1A6B8F10ED39}
static const GUID IID_LoginDataAddress = 
{ 0x830b9fcd, 0x3a09, 0x4485, { 0x8b, 0xa6, 0x1a, 0x6b, 0x8f, 0x10, 0xed, 0x39 } };


class LoginDataAddress : public LoginData
{

protected:
	LoginDataAddress(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszAddress);
	~LoginDataAddress();

public: 
	static HRESULT CreateInstance(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszAddress, LoginDataAddress **instance);

public:
	virtual HRESULT QueryInterface(REFIID riid, void** ppObject);
	LPCWSTR GetAddress();
	

protected:
	LPWSTR address;
};

#endif //NULLSOFT_AUTH_LOGINDATA_ADDRESS_HEADER