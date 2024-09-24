#include "main.h"
#include "./deviceTypeParser.h"

#include "../../xml/obj_xml.h"

typedef void (*TYPETAGCALLBACK)(DeviceTypeParser* /*self*/, ifc_devicetypeeditor* /*editor*/, const wchar_t* /*value*/);

typedef struct TYPETAG
{
	const wchar_t *name;
	BOOL multiEntry;
	TYPETAGCALLBACK callback;
} TYPETAG;

static void 
DeviceTypeParser_DisplayNameCb(DeviceTypeParser *self, ifc_devicetypeeditor *editor, const wchar_t *value)
{
	editor->SetDisplayName(value);
}

static void
DeviceTypeParser_IconCb(DeviceTypeParser *self, ifc_devicetypeeditor *editor, const wchar_t *value)
{
	ifc_deviceiconstore *iconStore;
	if (SUCCEEDED(editor->GetIconStore(&iconStore)))
	{
		iconStore->Add(value, self->iconSize.cx, self->iconSize.cy, TRUE);
		iconStore->Release();
	}
}

static const TYPETAG knownTags[TYPE_TAG_MAX] = 
{
	{L"displayName", FALSE, DeviceTypeParser_DisplayNameCb},
	{L"icon", TRUE, DeviceTypeParser_IconCb},
};

DeviceTypeParser::DeviceTypeParser()
	: editor(NULL)
{
}

DeviceTypeParser::~DeviceTypeParser()
{
	if (NULL != editor)
		editor->Release();
}

BOOL DeviceTypeParser::Begin(obj_xml *reader, ifc_xmlreaderparams *params)
{
	const wchar_t *name;
	ifc_devicetype *type;
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
		SUCCEEDED(WASABI_API_DEVICES->CreateType(nameAnsi, &type)))
	{
		if(FAILED(type->QueryInterface(IFC_DeviceTypeEditor, (void**)&editor)))
			editor = NULL;

		type->Release();
	}
	
	AnsiString_Free(nameAnsi);

	if (NULL == editor)
		return FALSE;

	reader->xmlreader_registerCallback(L"testprovider\ftypes\ftype\fdisplayName", this);
	reader->xmlreader_registerCallback(L"testprovider\ftypes\ftype\ficon", this);
	ZeroMemory(hitList, sizeof(hitList));
	return TRUE;
}

BOOL DeviceTypeParser::End(obj_xml *reader, ifc_devicetype **deviceType)
{
	BOOL result;

	if (NULL != reader)
		reader->xmlreader_unregisterCallback(this);

	if (NULL == editor)
		return FALSE;

	if (NULL != deviceType)
	{
		if (FAILED(editor->QueryInterface(IFC_DeviceType, (void**)deviceType)))
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

void DeviceTypeParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
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

void DeviceTypeParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (NULL == editor)
		return;

	for (size_t i = 0; i < TYPE_TAG_MAX; i++)
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

void DeviceTypeParser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void DeviceTypeParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS DeviceTypeParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS