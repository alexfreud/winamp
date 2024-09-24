#ifndef NULLSOFT_WINAMP_OMCACHE_MANAGER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMCACHE_MANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {95673D91-A7D3-4797-A599-2CE0E4CD2A20}
static const GUID IFC_OmCacheManager = 
{ 0x95673d91, 0xa7d3, 0x4797, { 0xa5, 0x99, 0x2c, 0xe0, 0xe4, 0xcd, 0x2a, 0x20 } };

class ifc_omcachegroup;

class __declspec(novtable) ifc_omcachemanager: public Dispatchable
{

protected:
	ifc_omcachemanager() {}
	~ifc_omcachemanager() {}

public:
	HRESULT Find(const wchar_t *group, BOOL registerMissing, ifc_omcachegroup **groupOut, BOOL *created);
	HRESULT Delete(const wchar_t *group);
	HRESULT Clear();
	
public:
	DISPATCH_CODES
	{	
		API_FIND = 10,
		API_DELETE = 20,
		API_CLEAR = 30,
	};
};

inline HRESULT ifc_omcachemanager::Find(const wchar_t *group, BOOL registerMissing, ifc_omcachegroup **groupOut, BOOL *created)
{
	return _call(API_FIND, (HRESULT)E_NOTIMPL, group, registerMissing, groupOut, created);
}

inline HRESULT ifc_omcachemanager::Delete(const wchar_t *group)
{
	return _call(API_DELETE, (HRESULT)E_NOTIMPL, group);
}

inline HRESULT ifc_omcachemanager::Clear()
{
	return _call(API_CLEAR, (HRESULT)E_NOTIMPL);
}

#endif //NULLSOFT_WINAMP_OMCACHE_MANAGER_INTERFACE_HEADER