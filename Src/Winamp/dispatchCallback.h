#ifndef NULLSOFT_DISPATCHCALLBACK_H
#define NULLSOFT_DISPATCHCALLBACK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include <oleauto.h>
#include <vector>

typedef void (*DispatchCallbackNotifyFunc)(IDispatch* /*dispatch*/, void* /*param*/);
typedef void (*DispatchCallbackFreeFunc)(void* /*param*/);

class DispatchCallback
{
protected:
	DispatchCallback();
	~DispatchCallback();

public:
	static HRESULT CreateInstance(IDispatch *dispatch, 
								  DispatchCallback **instance);

public:
	unsigned long AddRef();
	unsigned long Release();

	IDispatch *GetDispatch();
	unsigned long GetThreadId();
	HANDLE GetThreadHandle();
	
protected:
	unsigned long ref;
	IDispatch *dispatch;
	unsigned long threadId;
	HANDLE threadHandle;

};

class DispatchCallbackEnum
{
protected:
	DispatchCallbackEnum();
	~DispatchCallbackEnum();

public:
	static HRESULT CreateInstance(DispatchCallback **objects, 
								  size_t count, 
								  DispatchCallbackEnum **instance);

public:
	unsigned long AddRef();
	unsigned long Release();

public:
	HRESULT Next(DispatchCallback **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

	HRESULT Notify(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb, void *param);

protected:
	unsigned long ref;
	DispatchCallback **buffer;
	size_t size;
	size_t cursor;
};

class DispatchCallbackStore
{
public:
	DispatchCallbackStore();
	~DispatchCallbackStore();

public:
	void Lock();
	void Unlock();
	CRITICAL_SECTION *GetLock();

	HRESULT Register(IDispatch *dispatch);
	HRESULT Unregister(IDispatch *dispatch);
	void UnregisterAll();
	HRESULT Enumerate(DispatchCallbackEnum **enumerator);

	/* Helpers*/
	HRESULT RegisterFromDispParam(DISPPARAMS *pdispparams, unsigned int position, unsigned int *puArgErr);
	HRESULT UnregisterFromDispParam(DISPPARAMS *pdispparams, unsigned int position, unsigned int *puArgErr);

	HRESULT Notify(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb, void *param);

protected:
	typedef std::vector<DispatchCallback*> CallbackList;

protected:
	CRITICAL_SECTION lock;
	CallbackList list;

};

/* Internals */

class DispatchCallbackApc
{
protected:
	DispatchCallbackApc();
	~DispatchCallbackApc();

public:
	static HRESULT CreateInstance(DispatchCallbackNotifyFunc notifyCb,
								  DispatchCallbackFreeFunc freeCb,
								  void *param,
								  DispatchCallbackApc **instance);

public:
	unsigned long AddRef();
	unsigned long Release();

	HRESULT Call(IDispatch *dispatch);
	HRESULT Queue(HANDLE threadHandle, IDispatch *dispatch);

private:
	static void CALLBACK QueueApcCallback(ULONG_PTR user);

protected:
	unsigned long ref;
	DispatchCallbackNotifyFunc notifyCb;
	DispatchCallbackFreeFunc freeCb;
	void *param;
	
};


class DispatchCallbackApcParam
{
public:
	DispatchCallbackApcParam(IDispatch *dispatch, DispatchCallbackApc *apc);
	~DispatchCallbackApcParam();

public:
	IDispatch *GetDispatch();
	DispatchCallbackApc *GetApc();

protected:
	IDispatch *dispatch;
	DispatchCallbackApc *apc;
};

#endif