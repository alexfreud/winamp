#include "main.h"
#include "./deviceConnectionParser.h"

#include "../../xml/obj_xml.h"

typedef void (*CONNECTIONTAGCALLBACK)(DeviceConnectionParser* /*self*/, ifc_deviceconnectioneditor * /*editor*/, const wchar_t* /*value*/);

typedef struct CONNECTIONTAG
{
	const wchar_t *name;
	BOOL multiEntry;
	CONNECTIONTAGCALLBACK callback;
} CONNECTIONTAG;

static void 
DeviceConnectionParser_DisplayNameCb(DeviceConnectionParser *self, ifc_deviceconnectioneditor *editor, const wchar_t *value)
{
	editor->SetDisplayName(value);
}

static void
DeviceConnectionParser_IconCb(DeviceConnectionParser *self, ifc_deviceconnectioneditor *editor, const wchar_t *value)
{
	ifc_deviceiconstore *iconStore;
	if (SUCCEEDED(editor->GetIconStore(&iconStore)))
	{
		iconStore->Add(value, self->iconSize.cx, self->iconSize.cy, TRUE);
		iconStore->Release();
	}
}

static const CONNECTIONTAG knownTags[CONNECTION_TAG_MAX] = 
{
	{L"displayName", FALSE, DeviceConnectionParser_DisplayNameCb},
	{L"icon", TRUE, DeviceConnectionParser_IconCb},
};

DeviceConnectionParser::DeviceConnectionParser()
	: editor(NULL)
{
}

DeviceConnectionParser::~DeviceConnectionParser()
{
	if (NULL != editor)
		editor->Release();
}

BOOL DeviceConnectionParser::Begin(obj_xml *reader, ifc_xmlreaderparams *params)
{
	const wchar_t *name;
	ifc_deviceconnection *connection;
	char *nameAnsi;

	if (NULL != editor)
		return FALSE;

	if (NULL == reader || NULL == params) 
		return FALSE;
		
	
	name = params->getItemValue(L"name");
	if (NULL == name)
		return FALSE;

	nameAnsi = String_ToAnsi(CP_UTF8, 0, name, -1, NULL, NULL);
	if (NULL == nameAnsi)
		return FALSE;

	if (NULL != WASABI_API_DEVICES && 
		SUCCEEDED(WASABI_API_DEVICES->CreateConnection(nameAnsi, &connection)))
	{
		if(FAILED(connection->QueryInterface(IFC_DeviceConnectionEditor, (void**)&editor)))
			editor = NULL;

		connection->Release();
	}
	AnsiString_Free(nameAnsi);

	if (NULL == editor)
		return FALSE;

	reader->xmlreader_registerCallback(L"testprovider\fconnections\fconnection\fdisplayName", this);
	reader->xmlreader_registerCallback(L"testprovider\fconnections\fconnection\ficon", this);
	ZeroMemory(hitList, sizeof(hitList));
	return TRUE;
}

BOOL DeviceConnectionParser::End(obj_xml *reader, ifc_deviceconnection **connection)
{
	BOOL result;

	if (NULL != reader)
		reader->xmlreader_unregisterCallback(this);

	if (NULL == editor)
		return FALSE;

	if (NULL != connection)
	{
		if (FAILED(editor->QueryInterface(IFC_DeviceConnection, (void**)connection)))
			result = FALSE;
		else 
			result = TRUE;
	}
	else
		result = TRUE;

	editor->Release();
	editor = NULL;
	
	return result;
}

void DeviceConnectionParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementString.Clear();

	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, L"icon", -1, xmltag, -1))
	{
		const wchar_t *sVal;
		int iVal;

		sVal = params->getItemValue(L"width");
		if (NULL == sVal ||
			FALSE == StrToIntEx(sVal, STIF_DEFAULT, &iVal))
		{
			iVal = 0;
		}
		
		iconSize.cx = iVal;
		
		sVal = params->getItemValue(L"height");
		if (NULL == sVal ||
			FALSE == StrToIntEx(sVal, STIF_DEFAULT, &iVal))
		{
			iVal = 0;
		}

		iconSize.cy = iVal;
	}
}

void DeviceConnectionParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (NULL == editor)
		return;

	for (size_t i = 0; i < CONNECTION_TAG_MAX; i++)
	{
		if (FALSE == hitList[i] &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, knownTags[i].name, -1, xmltag, -1))
		{
			knownTags[i].callback(this, editor, elementString.Get());

			if (FALSE == knownTags[i].multiEntry)
				hitList[i] = TRUE;

			break;
		}
	}
}

void DeviceConnectionParser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void DeviceConnectionParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS DeviceConnectionParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS