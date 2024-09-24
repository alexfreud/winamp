#ifndef NULLSOFT_AUTH_LOGIN_CONFIG_HEADER
#define NULLSOFT_AUTH_LOGIN_CONFIG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class LoginConfig
{
protected:
	LoginConfig();
	~LoginConfig();
public:
	static HRESULT CreateInstance(LoginConfig **instance);

public:
	ULONG AddRef();
	ULONG Release();

public:
	HRESULT GetPath(LPCSTR *ppPath);
	DWORD ReadAnsiStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize);
  	UINT ReadInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nDefault);
	HRESULT WriteAnsiStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpString);
	HRESULT WriteInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nValue);
	

protected:
	ULONG ref;
	LPSTR configPath;
	BOOL pathValidated;
	void *buffer;


};


#endif //NULLSOFT_AUTH_LOGIN_CONFIG_HEADER