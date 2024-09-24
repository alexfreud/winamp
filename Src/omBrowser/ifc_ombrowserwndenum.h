#ifndef NULLSOFT_WINAMP_OMBROWSER_WINDOW_ENUMERATOR_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_WINDOW_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {8B184E07-1DC6-4d76-8FB3-A793FF4A062B}
static const GUID IFC_OmBrowserWindowEnumerator = 
{ 0x8b184e07, 0x1dc6, 0x4d76, { 0x8f, 0xb3, 0xa7, 0x93, 0xff, 0x4a, 0x6, 0x2b } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_ombrowserwndenum : public Dispatchable
{

protected:
	ifc_ombrowserwndenum() {}
	~ifc_ombrowserwndenum() {}

public:
	HRESULT Next(unsigned long listSize, HWND *elementList, unsigned long *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(unsigned long elementCount);

public:
	DISPATCH_CODES
	{
		API_NEXT = 10,
		API_RESET = 20,
		API_SKIP = 30,
	};
};

inline HRESULT ifc_ombrowserwndenum::Next(unsigned long listSize, HWND *elementList, unsigned long *elementCount)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, listSize, elementList, elementCount);
}

inline HRESULT ifc_ombrowserwndenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_ombrowserwndenum::Skip(unsigned long elementCount)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, elementCount);
}

#endif //NULLSOFT_WINAMP_OMBROWSER_WINDOW_ENUMERATOR_INTERFACE_HEADER