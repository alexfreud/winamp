#ifndef NULLSOFT_HTTPREADER_H
#define NULLSOFT_HTTPREADER_H

#include "api/service/svcs/svc_fileread.h"

class HttpReader;

class HTTPReader : public svc_fileReader
{
public:
	HTTPReader()                                                      {}
	virtual ~HTTPReader()                                             { close(); }

	int isMine( const wchar_t *filename, int mode = SvcFileReader::READ );
	int open( const wchar_t *filename, int mode = SvcFileReader::READ );

	size_t read( int8_t *buffer, size_t length );
	size_t write( const int8_t *buffer, size_t length )               { return 0; }
	
	uint64_t bytesAvailable( uint64_t requested );
	
	void close();
	void abort();
	
	uint64_t getLength();
	uint64_t getPos();

	int canSeek();
	int seek( uint64_t position );

	int hasHeaders();
	const char *getHeader( const char *header );

	void setMetaDataCallback( api_readercallback *cb );

	int canPrefetch()                                                 { return 0; } // no info fetch on HTTP files

/*int remove(const char *filename) { return 0; }
int move(const char *filename, const char *destfilename) { return 0; }*/

protected:
	RECVS_DISPATCH;

private:
	HttpReader *reader       = NULL;
	int         hasConnected = 0;
	char       *m_filename   = 0;
};

#endif