#include "main.h"
#include "./deviceCommandParser.h"

#include "../../xml/obj_xml.h"

typedef void (*COMMANDTAGCALLBACK)(DeviceCommandParser* /*self*/, ifc_devicecommandeditor* /*editor*/, const wchar_t* /*value*/);

typedef struct COMMANDTAG
{
	const wchar_t *name;
	BOOL multiEntry;
	COMMANDTAGCALLBACK callback;
} COMMANDTAG;

static void 
DeviceCommandParser_DisplayNameCb(DeviceCommandParser *self, ifc_devicecommandeditor *editor, const wchar_t *value)
{
	editor->SetDisplayName(value);
}

static void
DeviceCommandParser_IconCb(DeviceCommandParser *self, ifc_devicecommandeditor *editor, const wchar_t *value)
{
	ifc_deviceiconstore *iconStore;
	if (SUCCEEDED(editor->GetIconStore(&iconStore)))
	{
		iconStore->Add(value, self->iconSize.cx, self->iconSize.cy, TRUE);
		iconStore->Release();
	}
}

static void
DeviceCommandParser_DescirptionCb(DeviceCommandParser *self, ifc_devicecommandeditor *editor, const wchar_t *value)
{
	editor->SetDescription(value);
}

static const COMMANDTAG knownTags[COMMAND_TAG_MAX] = 
{
	{L"displayName", FALSE, DeviceCommandParser_DisplayNameCb},
	{L"icon", TRUE, DeviceCommandParser_IconCb},
	{L"description", FALSE, DeviceCommandParser_DescirptionCb},
};

DeviceCommandParser::DeviceCommandParser()
	: editor(NULL)
{
}

DeviceCommandParser::~DeviceCommandParser()
{
	if (NULL != editor)
		editor->Release();
}

BOOL DeviceCommandParser::Begin(obj_xml *reader, ifc_xmlreaderparams *params)
{
	const wchar_t *name;
	ifc_devicecommand *command;
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
		SUCCEEDED(WASABI_API_DEVICES->CreateCommand(nameAnsi, &command)))
	{
		if(FAILED(command->QueryInterface(IFC_DeviceCommandEditor, (void**)&editor)))
			editor = NULL;

		command->Release();
	}

	AnsiString_Free(nameAnsi);

	if (NULL == editor)
		return FALSE;

	reader->xmlreader_registerCallback(L"testprovider\fcommands\fcommand\fdisplayName", this);
	reader->xmlreader_registerCallback(L"testprovider\fcommands\fcommand\fdescription", this);
	reader->xmlreader_registerCallback(L"testprovider\fcommands\fcommand\ficon", this);
	ZeroMemory(hitList, sizeof(hitList));
	return TRUE;
}

BOOL DeviceCommandParser::End(obj_xml *reader, ifc_devicecommand **command)
{
	BOOL result;

	if (NULL != reader)
		reader->xmlreader_unregisterCallback(this);

	if (NULL == command)
		return FALSE;

	if (NULL == editor)
		return FALSE;

	if (NULL != command)
	{
		if (FAILED(editor->QueryInterface(IFC_DeviceCommand, (void**)command)))
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

void DeviceCommandParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
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

void DeviceCommandParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (NULL == editor)
		return;

	for (size_t i = 0; i < COMMAND_TAG_MAX; i++)
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

void DeviceCommandParser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void DeviceCommandParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS DeviceCommandParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS