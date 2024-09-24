#ifndef NULLSOFT_WINAMP_OMSERVICE_COPIER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_COPIER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {24E24C55-0A9D-4c54-9589-95FE07FC1FB2}
static const GUID IFC_OmServiceCopier = 
{ 0x24e24c55, 0xa9d, 0x4c54, { 0x95, 0x89, 0x95, 0xfe, 0x7, 0xfc, 0x1f, 0xb2 } };


class ifc_omservice;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_omservicecopier: public Dispatchable
{
protected:
	ifc_omservicecopier() {}
	~ifc_omservicecopier() {}

public:
	HRESULT CopyTo(ifc_omservice *service, unsigned int *modifiedFlags);
	
public:
	DISPATCH_CODES
	{
		API_COPYTO	= 10,
	};
};


inline HRESULT ifc_omservicecopier::CopyTo(ifc_omservice *service, unsigned int *modifiedFlags)
{
	return _call(API_COPYTO, (HRESULT)E_NOTIMPL, service, modifiedFlags);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_COPIER_INTERFACE_HEADER