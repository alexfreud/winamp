#ifndef NULLSOFT_AUTH_LOGIN_TEMPLATE_HEADER
#define NULLSOFT_AUTH_LOGIN_TEMPLATE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class __declspec(novtable) LoginTemplate
{
protected:
	LoginTemplate() {}
	~LoginTemplate(){}

public:
	virtual ULONG AddRef() = 0;
	virtual ULONG Release() = 0;

	virtual HRESULT GetType(GUID *templateUid) = 0;
	
	virtual HRESULT SetParameter(LPCWSTR pszKey, LPCWSTR pszValue) = 0;
	virtual HRESULT IsValid() = 0;
	virtual HRESULT IsIdentical(LoginTemplate *test) = 0;

	virtual HWND CreatePage(HWND hLoginbox, HWND hParent) = 0;

};

#endif //NULLSOFT_AUTH_LOGIN_TEMPLATE_HEADER