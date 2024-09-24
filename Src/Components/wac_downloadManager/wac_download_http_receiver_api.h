#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_API_HTTP_RECEIVER_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_API_HTTP_RECEIVER_H

#define PACKET_SIZE 16384

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"

#include "../wac_network/wac_network_dns_api.h"

enum
{
	HTTP_RECEIVER_STATUS_ERROR           = -1,
	HTTP_RECEIVER_STATUS_CONNECTING      = 0,
	HTTP_RECEIVER_STATUS_READING_HEADERS = 1,
	HTTP_RECEIVER_STATUS_READING_CONTENT = 2,
};

enum
{
	HTTP_RECEIVER_RUN_ERROR              = -1,
	HTTP_RECEIVER_RUN_OK                 = 0,
	HTTP_RECEIVER_RUN_CONNECTION_CLOSED  = 1,
};


class NOVTABLE api_wac_download_manager_http_receiver : public Dispatchable
{
protected:
	api_wac_download_manager_http_receiver()                          {}
	~api_wac_download_manager_http_receiver()                         {}

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
	const char *GetReply();

	const char *geterrorstr();

	//api_connection *GetConnection();

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

inline void api_wac_download_manager_http_receiver::open( api_dns *dns, size_t recvbufsize, const char *proxy )
{
	_voidcall( API_HTTPRECEIVER_OPEN, dns, recvbufsize, proxy );
}


inline void api_wac_download_manager_http_receiver::addheader( const char *header )
{
	_voidcall( API_HTTPRECEIVER_ADDHEADER, header );
}

inline void api_wac_download_manager_http_receiver::AddHeaderValue( const char *header, const char *value )
{
	_voidcall( API_HTTPRECEIVER_ADDHEADERVALUE, header, value );
}

inline void api_wac_download_manager_http_receiver::reset_headers()
{
	_voidcall( API_HTTPRECEIVER_RESET_HEADERS );
}


inline char *api_wac_download_manager_http_receiver::getheader( char *headername )
{
	return _call( API_HTTPRECEIVER_GETHEADER, (char *)NULL, headername );
}

inline const char *api_wac_download_manager_http_receiver::getallheaders()
{
	return _call( API_HTTPRECEIVER_GETALLHEADERS, (const char *)NULL );
}


inline void api_wac_download_manager_http_receiver::connect( const char *url, int ver, const char *requestmethod )
{
	_voidcall( API_HTTPRECEIVER_CONNECT, url, ver, requestmethod );
}


inline int api_wac_download_manager_http_receiver::run()
{
	return _call( API_HTTPRECEIVER_RUN, (int)HTTP_RECEIVER_RUN_ERROR );
}

inline int api_wac_download_manager_http_receiver::get_status()
{
	return _call( API_HTTPRECEIVER_GETSTATUS, (int)HTTP_RECEIVER_STATUS_ERROR );
}


inline int api_wac_download_manager_http_receiver::bytes_available()
{
	return _call( API_HTTPRECEIVER_GETBYTESAVAILABLE, (int)0 );
}

inline int api_wac_download_manager_http_receiver::get_bytes( void *buf, int len )
{
	return _call( API_HTTPRECEIVER_GETBYTES, (int)0, buf, len );
}

inline int api_wac_download_manager_http_receiver::peek_bytes( void *buf, int len )
{
	return _call( API_HTTPRECEIVER_PEEKBYTES, (int)0, buf, len );
}


inline uint64_t api_wac_download_manager_http_receiver::content_length()
{
	return _call( API_HTTPRECEIVER_GETCONTENTLENGTH, (uint64_t)0 );
}


inline int api_wac_download_manager_http_receiver::getreplycode()
{
	return _call( API_HTTPRECEIVER_GETREPLYCODE, 0 );
}

inline const char *api_wac_download_manager_http_receiver::GetReply()
{
	return _call( API_HTTPRECEIVER_GETREPLY, (const char *)NULL );
}


inline const char *api_wac_download_manager_http_receiver::geterrorstr()
{
	return _call( API_HTTPRECEIVER_GETERROR, (const char *)NULL );
}


//inline api_connection *api_wac_download_manager_http_receiver::GetConnection()
//{
//	return _call( API_HTTPRECEIVER_GETCONNECTION, (api_connection *)NULL );
//}


inline void api_wac_download_manager_http_receiver::AllowCompression()
{
	_voidcall( API_HTTPRECEIVER_ALLOW_COMPRESSION );
}

inline const char *api_wac_download_manager_http_receiver::get_url()
{
	return _call( API_HTTPRECEIVER_GET_URL, (const char *)0 );
}


inline void api_wac_download_manager_http_receiver::set_sendbufsize( int sendbufsize )
{
	_voidcall( API_HTTPRECEIVER_SET_SENDBUFSIZE, sendbufsize );
}

inline void api_wac_download_manager_http_receiver::set_accept_all_reply_codes()
{
	_voidcall( API_HTTPRECEIVER_SET_ACCEPT_ALL_REPLY_CODES );
}

inline void api_wac_download_manager_http_receiver::set_persistent()
{
	_voidcall( API_HTTPRECEIVER_SET_PERSISTENT );
}

//////// {12475CD9-1BA3-4665-B395-F87DBF31E30F}
//////static const GUID httpreceiverGUID =
//////{ 0x12475cd9, 0x1ba3, 0x4665, { 0xb3, 0x95, 0xf8, 0x7d, 0xbf, 0x31, 0xe3, 0xf } };

// {1683C6B7-1F6A-4C56-8496-525A3F5929D9}
static const GUID httpreceiverGUID2 =
{ 0x1683c6b7, 0x1f6a, 0x4c56, { 0x84, 0x96, 0x52, 0x5a, 0x3f, 0x59, 0x29, 0xd9 } };


#endif  // !NULLSOFT_WAC_DOWNLOAD_MANAGER_API_HTTP_RECEIVER_H
