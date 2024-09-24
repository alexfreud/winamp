#ifndef __API_HTTPSERV_H_
#define __API_HTTPSERV_H_

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"

#include "wac_network_connection_api.h"

class api_httpserv : public Dispatchable
{
protected:
	api_httpserv()                                                    {}
	~api_httpserv()                                                   {}

public:
	DISPATCH_CODES
	{
		API_HTTPSERV_RUN            =  10,
		API_HTTPSERV_GETERRSTR      =  20,
		API_HTTPSERV_GETREQUESTFILE =  30,
		API_HTTPSERV_GETREQUESTPARM =  40,
		API_HTTPSERV_GETALLHEADERS  =  50,
		API_HTTPSERV_GETHEADER      =  60,
		API_HTTPSERV_SETREPLYSTR    =  70,
		API_HTTPSERV_SETREPLYHEADER =  80,
		API_HTTPSERV_SENDREPLY      =  90,
		API_HTTPSERV_BYTESINQUEUE   = 100,
		API_HTTPSERV_BYTESCANSEND   = 110,
		API_HTTPSERV_WRITEBYTES     = 120,
		API_HTTPSERV_CLOSE          = 130,
		API_HTTPSERV_GETCON         = 140,
		API_HTTPSERV_GETMETHOD      = 150,
	};

	int             run();
	char           *geterrorstr();
	char           *get_request_file();
	char           *get_request_parm( char *parmname );
	char           *getallheaders();
	char           *getheader( char *headername );

	void            set_reply_string( char *reply_string );
	void            set_reply_header( char *header );
	void            send_reply();

	int             bytes_inqueue();
	int             bytes_cansend();

	void            write_bytes( char *bytes, int length );
	void            close( int quick );

	api_connection *get_con();
	const char     *get_method(); // GET or POST or HEAD or whatnot
};

inline int api_httpserv::run()
{
	return _call( API_HTTPSERV_RUN, (int)0 );
}

inline char *api_httpserv::geterrorstr()
{
	return _call( API_HTTPSERV_GETERRSTR, (char *)NULL );
}

inline char *api_httpserv::get_request_file()
{
	return _call( API_HTTPSERV_GETREQUESTFILE, (char *)NULL );
}

inline char *api_httpserv::get_request_parm( char *parmname )
{
	return _call( API_HTTPSERV_GETREQUESTPARM, (char *)NULL, parmname );
}

inline char *api_httpserv::getallheaders()
{
	return _call( API_HTTPSERV_GETALLHEADERS, (char *)NULL );
}

inline char *api_httpserv::getheader( char *headername )
{
	return _call( API_HTTPSERV_GETHEADER, (char *)NULL, headername );
}

inline void api_httpserv::set_reply_string( char *reply_string )
{
	_voidcall( API_HTTPSERV_SETREPLYSTR, reply_string );
}

inline void api_httpserv::set_reply_header( char *header )
{
	_voidcall( API_HTTPSERV_SETREPLYHEADER, header );
}

inline void api_httpserv::send_reply()
{
	_voidcall( API_HTTPSERV_SENDREPLY );
}

inline int api_httpserv::bytes_inqueue()
{
	return _call( API_HTTPSERV_BYTESINQUEUE, (int)0 );
}

inline int api_httpserv::bytes_cansend()
{
	return _call( API_HTTPSERV_BYTESCANSEND, (int)0 );
}

inline void api_httpserv::write_bytes( char *bytes, int length )
{
	_voidcall( API_HTTPSERV_WRITEBYTES, bytes, length );
}

inline void api_httpserv::close( int quick )
{
	_voidcall( API_HTTPSERV_CLOSE, quick );
}

inline api_connection *api_httpserv::get_con()
{
	return _call( API_HTTPSERV_GETCON, (api_connection *)NULL );
}

inline const char *api_httpserv::get_method()
{
	return _call( API_HTTPSERV_GETMETHOD, (const char *)0 );
}

// {105ADFDC-0D82-44f0-AA52-FB911401A04C}
static const GUID httpservGUID =
{ 0x105adfdc, 0xd82, 0x44f0, { 0xaa, 0x52, 0xfb, 0x91, 0x14, 0x1, 0xa0, 0x4c } };

#endif