#ifndef NULLSOFT_WINAMP_OMBROWSER_CONFIG_CALLBACK_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_CONFIG_CALLBACK_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {FF6CA21B-FF24-4e51-B594-60BD9D9E7C3D}
static const GUID IFC_OmConfigCallback = 
{ 0xff6ca21b, 0xff24, 0x4e51, { 0xb5, 0x94, 0x60, 0xbd, 0x9d, 0x9e, 0x7c, 0x3d } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omconfigcallback : public Dispatchable
{
protected:
	ifc_omconfigcallback() {}
	~ifc_omconfigcallback() {}

public:
	HRESULT ValueChanged(const GUID *configUid, UINT valueId, ULONG_PTR value);
    

public:
	DISPATCH_CODES
	{
		API_VALUECHANGED	= 10,	
	};
};

inline HRESULT ifc_omconfigcallback::ValueChanged(const GUID *configUid, UINT valueId, ULONG_PTR value)
{
	return _call(API_VALUECHANGED, (HRESULT)E_NOTIMPL, configUid, valueId, value); 
}

#endif // NULLSOFT_WINAMP_OMBROWSER_CONFIG_CALLBACK_INTERFACE_HEADER