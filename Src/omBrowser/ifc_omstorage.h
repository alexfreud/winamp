#ifndef NULLSOFT_WINAMP_OMSTORAGE_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {DEE1FCCD-8FFC-4ed2-8104-323382670BE0}
static const GUID IFC_OmStorage = 
{ 0xdee1fccd, 0x8ffc, 0x4ed2, { 0x81, 0x4, 0x32, 0x33, 0x82, 0x67, 0xb, 0xe0 } };

#include <ifc_omstorageasync.h>
#include <bfc/dispatch.h>

class ifc_omservice;
class ifc_omservicehost;
class ifc_omserviceenum;

#define OMSTORAGE_E_UNKNOWN_FORMAT MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_INVALID_DATA)

class __declspec(novtable) ifc_omstorage : public Dispatchable
{
public:
	typedef enum 
	{
		capLoad			= 0x00000001,
		capSave			= 0x00000002,
		capDelete		= 0x00000004,
		capReload		= 0x00000008,
		capPublic		= 0x00001000,
	} Capabilities;

	typedef enum
	{
		saveModifiedOnly = 0x00000001,
		saveClearModified = 0x00000002,
	} SaveFlags;

protected:
	ifc_omstorage() {}
	~ifc_omstorage() {}

public:
	HRESULT GetId(GUID *storageUid);
	HRESULT GetType(GUID *storageType);
	UINT GetCapabilities();
	HRESULT GetDescription(wchar_t *buffer, unsigned int bufferMax);
	
	HRESULT Load(const wchar_t *address, ifc_omservicehost *host, ifc_omserviceenum **ppEnum);
	HRESULT Save(ifc_omservice **serviceList, unsigned long listCount, unsigned int saveFlags, unsigned long *savedCount);
	HRESULT Delete(ifc_omservice **serviceList, unsigned long listCount, unsigned long *deletedCount);
	HRESULT Reload(ifc_omservice **serviceList, unsigned long listCount, unsigned long *reloadedCount);

	/* async calls */
	HRESULT BeginLoad(const wchar_t *address, ifc_omservicehost *host, ifc_omstorageasync::AsyncCallback callback, void *data, ifc_omstorageasync **async);
	HRESULT EndLoad(ifc_omstorageasync *async, ifc_omserviceenum **ppEnum);
	HRESULT RequestAbort(ifc_omstorageasync *async, BOOL drop);
	
public:
	DISPATCH_CODES
	{
		API_GETID			= 10,
		API_GETTYPE			= 20,
		API_GETCAPABILITIES = 30,
		API_GETDESCRIPTION  = 40,
		API_LOAD			= 60,
		API_SAVE			= 70,
		API_DELETE			= 80,
		API_RELOAD			= 90,
		API_BEGINLOAD		= 100,
		API_ENDLOAD			= 110,
		API_REQUESTABORT	= 120,
	};
};

inline HRESULT ifc_omstorage::GetId(GUID *storageUid)
{
	return _call(API_GETID, (HRESULT)E_NOTIMPL, storageUid);
}

inline HRESULT ifc_omstorage::GetType(GUID *storageType)
{
	return _call(API_GETTYPE, (HRESULT)E_NOTIMPL, storageType);
}

inline UINT ifc_omstorage::GetCapabilities()
{
	return (UINT)_call(API_GETCAPABILITIES, (UINT)0);
}

inline HRESULT ifc_omstorage::GetDescription(wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETDESCRIPTION, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

inline HRESULT ifc_omstorage::Load(const wchar_t *address, ifc_omservicehost *host, ifc_omserviceenum **ppEnum)
{
	return _call(API_LOAD, (HRESULT)E_NOTIMPL, address, host, ppEnum);
}

inline HRESULT ifc_omstorage::Save(ifc_omservice **serviceList, unsigned long listCount, unsigned int saveFlags, unsigned long *savedCount)
{
	return _call(API_SAVE, (HRESULT)E_NOTIMPL, serviceList, listCount, saveFlags, savedCount);
}

inline HRESULT ifc_omstorage::Delete(ifc_omservice **serviceList, unsigned long listCount, unsigned long *deletedCount)
{
	return _call(API_DELETE, (HRESULT)E_NOTIMPL, serviceList, listCount, deletedCount);
}

inline HRESULT ifc_omstorage::Reload(ifc_omservice **serviceList, unsigned long listCount, unsigned long *reloadedCount)
{
	return _call(API_RELOAD, (HRESULT)E_NOTIMPL, serviceList, listCount, reloadedCount);
}

inline HRESULT ifc_omstorage::BeginLoad(const wchar_t *address, ifc_omservicehost *host, ifc_omstorageasync::AsyncCallback callback, void *data, ifc_omstorageasync **async)
{
	return _call(API_BEGINLOAD, (HRESULT)E_NOTIMPL, address, host, callback, data, async);
}

inline HRESULT ifc_omstorage::EndLoad(ifc_omstorageasync *async, ifc_omserviceenum **ppEnum)
{
	return _call(API_ENDLOAD, (HRESULT)E_NOTIMPL, async, ppEnum);
}

inline HRESULT ifc_omstorage::RequestAbort(ifc_omstorageasync *async, BOOL drop)
{
	return _call(API_REQUESTABORT, (HRESULT)E_NOTIMPL, async, drop);
}

#endif //NULLSOFT_WINAMP_OMSTORAGE_INTERFACE_HEADER