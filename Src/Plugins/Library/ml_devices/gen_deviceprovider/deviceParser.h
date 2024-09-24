#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_PARSER_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include "./device.h"

class obj_xml;

#define DEVICE_TAG_MAX	9

class DeviceParser : public ifc_xmlreadercallback
{
public:
	DeviceParser();
	~DeviceParser();
public:
	BOOL Begin(obj_xml *reader, ifc_xmlreaderparams *params);
	BOOL End(obj_xml *reader, Device **result);

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);
protected:
	friend static void DeviceParser_IconCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_ConnectionCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_DisplayNameCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_TotalSpaceCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_UsedSpaceCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_HiddenCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_CommandCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_ModelCb(DeviceParser *self, Device *device, const wchar_t *value);
	friend static void DeviceParser_StatusCb(DeviceParser *self, Device *device, const wchar_t *value);

protected:
	StringBuilder elementString;
	Device *device;
	BOOL hitList[DEVICE_TAG_MAX];
	SIZE iconSize;
	DeviceCommandFlags commandFlags;

protected:
	RECVS_DISPATCH;

};


#endif // _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_PARSER_HEADER