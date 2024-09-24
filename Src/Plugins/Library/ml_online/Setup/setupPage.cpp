#include "./setupPage.h"
#include "./setupListbox.h"
#include "./setupGroupList.h"
#include "./setupGroupFilter.h"
#include "./setupListboxLabel.h"
#include "../common.h"
#include "../config.h"
#include "../resource.h"
#include "../api__ml_online.h"

#include "./setupDetails.h"
#include "./setupLog.h"

#include "../../winamp/setup/svc_setup.h"

#include <ifc_omservice.h>
#include <ifc_omfilestorage.h>
#include <ifc_omwebstorage.h>

#include <windows.h>
#include <strsafe.h>
#include <vector>

#define FEATURED_SERVICES_URL		L"http://services.winamp.com/svc/default"

#define IDC_DETAILS			10000

typedef std::vector<ifc_omservice*> ServiceList;
typedef std::vector<UINT> ServiceIdList;

HWND SetupPage_CreateWindow(HWND hParent, SetupPage *page);
static INT_PTR SetupPage_ModalLoop(HWND hwnd, HACCEL  hAccel, HANDLE hCancel);

struct AppendServiceToStringData
{
	AppendServiceToStringData() : formatFirst(NULL), formatNext(NULL), cursor(NULL),
								  remaining(0), inserted(0), result(S_OK) {}

	LPCWSTR formatFirst;
	LPCWSTR formatNext;
	LPWSTR cursor;
	size_t remaining;
	size_t inserted;
	HRESULT result;
};

static BOOL CALLBACK SetupPage_AppendServiceToStringCallback(UINT serviceId, void *data)
{
	AppendServiceToStringData *param = (AppendServiceToStringData*)data;
	if (NULL == param) return FALSE;

	param->result = StringCchPrintfEx(param->cursor, param->remaining, &param->cursor, &param->remaining, 
									  STRSAFE_NULL_ON_FAILURE,
									  ((0 == param->inserted) ? param->formatFirst : param->formatNext), serviceId);

	if (FAILED(param->result))
		return FALSE;

	param->inserted++;
	return TRUE;
}

static BOOL CALLBACK SetupPage_AppendServiceIdCallback(UINT serviceId, void *data)
{
	ServiceIdList *list = (ServiceIdList*)data;
	if (NULL == list) return FALSE;
	list->push_back(serviceId);
	return TRUE;
}

static HRESULT SetupPage_GetFeaturedServicesUrl(LPWSTR pszBuffer, UINT cchBufferMax)
{
	LPWSTR cursor = pszBuffer;
	size_t remaining = cchBufferMax;

	HRESULT hr = StringCchCopyEx(cursor, remaining, FEATURED_SERVICES_URL, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
	if (FAILED(hr))
		return hr;

	AppendServiceToStringData param;
	param.cursor = cursor;
	param.remaining = remaining;
	param.formatFirst = L"?svc_ids=%u";
	param.formatNext = L",%u";
	param.inserted = 0;
	param.result = S_OK;

	hr = Config_ReadServiceIdList("Setup", "featuredExtra", ',', SetupPage_AppendServiceToStringCallback, &param);
	if (SUCCEEDED(hr))
		hr = param.result;

	return hr;
}

SetupPage* SetupPage::CreateInstance()
{
	SetupPage *instance = new SetupPage();
	return instance;
}

SetupPage::SetupPage() 
	: ref(1), hwnd(NULL), name(NULL), title(NULL),
	  groupList(NULL), completeEvent(NULL),
	  servicesInitialized(FALSE)
{
	WasabiApi_AddRef();
	SetupDetails_Initialize();
}

SetupPage::~SetupPage()
{
	if (NULL != name)
	{
		Plugin_FreeString(name);
		name = NULL;
	}

	if (NULL != title)
	{
		Plugin_FreeString(title);
		title = NULL;
	}

	if (NULL != groupList)
	{
		groupList->Release();
		groupList = NULL;
	}

	if (NULL != completeEvent)
	{
		CloseHandle(completeEvent);
		completeEvent = NULL;
	}

	SetupDetails_Uninitialize();

	if (FALSE != servicesInitialized)
	{
		if (NULL != OMBROWSERMNGR)
			OMBROWSERMNGR->Finish();
	}

	WasabiApi_Release();
}

size_t SetupPage::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t SetupPage::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int SetupPage::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	return E_NOTIMPL;
}

HRESULT SetupPage::GetName(bool bShort, const wchar_t **pszName)
{
	InitializeServices();

	if (false == bShort)
	{
		if (NULL == title)
		{
			WCHAR szBuffer[128] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_SETUPPAGE_TITLE, szBuffer, ARRAYSIZE(szBuffer));
			title = Plugin_CopyString(szBuffer);
		}
		*pszName = title;
	}
	else
	{
		if (NULL == name)
		{
			WCHAR szBuffer[128] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_ONLINE_SERVICES, szBuffer, ARRAYSIZE(szBuffer));
			name = Plugin_CopyString(szBuffer);
		}
		*pszName = name;
	}
	return S_OK;
}

HRESULT SetupPage::Save(HWND hwndText)
{
	if (NULL == groupList)
		return S_OK;

	SetupLog *log = SetupLog::Open();
	HRESULT hr = groupList->Save(log);

	if (NULL != log)
	{
		log->Save();
		log->Release();
	}

	return hr;
}

HRESULT SetupPage::Revert(void)
{
	HRESULT hr(S_OK);

	if (NULL != groupList)
	{
		groupList->Release();
		groupList = NULL;
	}
	return hr;
}

HRESULT SetupPage::IsDirty(void)
{
	return (NULL != groupList && groupList->IsModified()) ? S_OK : S_FALSE;
}

HRESULT SetupPage::Validate(void)
{
	return S_OK;
}

HRESULT SetupPage::InitializeServices()
{
	if (FALSE != servicesInitialized)
		return S_FALSE;

	HWND hWinamp = NULL;
	svc_setup *setupSvc = QueryWasabiInterface(svc_setup, UID_SVC_SETUP);
	if (NULL == setupSvc) return E_UNEXPECTED;
	HRESULT hr = setupSvc->GetWinampWnd(&hWinamp);
	ReleaseWasabiInterface(UID_SVC_SETUP, setupSvc);

	if (SUCCEEDED(hr))
	{
		hr = WasabiApi_LoadDefaults();
		if (SUCCEEDED(hr))
		{
			if (NULL != OMBROWSERMNGR && 
				NULL != OMSERVICEMNGR &&
				NULL != OMUTILITY) 
			{
				hr = OMBROWSERMNGR->Initialize(NULL, hWinamp);
			}
			else
				hr = E_UNEXPECTED;

			if (SUCCEEDED(hr))
				servicesInitialized = TRUE;
		}
	}

	return hr;
}

HRESULT SetupPage::CreateView(HWND hParent, HWND *phwnd)
{
	if (NULL == phwnd)
		return E_INVALIDARG;

	if (FAILED(InitializeServices()))
	{
		*phwnd = NULL;
		return E_FAIL;
	}

	*phwnd = SetupPage_CreateWindow(hParent, this);
	return (NULL == phwnd) ? E_FAIL : S_OK;
}


BOOL SetupPage::UpdateListAsync(INT groupId)
{
	if (NULL == hwnd) 
		return FALSE;

	return PostMessage(hwnd, SPM_UPDATELIST, (WPARAM)groupId, NULL);
}

BOOL SetupPage::AttachWindow(HWND hAttach)
{
	hwnd = hAttach;

	if (NULL == completeEvent)
		completeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


	if (NULL == groupList)
	{
		groupList = SetupGroupList::CreateInstance();
		if (NULL != groupList)
		{
			WCHAR szBuffer[4096] = {0};
			SetupPage_GetFeaturedServicesUrl(szBuffer, ARRAYSIZE(szBuffer));

			SetupGroup *group = SetupGroup::CreateInstance(ID_FEATUREDGROUP, MAKEINTRESOURCE(IDS_SERVICEGROUP_FEATURED), 
														   szBuffer, &SUID_OmStorageUrl, &FUID_SetupFeaturedGroupFilter,
														   SetupGroup::styleSortAlphabetically | SetupGroup::styleDefaultSubscribed | SetupGroup::styleSaveAll);
			if (NULL != group)
			{
				group->SetLongName(MAKEINTRESOURCE(IDS_SERVICEGROUP_FEATUREDLONG));
				group->SetDescription(MAKEINTRESOURCE(IDS_SERVICEGROUP_FEATURED_DESC));
				groupList->AddGroup(group);
				group->RequestReload();
				group->Release();
			}

			group = SetupGroup::CreateInstance(ID_KNOWNGROUP, MAKEINTRESOURCE(IDS_SERVICEGROUP_KNOWN), 
											   L"*.ini", &SUID_OmStorageIni, &FUID_SetupKnownGroupFilter,
											   SetupGroup::styleSortAlphabetically);
			if (NULL != group)
			{
 				group->SetLongName(MAKEINTRESOURCE(IDS_SERVICEGROUP_KNOWNLONG));
				group->SetDescription(MAKEINTRESOURCE(IDS_SERVICEGROUP_KNOWN_DESC));
				group->RequestReload();
				groupList->AddGroup(group);
				group->Release();
			}
		}
	}

	if (NULL != groupList)
		groupList->SetPageWnd(hwnd);

	HWND hList = GetDlgItem(hwnd, IDC_SERVICELIST);
	if (NULL != hList)
	{	
		SetupListbox *listbox;
		if (SUCCEEDED(SetupListbox::CreateInstance(hList, groupList, &listbox)))
		{
		}
		ListboxSelectionChanged();
	}

	return TRUE;
}

void SetupPage::DetachWindow()
{
	hwnd = NULL;
	if (NULL != groupList)
	{
		groupList->SetPageWnd(NULL);
	}
}


HRESULT SetupPage_AppendUnselectedServices(LPCSTR pszSection, LPCSTR pszKey, SetupGroup *group)
{
	size_t index = group->GetRecordCount();
	size_t filterIndex, filterSize;

	ServiceIdList list;
	Config_ReadServiceIdList(pszSection, pszKey, ',', SetupPage_AppendServiceIdCallback, &list);
	filterSize = list.size();

	while(index--)
	{
		SetupRecord *record = group->GetRecord(index);
		if (NULL == record || FALSE != record->IsSelected()) continue;
		
		ifc_omservice *service = record->GetService();
		if (NULL == service) continue;
		
		UINT serviceId = service->GetId();
		for(filterIndex = 0; filterIndex < filterSize; filterIndex++)
		{
			if (list[filterIndex] == serviceId)
				break;
		}

		if (filterIndex == filterSize)
			list.push_back(serviceId);
	}

	if (0 != list.size())
	{
		size_t bufferAlloc = (list.size() * 11) * sizeof(CHAR);
		LPSTR buffer = Plugin_MallocAnsiString(bufferAlloc);
		if (NULL != buffer)
		{
			filterSize = list.size();
			LPSTR cursor = buffer;
			size_t remaining = bufferAlloc;
			for(index = 0; index < filterSize; index++)
			{
				if (FAILED(StringCchPrintfExA(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
					((0 == index) ? "%u" : ",%u"), list[index])))
				{
					break;
				}
			}
			Config_WriteStr(pszSection, pszKey, buffer);
			Plugin_FreeAnsiString(buffer);
		}
	}
	else
		Config_WriteStr(pszSection, pszKey, NULL);

	return S_OK;
}

HRESULT SetupPage::Execute(HWND hwndText)
{
	SetupGroup *group = NULL;
	SetupLog *log = SetupLog::Open();
	WCHAR szBuffer[128] = {0};

	if (FAILED(InitializeServices()))
		return E_FAIL;

	if (NULL == groupList)
	{
		groupList = SetupGroupList::CreateInstance();
		if (NULL == groupList) return E_UNEXPECTED;
	}

	if (S_OK != groupList->FindGroupById(ID_FEATUREDGROUP, &group) && group != NULL)
	{
		WCHAR szBuffer[4096] = {0};
		SetupPage_GetFeaturedServicesUrl(szBuffer, ARRAYSIZE(szBuffer));

		group = SetupGroup::CreateInstance(ID_FEATUREDGROUP, MAKEINTRESOURCE(IDS_SERVICEGROUP_FEATURED), 
										   szBuffer, &SUID_OmStorageUrl, &FUID_SetupFeaturedGroupFilter,
										   SetupGroup::styleDefaultSubscribed | SetupGroup::styleSaveAll);

		if (NULL != group)
		{
			group->SetLongName(MAKEINTRESOURCE(IDS_SERVICEGROUP_FEATUREDLONG));
			group->SetDescription(MAKEINTRESOURCE(IDS_SERVICEGROUP_FEATURED_DESC));
			groupList->AddGroup(group);
			group->RequestReload();
		}
	}

	if (NULL == completeEvent)
		completeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (NULL != group)
	{
		if (SUCCEEDED(group->SignalLoadCompleted(completeEvent)))
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_DOWNLOADSERVICE_JOB, szBuffer, ARRAYSIZE(szBuffer));
			SetWindowText(hwndText, szBuffer);
			SetupPage_ModalLoop(hwndText, NULL, completeEvent);
		}

		WASABI_API_LNGSTRINGW_BUF(IDS_SAVESERVICE_JOB, szBuffer, ARRAYSIZE(szBuffer));
		SetWindowText(hwndText, szBuffer);
		group->Save(log);
		SetupPage_AppendUnselectedServices("Setup", "featuredHistory", group);
		group->Release();
		group = NULL;
	}

	// ensure that promotions are subscribed 
	if (S_OK != groupList->FindGroupById(ID_KNOWNGROUP, &group) && group != NULL)
	{
		group = SetupGroup::CreateInstance(ID_KNOWNGROUP, MAKEINTRESOURCE(IDS_SERVICEGROUP_KNOWN), 
										   L"*.ini", &SUID_OmStorageIni, &FUID_SetupKnownGroupFilter,
										   SetupGroup::styleSortAlphabetically);
		if (NULL != group)
		{
 			group->SetLongName(MAKEINTRESOURCE(IDS_SERVICEGROUP_KNOWNLONG));
			group->SetDescription(MAKEINTRESOURCE(IDS_SERVICEGROUP_KNOWN_DESC));
			group->RequestReload();
			groupList->AddGroup(group);
		}
	}

	if (NULL != group)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_SAVESERVICE_JOB, szBuffer, ARRAYSIZE(szBuffer));
		SetWindowText(hwndText, szBuffer);

		ResetEvent(completeEvent);
		if (SUCCEEDED(group->SignalLoadCompleted(completeEvent)))
			SetupPage_ModalLoop(hwndText, NULL, completeEvent);

		group->Save(log);
		group->Release();
		group = NULL;
	}

	if (NULL != log)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_REGISTERINGSERVICE_JOB, szBuffer, ARRAYSIZE(szBuffer));
		SetWindowText(hwndText, szBuffer);
		ResetEvent(completeEvent);
		log->Send(completeEvent);
		SetupPage_ModalLoop(hwndText, NULL, completeEvent);
		log->Release();
	}

	SetupLog::Erase();
	Config_WriteStr("Setup", "featuredExtra", NULL); // delete promo offer
	return S_OK;
}

HRESULT SetupPage::Cancel(HWND hwndText)
{
	if (NULL != completeEvent)
		SetEvent(completeEvent);
	return S_OK;
}

HRESULT SetupPage::IsCancelSupported(void)
{
	return S_OK;
}
void SetupPage::ListboxSelectionChanged()
{
	if (NULL == hwnd) return;
	HWND hList = GetDlgItem(hwnd, IDC_SERVICELIST);
	if (NULL == hList) return;

	SetupListboxItem *item = NULL;
	INT iSelection = (INT)SendMessage(hList, LB_GETCURSEL, 0, 0L);
	if (LB_ERR == iSelection ||
		FAILED(groupList->FindListboxItem(iSelection, &item)))
	{
		item = NULL;
	}

	HWND hDiscard = GetDlgItem(hwnd, IDC_DETAILS);
	if (NULL != hDiscard)
	{
		WCHAR szPanel[64] = {0}, szItem[64] = {0};
		if (FALSE != SetupDetails_GetUniqueName(hDiscard, szPanel, ARRAYSIZE(szPanel)) &&
			FALSE != item->GetUniqueName(szItem, ARRAYSIZE(szItem)) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, szPanel, - 1, szItem, -1))
		{
			return;
		}
	}

	HWND hDetails = (NULL != item) ? item->CreateDetailsView(hwnd) : NULL;
	if (NULL != hDiscard)
	{
		SetWindowLongPtr(hDiscard, GWL_STYLE, GetWindowLongPtr(hDiscard, GWL_STYLE) & ~WS_VISIBLE);
	}

	if (NULL != hDetails)
	{
		HWND hPlaceholder = GetDlgItem(hwnd, IDC_PLACEHOLDER);
		if (NULL != hPlaceholder)
		{
			RECT windowRect;
			if (GetWindowRect(hPlaceholder,  &windowRect))
			{
				MapWindowPoints(HWND_DESKTOP,hwnd, (POINT*)&windowRect,2);
				SetWindowPos(hDetails, NULL, windowRect.left, windowRect.top, 
							windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 
							SWP_NOACTIVATE | SWP_NOZORDER);
			}
			SetWindowLongPtr(hDetails, GWLP_ID, IDC_DETAILS);
			ShowWindow(hDetails, SW_SHOWNA);
			RedrawWindow(hDetails, NULL, NULL, RDW_ERASENOW |RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}

	if (NULL != hDiscard)
	{
		DestroyWindow(hDiscard);
	}
}

static INT_PTR SetupPage_ModalLoop(HWND hwnd, HACCEL  hAccel, HANDLE hCancel)
{
	MSG msg = {0};
	HWND hParent = NULL;
	while (NULL != (hParent = GetAncestor(hwnd, GA_PARENT)))
	{
		hwnd = hParent;
	}

	if (NULL == hwnd) 
		return 0;

	for (;;)
	{
		DWORD status = MsgWaitForMultipleObjectsEx(1, &hCancel, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		if (WAIT_OBJECT_0 == status)
		{
			return 0;
		}
		else if ((WAIT_OBJECT_0  + 1)== status)
		{
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				//if (!CallMsgFilter(&msg, MSGF_DIALOGBOX))
				{
					if (msg.message == WM_QUIT)
					{
						PostQuitMessage((INT)msg.wParam);
						return msg.wParam;
					}

					if (!TranslateAcceleratorW(hwnd, hAccel, &msg) && 
						!IsDialogMessageW(hwnd, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessageW(&msg);
					}
				}
			}
		}
	}
	return 0;
}

#define CBCLASS SetupPage
START_MULTIPATCH;
 START_PATCH(MPIID_SETUPPAGE)
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, ADDREF, AddRef);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, RELEASE, Release);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, API_SETUPPAGE_GET_NAME, GetName);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, API_SETUPPAGE_CREATEVIEW, CreateView);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, API_SETUPPAGE_SAVE, Save);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, API_SETUPPAGE_REVERT, Revert);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, API_SETUPPAGE_ISDIRTY, IsDirty);
  M_CB(MPIID_SETUPPAGE, ifc_setuppage, API_SETUPPAGE_VALIDATE, Validate);
 
 NEXT_PATCH(MPIID_SETUPJOB)
  M_CB(MPIID_SETUPJOB, ifc_setupjob, ADDREF, AddRef);
  M_CB(MPIID_SETUPJOB, ifc_setupjob, RELEASE, Release);
  M_CB(MPIID_SETUPJOB, ifc_setupjob, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_SETUPJOB, ifc_setupjob, API_SETUPJOB_EXECUTE, Execute);
  M_CB(MPIID_SETUPJOB, ifc_setupjob, API_SETUPJOB_CANCEL, Cancel);
  M_CB(MPIID_SETUPJOB, ifc_setupjob, API_SETUPJOB_ISCANCELSUPPORTED, IsCancelSupported);

 END_PATCH
END_MULTIPATCH;
#undef CBCLASS