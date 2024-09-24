#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_ICON_STORE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_ICON_STORE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_deviceiconstore.h"
#include <vector>

class DeviceIconStore : public ifc_deviceiconstore
{

protected:
	DeviceIconStore();
	~DeviceIconStore();

public:
	static HRESULT CreateInstance(DeviceIconStore **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_deviceiconstore */
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
	void Lock();
	void Unlock();

	HRESULT GetFullPath(wchar_t *buffer, size_t bufferMax, const wchar_t *path);

protected:
	typedef struct Record
	{
		unsigned int width;
		unsigned int height;
		wchar_t *path;
	} Record;

	typedef std::vector<Record> RecordList;

protected:
	size_t ref;
	wchar_t *base;
	RecordList list;
	CRITICAL_SECTION lock;

protected:
	RECVS_DISPATCH;
	
	

};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_ICON_STORE_HEADER