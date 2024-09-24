#include "VirtualIO.h"
#include "api__in_wave.h"
#include <api/service/waservicefactory.h>
#include "../nu/AutoWide.h"
#include <assert.h>
#include <windows.h>

// TODO: extend this to use api_filereader
// instead of just the HTTP reader directly
// then we could use this is a workaround for unicode filenames

sf_count_t VirtualGetFileLength( void *user_data )
{
	svc_fileReader *reader = (svc_fileReader *)user_data;

	return reader->getLength();
}

sf_count_t VirtualSeek( sf_count_t offset, int whence, void *user_data )
{
	svc_fileReader *reader = (svc_fileReader *)user_data;
	switch ( whence )
	{
		case SEEK_SET:
			reader->seek( offset );
			break;
		case SEEK_CUR:
		{
			uint64_t cur = reader->getPos();
			reader->seek( offset + cur );
		}
		break;
		case SEEK_END:
		{
			uint64_t total = reader->getLength();
			reader->seek( total + offset );
		}
		break;

	}

	return reader->getPos();
}

sf_count_t VirtualRead( void *ptr, sf_count_t count, void *user_data )
{
	svc_fileReader *reader = (svc_fileReader *)user_data;

	return reader->read( (int8_t *)ptr, (size_t)count );
}

sf_count_t VirtualWrite( const void *ptr, sf_count_t count, void *user_data )
{
	svc_fileReader *reader = (svc_fileReader *)user_data;

	return reader->write( (int8_t *)ptr, (size_t)count );
}

sf_count_t VirtualTell( void *user_data )
{
	svc_fileReader *reader = (svc_fileReader *)user_data;

	return reader->getPos();
}

SF_VIRTUAL_IO httpIO =
{
	VirtualGetFileLength,
	VirtualSeek,
	VirtualRead,
	VirtualWrite,
	VirtualTell
};

static const GUID HTTPReaderGUID = 
{ 0xbc10fa00, 0x53f5, 0x4032, { 0xa0, 0x09, 0x2, 0x2b, 0x87, 0xec, 0x34, 0x04 } };

void *CreateReader( const wchar_t *url )
{
	static waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid( HTTPReaderGUID );
	if ( sf )
	{
		svc_fileReader *l_reader = (svc_fileReader *)sf->getInterface();
		if ( l_reader )
			l_reader->open( url );

		return l_reader;
	}
	else
		return 0;
}

void DestroyReader( void *reader )
{
	static waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid( HTTPReaderGUID );
	if ( sf )
		sf->releaseInterface( reader );
}