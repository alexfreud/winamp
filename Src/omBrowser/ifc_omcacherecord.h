#ifndef NULLSOFT_WINAMP_OMCACHE_RECORD_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMCACHE_RECORD_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {50C4A9E1-0F31-4692-8084-3A7835647EAE}
static const GUID IFC_OmCacheRecord = 
{ 0x50c4a9e1, 0xf31, 0x4692, { 0x80, 0x84, 0x3a, 0x78, 0x35, 0x64, 0x7e, 0xae } };

class ifc_omcachecallback;

class __declspec(novtable) ifc_omcacherecord : public Dispatchable
{
public:
	typedef enum
	{
		flagNoStore	= 0x00000001,
	} Flags;

protected:
	ifc_omcacherecord() {}
	~ifc_omcacherecord() {}

public:
	HRESULT GetName(wchar_t *buffer, unsigned int bufferMax);
	HRESULT GetPath(wchar_t *buffer, unsigned int bufferMax);
	HRESULT SetPath(const wchar_t *path);
	HRESULT SetFlags(unsigned int flags, unsigned int mask);
	HRESULT GetFlags(unsigned int *flags);

	HRESULT Download();

	HRESULT RegisterCallback(ifc_omcachecallback *callback);
	HRESULT UnregisterCallback(ifc_omcachecallback *callback);
	

public:
	DISPATCH_CODES
	{	
		API_GETNAME = 10,
		API_GETPATH = 20,
		API_SETPATH = 30,
		API_SETFLAGS = 40,
		API_GETFLAGS = 50,

		API_DOWNLOAD = 60,

		API_REGISTERCALLBACK = 70,
		API_UNREGISTERCALLBACK = 80,

	};
};

inline HRESULT ifc_omcacherecord::GetName(wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETNAME, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

inline HRESULT ifc_omcacherecord::GetPath(wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETPATH, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

inline HRESULT ifc_omcacherecord::SetPath(const wchar_t *path)
{
	return _call(API_SETPATH, (HRESULT)E_NOTIMPL, path);
}

inline HRESULT ifc_omcacherecord::SetFlags(unsigned int flags, unsigned int mask)
{
	return _call(API_SETFLAGS, (HRESULT)E_NOTIMPL, flags, mask);
}

inline HRESULT ifc_omcacherecord::GetFlags(unsigned int *flags)
{
	return _call(API_GETFLAGS, (HRESULT)E_NOTIMPL, flags);
}

inline HRESULT ifc_omcacherecord::Download()
{
	return _call(API_DOWNLOAD, (HRESULT)E_NOTIMPL);
}


inline HRESULT ifc_omcacherecord::RegisterCallback(ifc_omcachecallback *callback)
{
	return _call(API_REGISTERCALLBACK, (HRESULT)E_NOTIMPL, callback);
}

inline HRESULT ifc_omcacherecord::UnregisterCallback(ifc_omcachecallback *callback)
{
	return _call(API_UNREGISTERCALLBACK, (HRESULT)E_NOTIMPL, callback);
}

#endif //NULLSOFT_WINAMP_OMCACHE_RECORD_INTERFACE_HEADER