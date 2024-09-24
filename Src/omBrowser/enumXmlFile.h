#ifndef NULLSOFT_WINAMP_ENUMERATOR_XML_HEADER
#define NULLSOFT_WINAMP_ENUMERATOR_XML_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omserviceenum.h"
#include "./ifc_omxmlserviceenum.h"
#include "./xmlResponseParser.h"

#include <bfc/multipatch.h>

class ifc_omservicehost;
class obj_xml;

#define MPIID_OMSERVICEENUM			10
#define MPIID_OMXMLSERVICEENUM		20

class EnumXmlFile : public MultiPatch<MPIID_OMSERVICEENUM, ifc_omserviceenum>,
					public MultiPatch<MPIID_OMXMLSERVICEENUM, ifc_omxmlserviceenum>
{
 
protected:
	EnumXmlFile(HANDLE xmlHandle, obj_xml *xmlReader, LPCWSTR pszAddress, ifc_omservicehost *serviceHost);
	~EnumXmlFile();

public:
	static HRESULT CreateInstance(LPCWSTR pszAddress, ifc_omservicehost *host, EnumXmlFile **instance);
	static HRESULT CheckXmlHeader(HANDLE hFile);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omserviceenum */
	HRESULT Next(ULONG listSize, ifc_omservice **elementList, ULONG *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(ULONG elementCount);

	/* ifc_omxmlserviceenum */
	HRESULT GetStatusCode(UINT *code);
	HRESULT GetStatusText(LPWSTR pszBuffer, UINT cchBufferMax);

protected:
	RECVS_MULTIPATCH;

protected:
	ULONG ref;
	LPWSTR address;
	HANDLE hFile;
	obj_xml *reader;
	INT readerError;
	XmlResponseParser parser;
	BYTE *buffer;
	UINT bufferMax;
};

#endif //NULLSOFT_WINAMP_ENUMERATOR_XML_HEADER