#include "api.h"
#include "../xml/obj_xml.h"
#include "api_auth.h"
#include "../nu/AutoChar.h"
#include "../jnetlib/api_httpget.h"
#include "ifc_authcallback.h"
#include <api/service/waservicefactory.h>
#include <strsafe.h>


static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};


#define USER_AGENT_SIZE (10 /*User-Agent*/ + 2 /*: */ + 6 /*Winamp*/ + 1 /*/*/ + 1 /*5*/ + 3/*.55*/ + 1 /*Null*/)
static void SetUserAgent(api_httpreceiver *http)
{
	char user_agent[USER_AGENT_SIZE] = {0};
	StringCchPrintfA(user_agent, USER_AGENT_SIZE, "User-Agent: Winamp/%S", WASABI_API_APP->main_getVersionNumString());
	http->addheader(user_agent);
}


#define HTTP_BUFFER_SIZE 8192

static int FeedXMLHTTP(api_httpreceiver *http, obj_xml *parser, bool *noData)
{
	char downloadedData[HTTP_BUFFER_SIZE] = {0};
	int xmlResult = API_XML_SUCCESS; 
	int downloadSize = http->get_bytes(downloadedData, HTTP_BUFFER_SIZE);
	if (downloadSize)
	{
		xmlResult = parser->xmlreader_feed((void *)downloadedData, downloadSize);
		*noData=false;
	}
	else	
		*noData = true;

	return xmlResult;
}


static int RunXMLDownload(api_httpreceiver *http, obj_xml *parser, ifc_authcallback *callback)
{
	int ret;
	bool noData;
	do
	{
		if (callback && callback->OnIdle())
		{
			return AUTH_ABORT;
		}
		else if (!callback)
		{
			Sleep(50);
		}

		ret = http->run();
		if (FeedXMLHTTP(http, parser, &noData) != API_XML_SUCCESS)
			return AUTH_ERROR_PARSING_XML;
	}
	while (ret == HTTPRECEIVER_RUN_OK);

	// finish off the data
	do
	{
		if (FeedXMLHTTP(http, parser, &noData) != API_XML_SUCCESS)
			return AUTH_ERROR_PARSING_XML;
	} while (!noData);

	parser->xmlreader_feed(0, 0);
	if (ret != HTTPRECEIVER_RUN_ERROR)
		return AUTH_SUCCESS;
	else
		return AUTH_CONNECTIONRESET;
}


int PostXML(const char *url, const char *post_data, obj_xml *parser, ifc_authcallback *callback)
{
	if (!parser)
		return AUTH_NOPARSER;

	api_httpreceiver *http = 0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf)
		http = (api_httpreceiver *)sf->getInterface();

	if (!http)
		return AUTH_NOHTTP;

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;
	size_t clen = strlen(post_data);

	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
	SetUserAgent(http);

	char clen_header[1024] = {0};
	StringCbPrintfA(clen_header, sizeof(clen_header), "Content-Length: %u", clen);
	http->addheader(clen_header);
	http->addheader("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	if (callback && callback->OnConnecting())
	{
		sf->releaseInterface(http);
		return AUTH_ABORT;
	}
	http->connect(url, 0, "POST");

	// POST the data
	api_connection *connection = http->GetConnection();
	if (connection)
	{
		if (callback && callback->OnIdle())
		{
			sf->releaseInterface(http);
			return AUTH_ABORT;
		}
		else if (!callback)
		{
			Sleep(50);
		}

		if (http->run() == -1)
			goto connection_failed;

		if (callback && callback->OnSending())
		{
			sf->releaseInterface(http);
			return AUTH_ABORT;
		}

		const char *dataIndex = post_data;
		while (clen)
		{
			if (callback && callback->OnIdle())
			{
				sf->releaseInterface(http);
				return AUTH_ABORT;
			}
			else if (!callback)
			{
				Sleep(50);
			}

			if (http->run() == -1)
				goto connection_failed;

			size_t lengthToSend = min(clen, connection->GetSendBytesAvailable());
			if (lengthToSend)
			{
				connection->send(dataIndex, (int)lengthToSend);
				dataIndex+=lengthToSend;
				clen-=lengthToSend;
			}			
		}
	}

	// retrieve reply
	if (callback && callback->OnReceiving())
	{
		sf->releaseInterface(http);
		return AUTH_ABORT;
	}

	int ret;
	do
	{
		if (callback && callback->OnIdle())
		{
			sf->releaseInterface(http);
			return AUTH_ABORT;
		}
		else if (!callback)
		{
			Sleep(50);
		}

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
				int downloadError;
				downloadError = RunXMLDownload(http, parser, callback);
				sf->releaseInterface(http);
				return downloadError;
			}
			break;
		case HTTPRECEIVER_STATUS_ERROR:
		default:
			sf->releaseInterface(http);
			return AUTH_404;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);


connection_failed:
	sf->releaseInterface(http);
	return AUTH_404;
}

