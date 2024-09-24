#include "main.h"
#include "api.h"
#include "resource.h"
#include "../xml/obj_xml.h"
#include "nu/AutoChar.h"
#include "../nu/AutoUrl.h"
#include "../nu/AutoHeader.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include "../agave/albumart/svc_albumartprovider.h"
#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include <strsafe.h>


static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};


#define USER_AGENT_SIZE (10 /*User-Agent*/ + 2 /*: */ + 6 /*Winamp*/ + 1 /*/*/ + 1 /*5*/ + 3/*.21*/ + 1 /*Null*/)
static void SetUserAgent(api_httpreceiver *http)
{

	char user_agent[USER_AGENT_SIZE] = {0};
	int bigVer = ((winampVersion & 0x0000FF00) >> 12);
	int smallVer = ((winampVersion & 0x000000FF));
	StringCchPrintfA(user_agent, USER_AGENT_SIZE, "User-Agent: Winamp/%01x.%02x", bigVer, smallVer);
	http->addheader(user_agent);
}


#define HTTP_BUFFER_SIZE 8192
#define POST_BUFFER_SIZE (128*1024)


int PostFile(const char *base_url, const wchar_t *filename, const itemRecordW *track, obj_xml *parser, int *killswitch,
						 void (*callback)(void *callbackContext, wchar_t *status), void *context, char *new_item_id, size_t new_item_id_len)
{
	//if (!parser)
//		return 1;
	bool first=true;
	char url[2048] = {0};
	char *p_url=url;
	size_t url_cch=sizeof(url)/sizeof(*url);
	FILE *f = _wfopen(filename, L"rb");
	if (!f)
		return 1;
	api_httpreceiver *http = 0;
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(httpreceiverGUID);
	if (sf)
		http = (api_httpreceiver *)sf->getInterface();

	if (!http)
		return 1;

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

	fseek(f, 0, SEEK_END);
	
	size_t clen = ftell(f);
	size_t transferred=0;
	size_t total_clen = clen;
	fseek(f, 0, SEEK_SET);

	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
	http->set_sendbufsize(POST_BUFFER_SIZE);
 	SetUserAgent(http);

	char data[POST_BUFFER_SIZE] = {0};

	StringCbCopyExA(p_url, url_cch, base_url, &p_url, &url_cch, 0);
		
	StringCbPrintfA(data, sizeof(data), "Content-Length: %u", clen);
	http->addheader(data);
//	http->addheader("Content-Type: application/octet-stream");

	/* send metadata */
	if (track->artist && track->artist[0])
	{
		if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?artist=%s", AutoUrl(track->artist));
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&artist=%s", AutoUrl(track->artist));
		first=false;
	}

	if (track->title && track->title[0])
	{
		if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?title=%s", AutoUrl(track->title));
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&title=%s", AutoUrl(track->title));
		first=false;
	}

	if (track->album && track->album[0])
	{
		if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?album=%s", AutoUrl(track->album));
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&album=%s", AutoUrl(track->album));
		first=false;
	}

	if (track->composer && track->composer[0])
	{
		if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?composer=%s", AutoUrl(track->composer));
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&composer=%s", AutoUrl(track->composer));
		first=false;
	}

	if (track->albumartist && track->albumartist[0])
	{
		if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?albumartist=%s", AutoUrl(track->albumartist));
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&albumartist=%s", AutoUrl(track->albumartist));
		first=false;
	}

	if (track->genre && track->genre[0])
	{
		if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?genre=%s", AutoUrl(track->genre));
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&genre=%s", AutoUrl(track->genre));
		first=false;
	}

	if (track->track > 0)
	{
		if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?track=%d", track->track);
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&track=%d", track->track);
		first=false;
	}

	const wchar_t *ext = PathFindExtension(filename);
	if (ext && ext[0])
	{
		if (ext[0] == '.') ext++;
		if (ext[0])
		{
					if (first)
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "?extension=%s", AutoUrl(ext));
		else
			StringCbPrintfExA(p_url, url_cch, &p_url, &url_cch, 0, "&extension=%s", AutoUrl(ext));
		first=false;
		}
	}


	wchar_t mime_type[128] = {0};
	if (AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"mime", mime_type, 128) == 1 && mime_type[0])
	{
		http->AddHeaderValue("Content-Type", AutoHeader(mime_type));
	}

	http->AddHeaderValue("X-Winamp-ID", winamp_id_str);
	http->AddHeaderValue("X-Winamp-Name", winamp_name);

	 http->AddHeaderValue("Expect", "100-continue");
	/* connect */
	callback(context, WASABI_API_LNGSTRINGW(IDS_CONNECTING));
	http->connect(url, 0, "POST");

	// spin and wait for a 100 response
	for (;;)
	{
		Sleep(55);
		if (*killswitch)
			goto connection_failed;
		int ret = http->run();
		if (ret != HTTPRECEIVER_RUN_OK) // connection failed or closed
			goto connection_failed;

		int reply_code = http->getreplycode();
		if (reply_code == 100)
			break;
		else if (reply_code)
			goto connection_failed;
	}

	
	/* POST the data */
	api_connection *connection = http->GetConnection();
	if (connection)
	{
		if (http->run() == -1)
			goto connection_failed;

		while (clen)
		{
			int percent = MulDiv(100, (int)transferred, (int)total_clen);
			wchar_t msg[128] = {0};
			StringCbPrintfW(msg, sizeof(msg), WASABI_API_LNGSTRINGW(IDS_UPLOADING), percent);
			callback(context, msg);
			if (*killswitch)
				goto connection_failed;
			if (http->run() == -1)
				goto connection_failed;

			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
				goto connection_failed;

			size_t lengthToSend = min(clen, connection->GetSendBytesAvailable());
			lengthToSend = min(lengthToSend, sizeof(data));

			if (lengthToSend)
			{
				int bytes_read = (int)fread(data, 1, lengthToSend, f);
				connection->send(data, bytes_read);
				clen-=bytes_read;
				transferred+=bytes_read;
			}
			else
			{
				Sleep(10);
			}
		}
		int x;
		while (x = (int)connection->GetSendBytesInQueue())
		{
			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
			goto connection_failed;
			Sleep(10);
			if (*killswitch)
				goto connection_failed;
			if (http->run() == -1)
				goto connection_failed;

		}
	}
	fclose(f);
	f=0;

	/* retrieve reply */
	int ret;
	do
	{
		Sleep(55);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = http->get_status();
		switch (status)
		{
		case HTTPRECEIVER_STATUS_CONNECTING:
		case HTTPRECEIVER_STATUS_READING_HEADERS:
			break;

		case HTTPRECEIVER_STATUS_READING_CONTENT:
			{
				const char *location = http->getheader("Location");
				if (location)
					StringCchCopyA(new_item_id, new_item_id_len, location);
				else
					new_item_id[0]=0;

				sf->releaseInterface(http);
				return 0;
			}
			break;
		case HTTPRECEIVER_STATUS_ERROR:
		default:
			sf->releaseInterface(http);
			return 1;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);


connection_failed:
	if (f)
		fclose(f);
	sf->releaseInterface(http);
	return 1;
}



int PostAlbumArt(const char *url, const itemRecordW *track, obj_xml *parser, int *killswitch, void (*callback)(void *callbackContext, wchar_t *status), void *context)
{

	void *artData=0;
	size_t datalen=0;
	wchar_t *mimeType=0;
	if (AGAVE_API_ALBUMART->GetAlbumArtData(track->filename, L"cover", &artData, &datalen, &mimeType) != ALBUMART_SUCCESS)
		return 1;

	api_httpreceiver *http = 0;
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(httpreceiverGUID);
	if (sf)
		http = (api_httpreceiver *)sf->getInterface();

	if (!http)
		return 1;

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

	uint8_t *artDataPtr=(uint8_t *)artData;
	size_t clen = datalen;
	size_t transferred=0;
	size_t total_clen = datalen;

	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
	http->set_sendbufsize(POST_BUFFER_SIZE);
 	SetUserAgent(http);

	char data[POST_BUFFER_SIZE] = {0};
		
	StringCbPrintfA(data, sizeof(data), "Content-Length: %u", datalen);
	http->addheader(data);
	if (mimeType)
	{
		StringCbPrintfA(data, sizeof(data), "Content-Type: %s", AutoHeader(mimeType));
		http->addheader(data);
	}

	http->AddHeaderValue("X-Winamp-ID", winamp_id_str);
	http->AddHeaderValue("X-Winamp-Name", winamp_name);

	/* connect */
	http->AddHeaderValue("Expect", "100-continue");
	callback(context, WASABI_API_LNGSTRINGW(IDS_CONNECTING));
	http->connect(url, 0, "POST");
	
	// spin and wait for a 100 response
	for (;;)
	{
		Sleep(55);
		int ret = http->run();
		if (ret != HTTPRECEIVER_RUN_OK) // connection failed or closed
			goto connection_failed;

		if (*killswitch)
			goto connection_failed;
		int reply_code = http->getreplycode();
		if (reply_code == 100)
			break;
		else if (reply_code)
			goto connection_failed;
	}

	/* POST the data */
	api_connection *connection = http->GetConnection();
	if (connection)
	{
		if (http->run() == -1)
			goto connection_failed;

		while (clen)
		{
			int percent = MulDiv(100, (int)transferred, (int)total_clen);
			wchar_t msg[128] = {0};
			StringCbPrintfW(msg, sizeof(msg), L"Uploading Album Art (%d%%)", percent);
			callback(context, msg);
			if (*killswitch)
				goto connection_failed;
			if (http->run() == -1)
				goto connection_failed;

			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
				goto connection_failed;

			size_t lengthToSend = min(clen, connection->GetSendBytesAvailable());

			if (lengthToSend)
			{
				connection->send(artDataPtr, (int)lengthToSend);
				artDataPtr += lengthToSend;
				clen-=lengthToSend;
				transferred+=lengthToSend;
			}
			else
			{
				Sleep(10);
			}
		}
		int x;
		while (x = (int)connection->GetSendBytesInQueue())
		{
			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
			goto connection_failed;
			Sleep(10);
			if (*killswitch)
				goto connection_failed;
			if (http->run() == -1)
				goto connection_failed;

		}
	}

	/* retrieve reply */
	int ret;
	do
	{
		Sleep(55);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = http->get_status();
		switch (status)
		{
		case HTTPRECEIVER_STATUS_CONNECTING:
		case HTTPRECEIVER_STATUS_READING_HEADERS:
			break;

		case HTTPRECEIVER_STATUS_READING_CONTENT:
			{
				sf->releaseInterface(http);
							WASABI_API_MEMMGR->sysFree(artData);
			WASABI_API_MEMMGR->sysFree(mimeType);
				return 0;
			}
			break;
		case HTTPRECEIVER_STATUS_ERROR:
		default:
			sf->releaseInterface(http);
						WASABI_API_MEMMGR->sysFree(artData);
			WASABI_API_MEMMGR->sysFree(mimeType);
			return 1;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);


connection_failed:
	WASABI_API_MEMMGR->sysFree(artData);
	WASABI_API_MEMMGR->sysFree(mimeType);
	sf->releaseInterface(http);
	return 1;
}


int HTTP_Delete(const char *url)
{
	api_httpreceiver *http = 0;
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(httpreceiverGUID);
	if (sf)
		http = (api_httpreceiver *)sf->getInterface();

	if (!http)
		return 1;

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
 	SetUserAgent(http);

	http->AddHeaderValue("X-Winamp-ID", winamp_id_str);
	http->AddHeaderValue("X-Winamp-Name", winamp_name);

	/* connect */
	http->connect(url, 0, "DELETE");
	
	/* retrieve reply */
	int ret;
	do
	{
		Sleep(55);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = http->get_status();
		switch (status)
		{
		case HTTPRECEIVER_STATUS_CONNECTING:
		case HTTPRECEIVER_STATUS_READING_HEADERS:
			break;

		case HTTPRECEIVER_STATUS_READING_CONTENT:
			{
				sf->releaseInterface(http);
				return 0;
			}
			break;
		case HTTPRECEIVER_STATUS_ERROR:
		default:
			sf->releaseInterface(http);
			return 1;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);

	sf->releaseInterface(http);
	return 1;
}

