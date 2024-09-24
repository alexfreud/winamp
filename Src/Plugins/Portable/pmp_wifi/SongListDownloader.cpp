#include "api.h"
#include "main.h"
#include "images.h"
#include "SongListDownloader.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include <api/service/waServiceFactory.h>
#include <strsafe.h>

WifiXML::WifiXML(obj_xml *parser) : parser(parser)
{
	wifi_track = 0;
	wifi_playlist = 0;

	parser->xmlreader_registerCallback(L"items", this);
	parser->xmlreader_registerCallback(L"items\fdevice", &info_xml);
	parser->xmlreader_registerCallback(L"items\fspace", &info_xml);
	parser->xmlreader_registerCallback(L"items\fitem", this);
	parser->xmlreader_registerCallback(L"items\fplaylist", this);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem", this);

	parser->xmlreader_registerCallback(L"items\fitem\fartist", &artist);
	parser->xmlreader_registerCallback(L"items\fitem\falbum", &album);
	parser->xmlreader_registerCallback(L"items\fitem\fcomposer", &composer);
	parser->xmlreader_registerCallback(L"items\fitem\fduration", &duration);
	parser->xmlreader_registerCallback(L"items\fitem\ftrack", &track);
	parser->xmlreader_registerCallback(L"items\fitem\fyear", &year);
	parser->xmlreader_registerCallback(L"items\fitem\fsize", &size);
	parser->xmlreader_registerCallback(L"items\fitem\ftitle", &title);
	parser->xmlreader_registerCallback(L"items\fitem\fmime", &mime_type);

	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\fartist", &artist);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\falbum", &album);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\fcomposer", &composer);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\fduration", &duration);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\ftrack", &track);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\fyear", &year);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\fsize", &size);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\ftitle", &title);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\fmime", &mime_type);
	parser->xmlreader_registerCallback(L"items\fplaylist\fitem\fmodified", &modified);
}

WifiXML::~WifiXML()
{
	parser->xmlreader_unregisterCallback(this);
	parser->xmlreader_unregisterCallback(&info_xml);

	parser->xmlreader_unregisterCallback(&artist);
	parser->xmlreader_unregisterCallback(&album);
	parser->xmlreader_unregisterCallback(&composer);
	parser->xmlreader_unregisterCallback(&duration);
	parser->xmlreader_unregisterCallback(&track);
	parser->xmlreader_unregisterCallback(&year);
	parser->xmlreader_unregisterCallback(&size);
	parser->xmlreader_unregisterCallback(&title);
	parser->xmlreader_unregisterCallback(&mime_type);
	parser->xmlreader_unregisterCallback(&modified);
}

void WifiXML::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	if (!wcscmp(xmltag, L"item"))
	{
		const wchar_t *value = params->getItemValue(L"id");
		if (value)
		{
			wifi_track = new WifiTrack;
			wifi_track->id = _wcsdup(value);
		}
	}
	else if (!wcscmp(xmltag, L"playlist"))
	{
		const wchar_t *value = params->getItemValue(L"id");
		const wchar_t *name = params->getItemValue(L"name");
		if (value && name)
		{
			wifi_playlist = new WifiPlaylist;
			wifi_playlist->id = _wcsdup(value);
			wifi_playlist->name = _wcsdup(name);
		}
	}
}

void WifiXML::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (!wcscmp(xmltag, L"item") && wifi_track)
	{
		wifi_track->artist = _wcsdup(artist.GetString()); artist.Reset();
		wifi_track->album = _wcsdup(album.GetString()); album.Reset();
		wifi_track->composer = _wcsdup(composer.GetString()); composer.Reset();
		wifi_track->duration = _wtoi(duration.GetString()); duration.Reset();
		wifi_track->track = _wtoi(track.GetString()); track.Reset();
		wifi_track->year = _wtoi(year.GetString()); year.Reset();
		wifi_track->size = _wtoi(size.GetString()); size.Reset();
		wifi_track->title = _wcsdup(title.GetString()); title.Reset();
		wifi_track->mime_type = _wcsdup(mime_type.GetString()); mime_type.Reset();
		wifi_track->last_updated = _wtoi64(modified.GetString()); modified.Reset();
		if (wifi_playlist)
			wifi_playlist->tracks.push_back(wifi_track);
		else
			tracks.push_back(wifi_track);
		wifi_track=0;
	}
	else if (!wcscmp(xmltag, L"playlist") && wifi_playlist)
	{
		playlists.push_back(wifi_playlist);
		wifi_playlist = 0;
	}
}

/* ------------------------------------------------------------------------------------------------------------ */

SongListDownloader::SongListDownloader(const char *root_url, WifiDevice *wifi_device) : root_url(root_url), wifi_device(wifi_device)
{
	device=0;
	wifi_device->AddRef();
	waServiceFactory *parserFactory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	parser->xmlreader_setCaseSensitive();
	wifi = new WifiXML(parser);
	parser->xmlreader_open();
}

SongListDownloader::~SongListDownloader()
{
	waServiceFactory *parserFactory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		delete wifi;
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
	}
}

void SongListDownloader::OnInit(DownloadToken token)
{
	api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (jnet)
	{
		jnet->AddHeaderValue("X-Winamp-ID", winamp_id_str);
		jnet->AddHeaderValue("X-Winamp-Name", winamp_name);
	}
}

void SongListDownloader::OnData(DownloadToken token, void *data, size_t datalen)
{
	if (parser->xmlreader_feed(data, datalen) != OBJ_XML_SUCCESS)
	{
		WAC_API_DOWNLOADMANAGER->CancelDownload(token);
	}
}

void SongListDownloader::OnCancel(DownloadToken token)
{
	wifi_device->OnConnectionFailed();
	wifi_device->Release();
	this->Release();
}	

void SongListDownloader::OnError(DownloadToken token, int error)
{
	wifi_device->OnConnectionFailed();
	wifi_device->Release();
	this->Release();
}

void SongListDownloader::OnFinish(DownloadToken token)
{
	parser->xmlreader_feed(0, 0);
	device = new TemplateDevice(wifi_device, root_url, &wifi->info_xml.device_info, &wifi->tracks, &wifi->playlists);
	wifi_device->OnConnected(device);
	wifi_device->Release();
	PostMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICECONNECTED);

	this->Release();
}
