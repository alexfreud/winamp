#ifndef NULLSOFT_AUTH_LOGINPROVIDER_HEADER
#define NULLSOFT_AUTH_LOGINPROVIDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class LoginTemplate;
class LoginCommand;

class LoginProvider
{

protected:
	LoginProvider(const GUID *providerUid);
	virtual ~LoginProvider();

public:
	static HRESULT CreateInstance(const GUID *providerUid, LoginProvider **instance);

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT IsIdentical(LoginProvider *test);
	HRESULT IsValid();

	// get
	HRESULT GetId(GUID *pId);
	HRESULT GetName(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetDescription(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetImagePath(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetTosLink(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetPrivacyLink(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetHelpLink(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetTemplate(LoginTemplate **ppTemplate);
	HRESULT GetCommand(LoginCommand **ppCommand);
		
	// set
	HRESULT SetName(LPCWSTR pszName);
	HRESULT SetDescription(LPCWSTR pszDescription);
	HRESULT SetImagePath(LPCWSTR pszImagePath);
	HRESULT SetTosLink(LPCWSTR pszUrl);
	HRESULT SetPrivacyLink(LPCWSTR pszUrl);
	HRESULT SetHelpLink(LPCWSTR pszUrl);
	HRESULT SetTemplate(LoginTemplate *pTemplate);
	HRESULT SetCommand(LoginCommand *pCommand);

protected:
	ULONG	ref;
	GUID	id;
	LPWSTR	name;
	LPWSTR	description;
	LPWSTR	imagePath;
	LPWSTR	tosUrl;
	LPWSTR	privacyUrl;
	LPWSTR	helpUrl;
	LoginTemplate *pageTemplate;
	LoginCommand *command;
};

#endif //NULLSOFT_AUTH_LOGINPROVIDER_HEADER