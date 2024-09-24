#ifndef NULLSOFT_WINAMP_OMSTORAGE_HELPER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_HELPER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {47243D54-37F3-4493-8643-BE75205E6CBF}
static const GUID IFC_OmStorageHelper = 
{ 0x47243d54, 0x37f3, 0x4493, { 0x86, 0x43, 0xbe, 0x75, 0x20, 0x5e, 0x6c, 0xbf } };

class ifc_omservice;
class ifc_omstoragehandler;
class ifc_omstoragehandlerenum;

class __declspec(novtable) ifc_omstoragehelper : public Dispatchable
{
public:
	typedef void (CALLBACK *HandlerProc)(ifc_omservice * /*service*/, const wchar_t * /*pszKey*/, const wchar_t * /*pszValue*/);

	typedef struct __TemplateRecord
	{
		LPCWSTR key;
		HandlerProc handler;
	} TemplateRecord;

protected:
	ifc_omstoragehelper() {}
	~ifc_omstoragehelper() {}

public:
	HRESULT CreateHandler(const wchar_t *key, HandlerProc proc, ifc_omstoragehandler **handler);
	HRESULT CreateEnumerator(const TemplateRecord *recordList, size_t recordCount, ifc_omstoragehandlerenum **enumerator);

public:
	DISPATCH_CODES
	{	
		API_CREATEHANDLER = 10,
		API_CREATEENUMERATOR = 20,
	};
};

inline HRESULT ifc_omstoragehelper::CreateHandler(const wchar_t *key, HandlerProc proc, ifc_omstoragehandler **handler)
{
	return _call(API_CREATEHANDLER, (HRESULT)E_NOTIMPL, key, proc, handler);
}

inline HRESULT ifc_omstoragehelper::CreateEnumerator(const TemplateRecord *recordList, size_t recordCount, ifc_omstoragehandlerenum **enumerator)
{
	return _call(API_CREATEENUMERATOR, (HRESULT)E_NOTIMPL, recordList, recordCount, enumerator);
}

#endif //NULLSOFT_WINAMP_OMSTORAGE_HELPER_INTERFACE_HEADER