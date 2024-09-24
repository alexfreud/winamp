#ifndef NULLSOFT_WINAMP_OMSTORAGETYPE_FILE_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGETYPE_FILE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {E49678EB-9DA4-43c3-8A80-18AA5EE369BF}
static const GUID STID_OmFileStorage = 
{ 0xe49678eb, 0x9da4, 0x43c3, { 0x8a, 0x80, 0x18, 0xaa, 0x5e, 0xe3, 0x69, 0xbf } };
#define IFC_OmFileStorage STID_OmFileStorage

// {E6D3D527-4721-4fb6-BAB2-304D17C549B2}
static const GUID SUID_OmStorageIni = 
{ 0xe6d3d527, 0x4721, 0x4fb6, { 0xba, 0xb2, 0x30, 0x4d, 0x17, 0xc5, 0x49, 0xb2 } };

// {0F356A24-114D-4582-9005-A59E08B3164A}
static const GUID SUID_OmStorageXml = 
{ 0xf356a24, 0x114d, 0x4582, { 0x90, 0x5, 0xa5, 0x9e, 0x8, 0xb3, 0x16, 0x4a } };

#include <bfc/dispatch.h>
#include <ifc_omstorage.h>

class __declspec(novtable) ifc_omfilestorage : public Dispatchable
{
protected:
	ifc_omfilestorage() {}
	~ifc_omfilestorage() {}

public:
	HRESULT GetFilter(wchar_t *buffer, unsigned int bufferMax);
	
public:
	DISPATCH_CODES
	{
		API_GETFILTER		= 10,
	};
};

inline HRESULT ifc_omfilestorage::GetFilter(wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETFILTER, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

#endif //NULLSOFT_WINAMP_OMSTORAGETYPE_FILE_INTERFACE_HEADER