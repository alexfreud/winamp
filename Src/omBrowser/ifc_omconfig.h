#ifndef NULLSOFT_WINAMP_OMBROWSER_CONFIG_INI_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_CONFIG_INI_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {73661787-0ACC-4692-B6A2-41D14E81A8CE}
static const GUID IFC_OmConfig = 
{ 0x73661787, 0xacc, 0x4692, { 0xb6, 0xa2, 0x41, 0xd1, 0x4e, 0x81, 0xa8, 0xce } };

#include <bfc/dispatch.h>

class ifc_omconfigcallback;

class __declspec(novtable) ifc_omconfig : public Dispatchable
{
protected:
	ifc_omconfig() {}
	~ifc_omconfig() {}

public:
	HRESULT GetPath(wchar_t *pszBuffer, int cchBufferMax);
    DWORD ReadString(const wchar_t *lpSectionName, const wchar_t *lpKeyName, const wchar_t *lpDefault, wchar_t *lpReturnedString, DWORD nSize);
	UINT ReadInt(const wchar_t *lpSectionName, const wchar_t *lpKeyName, int nDefault);
	BOOL ReadBool(const wchar_t *lpSectionName, const wchar_t *lpKeyName, BOOL bDefault);
	HRESULT WriteString(const wchar_t *lpSectionName, const wchar_t *lpKeyName, const wchar_t * lpString);
	HRESULT WriteInt(const wchar_t *lpSectionName, const wchar_t *lpKeyName, int nValue);
	HRESULT WriteBool(const wchar_t *lpSectionName, const wchar_t *lpKeyName, BOOL bValue);

	HRESULT RegisterCallback(ifc_omconfigcallback *callback, unsigned int *cookie);
	HRESULT UnregisterCallback(unsigned int cookie);

public:
	DISPATCH_CODES
	{
		API_GETPATH				= 10,
		API_READSTRING			= 20,
		API_READINT				= 30,
		API_READBOOL			= 40,
		API_WRITESTRING			= 50,
		API_WRITEINT				= 60,
		API_WRITEBOOL			= 70,
		API_REGISTERCALLBACK	= 80,
		API_UNREGISTERCALLBACK	= 90,
	};
};

inline HRESULT ifc_omconfig::GetPath(LPWSTR pszBuffer, INT cchBufferMax)
{
	return _call(API_GETPATH, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax); 
}

inline DWORD ifc_omconfig::ReadString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize)
{
	return _call(API_READSTRING, (DWORD)0, lpSectionName, lpKeyName, lpDefault, lpReturnedString, nSize);
}

inline UINT ifc_omconfig::ReadInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, INT nDefault)
{
	return _call(API_READINT, (UINT)0, lpSectionName, lpKeyName, nDefault); 
}

inline BOOL ifc_omconfig::ReadBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault)
{
	return _call(API_READBOOL, (BOOL)FALSE, lpSectionName, lpKeyName, bDefault); 
}

inline HRESULT ifc_omconfig::WriteString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
	return _call(API_WRITESTRING, (HRESULT)E_NOTIMPL, lpSectionName, lpKeyName, lpString); 
}

inline HRESULT ifc_omconfig::WriteInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, INT nValue)
{
	return _call(API_WRITEINT, (HRESULT)E_NOTIMPL, lpSectionName, lpKeyName, nValue); 
}

inline HRESULT ifc_omconfig::WriteBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue)
{
	return _call(API_WRITEBOOL, (HRESULT)E_NOTIMPL, lpSectionName, lpKeyName, bValue); 
}

inline HRESULT ifc_omconfig::RegisterCallback(ifc_omconfigcallback *callback, unsigned int *cookie)
{
	return _call(API_REGISTERCALLBACK, (HRESULT)API_REGISTERCALLBACK, callback, cookie); 
}

inline HRESULT ifc_omconfig::UnregisterCallback(unsigned int cookie)
{
	return _call(API_UNREGISTERCALLBACK, (HRESULT)E_NOTIMPL, cookie); 
}

#endif // NULLSOFT_WINAMP_OMBROWSER_CONFIG_INI_INTERFACE_HEADER