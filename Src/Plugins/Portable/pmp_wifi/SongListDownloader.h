#pragma once
#include "device.h"
#include "../xml/obj_xml.h"
#include "XMLString.h"
#include "../Components/wac_downloadManager/DownloadCallbackT.h"
#include "WifiDevice.h"
#include "../xml/ifc_xmlreadercallbackT.h"
#include "main.h"
#include "InfoDownloader.h" // for InfoXML

class WifiXML : public ifc_xmlreadercallbackT<WifiXML>
{
public:
	WifiXML(obj_xml *parser);
	~WifiXML();
	
public:	
	void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);

	DeviceXML info_xml;
	TemplateDevice::PlaylistsList playlists;
	TemplateDevice::TrackList tracks;
private:
	obj_xml *parser;
	TemplateDevice *device;
	XMLString artist;
	XMLString album;
	XMLString composer;
	XMLString duration;
	XMLString track;
	XMLString year;
	XMLString size;
	XMLString title;
	XMLString mime_type;
	XMLString modified;
	WifiTrack *wifi_track;
	WifiPlaylist *wifi_playlist;
};

class SongListDownloader : public DownloadCallbackT<SongListDownloader>
{
public:
	SongListDownloader(const char *root_url, WifiDevice *wifi_device);
	~SongListDownloader();
	void OnInit(DownloadToken token);
	void OnData(DownloadToken token, void *data, size_t datalen);
	void OnCancel(DownloadToken token);
	void OnError(DownloadToken token, int error);
	void OnFinish(DownloadToken token);	

private:
	obj_xml *parser;
	WifiXML *wifi;
	TemplateDevice *device;
	WifiDevice *wifi_device;
	const char *root_url;
};
