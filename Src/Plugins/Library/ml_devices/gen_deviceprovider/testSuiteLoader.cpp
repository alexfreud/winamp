#include "main.h"
#include "./testSuiteLoader.h"

#include "../../xml/obj_xml.h"
#include <api/service/waservicefactory.h>

#include <strsafe.h>


typedef void (CALLBACK *LOADERTAGCALLBACK)(TestSuiteLoader* /*loader*/, const wchar_t* /*value*/);

typedef struct LOADERTAG
{
	const wchar_t *name;
	LOADERTAGCALLBACK callback;
} LOADERTAG;

static void CALLBACK 
LoaderTag_ImageBase(TestSuiteLoader *loader, const wchar_t *value)
{
	String_Free(loader->imageBase);
	loader->imageBase = String_Duplicate(value);
}

static void CALLBACK 
LoaderTag_Connect(TestSuiteLoader *loader, const wchar_t *value)
{
	if (IS_STRING_EMPTY(value))
		return;

	const wchar_t *block, *cursor;
	char *name;
	size_t length;

	block = value; 
	cursor = block;
	for(;;)
	{
		if (L'\0' == *cursor ||
			L';' == *cursor ||
			L',' == *cursor)
		{
			
			if (block < cursor)
			{
				length = cursor - block;
				name = String_ToAnsi(CP_UTF8, 0, block, (int)length, NULL, NULL);
				if (NULL != name)
					loader->connectList.push_back(name);
			}

			if (L'\0' == *cursor)
				break;

			block = cursor + 1;
		}
		cursor++;
	}
 //loader->SetImageBase(value);
}


static const LOADERTAG knownTags[LOADER_TAG_MAX] = 
{
	{L"imageBase", LoaderTag_ImageBase},
	{L"connect", LoaderTag_Connect},
};

TestSuiteLoader::TestSuiteLoader() 
	: imageBase(NULL)
{	
}

TestSuiteLoader::~TestSuiteLoader()
{
	size_t index;

	index = connectList.size();
	while(index--)
	{
		AnsiString_Free(connectList[index]);
	}

	String_Free(imageBase);

}

BOOL TestSuiteLoader::Load(const wchar_t *path, TestSuite *testSuite)
{
	BOOL result;
	HANDLE fileHandle;
	obj_xml *reader;

	if (NULL == testSuite)
		return FALSE;

	if (NULL == path || L'\0' == *path) 
		return FALSE;

	fileHandle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == fileHandle)
		return FALSE;

	result = FALSE;
	if (NULL != WASABI_API_SVC)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
		reader = (NULL != sf) ? (obj_xml*)sf->getInterface() : NULL;
		if (NULL != reader)
		{
			if (OBJ_XML_SUCCESS == reader->xmlreader_open())
			{
				reader->xmlreader_registerCallback(L"testprovider\fimageBase", this);
				reader->xmlreader_registerCallback(L"testprovider\fconnect", this);
				ZeroMemory(hitList, sizeof(hitList));

				deviceParser.Begin(reader, testSuite);
				typeParser.Begin(reader, testSuite);
				connectionParser.Begin(reader, testSuite);
				commandParser.Begin(reader, testSuite);

				result = FeedFile(reader, fileHandle, 8192);

				deviceParser.End();
				typeParser.End();
				connectionParser.End();
				commandParser.End();

				reader->xmlreader_close();
		
				testSuite->SetIconBase(imageBase);
				testSuite->SetConnectList(connectList.begin(), connectList.size());

			}
			sf->releaseInterface(reader);
		}
	}

	CloseHandle(fileHandle);

	return result;
}

BOOL TestSuiteLoader::FeedFile(obj_xml *reader, HANDLE fileHandle, DWORD bufferSize)
{
	BOOL result;
	DWORD read;
	BYTE *buffer;
	int readerCode;

	if (NULL == reader || 
		INVALID_HANDLE_VALUE == fileHandle || 
		0 == bufferSize)
	{
		return FALSE;
	}

	buffer = (BYTE*)malloc(bufferSize);
	if (NULL == buffer) 
		return FALSE;
	
	
	readerCode = OBJ_XML_SUCCESS;
	result = TRUE;

	for(;;)
	{
		if (FALSE == ReadFile(fileHandle, buffer, bufferSize, &read, NULL) || 0 == read)
		{
			result = FALSE;

			if (0 == read && OBJ_XML_SUCCESS == readerCode)
				reader->xmlreader_feed(0, 0);

			break;
		}
		
		readerCode = reader->xmlreader_feed(buffer, read);
		if (OBJ_XML_SUCCESS != readerCode) 
		{
			result = FALSE;
			break;
		}
	}
	
	free(buffer);
	return result;
}

void TestSuiteLoader::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementString.Clear();
}

void TestSuiteLoader::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	for (size_t i = 0; i < LOADER_TAG_MAX; i++)
	{
		if (FALSE == hitList[i] &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, knownTags[i].name, -1, xmltag, -1))
		{
			knownTags[i].callback(this, elementString.Get());
			hitList[i] = TRUE;
			break;
		}
	}
}

void TestSuiteLoader::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void TestSuiteLoader::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS TestSuiteLoader
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS