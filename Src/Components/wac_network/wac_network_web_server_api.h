#ifndef __WASABI_API_WEBSERV_H
#define __WASABI_API_WEBSERV_H

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"

#include "wac_network_dns_api.h"
#include "wac_network_onconncb_api.h"

class IPageGenerator;
class api_onconncb;
class WAC_Network_Connection;

class NOVTABLE api_webserv : public Dispatchable
{
protected:
	api_webserv()                                                     {}
	~api_webserv()                                                    {}

public:
	DISPATCH_CODES
	{
		API_WEBSERV_RUN               =  10,
		API_WEBSERV_SETMAXCONNECTIONS =  20,
		API_WEBSERV_SETREQUESTTIMEOUT =  30,
		API_WEBSERV_ADDLISTENPORT     =  40,
		API_WEBSERV_GETLISTENPORT     =  50,
		API_WEBSERV_REMOVELISTENPORT  =  60,
		API_WEBSERV_REMOVELISTENIDX   =  70,
		API_WEBSERV_ATTACHCONNECTION  =  80,
		API_WEBSERV_URLENCODE         = 100,
		API_WEBSERV_URLDECODE         = 110,
		API_WEBSERV_BASE64ENCODE      = 120,
		API_WEBSERV_BASE64DECODE      = 130,
		API_WEBSERV_PARSEAUTH         = 140,
		API_WEBSERV_SETCONNECTIONCB   = 150,
		API_WEBSERV_ALLOWCOMPRESSION  = 160,
	};

	//to make onConnection method derivable across dlls, user must make their own api_onconncb and make their own onConnection method
	void SetConnectionCallback( api_onconncb *newConnectionCallback );
	void AllowCompression();
	void run( void );

	void setMaxConnections( int max_con );
	void setRequestTimeout( int timeout_s );

	int  addListenPort( int port, unsigned long which_interface = 0 ); // TODO: add Protocol Family as parameter
	int  getListenPort( int idx, int *err = 0 );

	void removeListenPort( int port );
	void removeListenIdx( int idx );

	void attachConnection( WAC_Network_Connection *con, int port );

	void url_encode( char *in, char *out, int max_out );
	void url_decode( char *in, char *out, int maxlen );

	void base64decode( char *src, char *dest, int destsize );
	void base64encode( char *in, char *out );

	int  parseAuth( char *auth_header, char *out, int out_len );
};


inline void api_webserv::run( void )
{
	_voidcall( API_WEBSERV_RUN );
}

inline void api_webserv::setMaxConnections( int max_con )
{
	_voidcall( API_WEBSERV_SETMAXCONNECTIONS, max_con );
}

inline void api_webserv::setRequestTimeout( int timeout_s )
{
	_voidcall( API_WEBSERV_SETREQUESTTIMEOUT, timeout_s );
}

inline void api_webserv::AllowCompression()
{
	_voidcall( API_WEBSERV_ALLOWCOMPRESSION );
}

inline int api_webserv::addListenPort( int port, unsigned long which_interface )
{
	return _call( API_WEBSERV_ADDLISTENPORT, (int)0, port, which_interface );
}

inline int api_webserv::getListenPort( int idx, int *err )
{
	return _call( API_WEBSERV_GETLISTENPORT, (int)0, idx, err );
}

inline void api_webserv::removeListenPort( int port )
{
	_voidcall( API_WEBSERV_REMOVELISTENPORT, port );
}

inline void api_webserv::removeListenIdx( int idx )
{
	_voidcall( API_WEBSERV_REMOVELISTENIDX, idx );
}

inline void api_webserv::attachConnection( WAC_Network_Connection *con, int port )
{
	_voidcall( API_WEBSERV_ATTACHCONNECTION, con, port );
}

inline void api_webserv::url_encode( char *in, char *out, int max_out )
{
	_voidcall( API_WEBSERV_URLENCODE, in, out, max_out );
}

inline void api_webserv::url_decode( char *in, char *out, int maxlen )
{
	_voidcall( API_WEBSERV_URLDECODE, in, out, maxlen );
}

inline void api_webserv::base64decode( char *src, char *dest, int destsize )
{
	_voidcall( API_WEBSERV_BASE64DECODE, src, dest, destsize );
}

inline void api_webserv::base64encode( char *in, char *out )
{
	_voidcall( API_WEBSERV_BASE64ENCODE, in, out );
}

inline int api_webserv::parseAuth( char *auth_header, char *out, int out_len )
{
	return _call( API_WEBSERV_PARSEAUTH, (int)0, auth_header, out, out_len );
}

inline void api_webserv::SetConnectionCallback( api_onconncb *newConnectionCallback )
{
	_voidcall( API_WEBSERV_SETCONNECTIONCB, newConnectionCallback );
}

// {A8880018-C62A-4f58-8B43-F424C9C01787}
static const GUID webservGUID =
{ 0xa8880018, 0xc62a, 0x4f58, { 0x8b, 0x43, 0xf4, 0x24, 0xc9, 0xc0, 0x17, 0x87 } };

#endif  // !__WASABI_API_WEBSERV_H