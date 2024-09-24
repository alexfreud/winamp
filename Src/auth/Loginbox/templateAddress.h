#ifndef NULLSOFT_AUTH_LOGIN_TEMPLATE_ADDRESS_HEADER
#define NULLSOFT_AUTH_LOGIN_TEMPLATE_ADDRESS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginTemplate.h"

// {F244385A-178A-4e8c-95DC-BF43058BAA9B}
static const GUID LTUID_ADDRESS = 
{ 0xf244385a, 0x178a, 0x4e8c, { 0x95, 0xdc, 0xbf, 0x43, 0x5, 0x8b, 0xaa, 0x9b } };

class LoginTemplateAddress : public LoginTemplate
{
protected:
	LoginTemplateAddress();
	~LoginTemplateAddress();

public:
	static HRESULT CreateInstance(LoginTemplateAddress **instance);

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT GetType(GUID *templateUid);

	HRESULT SetParameter(LPCWSTR pszKey, LPCWSTR pszValue);
	HRESULT IsValid();
	HRESULT IsIdentical(LoginTemplate *test);

	HWND CreatePage(HWND hLoginbox, HWND hParent);

protected:
	ULONG ref;
	LPWSTR title;
	LPWSTR message;
	LPWSTR address;
	LPWSTR addressTitle;
	BOOL replaceUsername;
};

#endif //NULLSOFT_AUTH_LOGIN_TEMPLATE_ADDRESS_HEADER