// IDeferZeroFill interface exposed a custom memory allocator and media sample.
//
// Copyright (C) 1997- Cakewalk Music Software.  All rights reserved.
//

#ifndef _DEFERZEROFILL_H_
#define _DEFERZEROFILL_H_

/////////////////////////////////////////////////////////////////////////////
// Clients that wish to create real copies of the GUID must first include INITGUID.H

// {447DA113-4AC8-4833-849A-2BA285E1E52B}
DEFINE_GUID(IID_IDeferZeroFill,
0x447da113, 0x4ac8, 0x4833, 0x84, 0x9a, 0x2b, 0xa2, 0x85, 0xe1, 0xe5, 0x2b);

#undef  INTERFACE
#define INTERFACE IDeferZeroFill

DECLARE_INTERFACE_( IDeferZeroFill, IUnknown )
{
	// *** IUnknown methods ***
	STDMETHOD_(HRESULT,	QueryInterface)( THIS_ REFIID riid, LPVOID* ppvObj ) PURE;
	STDMETHOD_(ULONG,		AddRef)( THIS )  PURE;
	STDMETHOD_(ULONG,		Release)( THIS ) PURE;

	// *** IDeferZeroFill methods ***
	STDMETHOD_(BOOL,		get_NeedsZerofill)( THIS ) PURE;
	STDMETHOD_(void,		put_NeedsZerofill)( THIS_ BOOL bZero ) PURE;
	STDMETHOD_(HRESULT,	GetRawPointer)( THIS_ BYTE** ppBuffer ) PURE;
};

#endif // _DEFERZEROFILL_H_

