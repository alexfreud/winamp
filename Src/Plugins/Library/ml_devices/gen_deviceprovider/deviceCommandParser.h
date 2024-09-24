#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_COMMAND_PARSER_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_COMMAND_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"

class obj_xml;

#define COMMAND_TAG_MAX	3

class DeviceCommandParser : public ifc_xmlreadercallback
{
public:
	DeviceCommandParser();
	~DeviceCommandParser();
public:
	BOOL Begin(obj_xml *reader, ifc_xmlreaderparams *params);
	BOOL End(obj_xml *reader, ifc_devicecommand **command);

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	friend static void DeviceCommandParser_DisplayNameCb(DeviceCommandParser *self, ifc_devicecommandeditor *editor, const wchar_t *value);
	friend static void DeviceCommandParser_IconCb(DeviceCommandParser *self, ifc_devicecommandeditor *editor, const wchar_t *value);
	friend static void DeviceCommandParser_DescirptionCb(DeviceCommandParser *self, ifc_devicecommandeditor *editor, const wchar_t *value);

protected:
	StringBuilder elementString;
	ifc_devicecommandeditor *editor;
	BOOL hitList[COMMAND_TAG_MAX];
	SIZE iconSize;

protected:
	RECVS_DISPATCH;

};


#endif // _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_COMMAND_PARSER_HEADER