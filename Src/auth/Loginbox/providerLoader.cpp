#include "./providerLoader.h"
#include "./providerEnumerator.h"
#include "./xmlInt32Parser.h"

#include "../api.h"
#include "../../xml/obj_xml.h"
#include <api/service/waservicefactory.h>


LoginProviderLoader::LoginProviderLoader() 
{
}

LoginProviderLoader::~LoginProviderLoader()
{
}

HRESULT LoginProviderLoader::ReadXml(LPCWSTR pszPath, LoginProviderEnumerator **enumerator, INT *prefVisible)
{
	if (NULL == enumerator) return E_POINTER;
	*enumerator = NULL;
	
	if (NULL == pszPath || L'\0' == *pszPath) 
		return E_INVALIDARG;

	HANDLE hFile = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	HRESULT hr;

	if (NULL != WASABI_API_SVC)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
		obj_xml *reader = (NULL != sf) ? (obj_xml*)sf->getInterface() : NULL;
		if (NULL != reader)
		{
			if (OBJ_XML_SUCCESS == reader->xmlreader_open())
			{
				providerList.clear();
				parser.SetReader(reader);

				XmlInt32Parser visibleParser;

				reader->xmlreader_registerCallback(L"loginProviders\fprovider", this);
				if (NULL != prefVisible)
					reader->xmlreader_registerCallback(L"loginProviders\fvisibleProviders", &visibleParser);

				hr = FeedFile(reader, hFile, 8192);
				reader->xmlreader_close();

				parser.SetReader(NULL);
				if (SUCCEEDED(hr))
					hr = LoginProviderEnumerator::CreateInstance(providerList.begin(), providerList.size(), enumerator);

				if (NULL != prefVisible && FAILED(visibleParser.GetValue(prefVisible)))
					*prefVisible = 0;

			}
			else
				hr = E_FAIL;

			sf->releaseInterface(reader);
		}
		else
			hr = E_FAIL;
	}
	else
		hr = E_UNEXPECTED;


	CloseHandle(hFile);
	return hr;
}


HRESULT LoginProviderLoader::FeedFile(obj_xml *reader, HANDLE hFile, DWORD bufferSize)
{
	if (NULL == reader || INVALID_HANDLE_VALUE == hFile || 0 == bufferSize)
		return E_INVALIDARG;

	BYTE *buffer = (BYTE*)calloc(bufferSize, sizeof(BYTE));
	if (NULL == buffer) return E_OUTOFMEMORY;
	
	HRESULT hr;
	INT readerCode = OBJ_XML_SUCCESS;
	
	for(;;)
	{
		DWORD read = 0;
		if (FALSE == ReadFile(hFile, buffer, bufferSize, &read, NULL) || 0 == read)
		{
			DWORD errorCode = GetLastError();
			hr = HRESULT_FROM_WIN32(errorCode);

			if (0 == read && OBJ_XML_SUCCESS == readerCode)
				reader->xmlreader_feed(0, 0);

			break;
		}

		readerCode = reader->xmlreader_feed(buffer, read);
		if (OBJ_XML_SUCCESS != readerCode) 
		{
			hr = E_FAIL;
			break;
		}
	}

	free(buffer);
	return hr;
}

void LoginProviderLoader::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	parser.Begin(params); 
}

void LoginProviderLoader::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	LoginProvider *provider;
	if (SUCCEEDED(parser.End(&provider)))
	{
		providerList.push_back(provider);
	}
}

void LoginProviderLoader::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{
	parser.End(NULL);
}

#define CBCLASS LoginProviderLoader
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS