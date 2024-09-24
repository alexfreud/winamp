#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_ICON_STORE_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_ICON_STORE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {0CEFAC39-DCA5-4c6f-89E3-1C7573B98664}
static const GUID IFC_DeviceIconStore = 
{ 0xcefac39, 0xdca5, 0x4c6f, { 0x89, 0xe3, 0x1c, 0x75, 0x73, 0xb9, 0x86, 0x64 } };

#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_deviceiconstore : public Dispatchable
{
public:
	typedef BOOL (*EnumeratorCallback)(const wchar_t *path, unsigned int width, unsigned int height, void *user);

protected:
	ifc_deviceiconstore() {}
	~ifc_deviceiconstore() {}

public:
	HRESULT Add(const wchar_t *path, unsigned int width, unsigned int height, BOOL replaceExisting);
	HRESULT Remove(unsigned int width, unsigned int height);
	HRESULT RemovePath(const wchar_t *path);
	HRESULT RemoveAll();
	HRESULT Get(wchar_t *buffer, size_t bufferMax, unsigned int width, unsigned int height);
	HRESULT GetExact(wchar_t *buffer, size_t bufferMax, unsigned int width, unsigned int height);

	HRESULT SetBasePath(const wchar_t *path);
	HRESULT GetBasePath(wchar_t *buffer, size_t bufferMax);

	HRESULT Clone(ifc_deviceiconstore **instance);

	HRESULT Enumerate(EnumeratorCallback callback, void *user);
	
public:
	DISPATCH_CODES
	{
		API_ADD			= 10,
		API_REMOVE		= 20,
		API_REMOVEPATH	= 30,
		API_REMOVEALL	= 40,
		API_GET			= 50,
		API_GETEXACT	= 60,
		API_GETBASEPATH	= 70,
		API_SETBASEPATH = 80,
		API_CLONE		= 90,
		API_ENUMERATE   = 100,
	};
};

inline HRESULT ifc_deviceiconstore::Add(const wchar_t *path, unsigned int width, unsigned int height, BOOL replaceExisting)
{
	return _call(API_ADD, (HRESULT)E_NOTIMPL, path, width, height, replaceExisting);
}

inline HRESULT ifc_deviceiconstore::Remove(unsigned int width, unsigned int height)
{
	return _call(API_REMOVE, (HRESULT)E_NOTIMPL, width, height);
}

inline HRESULT ifc_deviceiconstore::RemovePath(const wchar_t *path)
{
	return _call(API_REMOVEPATH, (HRESULT)E_NOTIMPL, path);
}

inline HRESULT ifc_deviceiconstore::RemoveAll()
{
	return _call(API_REMOVEALL, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_deviceiconstore::Get(wchar_t *buffer, size_t bufferMax, unsigned int width, unsigned int height)
{
	return _call(API_GET, (HRESULT)E_NOTIMPL, buffer, bufferMax, width, height);
}

inline HRESULT ifc_deviceiconstore::GetExact(wchar_t *buffer, size_t bufferMax, unsigned int width, unsigned int height)
{
	return _call(API_GETEXACT, (HRESULT)E_NOTIMPL, buffer, bufferMax, width, height);
}

inline HRESULT ifc_deviceiconstore::SetBasePath(const wchar_t *path)
{
	return _call(API_SETBASEPATH, (HRESULT)E_NOTIMPL, path);
}

inline HRESULT ifc_deviceiconstore::GetBasePath(wchar_t *buffer, size_t bufferMax)
{
	return _call(API_GETBASEPATH, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

inline HRESULT ifc_deviceiconstore::Clone(ifc_deviceiconstore **instance)
{
	return _call(API_CLONE, (HRESULT)E_NOTIMPL, instance);
}

inline HRESULT ifc_deviceiconstore::Enumerate(EnumeratorCallback callback, void *user)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, callback, user);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_ICON_STORE_INTERFACE_HEADER