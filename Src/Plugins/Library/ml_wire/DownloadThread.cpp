#include "Main.h"

#pragma warning(disable:4786)

#include "DownloadThread.h"
#include "api__ml_wire.h"
#include "api/service/waServiceFactory.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include "errors.h"
#include <strsafe.h>

extern int winampVersion;

#define USER_AGENT_SIZE (10 /*User-Agent*/ + 2 /*: */ + 6 /*Winamp*/ + 1 /*/*/ + 1 /*5*/ + 3/*.21*/ + 1 /*Null*/)
void SetUserAgent( api_httpreceiver *http )
{
	char user_agent[ USER_AGENT_SIZE ] = { 0 };
	int bigVer = ( ( winampVersion & 0x0000FF00 ) >> 12 );
	int smallVer = ( ( winampVersion & 0x000000FF ) );
	StringCchPrintfA( user_agent, USER_AGENT_SIZE, "User-Agent: Winamp/%01x.%02x", bigVer, smallVer );
	http->addheader( user_agent );
}

#define HTTP_BUFFER_SIZE 32768

static int FeedXMLHTTP( api_httpreceiver *http, obj_xml *parser, bool *noData )
{
	char downloadedData[ HTTP_BUFFER_SIZE ] = { 0 };
	int xmlResult = API_XML_SUCCESS;
	int downloadSize = http->get_bytes( downloadedData, HTTP_BUFFER_SIZE );
	if ( downloadSize )
	{
		xmlResult = parser->xmlreader_feed( (void *)downloadedData, downloadSize );
		*noData = false;
	}
	else
		*noData = true;

	return xmlResult;
}


DownloadThread::DownloadThread() : parser( 0 ), parserFactory( 0 )
{
	parserFactory = plugin.service->service_getServiceByGuid( obj_xmlGUID );
	if ( parserFactory )
		parser = (obj_xml *)parserFactory->getInterface();

	if ( parser )
	{
		parser->xmlreader_setCaseSensitive();
		parser->xmlreader_registerCallback( L"*", &xmlDOM );
		parser->xmlreader_open();
	}
}

DownloadThread::~DownloadThread()
{
	if ( parser )
	{
		parser->xmlreader_unregisterCallback( &xmlDOM );
		parser->xmlreader_close();
	}

	if ( parserFactory && parser )
		parserFactory->releaseInterface( parser );

	parserFactory = 0;
	parser        = 0;
}

void URLToFileName( wchar_t *url )
{
	while ( url && *url != 0 )
	{
		switch ( *url )
		{
			case ':':
			case '/':
			case '\\':
			case '*':
			case '?':
			case '"':
			case '<':
			case '>':
			case '|':
				*url = '_';
		}
		url++;
	}
}

#define FILE_BUFFER_SIZE 32768
void DownloadThread::DownloadFile( const wchar_t *fileName )
{
	if ( !parser )
		return; // no sense in continuing if there's no parser available

	HANDLE file = CreateFile( fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( file == INVALID_HANDLE_VALUE )
		return;

	while ( true )
	{
		char data[ FILE_BUFFER_SIZE ] = { 0 };
		DWORD bytesRead = 0;
		if ( ReadFile( file, data, FILE_BUFFER_SIZE, &bytesRead, NULL ) && bytesRead )
		{
			parser->xmlreader_feed( (void *)data, bytesRead );
		}
		else
			break;
	}

	CloseHandle( file );
	parser->xmlreader_feed( 0, 0 );
	ReadNodes( fileName );
}

#ifdef _DEBUG
#include <iostream>
void ShowAllHeaders( api_httpreceiver *http )
{
	std::cout << "--------------------" << std::endl;
	const char *blah = http->getallheaders();
	while ( blah && *blah )
	{
		std::cout << blah << std::endl;
		blah += strlen( blah ) + 1;
	}
}
#else
#define ShowAllHeaders(x)
#endif

int RunXMLDownload( api_httpreceiver *http, obj_xml *parser )
{
	int ret;
	bool noData;
	do
	{
		Sleep( 50 );
		ret = http->run();
		if ( FeedXMLHTTP( http, parser, &noData ) != API_XML_SUCCESS )
			return DOWNLOAD_ERROR_PARSING_XML;
	} while ( ret == HTTPRECEIVER_RUN_OK );

	// finish off the data
	do
	{
		if ( FeedXMLHTTP( http, parser, &noData ) != API_XML_SUCCESS )
			return DOWNLOAD_ERROR_PARSING_XML;
	} while ( !noData );

	parser->xmlreader_feed( 0, 0 );
	if ( ret != HTTPRECEIVER_RUN_ERROR )
		return DOWNLOAD_SUCCESS;
	else
		return DOWNLOAD_CONNECTIONRESET;
}


int DownloadThread::DownloadURL( const wchar_t *url )
{
	if ( !parser )
		return DOWNLOAD_NOPARSER; // no sense in continuing if there's no parser available

	api_httpreceiver *http = 0;
	waServiceFactory *sf = plugin.service->service_getServiceByGuid( httpreceiverGUID );
	if ( sf )
		http = (api_httpreceiver *)sf->getInterface();

	if ( !http )
		return DOWNLOAD_NOHTTP;

	http->AllowCompression();
	http->open( API_DNS_AUTODNS, HTTP_BUFFER_SIZE, mediaLibrary.GetProxy() );

	SetUserAgent( http );

	http->connect( AutoChar( url ) );
	int ret;

	do
	{
		Sleep( 50 );
		ret = http->run();
		if ( ret == -1 ) // connection failed
			break;

		// ---- check our reply code ----
		int status = http->get_status();
		switch ( status )
		{
			case HTTPRECEIVER_STATUS_CONNECTING:
			case HTTPRECEIVER_STATUS_READING_HEADERS:
				break;

			case HTTPRECEIVER_STATUS_READING_CONTENT:
			{
				ShowAllHeaders( http ); // benski> don't cut, only enabled in debug mode
				int downloadError;
				downloadError = RunXMLDownload( http, parser );
				if ( downloadError == DOWNLOAD_SUCCESS )
					ReadNodes( url );
				sf->releaseInterface( http );
				return downloadError;
			}
			break;
			case HTTPRECEIVER_STATUS_ERROR:
			default:
				sf->releaseInterface( http );
				return DOWNLOAD_404;
		}
	} while ( ret == HTTPRECEIVER_RUN_OK );

	const char *er = http->geterrorstr();

	sf->releaseInterface( http );

	return DOWNLOAD_404;
}