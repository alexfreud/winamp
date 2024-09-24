#ifndef NULLSOFT_WINAMP_CACHE_GROUP_HEADER
#define NULLSOFT_WINAMP_CACHE_GROUP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_omcachegroup.h"
#include <vector>


class CacheRecord;
class CacheManager;

class CacheGroup : public ifc_omcachegroup
{

protected:
	CacheGroup(LPCWSTR pszName);
	~CacheGroup();

public:
	static HRESULT CreateInstance(LPCWSTR pszName, CacheGroup **instace);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	HRESULT SetOwner(CacheManager *group);
	HRESULT IsEqual(CacheGroup *group);
	HRESULT IsEqualName(LPCWSTR pszGroup);
	HRESULT IsEmpty();

	HRESULT GetName(LPWSTR pszBuffer, UINT cchBufferMax);

	HRESULT Find(LPCWSTR pszName, BOOL fInsertMissing, CacheRecord **recordOut, BOOL *created);
	HRESULT Delete(LPCWSTR pszName);
	HRESULT Clear();
	
	HRESULT Store(LPCWSTR pszName, LPCWSTR pszPath);
	HRESULT Load();

	HRESULT GetPath(LPWSTR pszBuffer, UINT cchBufferMax);

protected:
	HRESULT Sort();

protected:

	RECVS_DISPATCH;
	typedef std::vector<CacheRecord*> RecordList;

protected:
	size_t ref;
	LPWSTR	name;
	CacheManager *owner;
	RecordList	recordList;
};

#endif //NULLSOFT_WINAMP_CACHE_HEADER