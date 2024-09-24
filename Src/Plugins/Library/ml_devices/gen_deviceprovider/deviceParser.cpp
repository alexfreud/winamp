#include "main.h"
#include "./deviceParser.h"

#include "../../xml/obj_xml.h"

typedef void (*DEVICETAGCALLBACK)(DeviceParser* /*self*/, Device* /*device*/, const wchar_t* /*value*/);

typedef struct DEVICETAG
{
	const wchar_t *name;
	BOOL multiEntry;
	DEVICETAGCALLBACK callback;
} DEVICETAG;


static void  
DeviceParser_ConnectionCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	char *connectionAnsi;

	connectionAnsi = (NULL != value) ? 
					String_ToAnsi(CP_UTF8, 0, value, -1, NULL, NULL) : 
					NULL;

	device->SetConnection(connectionAnsi);
	AnsiString_Free(connectionAnsi);
}


static void  
DeviceParser_DisplayNameCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	device->SetDisplayName(value);
}

static void  
DeviceParser_IconCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	device->AddIcon(value, self->iconSize.cx, self->iconSize.cy);
}

static void  
DeviceParser_TotalSpaceCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	size_t size;
	
	if (NULL == value)
		size = -1;
	else
	{
		LONGLONG lval;
		if (FALSE == StrToInt64Ex(value, STIF_DEFAULT, &lval))
			return;

		size = (size_t)lval;
	}
	device->SetTotalSpace(size);
}

static void
DeviceParser_UsedSpaceCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	size_t size;
	
	if (NULL == value)
		size = -1;
	else
	{
		LONGLONG lval;
		if (FALSE == StrToInt64Ex(value, STIF_DEFAULT, &lval))
			return;

		size = (size_t)lval;
	}
	device->SetUsedSpace(size);
}

static void 
DeviceParser_HiddenCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	int hidden;
	if (FALSE != StrToIntEx(value, STIF_DEFAULT, &hidden))
		device->SetHidden(0 != hidden);
}

static void 
DeviceParser_CommandCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	char *command;

	if (FALSE != IS_STRING_EMPTY(value))
		return;
	command = String_ToAnsi(CP_UTF8, 0, value, -1, NULL, NULL);
	if (NULL != command)
	{
		device->AddCommand(command, self->commandFlags);
		AnsiString_Free(command);
	}
}


static void  
DeviceParser_ModelCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	device->SetModel(value);
}

static void  
DeviceParser_StatusCb(DeviceParser *self, Device *device, const wchar_t *value)
{
	device->SetStatus(value);
}

static const DEVICETAG knownTags[DEVICE_TAG_MAX] = 
{
	{L"connection", FALSE, DeviceParser_ConnectionCb},
	{L"displayName", FALSE, DeviceParser_DisplayNameCb},
	{L"icon", TRUE, DeviceParser_IconCb},
	{L"totalSpace", FALSE, DeviceParser_TotalSpaceCb},
	{L"usedSpace", FALSE, DeviceParser_UsedSpaceCb},
	{L"hidden", FALSE, DeviceParser_HiddenCb},
	{L"command", TRUE, DeviceParser_CommandCb},
	{L"model", FALSE, DeviceParser_ModelCb},
	{L"status", FALSE, DeviceParser_StatusCb},
	
};

DeviceParser::DeviceParser()
	: device(NULL)
{
}

DeviceParser::~DeviceParser()
{
	if (NULL != device)
		device->Release();
}

BOOL DeviceParser::Begin(obj_xml *reader, ifc_xmlreaderparams *params)
{
	const wchar_t *value;
	char *nameAnsi, *typeAnsi;
	if (NULL != device)
		return FALSE;

	if (NULL == reader || NULL == params) 
		return FALSE;
		
	value = params->getItemValue(L"name");
	nameAnsi = (NULL != value) ? String_ToAnsi(CP_UTF8, 0, value, -1, NULL, NULL) : NULL;

	value = params->getItemValue(L"type");
	typeAnsi = (NULL != value) ? String_ToAnsi(CP_UTF8, 0, value, -1, NULL, NULL) : NULL;
	
	if (NULL == nameAnsi ||
		NULL == typeAnsi || 
		FAILED(Device::CreateInstance(nameAnsi, typeAnsi, NULL, &device)))
	{
		device = NULL;
	}

	AnsiString_Free(nameAnsi);
	AnsiString_Free(typeAnsi);

	if (NULL == device)
		return FALSE;

	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\fconnection", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\fdisplayName", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\ficon", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\ftotalSpace", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\fusedSpace", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\fhidden", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\fcommand", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\fmodel", this);
	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice\fstatus", this);
	ZeroMemory(hitList, sizeof(hitList));
	return TRUE;
}

BOOL DeviceParser::End(obj_xml *reader, Device **result)
{
	if (NULL != reader)
		reader->xmlreader_unregisterCallback(this);

	if (NULL == device)
		return FALSE;

	if (NULL != result)
	{
		*result = device;
		device->AddRef();
	}

	device->Release();
	device = NULL;
	
	return TRUE;
}

void DeviceParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
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
	else if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, L"command", -1, xmltag, -1))
	{
		const wchar_t *sVal;
		int iVal;

		commandFlags = DeviceCommandFlag_None;

		sVal = params->getItemValue(L"primary");
		if (FALSE == IS_STRING_EMPTY(sVal) &&
			FALSE != StrToIntEx(sVal, STIF_DEFAULT, &iVal) &&
			0 != iVal)
		{
			commandFlags |= DeviceCommandFlag_Primary;
		}

		sVal = params->getItemValue(L"group");
		if (FALSE == IS_STRING_EMPTY(sVal) &&
			FALSE != StrToIntEx(sVal, STIF_DEFAULT, &iVal) &&
			0 != iVal)
		{
			commandFlags |= DeviceCommandFlag_Group;
		}
		
		sVal = params->getItemValue(L"disabled");
		if (FALSE == IS_STRING_EMPTY(sVal) &&
			FALSE != StrToIntEx(sVal, STIF_DEFAULT, &iVal) &&
			0 != iVal)
		{
			commandFlags |= DeviceCommandFlag_Disabled;
		}

		sVal = params->getItemValue(L"hidden");
		if (FALSE == IS_STRING_EMPTY(sVal) &&
			FALSE != StrToIntEx(sVal, STIF_DEFAULT, &iVal) &&
			0 != iVal)
		{
			commandFlags |= DeviceCommandFlag_Hidden;
		}
	}

}

void DeviceParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (NULL == device)
		return;

	for (size_t i = 0; i < DEVICE_TAG_MAX; i++)
	{
		if (FALSE == hitList[i] &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, knownTags[i].name, -1, xmltag, -1))
		{
			knownTags[i].callback(this, device, elementString.Get());

			if (FALSE == knownTags[i].multiEntry)
				hitList[i] = TRUE;

			break;
		}
	}
}

void DeviceParser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void DeviceParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS DeviceParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS