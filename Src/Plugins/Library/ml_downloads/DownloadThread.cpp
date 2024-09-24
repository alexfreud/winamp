#include "Main.h"

#pragma warning(disable:4786)

#include "DownloadThread.h"
#include "api__ml_downloads.h"
#include <api/service/waServiceFactory.h>
#include "errors.h"
#include <strsafe.h>

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