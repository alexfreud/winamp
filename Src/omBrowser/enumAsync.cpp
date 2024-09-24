#include "main.h"
#include "./enumAsync.h"
#include "./ifc_omservice.h"
#include "./ifc_omserviceenum.h"
#include "./ifc_wasabihelper.h"
#include "./serviceList.h"

EnumAsyncWrapper::EnumAsyncWrapper(ifc_omserviceenum *enumerator)
  : ref(1), enumerator(enumerator), userCallback(NULL), userData(NULL),
	completed(NULL), state(stateReady), resultCode(E_PENDING), serviceList(NULL)
{
	if (NULL != enumerator)
		enumerator->AddRef();

	InitializeCriticalSection(&lock);
}

EnumAsyncWrapper::~EnumAsyncWrapper()
{
	EnterCriticalSection(&lock);

	if (NULL != enumerator)
		enumerator->Release();

	if (NULL != completed)
		CloseHandle(completed);
	
	if (NULL != serviceList)
		serviceList->Release();
	
	LeaveCriticalSection(&lock);

	DeleteCriticalSection(&lock);
}

HRESULT EnumAsyncWrapper::CreateInstance(ifc_omserviceenum *enumerator, EnumAsyncWrapper **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == enumerator)
		return E_INVALIDARG;
	
	*instance = new EnumAsyncWrapper(enumerator);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t EnumAsyncWrapper::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t EnumAsyncWrapper::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int EnumAsyncWrapper::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmStorageAsync))
		*object = static_cast<ifc_omstorageasync*>(this);
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
	
HRESULT EnumAsyncWrapper::GetState(UINT *state)
{
	if (NULL == state)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*state  = this->state;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT EnumAsyncWrapper::GetWaitHandle(HANDLE *handle)
{
	if (NULL == handle)
		return E_POINTER;

	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	if (NULL == completed)
	{
		completed = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == completed)
		{
			*handle = NULL;
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32(error);
		}
	}

	if (SUCCEEDED(hr) && FALSE == DuplicateHandle(GetCurrentProcess(), completed, 
					GetCurrentProcess(), handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		*handle = NULL;
		DWORD error = GetLastError();
		hr = HRESULT_FROM_WIN32(error);
	}
	
	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT EnumAsyncWrapper::GetData(void **data)
{
	if (NULL == data)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*data = userData;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT EnumAsyncWrapper::GetCallback(AsyncCallback *callback)
{
	if (NULL == callback)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*callback = userCallback;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT EnumAsyncWrapper::SetData(void *data)
{
	EnterCriticalSection(&lock);
	userData = data;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT EnumAsyncWrapper::SetCallback(AsyncCallback callback)
{
	EnterCriticalSection(&lock);
	userCallback = callback;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT EnumAsyncWrapper::RequestAbort(BOOL fDrop)
{
	HRESULT hr = S_FALSE;
	
	EnterCriticalSection(&lock);

	if (stateInitializing == state || stateReceiving == state)
	{
		state = stateAborting;
		if (FALSE != fDrop)
		{
			userCallback = NULL;
			userData = NULL;
		}
	}
	
	LeaveCriticalSection(&lock);

	return hr;
}

static int EnumAsyncWrapper_ThreadFunc(HANDLE handle, void *user_data, intptr_t id)
{
	EnumAsyncWrapper *instance = (EnumAsyncWrapper*)user_data;
	if (NULL != instance) instance->Enumerate();
	return 0;
}

HRESULT EnumAsyncWrapper::BeginEnumerate()
{
	ifc_wasabihelper *wasabi = NULL;
	HRESULT hr;

	EnterCriticalSection(&lock);
	
	if (stateReady != state && stateCompleted != state)
	{
		hr = E_PENDING;
	}
	else
	{
		state = stateInitializing;

		if (NULL != serviceList)
		{
			serviceList->Release();
			serviceList = NULL;
		}

		hr = Plugin_GetWasabiHelper(&wasabi);
		if (SUCCEEDED(hr) && wasabi != NULL)
		{	
			api_threadpool *threadpool;
			hr = wasabi->GetThreadpoolApi(&threadpool);
			if (SUCCEEDED(hr))
			{
				if (0 != threadpool->RunFunction(NULL, EnumAsyncWrapper_ThreadFunc, this, 0, 0))
				{
					hr = E_FAIL;
				}
				else
				{
					AddRef();
				}
				threadpool->Release();
			}
			else
			{
				hr = E_NOINTERFACE;
			}

			wasabi->Release();
		}

		if (FAILED(hr))
			state = stateCompleted;
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT EnumAsyncWrapper::Enumerate()
{	
	EnterCriticalSection(&lock);
	state = stateReceiving;
	resultCode = OmServiceList::CreateInstance(&serviceList);
	LeaveCriticalSection(&lock);

	if (SUCCEEDED(resultCode))
	{
		if (NULL == enumerator)
		{
			resultCode  = E_UNEXPECTED;
		}
		else
		{
			ifc_omservice *service;		
			while(S_OK == enumerator->Next(1, &service, NULL))
			{
				if (stateAborting == state)
				{
					resultCode = E_ABORT;
					break;
				}

				if (NULL != service)
				{
					serviceList->Add(service);
					service->Release();
				}
			}
		}
	}

	EnterCriticalSection(&lock);
	state = stateCompleted;
	HANDLE event = completed;
	LeaveCriticalSection(&lock);
	
	if (NULL != event)
	{
		SetEvent(event);
	}

	EnterCriticalSection(&lock);
	AsyncCallback cb = userCallback;
	LeaveCriticalSection(&lock);

	if (NULL != cb)
	{
		cb(this);
	}
		
	Release();

	return resultCode;
}

HRESULT EnumAsyncWrapper::GetResultCode()
{
	return resultCode;
}

HRESULT EnumAsyncWrapper::GetServiceList(ifc_omserviceenum **list)
{
	if (NULL == list)
		return E_POINTER;
	
	if (NULL == serviceList)
	{
		*list = NULL;
		return E_UNEXPECTED;
	}

	EnterCriticalSection(&lock);
	
	*list = serviceList;
	serviceList->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

#define CBCLASS EnumAsyncWrapper
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETSTATE, GetState)
CB(API_GETWAITHANDLE, GetWaitHandle)
CB(API_GETDATA, GetData)
END_DISPATCH;
#undef CBCLASS