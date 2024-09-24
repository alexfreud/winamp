#include "InfoDownloader.h"

#include "api.h"
#include "main.h"
#include "images.h"
#include "InfoDownloader.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include <api/service/waServiceFactory.h>
#include <strsafe.h>

void OnInfoDownloadDone(InfoDownloader *_info);

void DeviceInfo_Init(DeviceInfo *device_info)
{
	device_info->id=0;
	device_info->modelInfo=NULL;
	device_info->total_space=0;
	device_info->used_space=0;
	device_info->model[0]=0;
	device_info->manufacturer[0]=0;
	device_info->name[0]=0;
	device_info->product[0]=0;
}

void DeviceInfo_Copy(DeviceInfo *dest, const DeviceInfo *source)
{
	dest->id=source->id;
	dest->modelInfo=source->modelInfo;
	dest->total_space=source->total_space;
	dest->used_space=source->used_space;
	StringCbCopy(dest->model, sizeof(dest->model), source->model);
	StringCbCopy(dest->manufacturer, sizeof(dest->manufacturer), source->manufacturer);
	StringCbCopy(dest->name, sizeof(dest->name), source->name);
	StringCbCopy(dest->product, sizeof(dest->product), source->product);
}


DeviceXML::DeviceXML()
{
	DeviceInfo_Init(&device_info);
}

InfoXML::InfoXML(obj_xml *parser) : parser(parser)
{
	parser->xmlreader_registerCallback(L"info\fdevice", this);
	parser->xmlreader_registerCallback(L"info\fspace", this);
}

InfoXML::~InfoXML()
{
	parser->xmlreader_unregisterCallback(this);
}

void DeviceXML::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	if (!wcscmp(xmltag, L"device"))
	{
		const wchar_t *value = params->getItemValue(L"id");
		if (value)
			device_info.id = _wcstoui64(value, 0, 16);

		value = params->getItemValue(L"manufacturer");
		if (value)
			StringCbCopyW(device_info.manufacturer, sizeof(device_info.manufacturer), value);

		value = params->getItemValue(L"model");
		if (value)
			StringCbCopyW(device_info.model, sizeof(device_info.model), value);

		value = params->getItemValue(L"name");
		if (value)
			StringCbCopyW(device_info.name, sizeof(device_info.name), value);

		value = params->getItemValue(L"product");
		if (value)
			StringCbCopyW(device_info.product, sizeof(device_info.product), value);

		device_info.modelInfo = FindModelInfo(device_info.manufacturer, device_info.model, FALSE);
	}
	else if (!wcscmp(xmltag, L"space"))
	{
		const wchar_t *value = params->getItemValue(L"total");
		if (value)
		{
			device_info.total_space = _wtoi64(value);
		}

		value = params->getItemValue(L"used");
		if (value)
		{
			device_info.used_space = _wtoi64(value);
		}
	}
}

WifiDevice *InfoXML::CreateDevice(uint64_t device_id_check, const char *root_url)
{
	if (device_info.id == device_id_check)
	{
		return new WifiDevice(root_url, &device_info);
	}
	return 0;
}

/* ------------------------------------------------------------------------------------------------------------ */

InfoDownloader::InfoDownloader(const char *_root_url, uint64_t id, Wasabi2::nx_string_t usn) : id(id)
{
	this->usn = NXStringRetain(usn);
	root_url = strdup(_root_url);
	done=0;
	waServiceFactory *parserFactory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	parser->xmlreader_setCaseSensitive();
	info = new InfoXML(parser);
	parser->xmlreader_open();
}

InfoDownloader::~InfoDownloader()
{
	NXStringRelease(usn);
	waServiceFactory *parserFactory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		delete info;
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
	}
	free(root_url);
}

void InfoDownloader::OnInit(DownloadToken token)
{
	if (done)
	{
		WAC_API_DOWNLOADMANAGER->CancelDownload(token);
		return;
	}

	api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (jnet)
	{
		jnet->AddHeaderValue("X-Winamp-ID", winamp_id_str);
		jnet->AddHeaderValue("X-Winamp-Name", winamp_name);
	}
}

void InfoDownloader::OnData(DownloadToken token, void *data, size_t datalen)
{
	if (done)
	{
		WAC_API_DOWNLOADMANAGER->CancelDownload(token);
		return;
	}

	if (parser->xmlreader_feed(data, datalen) != OBJ_XML_SUCCESS)
	{
		WAC_API_DOWNLOADMANAGER->CancelDownload(token);
	}
}

void InfoDownloader::OnCancel(DownloadToken token)
{
	done=2;
	OnInfoDownloadDone(this);
}	

void InfoDownloader::OnError(DownloadToken token, int error)
{
	// TODO
	done=2;
		OnInfoDownloadDone(this);
}

void InfoDownloader::OnFinish(DownloadToken token)
{
	if (!done)
	{
		parser->xmlreader_feed(0, 0);
	}

	done=1;
		OnInfoDownloadDone(this);
	}

bool InfoDownloader::Done(WifiDevice **out_device)
{
	if (done != 1)
		return false;

	*out_device = info->CreateDevice(id, root_url);

	return true;
}

void InfoDownloader::Cancel()
{
	done=2;
}