#ifndef NULLSOFT_WINAMP_OMSTORAGE_ASYNC_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_ASYNC_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {5D6C5C55-E744-4936-A31D-B9D8C0F4EF86}
static const GUID IFC_OmStorageAsync = 
{ 0x5d6c5c55, 0xe744, 0x4936, { 0xa3, 0x1d, 0xb9, 0xd8, 0xc0, 0xf4, 0xef, 0x86 } };

class __declspec(novtable) ifc_omstorageasync : public Dispatchable
{
public:
	typedef void (CALLBACK *AsyncCallback)(ifc_omstorageasync *result);

	typedef enum
	{
		stateReady = 0,
		stateInitializing = 1,
		stateConnecting = 2,
		stateReceiving = 3,
		stateCompleted = 4,
		stateAborting = 5,
	} States;

protected:
	ifc_omstorageasync() {}
	~ifc_omstorageasync() {}

public:
	HRESULT GetState(unsigned int *state);
	HRESULT GetWaitHandle(HANDLE *handle);
	HRESULT GetData(void **data);
			
public:
	DISPATCH_CODES
	{
		API_GETSTATE		= 10,
		API_GETWAITHANDLE	= 20,
		API_GETDATA			= 30,
	};
};

inline HRESULT ifc_omstorageasync::GetWaitHandle(HANDLE *handle)
{
	return _call(API_GETWAITHANDLE, (HRESULT)E_NOTIMPL, handle);
}

inline HRESULT ifc_omstorageasync::GetData(void **data)
{
	return _call(API_GETDATA, (HRESULT)E_NOTIMPL, data);
}

inline HRESULT ifc_omstorageasync::GetState(unsigned int *state)
{
	return _call(API_GETSTATE, (HRESULT)E_NOTIMPL, state);
}

#endif //NULLSOFT_WINAMP_OMSTORAGE_ASYNC_INTERFACE_HEADER