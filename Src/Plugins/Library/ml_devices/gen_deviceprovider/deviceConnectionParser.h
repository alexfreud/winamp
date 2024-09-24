#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_CONNECTION_PARSER_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_CONNECTION_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"

class obj_xml;

#define CONNECTION_TAG_MAX	2

class DeviceConnectionParser : public ifc_xmlreadercallback
{
public:
	DeviceConnectionParser();
	~DeviceConnectionParser();
public:
	BOOL Begin(obj_xml *reader, ifc_xmlreaderparams *params);
	BOOL End(obj_xml *reader, ifc_deviceconnection **connection);

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	friend static void DeviceConnectionParser_DisplayNameCb(DeviceConnectionParser *self, ifc_deviceconnectioneditor *editor, const wchar_t *value);
	friend static void DeviceConnectionParser_IconCb(DeviceConnectionParser *self, ifc_deviceconnectioneditor *editor, const wchar_t *value);

protected:
	StringBuilder elementString;
	ifc_deviceconnectioneditor *editor;
	BOOL hitList[CONNECTION_TAG_MAX];
	SIZE iconSize;

protected:
	RECVS_DISPATCH;

};


#endif // _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_CONNECTION_PARSER_HEADER