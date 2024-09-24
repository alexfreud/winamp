#ifndef NULLSOFT_AUTH_LOGIN_TEMPLATE_INFO_HEADER
#define NULLSOFT_AUTH_LOGIN_TEMPLATE_INFO_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginTemplate.h"

// {AF0F69A5-61B5-4e22-BC20-C075AEAB6C0A}
static const GUID LTUID_INFO = 
{ 0xaf0f69a5, 0x61b5, 0x4e22, { 0xbc, 0x20, 0xc0, 0x75, 0xae, 0xab, 0x6c, 0xa } };


class LoginTemplateInfo : public LoginTemplate
{
protected:
	LoginTemplateInfo();
	~LoginTemplateInfo();

public:
	static HRESULT CreateInstance(LoginTemplateInfo **instance);

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
};

#endif //NULLSOFT_AUTH_LOGIN_TEMPLATE_INFO_HEADER