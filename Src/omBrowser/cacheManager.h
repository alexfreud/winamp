#ifndef NULLSOFT_WINAMP_CACHE_MANAGER_HEADER
#define NULLSOFT_WINAMP_CACHE_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_omcachemanager.h"
#include <vector>

class CacheGroup;

class CacheManager : public ifc_omcachemanager
{
protected:
	CacheManager();
	~CacheManager();

public:
	static HRESULT CreateInstance(CacheManager **instace);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

    HRESULT Load();

	/* group managment */
	HRESULT Find(LPCWSTR pszGroup, BOOL fInsertMissing, CacheGroup **groupOut, BOOL *created);
	HRESULT Delete(LPCWSTR pszGroup);
	HRESULT Clear();

	HRESULT GetPath(LPWSTR pszBuffer, UINT cchBufferMax);

protected:
	RECVS_DISPATCH;
	typedef std::vector<CacheGroup*> GroupList;

protected:
	size_t ref;
	GroupList groupList;
};

#endif //NULLSOFT_WINAMP_CACHE_MANAGER_HEADER