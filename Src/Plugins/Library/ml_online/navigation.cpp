#include "main.h"
#include "./navigation.h"
#include "./resource.h"
#include "./api__ml_online.h"
#include "./local_menu.h"
#include "./commands.h"
#include "./config.h"

#include "../omBrowser/browserView.h"
#include "../winamp/wa_ipc.h"

#include "./serviceHost.h"
#include "./serviceHelper.h"
#include "./browserEvent.h"

#include <ifc_omservice.h>
#include <ifc_omserviceeditor.h>
#include <storageIni.h>
#include <ifc_omserviceenum.h>
#include <ifc_mlnavigationhelper.h>
#include <ifc_omserviceeventmngr.h>
#include <ifc_ombrowserwndmngr.h>
#include <ifc_ombrowserwndenum.h>
#include <ifc_ombrowsereventmngr.h>

#include "../../General/gen_ml/menu.h"
#include "../../General/gen_ml/ml_ipc_0313.h"

#include <vector>
#include "../nu/sort.h"

#include <shlwapi.h>
#include <strsafe.h>
#include <algorithm>

#define NAVITEM_PREFIX			L"om_svc_"
#define E_NAVITEM_UNKNOWN		E_NOINTERFACE

typedef struct __NAVASYNCPARAM
{
	Navigation *instance;
	NavigationCallback callback;
	ULONG_PTR	param;
}NAVASYNCPARAM;



static BOOL Navigation_CheckInvariantName(LPCWSTR pszInvarian)
{
	INT cchInvariant = (NULL != pszInvarian) ? lstrlen(pszInvarian) : 0;
	INT cchPrefix = ARRAYSIZE(NAVITEM_PREFIX) - 1;
	return (cchInvariant > cchPrefix &&
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, NAVITEM_PREFIX, cchPrefix, pszInvarian, cchPrefix));
}


Navigation::Navigation()
	: ref(1), cookie(0), hRoot(NULL), hLibrary(NULL)
{
}

Navigation::~Navigation()
{
}

HRESULT Navigation::CreateInstance(Navigation **instance)
{
	if (NULL == instance) return E_POINTER;

	HRESULT hr;

	Navigation *navigation = new Navigation();
	if (NULL != navigation)
	{
		hr = navigation->Initialize();
		if (FAILED(hr))
		{
			navigation->Release();
			navigation = NULL;
		}
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}
	
	*instance = navigation;
	return hr;
}

size_t Navigation::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t Navigation::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int Navigation::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_MlNavigationCallback))
		*object = static_cast<ifc_mlnavigationcallback*>(this);
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

HRESULT Navigation::Initialize()
{
	hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	if (0 == cookie)
	{
		ifc_mlnavigationhelper *navHelper;
		if (NULL != OMUTILITY && SUCCEEDED(OMUTILITY->GetMlNavigationHelper(Plugin_GetLibrary(), &navHelper)))
		{		
			navHelper->RegisterCallback(this, &cookie);
			navHelper->Release();
		}
	}

	ifc_omservice *service;

	MLNavCtrl_BeginUpdate(hLibrary, NUF_LOCK_TOP);

	// TODO make thie configurable?
	if (SUCCEEDED(ServiceHelper_Create(ROOTSERVICE_ID, MAKEINTRESOURCE(IDS_ONLINE_SERVICES), 
					NULL, L"http://client.winamp.com/services", SVCF_SPECIAL | SVCF_SUBSCRIBED, 1, FALSE, &service)))
	{
		hRoot = CreateItemInt(NULL, service);
		service->Release();
	}

	if (NULL == hRoot) 
	{
		MLNavCtrl_EndUpdate(hLibrary);
		return E_FAIL;
	}


	ifc_omserviceenum *enumerator;
	if (SUCCEEDED(ServiceHelper_Load(&enumerator)))
	{
		ifc_omservice *service;
		std::vector<ifc_omservice*> serviceList;
		while (S_OK == enumerator->Next(1, &service, NULL))
		{
			if (S_OK == ServiceHelper_IsSubscribed(service))
			{
				serviceList.push_back(service);
			}
			else
				service->Release();
		}
		enumerator->Release();

		size_t count = serviceList.size();
		Order(serviceList);
		for(size_t i =0; i < count; i++)
			{
				service = serviceList[i];
				CreateItemInt(hRoot, service);
				service->Release();
			}
	}
	
	MLNavCtrl_EndUpdate(hLibrary);

	return S_OK;
}

HRESULT Navigation::Finish()
{
	if (0 != cookie)
	{
		ifc_mlnavigationhelper *navHelper;
		if (NULL != OMUTILITY && SUCCEEDED(OMUTILITY->GetMlNavigationHelper(Plugin_GetLibrary(), &navHelper)))
		{		
			navHelper->UnregisterCallback(cookie);
			navHelper->Release();
		}
	}

	if (NULL != OMBROWSERMNGR)
	{
		OMBROWSERMNGR->Finish();
	}

	return S_OK;
}

HRESULT Navigation::SaveOrder()
{
	if (NULL == hRoot || NULL == hLibrary)
		return E_UNEXPECTED;
	
	LPSTR buffer = NULL;
	INT count = MLNavItem_GetChildrenCount(hLibrary, hRoot);
	if (count > 0)
	{
		size_t bufferMax = 11 * count;
        buffer = Plugin_MallocAnsiString(bufferMax);
		if (NULL == buffer) return E_OUTOFMEMORY;
		*buffer = '\0';
	
		LPSTR cursor = buffer;
		size_t remaining = bufferMax;

		NAVITEM item;
		item.cbSize = sizeof(item);
		item.mask = NIMF_PARAM;
		item.hItem = MLNavItem_GetChild(hLibrary, hRoot);
		while (NULL != item.hItem)
		{
			if (FALSE != MLNavItem_GetInfo(hLibrary, &item))
			{
				ifc_omservice *service = (ifc_omservice*)item.lParam;
				if (NULL != service)
				{
					UINT serviceFlags;
					if (SUCCEEDED(service->GetFlags(&serviceFlags)) &&
						0 == (SVCF_SPECIAL & serviceFlags))
					{
						if (cursor == buffer || 
							SUCCEEDED(StringCchCopyExA(cursor, remaining, ";", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE)))
						{
							StringCchPrintfExA(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, "%d", service->GetId());
						}
					}
				}
			}
			item.hItem = MLNavItem_GetNext(hLibrary, item.hItem);
		}
		
	}

	Config_WriteStr("Navigation", "order", buffer);
	Plugin_FreeAnsiString(buffer);

	return S_OK;
}

//static int __fastcall Navigation_OrderComparer(const void *elem1, const void *elem2, const void *context)
//{
//	std::vector<UINT> *orderList = (std::vector<UINT>*)context;
//	
//	UINT serviceId;
//	size_t index1, index2;
//	size_t count = orderList->size();
//
//	serviceId = (*(ifc_omservice**)elem1)->GetId();
//    for (index1 = 0; index1 < count && serviceId != orderList->at(index1); index1++);
//
//	serviceId = (*(ifc_omservice**)elem2)->GetId();
//    for (index2 = 0; index2 < count && serviceId != orderList->at(index2); index2++);
//	
//	return (INT)(index1 - index2);
//}
class Navigation_OrderComparer
{
public:
	Navigation_OrderComparer(const void* ctx)
		: context(ctx)
	{
	}

	bool operator()(const void* elem1, const void* elem2)
	{
		std::vector<UINT>* orderList = (std::vector<UINT>*)context;

		UINT serviceId;
		size_t index1, index2;
		size_t count = orderList->size();

		serviceId = ((ifc_omservice*)elem1)->GetId();
		for (index1 = 0; index1 < count && serviceId != orderList->at(index1); index1++);

		serviceId = ((ifc_omservice*)elem2)->GetId();
		for (index2 = 0; index2 < count && serviceId != orderList->at(index2); index2++);

		return (INT)(index1 - index2) < 0;
	}


private:
	const void* context;
};

HRESULT Navigation::Order(std::vector<ifc_omservice*> &list)
{
	size_t listSize = list.size();

	if (listSize < 2) 
		return S_FALSE;
	
	//if (NULL == list) return E_INVALIDARG;

	size_t bufferMax = 16384;
	LPSTR buffer = Plugin_MallocAnsiString(bufferMax);
	if (NULL == buffer) return E_OUTOFMEMORY;
	
	UINT len = Config_ReadStr("Navigation", "order", NULL, buffer, (UINT)bufferMax);
	std::vector<UINT> orderList;

	LPCSTR end = buffer + len;
	LPCSTR block = buffer;
	LPCSTR cursor = block;
	for(;;)
	{
		if (cursor == end || ';' == *cursor)
		{
			if (block != cursor)
			{
				INT serviceId;
				if (FALSE != StrToIntExA(block, STIF_DEFAULT, &serviceId))
					orderList.push_back(serviceId);
			}

			if (cursor == end) break;
			cursor++;
			block = cursor;
		}
		cursor++;
	}
	

	if (0 != orderList.size())
	{
		//nu::qsort(list, listSize, sizeof(ifc_omservice*), &orderList, Navigation_OrderComparer);
		std::sort(list.begin(), list.end(), Navigation_OrderComparer(&orderList));
	}
	
	Plugin_FreeAnsiString(buffer);
	return S_OK;
}

typedef struct __IMAGEAPCPARAM
{
	LPWSTR name;
	INT index;
} IMAGEAPCPARAM;

static void CALLBACK Navigtaion_ImageChangedApc(Navigation *instance, ULONG_PTR param)
{
	IMAGEAPCPARAM *image = (IMAGEAPCPARAM*)param;
	if (NULL == image) return;

	instance->ImageChanged(image->name, image->index);

	Plugin_FreeString(image->name);
	free(image);
}


void Navigation::ImageChanged(LPCWSTR pszName, INT index)
{
	if (NULL == hRoot || NULL == hLibrary || NULL == pszName)
		return;

	
	DWORD libraryTID = GetWindowThreadProcessId(hLibrary, NULL);
	DWORD currentTID = GetCurrentThreadId();
	if (libraryTID != currentTID)
	{
		if (NULL != OMUTILITY)
		{
			IMAGEAPCPARAM *param = (IMAGEAPCPARAM*)calloc(1, sizeof(IMAGEAPCPARAM));
			if (NULL != param)
			{
				param->name = Plugin_CopyString(pszName);
				param->index = index;
				if (FAILED(PostMainThreadCallback(Navigtaion_ImageChangedApc, (ULONG_PTR)param)))
				{
					free(param);
				}
			}
		}
		return; 
	}
	
	WCHAR szBuffer[2048] = {0};
	NAVITEM item = {0};
	item.cbSize = sizeof(item);
	item.mask = NIMF_TEXTINVARIANT | NIMF_PARAM;
	item.pszInvariant = szBuffer;
	item.cchInvariantMax = ARRAYSIZE(szBuffer);
	
	item.hItem = hRoot;
	
	while (NULL != item.hItem)
	{
		if (FALSE != MLNavItem_GetInfo(hLibrary, &item) && 
		FALSE != Navigation_CheckInvariantName(item.pszInvariant))
		{
			ifc_omservice *service = (ifc_omservice*)item.lParam;
			if (NULL != service && 
				SUCCEEDED(service->GetIcon(szBuffer, ARRAYSIZE(szBuffer))) &&
				CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szBuffer, -1, pszName, -1))
			{
					
				item.iImage = index;
				item.iSelectedImage = index;
				item.mask = NIMF_IMAGE | NIMF_IMAGESEL;
				MLNavItem_SetInfo(hLibrary, &item);
				return;
			}
		}
		
		item.hItem = (HNAVITEM)SENDMLIPC(hLibrary, 
						(item.hItem == hRoot) ? ML_IPC_NAVITEM_GETCHILD : ML_IPC_NAVITEM_GETNEXT, 
						(WPARAM)item.hItem); 
		
	}
}



BOOL Navigation::ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result)
{
	if (msg < ML_MSG_TREE_BEGIN || msg > ML_MSG_TREE_END)
		return FALSE;

	HRESULT hr;

	switch(msg)
	{
		case ML_MSG_TREE_ONCREATEVIEW: 
			{
				HWND hView;
				hr = OnCreateView(GetMessageItem(msg, param1), (HWND)param2, &hView);
				*result = (SUCCEEDED(hr)) ? (INT_PTR)hView : NULL;
			}
			return TRUE;

		case ML_MSG_NAVIGATION_CONTEXTMENU:
			hr = OnContextMenu(GetMessageItem(msg, param1), (HWND)param2, MAKEPOINTS(param3));
			*result = SUCCEEDED(hr);
			return TRUE;

		case ML_MSG_NAVIGATION_ONDELETE:
			hr = OnDeleteItem(GetMessageItem(msg, param1));
			*result = SUCCEEDED(hr);
			return TRUE;

		case ML_MSG_NAVIGATION_ONENDTITLEEDIT:
			hr = OnEndTitleEdit(GetMessageItem(msg, param1), (LPCWSTR)param2);
			*result = SUCCEEDED(hr);
			return TRUE;

		case ML_MSG_TREE_ONKEYDOWN:
			hr = OnKeyDown(GetMessageItem(msg, param1), (NMTVKEYDOWN*)param2);
			*result = SUCCEEDED(hr);
			return TRUE;

		case ML_MSG_NAVIGATION_ONDESTROY:
			OnControlDestroy();
			*result = 0;
			return TRUE;
	}

	return FALSE;
}
	
HNAVITEM Navigation::GetActive(ifc_omservice **serviceOut)
{	
	ifc_omservice *service;
	HNAVITEM hActive = (NULL != hLibrary) ?  MLNavCtrl_GetSelection(hLibrary) : NULL;
	if (NULL == hActive || FAILED(GetService(hActive, &service)))
	{
		hActive = NULL;
		service = NULL;
	}
		
	if (NULL != serviceOut) 
		*serviceOut = service;
    else if (NULL != service)
		service->Release();

	return hActive;
}

HWND Navigation::GetActiveView(ifc_omservice **serviceOut)
{
	HWND hView = (NULL != hLibrary) ? ((HWND)SENDMLIPC(hLibrary, ML_IPC_GETCURRENTVIEW, 0)) : NULL;
	if (NULL != hView)
	{
		WCHAR szBuffer[128] = {0};
		if (!GetClassName(hView, szBuffer, ARRAYSIZE(szBuffer)) ||
			CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, szBuffer, -1,
			L"Nullsoft_omBrowserView", -1))
		{
			hView = NULL;
		}
	}

	ifc_omservice *service;
	if (NULL == hView || FALSE == BrowserView_GetService(hView, &service))
	{
		hView = NULL;
		service = NULL;
	}

	if (NULL != serviceOut) 
		*serviceOut = service;
	else if (NULL != service)
		service->Release();
	
	return hView;
}

HRESULT Navigation::SelectItem(HNAVITEM hItem, LPCWSTR pszUrl)
{
	if (NULL == hItem) return E_INVALIDARG;
	
	ifc_omservice *service;
	HRESULT hr = GetService(hItem, &service);
	if (FAILED(hr)) return hr;

	hr = SelectItemInt(hItem, service->GetId(), pszUrl);
	service->Release();

	return hr;
}

HRESULT Navigation::DeleteItem(HNAVITEM hItem)
{
	if (NULL == hItem) return E_INVALIDARG;
	if (NULL == hLibrary) return E_UNEXPECTED;

	ifc_omservice *service;
	if (FAILED(GetService(hItem, &service)))
		return E_FAIL;
	
	HRESULT hr;
	UINT serviceFlags;
	if (FAILED(service->GetFlags(&serviceFlags))) serviceFlags = 0;
	if (0 == (SVCF_SPECIAL & serviceFlags))
	{
		MLNavCtrl_BeginUpdate(hLibrary, 0);
		HNAVITEM hSelection = MLNavCtrl_GetSelection(hLibrary);
		if (hSelection == hItem)
		{
			HNAVITEM hNext = MLNavItem_GetNext(hLibrary, hItem);
			if (NULL == hNext) 
				hNext = MLNavItem_GetPrevious(hLibrary, hItem);

			if (NULL != hNext)
			{
				MLNavItem_Select(hLibrary, hNext);
			}
		}
		
		BOOL result = MLNavCtrl_DeleteItem(hLibrary, hItem);
		hr = (FALSE != result) ? S_OK : E_FAIL;

		MLNavCtrl_EndUpdate(hLibrary);
	}
	else
	{
		hr = E_FAIL;
	}
	
	service->Release();
		
	return hr;
}

HRESULT Navigation::DeleteAll()
{
	if (NULL == hRoot || NULL == hLibrary) return E_UNEXPECTED;

	std::vector<HNAVITEM> itemList;
	HNAVITEM hItem = MLNavItem_GetChild(hLibrary, hRoot);
	while (NULL != hItem)
	{		
		itemList.push_back(hItem);
		hItem = MLNavItem_GetNext(hLibrary, hItem);
	}

	MLNavCtrl_BeginUpdate(hLibrary, 0);

	NAVITEM item;
	item.cbSize = sizeof(item);
	item.mask = NIMF_PARAM;
	
	size_t index = itemList.size();
	while(index--)
	{
		item.hItem = itemList[index];
		if (FALSE != MLNavItem_GetInfo(hLibrary, &item))
		{
			ifc_omservice *service = (ifc_omservice*)item.lParam;
			if (NULL != service)
			{
				service->AddRef();

				UINT serviceFlags;
				if (SUCCEEDED(service->GetFlags(&serviceFlags)) &&
					0 == (SVCF_SPECIAL & serviceFlags) && 
					FALSE != MLNavCtrl_DeleteItem(hLibrary, item.hItem))
				{
				}

				service->Release();
			}
		}
	}

	MLNavCtrl_EndUpdate(hLibrary);
	return S_OK;
}
HRESULT Navigation::InitializeBrowser()
{
	if (NULL == OMBROWSERMNGR) 
		return E_UNEXPECTED;

	HWND hWinamp = Plugin_GetWinamp();

	HRESULT hr = OMBROWSERMNGR->Initialize(NULL, hWinamp);
	if (SUCCEEDED(hr))
	{
		if (S_OK == hr)
		{
			ifc_ombrowsereventmngr *eventManager;
			if (SUCCEEDED(OMBROWSERMNGR->QueryInterface(IFC_OmBrowserEventManager, (void**)&eventManager)))
			{
				BrowserEvent *eventHandler;
				if (SUCCEEDED(BrowserEvent::CreateInstance(&eventHandler)))
				{
					eventManager->RegisterHandler(eventHandler);
					eventHandler->Release();
				}
				eventManager->Release();
			}
		}
	}
	return hr;
}

HRESULT Navigation::CreatePopup(HNAVITEM hItem, HWND *hwnd)
{
	if (NULL == hwnd) return E_POINTER;
	*hwnd = NULL;

	if (NULL == hLibrary) return E_UNEXPECTED;
	if (NULL == hItem) return E_INVALIDARG;

	HRESULT hr;

	ifc_omservice *service;
	hr = GetService(hItem, &service);
	if (SUCCEEDED(hr))
	{
		HWND hWinamp = Plugin_GetWinamp();

		hr = InitializeBrowser();
		if (SUCCEEDED(hr))
		{
			RECT rect;
			HWND hFrame = (HWND)SENDMLIPC(hLibrary, ML_IPC_GETCURRENTVIEW, 0);
			if (NULL == hFrame) 	hFrame = hLibrary;	
			if (NULL == hFrame || FALSE == GetWindowRect(hFrame, &rect))
			{
				hr = E_FAIL;
			}
							
			if (SUCCEEDED(hr))
			{
				rect.left += 16;
				rect.top += 16;
				
				hr = OMBROWSERMNGR->CreatePopup(service, rect.left, rect.top, 
								rect.right - rect.left, rect.bottom - rect.top,	hWinamp, NULL, 0, hwnd);
			}
		}
		service->Release();
	}
	return hr;	
}

static void CALLBACK Navigation_AsyncCallback(ULONG_PTR data)
{
	NAVASYNCPARAM *async = (NAVASYNCPARAM*)data;
	if (NULL == async) return;

	async->callback(async->instance, async->param);
	if (NULL != async->instance)
		async->instance->Release();
	free(async);
}

HRESULT Navigation::PostMainThreadCallback(NavigationCallback callback, ULONG_PTR param)
{
	if (NULL == callback) 
		return E_INVALIDARG;

	NAVASYNCPARAM *async = (NAVASYNCPARAM*)calloc(1, sizeof(NAVASYNCPARAM));
	if (NULL == async) return E_OUTOFMEMORY;

	async->instance = this;
	async->callback = callback;
	async->param = param;
	this->AddRef();

	HRESULT hr;
	if (NULL == OMUTILITY)
		hr = E_FAIL;
	else
	{
		hr = OMUTILITY->PostMainThreadCallback(Navigation_AsyncCallback, (ULONG_PTR)async);
		if (FAILED(hr))
		{
			Release();
			free(async);
		}
	}
	
	return hr;
}

static void CALLBACK Navigation_CreateItemAsyncCallback(Navigation *instance, ULONG_PTR param)
{
	ifc_omservice *service= (ifc_omservice*)param;

	if (NULL != service)
	{
		if (NULL != instance)
			instance->CreateItem(service);
		
		service->Release();
	}
}

HRESULT Navigation::CreateItemAsync(ifc_omservice *service)
{
	if (NULL == service)
		return E_INVALIDARG;;
	
	service->AddRef();

	HRESULT hr = PostMainThreadCallback(Navigation_CreateItemAsyncCallback, (ULONG_PTR)service);
	if (FAILED(hr))
		service->Release();

	return hr;
}

	
HRESULT Navigation::GetService(HNAVITEM hItem, ifc_omservice **service)
{
	WCHAR szBuffer[64] = {0};
	
	if (NULL == service) return E_POINTER;
	*service = NULL;

	if (NULL == hLibrary || NULL == hItem) 
		return E_INVALIDARG;

	NAVITEM itemInfo;
	itemInfo.cbSize = sizeof(NAVITEM);
	itemInfo.hItem = hItem;
	itemInfo.pszInvariant = szBuffer;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.mask = NIMF_PARAM | NIMF_TEXTINVARIANT;

	if (FALSE == MLNavItem_GetInfo(hLibrary, &itemInfo))
		return E_FAIL;

	if (FALSE == Navigation_CheckInvariantName(szBuffer))
		return E_NAVITEM_UNKNOWN;

	*service = (ifc_omservice*)itemInfo.lParam;
	(*service)->AddRef();
	return S_OK;
}

static void CALLBACK Navigtaion_UpdateServiceApc(Dispatchable *object, ULONG_PTR param1, ULONG_PTR param2)
{
	Navigation *navigation = (Navigation*)object;
	if (NULL != navigation)
	{
		ifc_omservice *service = (ifc_omservice*)param1;
		navigation->UpdateService(service, (UINT)param2);
		if (NULL != service) service->Release();
	}
}

HRESULT Navigation::UpdateService(ifc_omservice *service, UINT modifiedFlags)
{
	if (NULL == hLibrary) return E_UNEXPECTED;

	DWORD libraryTID = GetWindowThreadProcessId(hLibrary, NULL);
	DWORD currentTID = GetCurrentThreadId();
	if (libraryTID != currentTID)
	{
		if (NULL != OMUTILITY)
		{
			service->AddRef();
			if (FAILED(OMUTILITY->PostMainThreadCallback2(Navigtaion_UpdateServiceApc, this, (ULONG_PTR)service, (ULONG_PTR)modifiedFlags)))
				service->Release();
		}
		return E_PENDING; 
	}

	HNAVITEM hItem = FindService(service->GetId(), NULL);
	if (NULL == hItem)
		return E_FAIL;
	
	if (0 != (ifc_omserviceeditor::modifiedFlags & modifiedFlags) &&
			S_FALSE == ServiceHelper_IsSubscribed(service))
	{
		DeleteItem(hItem);
		return S_OK;
	}

	NAVITEM itemInfo;
	itemInfo.cbSize = sizeof(NAVITEM);
	itemInfo.hItem = hItem;

	itemInfo.mask = NIMF_IMAGE;
	if (FALSE == MLNavItem_GetInfo(hLibrary, &itemInfo))
		itemInfo.iImage= -1;

	itemInfo.mask = 0;

	WCHAR szName[512] = {0};
	if (0 != (ifc_omserviceeditor::modifiedName & modifiedFlags) && 
		SUCCEEDED(service->GetName(szName, ARRAYSIZE(szName))))
	{
		itemInfo.mask |= NIMF_TEXT;
		itemInfo.pszText = szName;
	}


	if (0 != (ifc_omserviceeditor::modifiedIcon & modifiedFlags))
	{
		ifc_mlnavigationhelper *navHelper;
		if (SUCCEEDED(OMUTILITY->GetMlNavigationHelper(Plugin_GetLibrary(), &navHelper)))
		{
			INT iImage;
			WCHAR szIcon[1024] = {0};
			if (FAILED(service->GetIcon(szIcon, ARRAYSIZE(szIcon))) ||
				FAILED(navHelper->QueryIndex(szIcon, &iImage, NULL)))
			{
				iImage = -1;
			}

			if (itemInfo.iImage != iImage)
			{
				itemInfo.mask |= NIMF_IMAGE | NIMF_IMAGESEL;
				itemInfo.iImage = iImage;
				itemInfo.iSelectedImage = iImage;
			}

			navHelper->Release();
		}
	}

	if (0 != itemInfo.mask)
	{
		if (FALSE == MLNavItem_SetInfo(hLibrary, &itemInfo))
			return E_FAIL;
	}

	NAVITEMINAVLIDATE invalidate;
	invalidate.hItem = hItem;
	invalidate.fErase = FALSE;
	invalidate.prc = NULL;
	MLNavItem_Invalidate(hLibrary, &invalidate);
	
	return S_OK;
}

HNAVITEM Navigation::FindService(UINT serviceId, ifc_omservice **serviceOut)
{
	if (NULL == hRoot || NULL == hLibrary)
	{
		if (NULL != serviceOut) *serviceOut = NULL;
		return NULL;
	}
	
	WCHAR szBuffer[128] = {0};
	NAVITEM item = {0};
	item.cbSize = sizeof(item);
	item.mask = NIMF_TEXTINVARIANT | NIMF_PARAM;
	item.pszInvariant = szBuffer;
	item.cchInvariantMax = ARRAYSIZE(szBuffer);
	
	item.hItem = hRoot;
	
	while (NULL != item.hItem)
	{
		if (FALSE != MLNavItem_GetInfo(hLibrary, &item) && 
		FALSE != Navigation_CheckInvariantName(item.pszInvariant))
		{
			ifc_omservice *service = (ifc_omservice*)item.lParam;
			if (NULL != service && serviceId == service->GetId())
			{
				if (NULL != serviceOut)
				{
					*serviceOut = service;
					service->AddRef();
				}
				return item.hItem;
			}
		}
		
		item.hItem = (HNAVITEM)SENDMLIPC(hLibrary, 
						(item.hItem == hRoot) ? ML_IPC_NAVITEM_GETCHILD : ML_IPC_NAVITEM_GETNEXT, 
						(WPARAM)item.hItem); 
		
	}
		
	if (NULL != serviceOut) *serviceOut = NULL;
	return NULL;
}

HRESULT Navigation::ShowService(UINT serviceId, LPCWSTR pszUrl)
{
	ifc_omservice *service;
	HNAVITEM hItem = FindService(serviceId, &service);
	if (NULL == hItem) return E_FAIL;
	
	HRESULT hr = SelectItemInt(hItem, serviceId, pszUrl);
	service->Release();

	return hr;
}

HNAVITEM Navigation::CreateItem(ifc_omservice *service, int altMode)
{	
	if (NULL == hLibrary || NULL == hRoot) return NULL;
	return CreateItemInt(hRoot, service, altMode);
}

HRESULT Navigation::GenerateServiceName(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	*pszBuffer = L'\0';

	if (NULL == hLibrary || NULL == hRoot) return E_UNEXPECTED;

	if (FAILED(Plugin_CopyResString(pszBuffer, cchBufferMax, MAKEINTRESOURCE(IDS_USERSERVICE_NAME))))
		return E_UNEXPECTED;

	INT cchName = lstrlen(pszBuffer);
	LPWSTR pszFormat = pszBuffer + cchName;
	INT cchFormatMax = cchBufferMax - cchName;
	
	WCHAR szText[512] = {0};
    NAVITEM item = {0};

	item.cbSize = sizeof(item);
	item.mask = NIMF_TEXT;
	item.pszText = szText;
	item.cchTextMax = ARRAYSIZE(szText);

	BOOL fFound = TRUE;

	for(INT index = 1; FALSE != fFound; index++)
	{
		fFound = FALSE;
		if (FAILED(StringCchPrintf(pszFormat, cchFormatMax, L" %d", index)))
		{
			pszFormat = L'\0';
			return E_FAIL;
		}

		item.hItem = MLNavItem_GetChild(hLibrary, hRoot);
		while(NULL != item.hItem)
		{
			if (FALSE != MLNavItem_GetInfo(hLibrary, &item) && 
				CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, item.pszText, -1, pszBuffer, -1))
			{
				fFound = TRUE;
				break;
			}
			item.hItem = MLNavItem_GetNext(hLibrary, item.hItem);
		}
	}

	return S_OK;
}

HRESULT Navigation::CreateUserService(HNAVITEM *itemOut)
{
	HRESULT hr;

	if (NULL != itemOut)
		*itemOut = NULL;

	if (NULL == hRoot) return E_FAIL;
	
	INT serviceId = 710;
	while(NULL != FindService(serviceId, NULL)) serviceId++;

	WCHAR szName[256] = {0};
	if (FAILED(GenerateServiceName(szName, ARRAYSIZE(szName))))
		return E_FAIL;
	
	ifc_omservice *service;
	hr = ServiceHelper_Create(serviceId, szName, NULL, L"about:blank", SVCF_SUBSCRIBED | SVCF_PREAUTHORIZED, 2, TRUE, &service);

    if (SUCCEEDED(hr))
	{
		HNAVITEM hItem = CreateItem(service, 1);
		if (NULL == hItem)
		{
			hr = E_FAIL;
		}
		else
		{
			if (NULL != itemOut)
				*itemOut = hItem;
		}
		
		service->Release();
	}

	return hr;
}

typedef struct __ICONPATCHREC
{
	LPCWSTR iconName;
	LPCWSTR	resourceName;
} ICONPATCHREC;

const static ICONPATCHREC szIconPatch[] = 
{
	//{ L"11000", MAKEINTRESOURCEW(IDR_ICON_AOL)},
	{ L"11001", MAKEINTRESOURCEW(IDR_ICON_SHOUTCASTRADIO)},
	/*{ L"11002", MAKEINTRESOURCEW(IDR_ICON_SHOUTCASTTV)},
	{ L"11003", MAKEINTRESOURCEW(IDR_ICON_WINAMPMUSIC)},
	{ L"11004", MAKEINTRESOURCEW(IDR_ICON_SINGINGFISH)},
	{ L"11005", MAKEINTRESOURCEW(IDR_ICON_MUSICNOW)},
	{ L"11006", MAKEINTRESOURCEW(IDR_ICON_AOL_GAMES)},
	{ L"11007", MAKEINTRESOURCEW(IDR_ICON_IN2TV)},
	{ L"11008", MAKEINTRESOURCEW(IDR_ICON_WINAMREMOTE)},*/
};

HRESULT Navigation::PatchIconName(LPWSTR pszIcon, UINT cchMaxIconLen, ifc_omservice *service)
{
	if (NULL == pszIcon) return E_INVALIDARG;
	if (L'\0' == *pszIcon) return S_FALSE;

	for(INT i = 0; i < ARRAYSIZE(szIconPatch); i++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szIconPatch[i].iconName, -1, pszIcon, -1))
		{
			if (IS_INTRESOURCE(szIconPatch[i].resourceName))
			{
				return Plugin_MakeResourcePath(pszIcon, cchMaxIconLen, RT_RCDATA, szIconPatch[i].resourceName, RESPATH_COMPACT);
			}
			return StringCchCopy(pszIcon, cchMaxIconLen, szIconPatch[i].resourceName);
		}
	}

	if (FALSE == PathIsURL(pszIcon) && FALSE != PathIsRelative(pszIcon))
	{
		WCHAR szTemp[2048] = {0};
		HRESULT hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszIcon);
		if (SUCCEEDED(hr))
		{
			ServiceHost *serviceHost;
			hr = ServiceHost::GetCachedInstance(&serviceHost);
			if (SUCCEEDED(hr))
			{
				WCHAR szBase[MAX_PATH] = {0};
				if (SUCCEEDED(serviceHost->GetBasePath(service, szBase, ARRAYSIZE(szBase))))
				{
					if (L'\0' != szBase[0] && FALSE != PathIsRelative(szBase))
					{
						LPCWSTR pszUser = (NULL != WASABI_API_APP) ? WASABI_API_APP->path_getUserSettingsPath() : NULL;
						if (NULL != pszUser)
						{
							StringCchCopy(pszIcon, cchMaxIconLen, pszUser);
						}
						PathAppend(pszIcon, szBase);
					}
					else
					{
						StringCchCopy(pszIcon, cchMaxIconLen, szBase);
					}

					PathAppend(pszIcon, szTemp);
				}
				serviceHost->Release();
			}
		}
		if (FAILED(hr))
			return hr;
	}

	return S_FALSE;
}

HNAVITEM Navigation::CreateItemInt(HNAVITEM hParent, ifc_omservice *service, int altMode)
{
	if (!altMode && (S_OK != ServiceHelper_IsSubscribed(service))) 
		return NULL;

	WCHAR szName[256] = {0}, szInvariant[64] = {0};
    if (FAILED(service->GetName(szName, ARRAYSIZE(szName))))
		return NULL;

	if (L'\0' == szName[0])
		WASABI_API_LNGSTRINGW_BUF(IDS_DEFAULT_SERVICENAME, szName, ARRAYSIZE(szName));
	
	if (FAILED(StringCchPrintf(szInvariant, ARRAYSIZE(szInvariant), NAVITEM_PREFIX L"%u", service->GetId())))
		return NULL;

	NAVINSERTSTRUCT nis = {0};
	nis.hInsertAfter = NULL;
	nis.hParent = hParent;
	
	INT iIcon = -1;
	ifc_mlnavigationhelper *navHelper;
	if (NULL != OMUTILITY && SUCCEEDED(OMUTILITY->GetMlNavigationHelper(Plugin_GetLibrary(), &navHelper)))
	{		
		WCHAR szIcon[2048] = {0};
		if (FAILED(service->GetIcon(szIcon, ARRAYSIZE(szIcon))) ||
			FAILED(PatchIconName(szIcon, ARRAYSIZE(szIcon), service)) ||
			FAILED(navHelper->QueryIndex(szIcon, &iIcon, NULL)))
		{
			iIcon = -1;
		}
		navHelper->Release();
	}
	
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.mask = NIMF_TEXT | NIMF_STYLE | NIMF_TEXTINVARIANT | NIMF_PARAM | NIMF_IMAGE | NIMF_IMAGESEL;
		
	nis.item.id = 0;
	nis.item.pszText = szName;
	nis.item.pszInvariant = szInvariant;
	nis.item.lParam = (LPARAM)service;
	
	nis.item.style = 0;
	UINT serviceFlags;
	if (FAILED(service->GetFlags(&serviceFlags)))
		serviceFlags = 0;

	if (0 != (SVCF_SPECIAL & serviceFlags))
	{
		nis.item.style |= (NIS_HASCHILDREN | NIS_ALLOWCHILDMOVE);
		iIcon = -1;
	}
	
	nis.item.styleMask = nis.item.style;

	nis.item.iImage = iIcon;
	nis.item.iSelectedImage = iIcon;

		
	HNAVITEM hItem = MLNavCtrl_InsertItem(hLibrary,  &nis);
	if (NULL != hItem)
	{
		ServiceHost *serviceHost;
		if (SUCCEEDED(ServiceHost::GetCachedInstance(&serviceHost)))
		{
			ifc_omserviceeventmngr *eventManager;
			if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEventMngr, (void**)&eventManager)))
			{
				eventManager->RegisterHandler(serviceHost);
				eventManager->Release();
			}

			serviceHost->Release();
		}

		service->AddRef();
	}
	
	return hItem;
}

HRESULT Navigation::SelectItemInt(HNAVITEM hItem, UINT serviceId, LPCWSTR pszUrl)
{
	if (NULL == hLibrary) return E_UNEXPECTED;

	if (NULL != pszUrl && L'\0' != *pszUrl)
	{
		HRESULT hr = forceUrl.Set(serviceId, pszUrl);
		if (FAILED(hr)) return hr;
	}
	else
	{
		forceUrl.Remove(serviceId);
	}
		
	if (FALSE == MLNavItem_Select(hLibrary, hItem))
	{
		forceUrl.Remove(serviceId);
		return E_FAIL;
	}

	return S_OK;

}
HNAVITEM Navigation::GetMessageItem(INT msg, INT_PTR param1)
{
	return (msg < ML_MSG_NAVIGATION_FIRST) ? 
			MLNavCtrl_FindItemById(hLibrary, param1) : 
			(HNAVITEM)param1;
}

HRESULT Navigation::OnCreateView(HNAVITEM hItem, HWND hParent, HWND *hView)
{
	if (NULL == hView) return E_POINTER;
	*hView = NULL;

	if (NULL == hLibrary) return E_UNEXPECTED;
	if (NULL == hItem || NULL == hParent) return E_INVALIDARG;

	HRESULT hr;

	ifc_omservice *service;
	hr = GetService(hItem, &service);
	if (SUCCEEDED(hr))
	{
		hr = InitializeBrowser();
		if (SUCCEEDED(hr))
		{
			LPWSTR pszUrl;
			if (S_OK != forceUrl.Peek(service->GetId(), &pszUrl))
				pszUrl = NULL;

			hr = OMBROWSERMNGR->CreateView(service, hParent, pszUrl, 0, hView);
			forceUrl.FreeString(pszUrl);
		}
		
		service->Release();
	}
	return hr;
}

HRESULT Navigation::OnContextMenu(HNAVITEM hItem, HWND hHost, POINTS pts)
{
	if (NULL == hItem || NULL == hHost)
		return E_INVALIDARG;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	HRESULT hr;
	ifc_omservice *service, *activeService;
	hr = GetService(hItem, &service);
	if (FAILED(hr)) return hr;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(hLibrary, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	HWND activeView = GetActiveView(&activeService);
	if (NULL == activeView)	activeService = NULL;

	INT menuKind = (hItem == hRoot) ? OMMENU_GALERYCONTEXT : OMMENU_SERVICECONTEXT;

	UINT menuFlags = MCF_VIEW;
	if (NULL != activeService && activeService->GetId() == service->GetId())
		menuFlags |= MCF_VIEWACTIVE;
			
	UINT rating;
	if (OMMENU_SERVICECONTEXT == menuKind && SUCCEEDED(service->GetRating(&rating)))
		menuFlags |= RATINGTOMCF(rating);

	HMENU hMenu = Menu_GetMenu(menuKind, menuFlags);
	if (NULL != hMenu)
	{
		INT commandId = DoTrackPopup(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
									 pt.x, pt.y, hHost, NULL);

		Menu_ReleaseMenu(hMenu, menuKind);

		if (0 != commandId && FALSE != Command_ProcessService(activeView, service, commandId, NULL))
			commandId = 0;

		if (0 != commandId && NULL != activeView && FALSE != Command_ProcessView(activeView, commandId, NULL))
			commandId = 0;

		if (0 != commandId && FALSE != Command_ProcessGeneral(commandId, NULL))
			commandId = 0;
	}

	if (NULL != activeService)
		activeService->Release();

	service->Release();
	
	return hr;
}

HRESULT Navigation::OnEndTitleEdit(HNAVITEM hItem, LPCWSTR pszNewTitle)
{
	if (NULL == hItem) return E_INVALIDARG;
	if (NULL == hLibrary) return E_UNEXPECTED;

	HRESULT hr;

	ifc_omservice *service;
	hr = GetService(hItem, &service);
	if (SUCCEEDED(hr))
	{
		if (NULL != pszNewTitle)
		{
			ifc_omserviceeditor *editor;
			hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
			if (SUCCEEDED(hr))
			{
				hr = editor->SetName(pszNewTitle, FALSE);
				editor->Release();
			}
		}

		if (SUCCEEDED(hr))
			ServiceHelper_Save(service);
		
		service->Release();
	}
	return hr;
}

HRESULT Navigation::OnDeleteItem(HNAVITEM hItem)
{
	if (NULL == hItem) return E_INVALIDARG;
	if (NULL == hLibrary) return E_UNEXPECTED;

	WCHAR szBuffer[2048] = {0};
	NAVITEM itemInfo = {0};

	itemInfo.cbSize = sizeof(itemInfo);
    itemInfo.hItem = hItem;
	itemInfo.pszInvariant = szBuffer;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.mask = NIMF_PARAM | NIMF_TEXTINVARIANT | NIMF_IMAGE;

	if (FALSE == MLNavItem_GetInfo(hLibrary, &itemInfo))
		return E_FAIL;
	if (FALSE == Navigation_CheckInvariantName(szBuffer))
		return E_NAVITEM_UNKNOWN;
			
	ifc_omservice *service = (ifc_omservice*)itemInfo.lParam;
	
	if (NULL != service)
	{
		if (SUCCEEDED(service->GetIcon(szBuffer, ARRAYSIZE(szBuffer))))
		{
			ifc_mlnavigationhelper *navHelper;
			if (SUCCEEDED(OMUTILITY->GetMlNavigationHelper(Plugin_GetLibrary(), &navHelper)))
			{		
				navHelper->ReleaseIndex(szBuffer);
				navHelper->Release();
			}
		}

		ifc_ombrowserwndmngr *windowManager;
		if (NULL != OMBROWSERMNGR && SUCCEEDED(OMBROWSERMNGR->QueryInterface(IFC_OmBrowserWindowManager, (void**)&windowManager)))
		{
			UINT serviceId = service->GetId();

			ifc_ombrowserwndenum *windowEnum;
			if (SUCCEEDED(windowManager->Enumerate(NULL, &serviceId, &windowEnum)))
			{
				HWND hwnd;
				while (S_OK == windowEnum->Next(1, &hwnd, NULL))
				{
					DestroyWindow(hwnd);
				}
				windowEnum->Release();
			}

			windowManager->Release();
		}

		ServiceHost *serviceHost;
		if (SUCCEEDED(ServiceHost::GetCachedInstance(&serviceHost)))
		{
			ifc_omserviceeventmngr *eventManager;
			if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEventMngr, (void**)&eventManager)))
			{
				eventManager->UnregisterHandler(serviceHost);
				eventManager->Release();
			}
			serviceHost->Release();
		}

		itemInfo.mask = NIMF_PARAM;
		itemInfo.lParam = 0L;
		MLNavItem_SetInfo(hLibrary, &itemInfo);
			
		service->Release();
	}		

	
	
	return S_OK;
}

HRESULT Navigation::OnKeyDown(HNAVITEM hItem, NMTVKEYDOWN *pnmkd)
{
	if (NULL == hItem) return E_INVALIDARG;
	if (NULL == hLibrary) return E_UNEXPECTED;

	ifc_omservice *service;
	HRESULT hr = GetService(hItem, &service);
	if (SUCCEEDED(hr))
	{
		switch(pnmkd->wVKey)
		{
			case VK_DELETE:
				{
					BOOL fSuccess;
					ifc_omservice *activeService;
					HWND activeView = GetActiveView(&activeService);
					if (IsWindow(activeView))
					{
						Command_ProcessService(activeView, service, ID_SERVICE_UNSUBSCRIBE, &fSuccess);
					}
				}
				break;
		}

		service->Release();
	}
	return hr;
}

HRESULT Navigation::OnControlDestroy()
{
	SaveOrder();
	return S_OK;
}

#define CBCLASS Navigation
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_IMAGECHANGED, ImageChanged)
END_DISPATCH;
#undef CBCLASS