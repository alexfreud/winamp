#ifndef NULLSOFT_WINAMP_CACHE_RECORD_HEADER
#define NULLSOFT_WINAMP_CACHE_RECORD_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_omcacherecord.h"
#include <vector>

class CacheGroup;
class CacheDownloader;

class CacheRecord : public ifc_omcacherecord
{
public:
	typedef enum
	{
		flagDownloadFailed = 0x80000000,
	} Flags;


protected:
	CacheRecord(LPCWSTR pszName, LPCWSTR pszAddress, UINT uFlags);
	~CacheRecord();

public:
	static HRESULT CreateInstance(LPCWSTR pszName, LPCWSTR pszAddress, UINT uFlags, CacheRecord **instance);
	static INT Compare(CacheRecord *record1, CacheRecord *record2);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	HRESULT SetOwner(CacheGroup *group);
	HRESULT IsEqual(CacheRecord *record);
	HRESULT IsEqualName(LPCWSTR pszName);
	INT CompareTo(LPCWSTR pszName);

	HRESULT GetName(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetPath(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT SetPath(LPCWSTR pszPath);
	HRESULT GetFlags(UINT *puFlags);
	HRESULT SetFlags(UINT uFlags, UINT uMask);

	HRESULT Download();

	HRESULT RegisterCallback(ifc_omcachecallback *callback);
	HRESULT UnregisterCallback(ifc_omcachecallback *callback);

	HRESULT DownloadCompleted(LPCWSTR pszFile, INT errorCode);

	HRESULT GetBasePath(LPWSTR pszBuffer, UINT cchBufferMax);

protected:
	RECVS_DISPATCH;
	typedef std::vector<ifc_omcachecallback*> CallbackList;

protected:
	size_t ref;
	CacheGroup *owner;
	LPWSTR name;
	LPWSTR path;
	UINT flags;
	CacheDownloader *downloader;
	CallbackList	*callbackList;
	CRITICAL_SECTION lock;
};

#endif //NULLSOFT_WINAMP_CACHE_RECORD_HEADER