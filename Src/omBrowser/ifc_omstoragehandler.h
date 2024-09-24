#ifndef NULLSOFT_WINAMP_OMSTORAGE_HANDLER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_HANDLER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {9B37B560-CF31-41c6-BB35-14F16E290AFB}
static const GUID IFC_OmStorageHandler = 
{ 0x9b37b560, 0xcf31, 0x41c6, { 0xbb, 0x35, 0x14, 0xf1, 0x6e, 0x29, 0xa, 0xfb } };

#include <bfc/dispatch.h>

class ifc_omstorage;
class ifc_omservice;

class __declspec(novtable) ifc_omstoragehandler : public Dispatchable
{
protected:
	ifc_omstoragehandler() {}
	~ifc_omstoragehandler() {}

public:
	HRESULT GetKey(const wchar_t **ppKey);
	void Invoke(ifc_omservice *service, const wchar_t *key, const wchar_t * value);
	
public:
	DISPATCH_CODES
	{
		API_GETKEY = 10,
		API_INVOKE = 20,
	};
};

inline HRESULT ifc_omstoragehandler::GetKey(const wchar_t **ppKey)
{
	return _call(API_GETKEY, (HRESULT)E_NOTIMPL, ppKey);
}

inline void ifc_omstoragehandler::Invoke(ifc_omservice *service, const wchar_t *key, const wchar_t *value)
{
	_voidcall(API_INVOKE, service, key, value);
}

#endif //NULLSOFT_WINAMP_OMSTORAGE_HANDLER_INTERFACE_HEADER