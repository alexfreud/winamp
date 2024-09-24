#ifndef NULLSOFT_WINAMP_LOADER_INI_HEADER
#define NULLSOFT_WINAMP_LOADER_INI_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>
class ifc_omservice;
class ifc_omservicehost;
class ifc_omstoragehandlerenum;

class LoaderIni
{
public:
	typedef enum
	{
		flagWriteNormal = 0x00000000,
		flagWriteZero = 0x00000001,
		flagHexMode = 0x00000002,
	} WriteFlags;
public:
	LoaderIni();
	~LoaderIni();

public:
	HRESULT Load(LPCWSTR pszAddress, ifc_omservicehost *host, ifc_omservice **serviceOut);
	HRESULT Reload(ifc_omservice *service);
	HRESULT Save(ifc_omservice *service, UINT flags);
	HRESULT RegisterHandlers(ifc_omstoragehandlerenum *enumerator);
	
private:
	HRESULT RequestAnsiBuffer(LPSTR *ppBuffer, UINT *pBufferMax, UINT requestSize);
	HRESULT RequestBuffer(LPWSTR *ppBuffer, UINT *pBufferMax, UINT requestSize);
	HRESULT MakeAnsiPath(LPCWSTR pszAddress);
	HRESULT GetServicePath(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax, BOOL *fGenerated);
	
	HRESULT WideCharToAnsiBuffer(UINT codePage, DWORD flags, LPCWSTR pszWideChar, INT cchWideChar, LPCSTR pDefaultChar, BOOL *pUsedDefaultChar);
	HRESULT MultiByteToBuffer( UINT codePage, DWORD flags, LPCSTR pszMultiByte, INT cbMultiByte);

	HRESULT Write(LPCSTR pszKey, LPCWSTR pszValue);
	HRESULT WriteAnsi(LPCSTR pszKey, LPCSTR pszValue);
	HRESULT WriteUint(LPCSTR pszKey, UINT uValue, UINT flags);
	HRESULT ReadAnsi(LPCSTR pszKey, LPCSTR pszDefault, LPCSTR *ppszValue);
	UINT ReadInt(LPCSTR pszKey, UINT nDefault);
	
private:
	template <class Object, class Getter>
	HRESULT WriteObjectStr(LPCSTR pszKey, Object *object, Getter getter);

	template <class Object, class Getter>
	HRESULT WriteObjectUInt(LPCSTR pszKey, Object *object, Getter getter, UINT flags);

protected:
	CHAR pathAnsi[MAX_PATH];
	LPSTR bufferAnsi;
	UINT bufferAnsiMax;
	LPWSTR buffer;
	UINT bufferMax;
	ifc_omstoragehandlerenum *handlerEnum;
};

#endif //NULLSOFT_WINAMP_LOADER_INI_HEADER