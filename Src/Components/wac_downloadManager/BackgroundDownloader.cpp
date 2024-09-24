#include "BackgroundDownloader.h"
#include "../../Components\wac_network/wac_network_http_receiver_api.h"

#include "api__wac_downloadManager.h"

#include "api/service/waservicefactory.h"

#include "../nu/AutoChar.h"

#include <strsafe.h>

#define HTTP_BUFFER_SIZE 32768
// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
static const GUID internetConfigGroupGUID =
{ 0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c } };


static void SetUserAgent( api_httpreceiver *http )
{
	char agent[ 256 ];
	StringCchPrintfA( agent, 256, "User-Agent: %S/%S", WASABI_API_APP->main_getAppName(), WASABI_API_APP->main_getVersionNumString() );
	http->addheader( agent );
}

static int FeedHTTP( api_httpreceiver *http, Downloader::DownloadCallback &callback, void *userData, bool *noData )
{
	char downloadedData[ HTTP_BUFFER_SIZE ];
	int result = 0;
	int downloadSize = http->get_bytes( downloadedData, HTTP_BUFFER_SIZE );
	if ( downloadSize )
	{
		result = callback( userData, downloadedData, downloadSize );
		*noData = false;
	}
	else
		*noData = true;

	return result;
}

static void RunDownload( api_httpreceiver *http, Downloader::DownloadCallback &callback, void *userData )
{
	int ret;
	bool noData;
	do
	{
		Sleep( 50 );
		ret = http->run();
		if ( FeedHTTP( http, callback, userData, &noData ) != 0 )
			return;
	} while ( ret == HTTPRECEIVER_RUN_OK );

	// finish off the data
	do
	{
		if ( FeedHTTP( http, callback, userData, &noData ) != 0 )
			return;
	} while ( !noData );
}


bool Downloader::Download( const char *url, Downloader::DownloadCallback &callback, void *userdata, uint64_t startPosition )
{
	api_httpreceiver *http = 0;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid( httpreceiverGUID );
	if ( sf )
		http = (api_httpreceiver *)sf->getInterface();

	if ( !http )
		return false;

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool( internetConfigGroupGUID, L"proxy80", false );
	if ( proxy80 && strstr( url, ":" ) && ( !strstr( url, ":80/" ) && strstr( url, ":80" ) != ( url + strlen( url ) - 3 ) ) )
		use_proxy = 0;

	const wchar_t *proxy = use_proxy ? AGAVE_API_CONFIG->GetString( internetConfigGroupGUID, L"proxy", 0 ) : 0;

	http->AllowCompression();
	http->open( API_DNS_AUTODNS, HTTP_BUFFER_SIZE, ( proxy && proxy[ 0 ] ) ? (const char *)AutoChar( proxy ) : NULL );

	if ( startPosition > 0 )
	{
		char temp[ 128 ];
		StringCchPrintfA( temp, 128, "Range: bytes=%d-", startPosition );
		http->addheader( temp );
	}

	SetUserAgent( http );
	http->connect( url );

	int ret;

	do
	{
		Sleep( 10 );

		ret = http->run();
		if ( ret == -1 ) // connection failed
			break;

		// ---- check our reply code ----
		int replycode = http->getreplycode();
		switch ( replycode )
		{
			case 0:
			case 100:
				break;
			case 200:
			{
				RunDownload( http, callback, userdata );

				sf->releaseInterface( http );

				return true;
			}
			break;
			default:
				sf->releaseInterface( http );
				return false;
		}
	} while ( ret == HTTPRECEIVER_RUN_OK );

	sf->releaseInterface( http );

	return false;
}
