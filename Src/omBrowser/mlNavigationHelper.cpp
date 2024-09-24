#include "main.h"
#include "./mlNavigationHelper.h"
#include "./ifc_omcachemanager.h"
#include "./ifc_omcachegroup.h"
#include "./ifc_omcacherecord.h"
#include "./ifc_mlnavigationcallback.h"
#include "./resource.h"
#include "./ifc_wasabihelper.h"

#include "../Plugins/General/gen_ml/ml.h"

#define _ML_HEADER_IMPMLEMENT
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"
#undef _ML_HEADER_IMPMLEMENT

#include <shlwapi.h>
#include <strsafe.h>
#include <algorithm>

MlNavigationHelper::MlNavigationHelper(HWND hLibraryWnd, ifc_omcachegroup *cacheGroup)
	: ref(1), hLibrary(hLibraryWnd), defaultIndex(-1), cacheGroup(cacheGroup), lastCookie(0)
{
	InitializeCriticalSection(&lock);

	if (NULL != cacheGroup)
		cacheGroup->AddRef();
}

MlNavigationHelper::~MlNavigationHelper()
{
	EnterCriticalSection(&lock);

	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
	{
		ifc_mlnavigationcallback *callback = iter->second;
		if (NULL != callback) callback->Release();
	}

	size_t index = recordList.size();
	while(index--)
	{
		Plugin_FreeString(recordList[index].name);
	}

	if (NULL != cacheGroup)
		cacheGroup->Release();

	LeaveCriticalSection(&lock);
	DeleteCriticalSection(&lock);
}

HRESULT MlNavigationHelper::CreateInstance(HWND hLibrary, ifc_omcachemanager *cacheManager, MlNavigationHelper **instance)
{
	if (NULL == instance) return E_POINTER;

	if (NULL == hLibrary || FALSE == IsWindow(hLibrary) || NULL == cacheManager)
	{
		*instance = NULL;
		return E_INVALIDARG;
	}

	ifc_omcachegroup *group = NULL;
	HRESULT hr = cacheManager->Find(L"icons", TRUE, &group, NULL);
	if (SUCCEEDED(hr) && group != NULL)
	{
		*instance = new MlNavigationHelper(hLibrary, group);
		if (NULL == *instance) 
		{
			hr = E_OUTOFMEMORY;
		}

		group->Release();
	}

	return hr;
}

size_t MlNavigationHelper::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t MlNavigationHelper::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int MlNavigationHelper::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_MlNavigationHelper))
		*object = static_cast<ifc_mlnavigationhelper*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}
	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT MlNavigationHelper::GetDefaultIndex(int *index)
{
	if (NULL == index) 
		return E_POINTER;

	EnterCriticalSection(&lock);

	if (-1 == defaultIndex)
	{
		WCHAR szPath[MAX_PATH] = {0};
		HRESULT hr = Plugin_MakeResourcePath(szPath, ARRAYSIZE(szPath), Plugin_GetInstance(), 
						RT_RCDATA, MAKEINTRESOURCE(IDR_SERVICEICON_IMAGE), RESPATH_COMPACT);

		if (SUCCEEDED(hr))
		{
			UpdateImageList(szPath, &defaultIndex);
		}
	}
	*index = defaultIndex;

	LeaveCriticalSection(&lock);

	return S_OK;
}


HRESULT	MlNavigationHelper::UpdateImageList(LPCWSTR address, INT *index)
{
	if (NULL == address || NULL == index)
	{
		if (NULL != index) *index = -1;
		return E_INVALIDARG;
	}

	HMLIMGLST hmlil =  MLNavCtrl_GetImageList(hLibrary);
	if (NULL == hmlil) return E_UNEXPECTED;

	MLIMAGESOURCE source = {0};
	source.cbSize = sizeof(source);
	source.lpszName = address;
	source.type = SRC_TYPE_PNG;
	source.bpp = 24;
	source.flags = ISF_LOADFROMFILE | ISF_FORCE_BPP | ISF_PREMULTIPLY;

	MLIMAGELISTIMAGESIZE imageSize = {0};
	imageSize.hmlil = hmlil;
	if (FALSE != MLImageList_GetImageSize(hLibrary, &imageSize))
	{
		source.cxDst = imageSize.cx;
		source.cyDst = imageSize.cy;
		source.flags |= ISF_FORCE_SIZE | ISF_SCALE;
	}

	if (FALSE == MLImageLoader_CheckExist(hLibrary, &source))
		return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

	MLIMAGELISTITEM item = {0};
	item.cbSize = sizeof(item);
	item.hmlil = hmlil;
	item.filterUID = MLIF_FILTER3_UID;
	item.pmlImgSource = &source;

	HRESULT hr = S_OK;
	INT iImage = *index;
	if (iImage >= 0)
	{
		INT imageCount = MLImageList_GetImageCount(hLibrary, item.hmlil);
		if (iImage >= imageCount) iImage = -1;
	}

	if (iImage >=0)
	{
		item.mlilIndex = iImage;
		if (FALSE == MLImageList_Replace(hLibrary, &item))
			hr = E_FAIL;
	}
	else
	{
		iImage = MLImageList_Add(hLibrary, &item);
		if (-1 == iImage) hr = E_FAIL;
	}

	*index = iImage;
	return hr;
}

HRESULT MlNavigationHelper::ImageLocator(LPCWSTR address, INT *index)
{
	if (NULL == address) return E_INVALIDARG;

	HRESULT hr;
	BOOL runtimeOnly = TRUE;

	if (PathIsURL(address) && 
		CSTR_EQUAL != CompareString(CSTR_INVARIANT, NORM_IGNORECASE, address, 6, L"res://", -1))
	{
		runtimeOnly = FALSE;
	}
	else
	{
		hr = UpdateImageList(address, index);
		if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) != hr)
			return hr;
	}

	BOOL fCreated = FALSE;
	ifc_omcacherecord *cacheRecord = NULL;
	hr = cacheGroup->Find(address, TRUE, &cacheRecord, &fCreated);
	if (SUCCEEDED(hr) && cacheRecord != NULL)
	{
		cacheRecord->RegisterCallback(this);

		if (FALSE != fCreated && FALSE != runtimeOnly)
		{
			cacheRecord->SetFlags(ifc_omcacherecord::flagNoStore, ifc_omcacherecord::flagNoStore);
		}

		WCHAR szBuffer[MAX_PATH * 2] = {0};
		if (SUCCEEDED(cacheRecord->GetPath(szBuffer, ARRAYSIZE(szBuffer))))
		{
			hr = UpdateImageList(szBuffer, index);
		}

		cacheRecord->Release();
	}
	return hr;
}

HRESULT MlNavigationHelper::QueryIndex(LPCWSTR pszName, INT *index, BOOL *defaultUsed)
{
	if (NULL == cacheGroup)
		return E_UNEXPECTED;

	if (NULL == index)
	{
		if (NULL != defaultUsed) *defaultUsed = FALSE;
		return E_POINTER;
	}

	if (NULL == pszName || L'\0' == *pszName)
	{
		if (SUCCEEDED(GetDefaultIndex(index)))
		{
			if (NULL != defaultUsed) *defaultUsed = TRUE;
		}
		else
		{
			*index = 0;
			if (NULL != defaultUsed) *defaultUsed = FALSE;
		}
		return S_OK;
	}

	EnterCriticalSection(&lock);

	RECORD *record = Find(pszName);
	if (NULL == record)
	{ // create new
		RECORD rec = {0};

		size_t index = recycledList.size();
		if (0 != index)
		{
			rec.index = recycledList[index - 1];
			recycledList.pop_back();
		}
		else
		{
			rec.index = -1;
		}

		rec.ref = 1;
		rec.name = Plugin_CopyString(pszName);
		recordList.push_back(rec);
		Sort();
		// locate record again;
		record = Find(pszName);
		ImageLocator(record->name, &record->index);
	}
	else
	{
		record->ref++;
		if (-1 == record->index)
			ImageLocator(record->name, &record->index);
	}

	if(-1 == record->index)
	{
		if (SUCCEEDED(GetDefaultIndex(index)) && NULL != defaultUsed) 
			*defaultUsed = TRUE;
	}
	else
	{
		*index = record->index;
	}
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT MlNavigationHelper::ReleaseIndex(LPCWSTR pszName)
{
	if(NULL == pszName || L'\0' == *pszName)
		return E_INVALIDARG;

	EnterCriticalSection(&lock);

	RECORD *record = Find(pszName);
	if (NULL != record)
	{
		record->ref--;
		if (0 == record->ref)
		{
			if (-1 != record->index)
				recycledList.push_back(record->index);

			Plugin_FreeString(record->name);
			record->name = NULL;

			size_t index = recordList.size();
			while(index--)
			{
				if (&recordList[index] == record)
				{
					recordList.erase(recordList.begin() + index);
					break;
				}
			}
		}
	}

	LeaveCriticalSection(&lock);
	return (NULL != record) ? S_OK : S_FALSE;
}

HRESULT MlNavigationHelper::RegisterAlias(LPCWSTR pszName, LPCWSTR pszAddress)
{
	if (NULL == cacheGroup)
		return E_UNEXPECTED;

	BOOL fCreated = FALSE;
	ifc_omcacherecord *record = NULL;
	HRESULT hr = cacheGroup->Find(pszName, TRUE, &record, &fCreated);
	if (SUCCEEDED(hr) && record != NULL)
	{
		if (FALSE != fCreated)
		{
			record->SetFlags(ifc_omcacherecord::flagNoStore, ifc_omcacherecord::flagNoStore);
		}

		hr = record->SetPath(pszAddress);
		record->RegisterCallback(this);
		record->Release();
	}
	return hr;
}

HWND MlNavigationHelper::GetLibrary()
{
	return hLibrary;
}

typedef struct __CACHECHANGEAPCPARAM
{
	MlNavigationHelper *instance;
	ifc_omcacherecord *record;
} CACHECHANGEAPCPARAM;

static void CALLBACK MlNavigationHelper_CacheRecordPathChangedApc(ULONG_PTR data)
{
	CACHECHANGEAPCPARAM *param = (CACHECHANGEAPCPARAM*)data;
	if (NULL != param)
	{
		if (NULL != param->instance)
		{
			param->instance->CacheRecordPathChangedApc(param->record);
			param->instance->Release();
		}
		if (NULL != param->record)
			param->record->Release();
		
		free(param);
	}
}

void MlNavigationHelper::CacheRecordPathChanged(ifc_omcacherecord *cacheRecord)
{
	if (NULL == cacheRecord) 
		return;

	HRESULT hr = E_FAIL;
	CACHECHANGEAPCPARAM *param = (CACHECHANGEAPCPARAM*)calloc(1, sizeof(CACHECHANGEAPCPARAM));
	if (NULL != param)
	{
		param->instance = this;
		param->instance->AddRef();
		param->record = cacheRecord;
		param->record->AddRef();

		ifc_wasabihelper *wasabi = NULL;
		if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)) && wasabi != NULL)
		{
			api_application *application = NULL;
			if (SUCCEEDED(wasabi->GetApplicationApi(&application)) && application != NULL)
			{
				HANDLE hThread = application->main_getMainThreadHandle();
				if (NULL != hThread && 
					0 != QueueUserAPC(MlNavigationHelper_CacheRecordPathChangedApc, hThread, (ULONG_PTR)param))
				{
					hr = S_OK;
				}
				application->Release();
			}
			wasabi->Release();
		}

		if (FAILED(hr))
		{
			if (NULL != param->instance) param->instance->Release();
			if (NULL != param->record) param->record->Release();
			free(param);
			param = NULL;
		}
	}
}

void MlNavigationHelper::CacheRecordPathChangedApc(ifc_omcacherecord *cacheRecord)
{
	if (NULL == cacheRecord) 
		return;

	WCHAR szBuffer[1024] = {0};
	if (FAILED(cacheRecord->GetName(szBuffer, ARRAYSIZE(szBuffer))))
		return;

	EnterCriticalSection(&lock);

	RECORD *record = Find(szBuffer);
	if (NULL != record && 
		SUCCEEDED(cacheRecord->GetPath(szBuffer, ARRAYSIZE(szBuffer))) &&
		SUCCEEDED(UpdateImageList(szBuffer, &record->index)))
	{
		for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
		{
			ifc_mlnavigationcallback *callback = iter->second;
			callback->ImageChanged(record->name, record->index);
		}
	}

	LeaveCriticalSection(&lock);
}

HRESULT MlNavigationHelper::RegisterCallback(ifc_mlnavigationcallback *callback, UINT *cookie)
{
	if (NULL == cookie) return E_POINTER;
	*cookie = 0;

	if (NULL == callback) 
		return E_INVALIDARG;

	EnterCriticalSection(&lock);

	*cookie = ++lastCookie;

	callbackMap.insert({ *cookie, callback });
	callback->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT MlNavigationHelper::UnregisterCallback(UINT cookie)
{
	if (0 == cookie) return E_INVALIDARG;

	ifc_mlnavigationcallback *callback = NULL;
	EnterCriticalSection(&lock);

	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
	{
		if (cookie == iter->first)
		{
			callback = iter->second;
			callbackMap.erase(iter);
			break;
		}
	}

	LeaveCriticalSection(&lock);

	if (NULL != callback)
	{
		callback->Release();
		return S_OK;
	}

	return S_FALSE;
}

__inline static int __cdecl MlNavigationHelper_RecordComparer(const void *elem1, const void *elem2)
{
	LPCWSTR s1 = ((MlNavigationHelper::RECORD*)elem1)->name;
	LPCWSTR s2 = ((MlNavigationHelper::RECORD*)elem2)->name;

	if (NULL == s1 || NULL == s2) return ((INT)(ULONG_PTR)(s1 - s2));
	return CompareString(CSTR_INVARIANT, NORM_IGNORECASE, s1, -1, s2, -1) - 2;
}
__inline static int __cdecl MlNavigationHelper_RecordComparer_V2(const void* elem1, const void* elem2)
{
	return MlNavigationHelper_RecordComparer(elem1, elem2) < 0;
}

__inline static int __cdecl MlNavigationHelper_RecordSearch(const void *elem1, const void *elem2)
{
	LPCWSTR s1 = (LPCWSTR)elem1;
	LPCWSTR s2 = ((MlNavigationHelper::RECORD*)elem2)->name;

	if (NULL == s1 || NULL == s2) return ((INT)(ULONG_PTR)(s1 - s2));
	INT r = CompareString(CSTR_INVARIANT, NORM_IGNORECASE, s1, -1, s2, -1) - 2; 
	return r;
}

HRESULT MlNavigationHelper::Sort()
{
	HRESULT hr;
	if (recordList.size() < 2)
	{
		hr = S_FALSE;
	}
	else
	{
		//qsort(recordList.begin(), recordList.size(), sizeof(RECORD), MlNavigationHelper_RecordComparer);
		std::sort(recordList.begin(), recordList.end(), 
			[&](RECORD lhs, RECORD rhs) -> bool
			{
				return MlNavigationHelper_RecordComparer_V2(&lhs, &rhs);
			}
			);

		hr = S_OK;
	}
	return hr;
}

MlNavigationHelper::RECORD *MlNavigationHelper::Find(LPCWSTR pszName)
{
	if (0 == recordList.size()) 
		return NULL;

	//return (RECORD*)bsearch(pszName, recordList.begin(), recordList.size(), 
	//	sizeof(RECORD), MlNavigationHelper_RecordSearch);

	auto it = std::find_if(recordList.begin(), recordList.end(),
		[&](RECORD upT) -> bool
		{
			return MlNavigationHelper_RecordSearch(pszName, &upT) == 0;
		}
	);
	if (it == recordList.end())
	{
		return NULL;
	}

	return &(* it);
}

#define CBCLASS MlNavigationHelper
START_MULTIPATCH;
 START_PATCH(MPIID_MLNAVIGATIONHELPER)
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, ADDREF, AddRef);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, RELEASE, Release);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, API_GETDEFAULTINDEX, GetDefaultIndex);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, API_QUERYINDEX, QueryIndex);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, API_RELEASEINDEX, ReleaseIndex);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, API_REGISTERALIAS, RegisterAlias);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, API_REGISTERCALLBACK, RegisterCallback);
  M_CB(MPIID_MLNAVIGATIONHELPER, ifc_mlnavigationhelper, API_UNREGISTERCALLBACK, UnregisterCallback);
 NEXT_PATCH(MPIID_OMCACHECALLBACK)
  M_CB(MPIID_OMCACHECALLBACK, ifc_omcachecallback, ADDREF, AddRef);
  M_CB(MPIID_OMCACHECALLBACK, ifc_omcachecallback, RELEASE, Release);
  M_CB(MPIID_OMCACHECALLBACK, ifc_omcachecallback, QUERYINTERFACE, QueryInterface);
  M_VCB(MPIID_OMCACHECALLBACK, ifc_omcachecallback, API_PATHCHANGED, CacheRecordPathChanged);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS