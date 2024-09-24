#pragma once
#ifndef __WASABI_API_HTTPGETH
#define __WASABI_API_HTTPGETH

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"

#include "wac_network_dns_api.h"
#include "wac_network_connection_api.h"

enum
{
	HTTPRECEIVER_STATUS_ERROR           = -1,
	HTTPRECEIVER_STATUS_CONNECTING      = 0,
	HTTPRECEIVER_STATUS_READING_HEADERS = 1,
	HTTPRECEIVER_STATUS_READING_CONTENT = 2,
};

enum
{
	HTTPRECEIVER_RUN_ERROR             = -1,
	HTTPRECEIVER_RUN_OK                = 0,
	HTTPRECEIVER_RUN_CONNECTION_CLOSED = 1,
};


class NOVTABLE api_httpreceiver : public Dispatchable
{
protected:
	api_httpreceiver()                                                {}
	~api_httpreceiver()                                               {}

public:
	void            open( api_dns *dns = API_DNS_AUTODNS, size_t recvbufsize = PACKET_SIZE, const char *proxy = NULL );

	void            addheader( const char *header );
	void            AddHeaderValue( const char *header, const char *value );
	void            reset_headers();

	char           *getheader( char *headername );
	const char     *getallheaders(); // double null terminated, null delimited list

	void            connect( const char *url, int ver = 0, const char *requestmethod = "GET" );

	int             run();           // see HTTPRECEIVER_RUN_* enum for return values
	int             get_status();    // see HTTPRECEIVER_STATUS_* enum for return values

	int             bytes_available();
	int             get_bytes( void *buf, int len );
	int             peek_bytes( void *buf, int len );

	uint64_t        content_length();

	int             getreplycode();  // returns 0 if none yet, otherwise returns http reply code.
	const char     *GetReply();

	const char     *geterrorstr();

	api_connection *GetConnection();

	void            AllowCompression();
	const char     *get_url();       // might not be the same as what you passed in if a redirect happened

	void            set_sendbufsize( int sendbufsize = PACKET_SIZE ); // call if you're going to POST or do any kind of bidirectional communications
	void            set_accept_all_reply_codes();
	void            set_persistent();

	DISPATCH_CODES
	{
		API_HTTPRECEIVER_OPEN                       =  10,

		API_HTTPRECEIVER_ADDHEADER                  =  20,
		API_HTTPRECEIVER_ADDHEADERVALUE             =  30,

		API_HTTPRECEIVER_CONNECT                    =  40,

		API_HTTPRECEIVER_RUN                        =  50,
		API_HTTPRECEIVER_GETSTATUS                  =  60,

		API_HTTPRECEIVER_GETBYTESAVAILABLE          =  70,
		API_HTTPRECEIVER_GETBYTES                   =  80,
		API_HTTPRECEIVER_PEEKBYTES                  =  90,

		API_HTTPRECEIVER_GETHEADER                  = 100,
		API_HTTPRECEIVER_GETCONTENTLENGTH           = 110,
		API_HTTPRECEIVER_GETALLHEADERS              = 120,

		API_HTTPRECEIVER_GETREPLYCODE               = 130,
		API_HTTPRECEIVER_GETREPLY                   = 140,

		API_HTTPRECEIVER_GETERROR                   = 150,
		API_HTTPRECEIVER_GETCONNECTION              = 160,

		API_HTTPRECEIVER_ALLOW_COMPRESSION          = 170,

		API_HTTPRECEIVER_RESET_HEADERS              = 180,

		API_HTTPRECEIVER_GET_URL                    = 190,

		API_HTTPRECEIVER_SET_SENDBUFSIZE            = 200,
		API_HTTPRECEIVER_SET_ACCEPT_ALL_REPLY_CODES = 210,
		API_HTTPRECEIVER_SET_PERSISTENT             = 220,
	};

};

inline void api_httpreceiver::open( api_dns *dns, size_t recvbufsize, const char *proxy )
{
	_voidcall( API_HTTPRECEIVER_OPEN, dns, recvbufsize, proxy );
}


inline void api_httpreceiver::addheader( const char *header )
{
	_voidcall( API_HTTPRECEIVER_ADDHEADER, header );
}

inline void api_httpreceiver::AddHeaderValue( const char *header, const char *value )
{
	_voidcall( API_HTTPRECEIVER_ADDHEADERVALUE, header, value );
}

inline void api_httpreceiver::reset_headers()
{
	_voidcall( API_HTTPRECEIVER_RESET_HEADERS );
}


inline char *api_httpreceiver::getheader( char *headername )
{
	return _call( API_HTTPRECEIVER_GETHEADER, (char *)NULL, headername );
}

inline const char *api_httpreceiver::getallheaders()
{
	return _call( API_HTTPRECEIVER_GETALLHEADERS, (const char *)NULL );
}


inline void api_httpreceiver::connect( const char *url, int ver, const char *requestmethod )
{
	_voidcall( API_HTTPRECEIVER_CONNECT, url, ver, requestmethod );
}


inline int api_httpreceiver::run()
{
	return _call( API_HTTPRECEIVER_RUN, (int)HTTPRECEIVER_RUN_ERROR );
}

inline int api_httpreceiver::get_status()
{
	return _call( API_HTTPRECEIVER_GETSTATUS, (int)HTTPRECEIVER_STATUS_ERROR );
}


inline int api_httpreceiver::bytes_available()
{
	return _call( API_HTTPRECEIVER_GETBYTESAVAILABLE, (int)0 );
}

inline int api_httpreceiver::get_bytes( void *buf, int len )
{
	return _call( API_HTTPRECEIVER_GETBYTES, (int)0, buf, len );
}

inline int api_httpreceiver::peek_bytes( void *buf, int len )
{
	return _call( API_HTTPRECEIVER_PEEKBYTES, (int)0, buf, len );
}


inline uint64_t api_httpreceiver::content_length()
{
	return _call( API_HTTPRECEIVER_GETCONTENTLENGTH, (uint64_t)0 );
}


inline int api_httpreceiver::getreplycode()
{
	return _call( API_HTTPRECEIVER_GETREPLYCODE, 0 );
}

inline const char *api_httpreceiver::GetReply()
{
	return _call( API_HTTPRECEIVER_GETREPLY, (const char *)NULL );
}


inline const char *api_httpreceiver::geterrorstr()
{
	return _call( API_HTTPRECEIVER_GETERROR, (const char *)NULL );
}


inline api_connection *api_httpreceiver::GetConnection()
{
	return _call( API_HTTPRECEIVER_GETCONNECTION, (api_connection *)NULL );
}


inline void api_httpreceiver::AllowCompression()
{
	_voidcall( API_HTTPRECEIVER_ALLOW_COMPRESSION );
}

inline const char *api_httpreceiver::get_url()
{
	return _call( API_HTTPRECEIVER_GET_URL, (const char *)0 );
}


inline void api_httpreceiver::set_sendbufsize( int sendbufsize )
{
	_voidcall( API_HTTPRECEIVER_SET_SENDBUFSIZE, sendbufsize );
}

inline void api_httpreceiver::set_accept_all_reply_codes()
{
	_voidcall( API_HTTPRECEIVER_SET_ACCEPT_ALL_REPLY_CODES );
}

inline void api_httpreceiver::set_persistent()
{
	_voidcall( API_HTTPRECEIVER_SET_PERSISTENT );
}

// {12475CD9-1BA3-4665-B395-F87DBF31E30F}
static const GUID httpreceiverGUID =
{ 0x12475cd9, 0x1ba3, 0x4665, { 0xb3, 0x95, 0xf8, 0x7d, 0xbf, 0x31, 0xe3, 0xf } };


#endif