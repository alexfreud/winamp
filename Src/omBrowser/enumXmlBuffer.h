#ifndef NULLSOFT_WINAMP_ENUMERATOR_XML_BUFFER_HEADER
#define NULLSOFT_WINAMP_ENUMERATOR_XML_BUFFER_HEADER

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

class EnumXmlBuffer : public MultiPatch<MPIID_OMSERVICEENUM, ifc_omserviceenum>,
					public MultiPatch<MPIID_OMXMLSERVICEENUM, ifc_omxmlserviceenum>
{
 
protected:
	EnumXmlBuffer(obj_xml *xmlReader, const void *buffer, size_t bufferSize, Dispatchable *bufferOwner, ifc_omservicehost *serviceHost);
	~EnumXmlBuffer();

public:
	static HRESULT CreateInstance(const void *buffer, size_t bufferSize, Dispatchable *bufferOwner, ifc_omservicehost *host, EnumXmlBuffer **instance);
	static HRESULT CheckXmlHeader(const void *buffer, size_t bufferSize);

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
	ULONG ref;
	const void *buffer;
	size_t bufferSize;
	size_t cursor;
	obj_xml *reader;
	INT readerError;
	XmlResponseParser parser;
	Dispatchable *bufferOwner;

protected:
	RECVS_MULTIPATCH;
};

#endif //NULLSOFT_WINAMP_ENUMERATOR_XML_BUFFER_HEADER