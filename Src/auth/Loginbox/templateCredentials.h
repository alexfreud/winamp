#ifndef NULLSOFT_AUTH_LOGIN_TEMPLATE_CREDENTIALS_HEADER
#define NULLSOFT_AUTH_LOGIN_TEMPLATE_CREDENTIALS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginTemplate.h"

// {13B3CEEB-A751-4864-8C69-A8E0566B169C}
static const GUID LTUID_CREDENTIALS = 
{ 0x13b3ceeb, 0xa751, 0x4864, { 0x8c, 0x69, 0xa8, 0xe0, 0x56, 0x6b, 0x16, 0x9c } };


class LoginTemplateCredentials : public LoginTemplate
{
protected:
	LoginTemplateCredentials();
	~LoginTemplateCredentials();

public:
	static HRESULT CreateInstance(LoginTemplateCredentials **instance);

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
	LPWSTR accountRecoverUrl;
	LPWSTR accountCreateUrl;
	LPWSTR usernameLabel;
	LPWSTR passwordLabel;

};

#endif //NULLSOFT_AUTH_LOGIN_TEMPLATE_CREDENTIALS_HEADER