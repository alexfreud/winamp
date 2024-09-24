#include "main.h"
#include "./enumXmlBuffer.h"
#include "./service.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omstorage.h"
#include "./ifc_wasabihelper.h"
#include "../xml/obj_xml.h"

#include <shlwapi.h>
#include <strsafe.h>

#define XMLSIG		"<?xml"
#define XMLSIG_LEN	5

	ULONG ref;
	const void *buffer;
	size_t bufferSize;
	size_t cursor;
	obj_xml *reader;
	INT readerError;
	XmlResponseParser parser;
	Dispatchable *bufferOwner;

EnumXmlBuffer::EnumXmlBuffer(obj_xml *xmlReader, const void *buffer, size_t bufferSize, Dispatchable *bufferOwner, ifc_omservicehost *serviceHost)
	: ref(1), buffer(buffer), bufferSize(bufferSize), cursor(0), reader(xmlReader), readerError(OBJ_XML_SUCCESS), bufferOwner(bufferOwner)
{
	if (NULL != reader) 
		reader->AddRef();

	if (NULL != bufferOwner)
		bufferOwner->AddRef();

	parser.Initialize(reader, serviceHost);
}

EnumXmlBuffer::~EnumXmlBuffer()
{
	parser.Finish();

	if (NULL != reader)
	{
		if (OBJ_XML_SUCCESS == readerError)
		{
			reader->xmlreader_feed(0, 0);
		}
		reader->xmlreader_close();
		ifc_wasabihelper *wasabi;
		if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)))
		{
			wasabi->ReleaseWasabiInterface(&obj_xmlGUID, reader);
			wasabi->Release();
		}
	}

	if (NULL != bufferOwner)
		bufferOwner->Release();
}

HRESULT EnumXmlBuffer::CheckXmlHeader(const void *buffer, size_t bufferSize)
{
	if (NULL == buffer) 
		return E_INVALIDARG;

	if (bufferSize >= XMLSIG_LEN)
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, XMLSIG, XMLSIG_LEN, (LPSTR)buffer, XMLSIG_LEN) ||
			CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, WTEXT(XMLSIG), XMLSIG_LEN, (LPWSTR)buffer, XMLSIG_LEN))
		{
			return S_OK;
		}
	}
	return OMSTORAGE_E_UNKNOWN_FORMAT;
}

HRESULT EnumXmlBuffer::CreateInstance(const void *buffer, size_t bufferSize, Dispatchable *bufferOwner, ifc_omservicehost *host, EnumXmlBuffer **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

		
	HRESULT hr = CheckXmlHeader(buffer, bufferSize);
	
	if (SUCCEEDED(hr))
	{		
		ifc_wasabihelper *wasabi;
		hr = Plugin_GetWasabiHelper(&wasabi);
        if (SUCCEEDED(hr))
		{		
			obj_xml *reader;
			wasabi->QueryWasabiInterface(&obj_xmlGUID, (void**)&reader);
			
			if (NULL != reader && OBJ_XML_SUCCESS == reader->xmlreader_open())
			{
				*instance = new EnumXmlBuffer(reader, buffer, bufferSize, bufferOwner, host);
				if (NULL == *instance) 
				{
					hr = E_OUTOFMEMORY;
					// reader stil has no support for AddRef()/Release()
					wasabi->ReleaseWasabiInterface(&obj_xmlGUID, reader);

				}
				reader->Release();
			}
			else 
			{
				hr = E_UNEXPECTED;
			}
			wasabi->Release();
		}
	}
	return hr;
}

size_t EnumXmlBuffer::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t EnumXmlBuffer::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int EnumXmlBuffer::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmServiceEnum))
		*object = static_cast<ifc_omserviceenum*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmXmlServiceEnum))
		*object = static_cast<ifc_omxmlserviceenum*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT EnumXmlBuffer::Next(ULONG listSize, ifc_omservice **elementList, ULONG *elementCount)
{
	if(NULL != elementCount)
		*elementCount = 0;
	
	if (0 == listSize || NULL == elementList) 
		return E_INVALIDARG;
	
	ULONG counter = 0;
	HRESULT hr = S_OK;
	ifc_omservice *service;

	while(counter < listSize)
	{
		hr = parser.PeekService(&service);
		if (FAILED(hr)) return hr;
		if (S_OK == hr)
		{
			elementList[counter] = service;
			counter++;
		}
		else
		{
			if (cursor < bufferSize)
			{
				readerError = reader->xmlreader_feed((BYTE*)buffer + cursor, bufferSize - cursor);
				if (OBJ_XML_SUCCESS != readerError) return E_FAIL;	
				cursor = bufferSize;
			}
			else
			{
				if (OBJ_XML_SUCCESS == readerError)
					reader->xmlreader_feed(0, 0);
				break;
			}

			
		}

	}

	if(NULL != elementCount)
		*elementCount = counter;
			
	return (counter > 0) ? S_OK : S_FALSE;
}

HRESULT EnumXmlBuffer::Reset(void)
{
	cursor = 0;
	readerError = OBJ_XML_SUCCESS;
	reader->xmlreader_reset();
	parser.Reset();
	return E_NOTIMPL;
}

HRESULT EnumXmlBuffer::Skip(ULONG elementCount)
{
	return E_NOTIMPL;
}

HRESULT EnumXmlBuffer::GetStatusCode(UINT *code)
{
	return parser.GetCode(code);
}

HRESULT EnumXmlBuffer::GetStatusText(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return parser.GetText(pszBuffer, cchBufferMax);
}

#define CBCLASS EnumXmlBuffer
START_MULTIPATCH;
 START_PATCH(MPIID_OMSERVICEENUM)

  M_CB(MPIID_OMSERVICEENUM, ifc_omserviceenum, ADDREF, AddRef);
  M_CB(MPIID_OMSERVICEENUM, ifc_omserviceenum, RELEASE, Release);
  M_CB(MPIID_OMSERVICEENUM, ifc_omserviceenum, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSERVICEENUM, ifc_omserviceenum, API_NEXT, Next);
  M_CB(MPIID_OMSERVICEENUM, ifc_omserviceenum, API_RESET, Reset);
  M_CB(MPIID_OMSERVICEENUM, ifc_omserviceenum, API_SKIP, Skip);
   
 NEXT_PATCH(MPIID_OMXMLSERVICEENUM)
  M_CB(MPIID_OMXMLSERVICEENUM, ifc_omxmlserviceenum, ADDREF, AddRef);
  M_CB(MPIID_OMXMLSERVICEENUM, ifc_omxmlserviceenum, RELEASE, Release);
  M_CB(MPIID_OMXMLSERVICEENUM, ifc_omxmlserviceenum, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMXMLSERVICEENUM, ifc_omxmlserviceenum, API_GETSTATUSCODE, GetStatusCode);
  M_CB(MPIID_OMXMLSERVICEENUM, ifc_omxmlserviceenum, API_GETSTATUSTEXT, GetStatusText);
 
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS