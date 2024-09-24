#include "main.h"
#include "http_avi_reader.h"
#include "../nu/AutoChar.h"
#include "api__in_avi.h"
#include <api/service/waservicefactory.h>
#include "../nu/ns_wc.h"
#include <strsafe.h>

static const int http_buffer_size=128*1024;
// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};

AVIReaderHTTP::AVIReaderHTTP(HANDLE killswitch, HANDLE seek_event)
{
	seekable = 0;
	http = 0;
	position = 0;
	handles[0]=killswitch;
	handles[1]=seek_event;
}

static void SetUserAgent(api_httpreceiver *http)
{
	char agent[256] = {0};
	StringCchPrintfA(agent, 256, "User-Agent: %S/%S", WASABI_API_APP->main_getAppName(), WASABI_API_APP->main_getVersionNumString());
	http->addheader(agent);
}

void AVIReaderHTTP::GetFilename(wchar_t *fn, size_t len)
{
	const char *url = http->get_url();
	MultiByteToWideCharSZ(CP_ACP, 0, url, -1, fn, (int)len);
}

int AVIReaderHTTP::Open(const wchar_t *url)
{
	int use_proxy = 1;

	const wchar_t *proxy = AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0);
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && wcsstr(url, L":") && (!wcsstr(url, L":80/") && wcsstr(url, L":80") != (url + wcslen(url) - 3)))
		use_proxy = 0;

	waServiceFactory *sf = plugin.service->service_getServiceByGuid(httpreceiverGUID);
	if (sf)  http = (api_httpreceiver *)sf->getInterface();

	http->open(API_DNS_AUTODNS, http_buffer_size, (use_proxy && proxy && proxy[0]) ? (char *)AutoChar(proxy) : NULL);
	http->addheader("Accept:*/*");
	SetUserAgent(http);

	http->connect(AutoChar(url, CP_UTF8), 0);

	return nsavi::READ_OK;
}


int AVIReaderHTTP::Open(const char *url, uint64_t start_offset)
{
	int use_proxy = 1;

	const wchar_t *proxy = AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0);
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	waServiceFactory *sf = plugin.service->service_getServiceByGuid(httpreceiverGUID);
	if (sf)  http = (api_httpreceiver *)sf->getInterface();

	http->open(API_DNS_AUTODNS, http_buffer_size, (use_proxy && proxy && proxy[0]) ? (char *)AutoChar(proxy) : NULL);
	http->addheader("Accept:*/*");
	SetUserAgent(http);

	if (start_offset)
	{
		char temp[128] = {0};
		StringCchPrintfA(temp, 128, "Range: bytes=%I64u-", start_offset);
		http->addheader(temp);
		position = start_offset;
	}

	http->connect(url, !!start_offset);

	return nsavi::READ_OK;
}

void AVIReaderHTTP::Close()
{
	if (http)
	{
		waServiceFactory *sf = plugin.service->service_getServiceByGuid(httpreceiverGUID);
		if (sf)  sf->releaseInterface(http);
	}
	http = 0;
}


int AVIReaderHTTP::Connect()
{
	http->run();
	int status = http->get_status();
	while (status == HTTPRECEIVER_STATUS_CONNECTING || status == HTTPRECEIVER_STATUS_READING_HEADERS)
	{
		int ret = WaitForMultipleObjects(2, handles, FALSE, 50);
		if (ret == WAIT_OBJECT_0+1)
			return -2;
		else if (ret != WAIT_TIMEOUT)
			return -1;
		http->run();
		status = http->get_status();
	}

	if (status == HTTPRECEIVER_STATUS_ERROR)
	{
		return nsavi::READ_FAILED;
	}
	const char *headers = http->getallheaders();
	const char *ranges = http->getheader("accept-ranges");
	seekable =  (ranges && _stricmp(ranges, "bytes"));

	return nsavi::READ_OK;
}

int AVIReaderHTTP::Buffer()
{
	while (http->bytes_available() < http_buffer_size && http->run() == HTTPRECEIVER_RUN_OK)
	{
		int ret = WaitForMultipleObjects(2, handles, FALSE, 50);
		if (ret == WAIT_OBJECT_0+1)
			return -2;
		else if (ret != WAIT_TIMEOUT)
			return -1;
	}

	return nsavi::READ_OK;
}

uint64_t AVIReaderHTTP::GetContentLength()
{
	const char *content_length = http->getheader("content-length");
	if (content_length)
		return _atoi64(content_length);
	else
		return 0;
}

int AVIReaderHTTP::Read(void *buffer, uint32_t read_length, uint32_t *bytes_read)
{
	uint32_t total_bytes_read=0;
	for(;;)
	{
		int ret = http->run();
		int http_bytes_read =	http->get_bytes(buffer, read_length);

		read_length -= http_bytes_read;
		buffer = (uint8_t *)buffer + http_bytes_read;
		total_bytes_read+=http_bytes_read;
		position += http_bytes_read;

		if (!read_length)
		{
			*bytes_read = total_bytes_read;
			return nsavi::READ_OK;
		}

		if (http->bytes_available() == 0)
		{
			if (ret == HTTPRECEIVER_RUN_CONNECTION_CLOSED)
			{
				if (position == http->content_length())
					return nsavi::READ_EOF;
				else
					return nsavi::READ_DISCONNECT;
			}
			else if (ret == HTTPRECEIVER_RUN_ERROR)
			{
				return nsavi::READ_DISCONNECT;
			}
		}

		ret = WaitForMultipleObjects(2, handles, FALSE, 50);
		if (ret == WAIT_OBJECT_0+1)
			return -2;
		else if (ret != WAIT_TIMEOUT)
			return -1;
	}

	return nsavi::READ_OK;
}

int AVIReaderHTTP::Peek(void *buffer, uint32_t read_length, uint32_t *bytes_read)
{
	uint32_t total_bytes_read=0;

	while (http->bytes_available() < (int)read_length && http->run() == HTTPRECEIVER_RUN_OK)
	{
		int ret = WaitForMultipleObjects(2, handles, FALSE, 50);
		if (ret == WAIT_OBJECT_0+1)
			return -2;
		else if (ret != WAIT_TIMEOUT)
			return -1;
	}

	*bytes_read = http->peek_bytes(buffer, read_length);
	return nsavi::READ_OK;
}

int AVIReaderHTTP::Seek(uint64_t seek_position)
{

	// if position is forward of our current position, see if we have enough in the buffer to just advance the buffer
	if (seek_position > position 
		&& seek_position - position <= http->bytes_available())
	{
		int bytes_read = http->get_bytes(0, (int)(seek_position - position));
		position += bytes_read;
		return nsavi::READ_OK;
	}
	else
	{
		// otherwise, close connection and re-open with a start position
		char *url = _strdup(http->get_url());
		Close();
		Open(url, seek_position);
		free(url);
		return Connect();
	}
}

uint64_t AVIReaderHTTP::Tell()
{
	return position;
}

int AVIReaderHTTP::Skip(uint32_t skip_bytes)
{
	// see if we have enough room in our buffer
	if (http->bytes_available() >= (int)skip_bytes)
	{
		int bytes_read = http->get_bytes(0, skip_bytes);
		position += bytes_read;
		return nsavi::READ_OK;
	}
	else
	{
		// close connection and re-open with a start position
		char *url = _strdup(http->get_url());
		Close();
		Open(url, position+skip_bytes);
		free(url);
		return Connect();
	}
}