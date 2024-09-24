#pragma once
#ifdef __cplusplus
#include <unknwn.h>
class ThreadRefCount : public IUnknown
{
public:
	ThreadRefCount();
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	LONG refCount;
};

class AppRefCount : public IUnknown
{
public:
	AppRefCount();
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	LONG refCount;
	DWORD m_dwThread;
};

extern "C" {
#endif
	void *InitAppRefCounterObject(DWORD threadId);
	void *GetAppRefCounterObject();
	void AppRefCount_Release();
	int AppRefCount_CanQuit();
#ifdef __cplusplus
}
#endif