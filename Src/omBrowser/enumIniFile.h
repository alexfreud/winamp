#ifndef NULLSOFT_WINAMP_ENUMERATOR_INI_HEADER
#define NULLSOFT_WINAMP_ENUMERATOR_INI_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omserviceenum.h"
#include "./loaderIni.h"

class ifc_omservicehost;

class EnumIniFile : public ifc_omserviceenum
{
protected:
	EnumIniFile(LPCWSTR pszAddress, ifc_omservicehost *serviceHost);
	~EnumIniFile();
public:
	static HRESULT CreateInstance(LPCWSTR pszAddress, ifc_omservicehost *host, EnumIniFile **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omserviceenum */
	HRESULT Next(ULONG listSize, ifc_omservice **elementList, ULONG *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(ULONG elementCount);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	LPWSTR address;
	ifc_omservicehost *host;
	LoaderIni reader;
	WIN32_FIND_DATA fData;
	HANDLE hFind;
};

#endif //NULLSOFT_WINAMP_ENUMERATOR_INI_HEADER