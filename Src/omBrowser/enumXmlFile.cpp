#include "main.h"
#include "./enumXmlFile.h"
#include "./service.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omstorage.h"
#include "./ifc_wasabihelper.h"
#include "../xml/obj_xml.h"

#include <shlwapi.h>
#include <strsafe.h>

#define XMLSIG		"<?xml"
#define XMLSIG_LEN	5

EnumXmlFile::EnumXmlFile(HANDLE xmlHandle, obj_xml *xmlReader, LPCWSTR pszAddress, ifc_omservicehost *serviceHost)
	: ref(1), address(NULL), hFile(xmlHandle), reader(xmlReader), readerError(OBJ_XML_SUCCESS), 
	buffer(NULL), bufferMax(4096)
{
	address = Plugin_CopyString(pszAddress);
	
	if (NULL != reader) 
		reader->AddRef();

	parser.Initialize(reader, serviceHost);
}

EnumXmlFile::~EnumXmlFile()
{
	Plugin_FreeString(address);

	if (NULL != hFile) 
		CloseHandle(hFile);

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

	if (NULL != buffer) 
		free(buffer);
}

HRESULT EnumXmlFile::CheckXmlHeader(HANDLE hFile)
{
	DWORD read = 0;
	BYTE szBuffer[XMLSIG_LEN * 2] = {0};

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	
	if (FALSE == ReadFile(hFile, (void*)szBuffer, sizeof(szBuffer), &read, NULL))
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

	if (read >= XMLSIG_LEN)
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, XMLSIG, XMLSIG_LEN, (LPSTR)szBuffer, XMLSIG_LEN) ||
			CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, WTEXT(XMLSIG), XMLSIG_LEN, (LPWSTR)szBuffer, XMLSIG_LEN))
		{
			return S_OK;
		}
	}

	return OMSTORAGE_E_UNKNOWN_FORMAT;
}

HRESULT EnumXmlFile::CreateInstance(LPCWSTR pszAddress, ifc_omservicehost *host, EnumXmlFile **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	HRESULT hr;
	WCHAR szBuffer[MAX_PATH * 2] = {0};
	hr = Plugin_ResolveRelativePath(pszAddress, host, szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr)) return hr;
	
	HANDLE hFile = CreateFile(szBuffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	hr = CheckXmlHeader(hFile);
	
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
				*instance = new EnumXmlFile(hFile, reader, szBuffer, host);
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
	
	if (FAILED(hr))
	{
		if (NULL != hFile) CloseHandle(hFile);
	}

	return hr;
}

size_t EnumXmlFile::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t EnumXmlFile::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int EnumXmlFile::QueryInterface(GUID interface_guid, void **object)
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

HRESULT EnumXmlFile::Next(ULONG listSize, ifc_omservice **elementList, ULONG *elementCount)
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
			if (NULL == buffer)
			{
				buffer = (BYTE*)calloc(bufferMax, sizeof(BYTE));
				if (NULL == buffer) return E_OUTOFMEMORY;
			}

			DWORD read = 0;
			if (FALSE == ReadFile(hFile, buffer, bufferMax, &read, NULL))
			{
				DWORD error = GetLastError();
				return HRESULT_FROM_WIN32(error);
			}

			if (0 == read)
			{
				if (OBJ_XML_SUCCESS == readerError)
					reader->xmlreader_feed(0, 0);
				break;
			}

			readerError = reader->xmlreader_feed(buffer, read);
			if (OBJ_XML_SUCCESS != readerError) return E_FAIL;
		}

	}

	if(NULL != elementCount)
		*elementCount = counter;
			
	return (counter > 0) ? S_OK : S_FALSE;
}

HRESULT EnumXmlFile::Reset(void)
{
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	readerError = OBJ_XML_SUCCESS;
	reader->xmlreader_reset();
	parser.Reset();
	return E_NOTIMPL;
}

HRESULT EnumXmlFile::Skip(ULONG elementCount)
{
	return E_NOTIMPL;
}

HRESULT EnumXmlFile::GetStatusCode(UINT *code)
{
	return parser.GetCode(code);
}

HRESULT EnumXmlFile::GetStatusText(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return parser.GetText(pszBuffer, cchBufferMax);
}

#define CBCLASS EnumXmlFile
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