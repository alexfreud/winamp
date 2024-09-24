#ifndef NULLSOFT_WINAMP_OMSERVICE_XML_ENUMERATOR_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_XML_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {E7105EE1-161B-4580-A8C0-6673FB59AB47}
static const GUID IFC_OmXmlServiceEnum = 
{ 0xe7105ee1, 0x161b, 0x4580, { 0xa8, 0xc0, 0x66, 0x73, 0xfb, 0x59, 0xab, 0x47 } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omxmlserviceenum : public Dispatchable
{
protected:
	ifc_omxmlserviceenum() {}
	~ifc_omxmlserviceenum() {}

public:
	HRESULT GetStatusCode(unsigned int *code);
	HRESULT GetStatusText(wchar_t *buffer, unsigned int bufferMax);

public:
	DISPATCH_CODES
	{
		API_GETSTATUSCODE = 10,
		API_GETSTATUSTEXT = 20,
	};
};

inline HRESULT ifc_omxmlserviceenum::GetStatusCode(unsigned int *code)
{
	return _call(API_GETSTATUSCODE, (HRESULT)E_NOTIMPL, code);
}

inline HRESULT ifc_omxmlserviceenum::GetStatusText(wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETSTATUSTEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_XML_ENUMERATOR_INTERFACE_HEADER