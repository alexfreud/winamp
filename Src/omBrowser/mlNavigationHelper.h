#ifndef NULLSOFT_WINAMP_ML_NAVIGATION_HELPER_HEADER
#define NULLSOFT_WINAMP_ML_NAVIGATION_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_mlnavigationhelper.h"
#include "./ifc_omcachecallback.h"
#include <vector>
#include <map>
#include <bfc/multipatch.h>

class ifc_omcachemanager;
class ifc_omcachegroup;
class ifc_mlnavigationcallback;

#define MPIID_MLNAVIGATIONHELPER	10
#define MPIID_OMCACHECALLBACK		20

class MlNavigationHelper : public MultiPatch<MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper>,
							public MultiPatch<MPIID_OMCACHECALLBACK, ifc_omcachecallback>
{
protected:
	MlNavigationHelper(HWND hLibrary, ifc_omcachegroup *cacheGroup);
	~MlNavigationHelper();

public:
	static HRESULT CreateInstance(HWND hLibrary, ifc_omcachemanager *cacheManager, MlNavigationHelper **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_mlnavigationhelper */
	HRESULT GetDefaultIndex(INT *index);
	HRESULT QueryIndex(LPCWSTR pszName, INT *index, BOOL *defaultUsed);
	HRESULT ReleaseIndex(LPCWSTR pszName);
	HRESULT RegisterAlias(LPCWSTR pszName, LPCWSTR pszAddress);
	HRESULT RegisterCallback(ifc_mlnavigationcallback *callback, UINT *cookie);
	HRESULT UnregisterCallback(UINT cookie);

	/* ifc_omcachecallback */
	void CacheRecordPathChanged(ifc_omcacherecord *cacheRecord);

protected:
	void CacheRecordPathChangedApc(ifc_omcacherecord *cacheRecord);

private:
	friend static void CALLBACK MlNavigationHelper_CacheRecordPathChangedApc(ULONG_PTR data);

public:
	HWND GetLibrary();

protected:
	typedef struct __RECORD
	{
		LPWSTR	name;
		INT		index;
		UINT	ref;
	} RECORD;

	typedef std::vector<RECORD> RecordList;
	typedef std::vector<INT> RecycledList;
	typedef std::map<UINT, ifc_mlnavigationcallback*> CallbackMap;

private:
	friend __inline static int __cdecl MlNavigationHelper_RecordComparer(const void *elem1, const void *elem2);
	friend __inline static int __cdecl MlNavigationHelper_RecordSearch(const void *elem1, const void *elem2);

	HRESULT ImageLocator(LPCWSTR address, INT *index);
	HRESULT	UpdateImageList(LPCWSTR address, INT *index);
	HRESULT Sort();
	RECORD *Find(LPCWSTR pszName);

protected:
	ULONG ref;
	HWND hLibrary;
	INT defaultIndex;
	ifc_omcachegroup *cacheGroup;
	RecordList recordList;
	CallbackMap callbackMap;
	RecycledList recycledList;
	UINT lastCookie;
	CRITICAL_SECTION lock;

protected:
	RECVS_MULTIPATCH;
};

#endif //NULLSOFT_WINAMP_ML_NAVIGATION_HELPER_HEADER