//#include "main.h"
#include "./dispatchCallback.h"
#include <new.h>

DispatchCallback::DispatchCallback() 
	: ref(1), dispatch(NULL), threadId(0), threadHandle(NULL)
{
}

DispatchCallback::~DispatchCallback()
{
	if (NULL != dispatch)
		dispatch->Release();

	if (NULL != threadHandle)
		CloseHandle(threadHandle);
}

HRESULT DispatchCallback::CreateInstance(IDispatch *dispatch, DispatchCallback **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	if (NULL == dispatch)
		return E_INVALIDARG;

	DispatchCallback *self = new DispatchCallback();
	if (NULL == self)
		return E_OUTOFMEMORY;

	self->dispatch = dispatch;
	self->dispatch->AddRef();
	self->threadId = GetCurrentThreadId();

	HANDLE processHandle = GetCurrentProcess();

	if (FALSE == DuplicateHandle(processHandle, 
								 GetCurrentThread(), 
								 processHandle, 
								 &self->threadHandle, 
								 0, 
								 FALSE, 
								 DUPLICATE_SAME_ACCESS))
	{
		self->threadHandle = NULL;
		delete(self);

		return E_FAIL;
	}

	*instance = self;
	return S_OK;
}

unsigned long DispatchCallback::AddRef()
{
	return InterlockedIncrement((long*)&ref);
}

unsigned long DispatchCallback::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((long*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

IDispatch *DispatchCallback::GetDispatch()
{
	return dispatch;
}

unsigned long DispatchCallback::GetThreadId()
{
	return threadId;
}

HANDLE DispatchCallback::GetThreadHandle()
{
	return threadHandle;
}


DispatchCallbackEnum::DispatchCallbackEnum()
	: ref(1), buffer(NULL), size(0), cursor(0) 
{
}

DispatchCallbackEnum::~DispatchCallbackEnum()
{
	if (NULL != buffer)
	{
		while(size--)
		{
			buffer[size]->Release();
		}
	}
}

HRESULT DispatchCallbackEnum::CreateInstance(DispatchCallback **objects, size_t count, DispatchCallbackEnum **instance)
{
	DispatchCallback *callback = NULL;
	DispatchCallbackEnum *enumerator = NULL;

	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	size_t size = sizeof(DispatchCallbackEnum) + (sizeof(DispatchCallback**) * count);
	void *storage = calloc(size, 1);
	if (NULL == storage)
		return E_OUTOFMEMORY;

	enumerator = new(storage) DispatchCallbackEnum();
	if (NULL == enumerator)
	{
		free(storage);
		return E_FAIL;
	}

	enumerator->buffer = (DispatchCallback**)(((BYTE*)enumerator) + sizeof(DispatchCallback));

	for (size_t index = 0; index < count; index++)
	{
		callback = objects[index];
		if (NULL != callback)
		{
			enumerator->buffer[enumerator->size] = callback;
			callback->AddRef();
			enumerator->size++;
		}
	}

	*instance = enumerator;
	return S_OK;
}

unsigned long DispatchCallbackEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

unsigned long DispatchCallbackEnum::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

HRESULT DispatchCallbackEnum::Next(DispatchCallback **objects, size_t bufferMax, size_t *fetched)
{
	if (NULL == objects)
		return E_POINTER;

	if (0 == bufferMax) 
		return E_INVALIDARG;

	if (cursor >= size)
	{
		if (NULL != fetched) 
			*fetched = 0;

		return S_FALSE;
	}

	size_t available = size - cursor;
	size_t copied = ((available > bufferMax) ? bufferMax : available);

	DispatchCallback **source = buffer + cursor;
	CopyMemory(objects, source, copied * sizeof(DispatchCallback*));

	for(size_t index = 0; index < copied; index++)
		objects[index]->AddRef();

	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT DispatchCallbackEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT DispatchCallbackEnum::Skip(size_t count)
{
	cursor += count;
	if (cursor > size)
		cursor = size;

	return (cursor < size) ? S_OK : S_FALSE;
}

HRESULT DispatchCallbackEnum::GetCount(size_t *count)
{
	if (NULL == count)
		return E_POINTER;

	*count = size;

	return S_OK;
}

HRESULT DispatchCallbackEnum::Notify(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb, void *param)
{
	DispatchCallbackApc *apc = NULL;
	unsigned long threadId = GetCurrentThreadId();

	if (NULL == buffer)
		return E_UNEXPECTED;

	HRESULT hr = DispatchCallbackApc::CreateInstance(notifyCb, freeCb, param, &apc);
	if (FAILED(hr) || apc == NULL)
		return hr;

	for (size_t index = 0; index < size; index++)
	{
		DispatchCallback *callback = buffer[index];
		if (callback)
		{
			if (callback->GetThreadId() == threadId)
				apc->Call(callback->GetDispatch());
			else
				apc->Queue(callback->GetThreadHandle(), callback->GetDispatch());
		}
	}

	apc->Release();
	return hr;
}

DispatchCallbackStore::DispatchCallbackStore()
{
	InitializeCriticalSection(&lock);
}

DispatchCallbackStore::~DispatchCallbackStore()
{
	UnregisterAll();
	DeleteCriticalSection(&lock);
}

void DispatchCallbackStore::Lock()
{
	EnterCriticalSection(&lock);
}

void DispatchCallbackStore::Unlock()
{
	LeaveCriticalSection(&lock);
}

CRITICAL_SECTION *DispatchCallbackStore::GetLock()
{
	return &lock;
}

HRESULT DispatchCallbackStore::Register(IDispatch *dispatch)
{
	DispatchCallback *callback = NULL;

	if (NULL == dispatch)
		return E_INVALIDARG;

	Lock();

	HRESULT hr = S_OK;
	size_t index = list.size();
	while(index--)
	{
		callback = list[index];
		if (callback->GetDispatch() == dispatch)
		{
			hr = S_FALSE;
			break;
		}
	}

	if (S_OK == hr)
	{
		hr = DispatchCallback::CreateInstance(dispatch, &callback);
		if (SUCCEEDED(hr))
			list.push_back(callback);
	}

	Unlock();

	return hr;
}

HRESULT DispatchCallbackStore::Unregister(IDispatch *dispatch)
{
	if (NULL == dispatch)
		return E_INVALIDARG;

	Lock();

	HRESULT hr = S_FALSE;
	size_t index = list.size();
	while(index--)
	{
		DispatchCallback *callback = list[index];
		if (callback->GetDispatch() == dispatch)
		{
			list.erase(list.begin() + index);
			callback->Release();
			hr = S_OK;
			break;
		}
	}

	Unlock();

	return hr;
}

void DispatchCallbackStore::UnregisterAll()
{
	Lock();

	size_t index = list.size();
	while(index--)
	{
		DispatchCallback *callback = list[index];
		callback->Release();
	}

	list.clear();

	Unlock();
}

HRESULT DispatchCallbackStore::Enumerate(DispatchCallbackEnum **enumerator)
{
	if (NULL == enumerator || !(list.size() > 0))
		return E_POINTER;
	
	Lock();
		HRESULT hr = DispatchCallbackEnum::CreateInstance(&list[0], list.size(), enumerator);
	Unlock();

	return hr;
}

HRESULT DispatchCallbackStore::RegisterFromDispParam(DISPPARAMS *pdispparams, unsigned int position, 
													 unsigned int *puArgErr)
{
	VARIANTARG varg;
	VariantInit(&varg);
	HRESULT hr = DispGetParam(pdispparams, position, VT_DISPATCH, &varg, puArgErr);
	if (SUCCEEDED(hr))
	{
		hr = Register(V_DISPATCH(&varg));
		VariantClear(&varg);
	}

	return hr;
}

HRESULT DispatchCallbackStore::UnregisterFromDispParam(DISPPARAMS *pdispparams, unsigned int position, 
													   unsigned int *puArgErr)
{
	VARIANTARG varg;
	VariantInit(&varg);
	HRESULT hr = DispGetParam(pdispparams, position, VT_DISPATCH, &varg, puArgErr);
	if (SUCCEEDED(hr))
	{
		hr = Unregister(V_DISPATCH(&varg));
		VariantClear(&varg);
	}

	return hr;
}

HRESULT DispatchCallbackStore::Notify(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb, void *param)
{
	DispatchCallbackEnum *enumerator = NULL;
	HRESULT hr = Enumerate(&enumerator);
	if (SUCCEEDED(hr))
	{
		hr = enumerator->Notify(notifyCb, freeCb, param);
		enumerator->Release();
	}

	return hr;
}

DispatchCallbackApc::DispatchCallbackApc()
	: ref(1), notifyCb(NULL), freeCb(NULL), param(NULL)
{
}

DispatchCallbackApc::~DispatchCallbackApc()
{
	if (NULL != freeCb)
		freeCb(param);
}

HRESULT DispatchCallbackApc::CreateInstance(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb,
											void *param, DispatchCallbackApc **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	if (NULL == notifyCb)
		return E_INVALIDARG;

	DispatchCallbackApc *self = new DispatchCallbackApc();
	if (NULL == self)
		return E_OUTOFMEMORY;

	self->notifyCb = notifyCb;
	self->freeCb = freeCb;
	self->param = param;

	*instance = self;

	return S_OK;
}

unsigned long DispatchCallbackApc::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

unsigned long DispatchCallbackApc::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

HRESULT DispatchCallbackApc::Call(IDispatch *dispatch)
{
	if (NULL == notifyCb)
		return E_UNEXPECTED;

	notifyCb(dispatch, param);
	return S_OK;
}

HRESULT DispatchCallbackApc::Queue(HANDLE threadHandle, IDispatch *dispatch)
{
	if (NULL == threadHandle || ((unsigned int)dispatch) < 65536)
		return E_INVALIDARG;

	DispatchCallbackApcParam *apcParam = new DispatchCallbackApcParam(dispatch, this);
	if (NULL == apcParam || ((unsigned int)apcParam) < 65536)
		return E_OUTOFMEMORY;

	if (0 == QueueUserAPC(QueueApcCallback, threadHandle, (ULONG_PTR)apcParam))
	{
		unsigned long errorCode = GetLastError();
		delete(apcParam);

		return HRESULT_FROM_WIN32(errorCode);
	}

	return S_OK;
}

void CALLBACK DispatchCallbackApc::QueueApcCallback(ULONG_PTR user)
{
	DispatchCallbackApcParam *apcParam = (DispatchCallbackApcParam*)user;
	if (NULL == apcParam)
		return;

	DispatchCallbackApc *apc = apcParam->GetApc();
	if (NULL != apc)
		apc->Call(apcParam->GetDispatch()), 

	delete(apcParam);
}

DispatchCallbackApcParam::DispatchCallbackApcParam(IDispatch *_dispatch, DispatchCallbackApc *_apc)
	: dispatch(_dispatch), apc(_apc)
{
	if (NULL != dispatch && ((unsigned long)dispatch >= 65536))
		dispatch->AddRef();

	if (NULL != apc && ((unsigned long)apc >= 65536))
		apc->AddRef();
}

DispatchCallbackApcParam::~DispatchCallbackApcParam()
{
	if (NULL != dispatch)
		dispatch->Release();

	if (NULL != apc)
		apc->Release();
}

IDispatch *DispatchCallbackApcParam::GetDispatch()
{
	return dispatch;
}

DispatchCallbackApc *DispatchCallbackApcParam::GetApc()
{
	return apc;
}