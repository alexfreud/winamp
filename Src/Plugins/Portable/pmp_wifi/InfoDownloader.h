#pragma once
#include "device.h"
#include "../xml/obj_xml.h"
#include "../Components/wac_downloadManager/DownloadCallbackT.h"
#include "WifiDevice.h"
#include "../xml/ifc_xmlreadercallbackT.h"
#include "main.h"
namespace Wasabi2
{
#include "nx/nxstring.h"
}

class DeviceXML : public ifc_xmlreadercallbackT<DeviceXML>
{
public:	
	DeviceXML();
	void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	DeviceInfo device_info;
};

class InfoXML : public DeviceXML
{
public:
	InfoXML(obj_xml *parser);
	~InfoXML();
	WifiDevice *CreateDevice(uint64_t device_id_check, const char *root_url);
private:
	obj_xml *parser;
};

class InfoDownloader : public DownloadCallbackT<InfoDownloader>
{
public:
	InfoDownloader(const char *root_url, uint64_t id, Wasabi2::nx_string_t usn);
	~InfoDownloader();
	void OnInit(DownloadToken token);
	void OnData(DownloadToken token, void *data, size_t datalen);
	void OnCancel(DownloadToken token);
	void OnError(DownloadToken token, int error);
	void OnFinish(DownloadToken token);	
	bool Done(WifiDevice **device);

	void Cancel();
	uint64_t id;
	Wasabi2::nx_string_t usn;
private:
	char *root_url;
	obj_xml *parser;
	InfoXML *info;
	volatile int done; // 1 for successfully done, 2 for cancelled/error
};
