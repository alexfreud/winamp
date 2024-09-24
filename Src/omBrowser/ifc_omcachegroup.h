#ifndef NULLSOFT_WINAMP_OMCACHE_GROUP_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMCACHE_GROUP_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {3249BCBE-A169-4051-8C36-2E355E63C27B}
static const GUID IFC_OmCacheGroup = 
{ 0x3249bcbe, 0xa169, 0x4051, { 0x8c, 0x36, 0x2e, 0x35, 0x5e, 0x63, 0xc2, 0x7b } };

class ifc_omcacherecord;

class __declspec(novtable) ifc_omcachegroup: public Dispatchable
{

protected:
	ifc_omcachegroup() {}
	~ifc_omcachegroup() {}

public:
	HRESULT GetName(wchar_t *buffer, unsigned int bufferMax);

	HRESULT Find(const wchar_t *name, BOOL registerMissing, ifc_omcacherecord **recordOut, BOOL *created);
	HRESULT Delete(const wchar_t *name);
	HRESULT Clear();
	
public:
	DISPATCH_CODES
	{	
		API_GETNAME = 10,
		API_FIND = 20,
		API_DELETE = 30,
		API_CLEAR = 40,
	};
};

inline HRESULT ifc_omcachegroup::GetName(wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETNAME, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

inline HRESULT ifc_omcachegroup::Find(const wchar_t *name, BOOL registerMissing, ifc_omcacherecord **recordOut, BOOL *created)
{
	return _call(API_FIND, (HRESULT)E_NOTIMPL, name, registerMissing, recordOut, created);
}

inline HRESULT ifc_omcachegroup::Delete(const wchar_t *name)
{
	return _call(API_DELETE, (HRESULT)E_NOTIMPL, name);
}

inline HRESULT ifc_omcachegroup::Clear()
{
	return _call(API_CLEAR, (HRESULT)E_NOTIMPL);
}

#endif //NULLSOFT_WINAMP_OMCACHE_GROUP_INTERFACE_HEADER