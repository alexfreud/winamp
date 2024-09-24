/*
** JNetLib
** Copyright (C) 2000-2003 Nullsoft, Inc.
** Author: Justin Frankel
** File: wac_network_web_server.cpp - Generic simple webserver baseclass
** License: see jnetlib.h
** see test.cpp for an example of how to use this class
*/

#include "wac_network_web_server.h"
#include <zlib.h>
#include "../nu/strsafe.h"

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

class WS_ItemList
{
	public:
	WS_ItemList()
	{
		m_size = 0; m_list = 0;
	}
	~WS_ItemList()
	{
		::free( m_list );
	}

	void *Add( void *i );
	void *Get( int w );
	void  Del( int idx );

	int   GetSize( void )
	{
		return m_size;
	}

	protected:
	void **m_list;
	int    m_size;
};

class WS_conInst
{
	public:
	WS_conInst( WAC_Network_Connection *c, JNL_Listen *l, int which_port );
	~WS_conInst();

	void close();
	// these will be used by WebServerBaseClass::onConnection yay
	wa::Components::WAC_Network_HTTP_Server       m_serv;
	api_pagegenerator *m_pagegen;
	api_onconncb *connCb;
	JNL_Listen *m_listener;
	int                m_port; // port this came in on
	time_t             m_connect_time;

	z_stream *zlibStream;
};



void *WS_ItemList::Add( void *i )
{
	if ( !m_list || !( m_size & 31 ) )
		m_list = ( void ** )::realloc( m_list, sizeof( void * ) * ( m_size + 32 ) );

	m_list[ m_size++ ] = i;

	return i;
}

void *WS_ItemList::Get( int w )
{
	if ( w >= 0 && w < m_size )
		return m_list[ w ];

	return NULL;
}

void WS_ItemList::Del( int idx )
{
	if ( m_list && idx >= 0 && idx < m_size )
	{
		m_size--;
		if ( idx != m_size )
			::memcpy( m_list + idx, m_list + idx + 1, sizeof( void * ) * ( m_size - idx ) );

		if ( !( m_size & 31 ) && m_size ) // resize down
		{
			m_list = ( void ** )::realloc( m_list, sizeof( void * ) * m_size );
		}
	}
}


WS_conInst::WS_conInst( WAC_Network_Connection *c, JNL_Listen *l, int which_port ) : m_serv( c ), m_listener( l ), m_port( which_port ), connCb( 0 )
{
	zlibStream = 0;
	m_pagegen = 0;
	time( &m_connect_time );
}

WS_conInst::~WS_conInst()
{
	if ( connCb && m_pagegen )
		connCb->destroyConnection( m_pagegen );

	if ( zlibStream )
		deflateEnd( zlibStream );

	free( zlibStream );
}

void WS_conInst::close()
{
	if ( connCb && m_pagegen )
		connCb->destroyConnection( m_pagegen );

	m_pagegen = 0;

	if ( zlibStream )
		deflateEnd( zlibStream );

	free( zlibStream );
	zlibStream = 0;

	time( &m_connect_time );
}




WebServerBaseClass::~WebServerBaseClass()
{
	int x;
	for ( x = 0; x < m_connections->GetSize(); x++ )
	{
		delete (WS_conInst *)m_connections->Get( x );
	}

	delete m_connections;

	for ( x = 0; x < m_listeners->GetSize(); x++ )
	{
		delete (JNL_Listen *)m_listeners->Get( x );
	}

	delete m_listeners;
}

WebServerBaseClass::WebServerBaseClass()
{
	allowCompression = false;
	m_connections = new WS_ItemList;
	m_listeners = new WS_ItemList;
	m_listener_rot = 0;
	m_timeout_s = 15;
	m_max_con = 100;
	extraDeflateBuffer = NULL;
	extraDeflateBufferSize = 0;
	extraDeflateBufferUsed = 0;
	connectionCallback = NULL;
}

void WebServerBaseClass::initForFactory()
{
	if ( m_connections )
	{
		for ( int x = 0; x < m_connections->GetSize(); x++ )
			delete (WS_conInst *)m_connections->Get( x );

		delete m_connections;
	}

	if ( m_listeners )
	{
		for ( int x = 0; x < m_listeners->GetSize(); x++ )
			delete (JNL_Listen *)m_listeners->Get( x );

		delete m_listeners;
	}

	m_connections = new WS_ItemList;
	m_listeners = new WS_ItemList;
	m_listener_rot = 0;
	m_timeout_s = 15;
	m_max_con = 100;
}


void WebServerBaseClass::setMaxConnections( int max_con )
{
	m_max_con = max_con;
}

void WebServerBaseClass::setRequestTimeout( int timeout_s )
{
	m_timeout_s = timeout_s;
}

void WebServerBaseClass::AllowCompression()
{
	allowCompression = true;
}

int WebServerBaseClass::addListenPort( int port, unsigned long which_interface )
{
	removeListenPort( port );

	sockaddr_in address;
	memset( &address, 0, sizeof( address ) );
	address.sin_family = AF_INET;
	address.sin_port = htons( (unsigned short)port );
	address.sin_addr.s_addr = which_interface ? which_interface : INADDR_ANY;

	JNL_Listen *p = new JNL_Listen();
	p->Initialize( port, (sockaddr *)&address, PF_INET );
	m_listeners->Add( (void *)p );

	if ( p->is_error() )
		return -1;

	return 0;
}

void WebServerBaseClass::removeListenPort( int port )
{
	for ( int x = 0; x < m_listeners->GetSize(); x++ )
	{
		JNL_Listen *p = (JNL_Listen *)m_listeners->Get( x );
		if ( p->get_port() == port )
		{
			delete p;
			m_listeners->Del( x );

			break;
		}
	}
}

void WebServerBaseClass::removeListenIdx( int idx )
{
	if ( idx >= 0 && idx < m_listeners->GetSize() )
	{
		JNL_Listen *p = (JNL_Listen *)m_listeners->Get( idx );
		delete p;

		m_listeners->Del( idx );
	}
}

int WebServerBaseClass::getListenPort( int idx, int *err )
{
	JNL_Listen *p = (JNL_Listen *)m_listeners->Get( idx );
	if ( p )
	{
		if ( err )
			*err = p->is_error();

		return p->get_port();
	}

	return 0;
}

void WebServerBaseClass::attachConnection( WAC_Network_Connection *con, JNL_Listen *listener )
{
	m_connections->Add( ( void * )new WS_conInst( con, listener, listener->get_port() ) );
}

void WebServerBaseClass::run( void )
{
	int nl;
	if ( m_connections->GetSize() < m_max_con && ( nl = m_listeners->GetSize() ) )
	{
		JNL_Listen *l = (JNL_Listen *)m_listeners->Get( m_listener_rot++ % nl );

		WAC_Network_Connection *c = l->get_connect( PACKET_SIZE, PACKET_SIZE );
		if ( c )
			attachConnection( c, l );
	}

	for ( int x = 0; x < m_connections->GetSize(); x++ )
	{
		WS_conInst *this_con = (WS_conInst *)m_connections->Get( x );
		switch ( run_connection( this_con ) )
		{
			case RUN_CONNECTION_DONE:
			{
				bool keep_alive = false;
				if ( this_con->m_serv.get_keep_alive() )
				{
					const char *connection_status = this_con->m_serv.getheader( "Connection" );
					if ( !connection_status || _stricmp( connection_status, "close" ) )
						keep_alive = true;
				}

				if ( !keep_alive )
				{
					delete this_con;
					m_connections->Del( x-- );
				}
				else
				{
					this_con->close();
					this_con->m_serv.reset();
				}
			}
			break;
			case RUN_CONNECTION_ERROR:
			case RUN_CONNECTION_TIMEOUT:
				delete this_con;
				m_connections->Del( x-- );
				break;
		}
	}
}

void WebServerBaseClass::SetConnectionCallback( api_onconncb *_connectionCallback )
{
	_connectionCallback->caller = this;
	connectionCallback = _connectionCallback;
}

void WebServerBaseClass::deflatehelper( WS_conInst *con, char *buf, int l, bool flush )
{
	char temp[ 8192 ] = { 0 };
	con->zlibStream->next_in = (Bytef *)buf;
	con->zlibStream->avail_in = l;
	con->zlibStream->avail_out = 0;

	int sent = 0;
	if ( flush )
		l = con->m_serv.bytes_cansend();

	while ( ( !flush && con->zlibStream->avail_in ) || ( flush && !con->zlibStream->avail_out ) )
	{
		con->zlibStream->next_out = (Bytef *)temp;
		con->zlibStream->avail_out = 8192;
		int r = deflate( con->zlibStream, flush ? Z_FINISH : Z_NO_FLUSH ); // TODO: if avail_in != 0, we might have to use Z_SYNC_FLUSH instead
		if ( sent + 8192 - (int)con->zlibStream->avail_out > l )
		{
			if ( !extraDeflateBuffer )
			{
				extraDeflateBufferSize = 8192 - con->zlibStream->avail_out;
				extraDeflateBuffer = (char *)malloc( extraDeflateBufferSize );
				memcpy( extraDeflateBuffer, temp, extraDeflateBufferSize );
			}
			else
			{
				// TODO need to sort out this to handle realloc failures...
				extraDeflateBuffer = (char *)realloc( extraDeflateBuffer, extraDeflateBufferSize + 8192 - con->zlibStream->avail_out );
				memcpy( extraDeflateBuffer + extraDeflateBufferSize, temp, 8192 - con->zlibStream->avail_out );
				extraDeflateBufferSize += 8192 - con->zlibStream->avail_out;
			}
		}
		else
		{
			con->m_serv.write_bytes( temp, 8192 - con->zlibStream->avail_out );
			sent += 8192 - con->zlibStream->avail_out;
		}

		if ( r != Z_OK )
			break;
	}
}

int WebServerBaseClass::run_connection( WS_conInst *con )
{
	// TODO: add a Run() method to WC_conInst, passing in connectionCallback
	int s = con->m_serv.run();
	if ( s < 0 )
	{
		// m_serv.geterrorstr()
		return RUN_CONNECTION_ERROR;
	}

	if ( s < 2 )
	{
		// return 1 if we timed out
		if ( time( NULL ) - con->m_connect_time > m_timeout_s )
			return RUN_CONNECTION_TIMEOUT;
		else
			return RUN_CONNECTION_CONTINUE;
	}

	if ( s < 3 )
	{
		con->connCb = connectionCallback;
		con->m_pagegen = connectionCallback->onConnection( &con->m_serv, con->m_port );

		const char *compression0 = allowCompression ? con->m_serv.getheader( "Accept-Encoding" ) : 0;
		if ( compression0 )
		{
			// the compression0 string is a comma (and possibly space) separated list of acceptable formats
			char *compression = _strdup( compression0 );
			bool supportsGzip = false, supportsDeflate = false;
			size_t l = strlen( compression );
			char *p = compression;
			while ( p && *p )
			{
				if ( *p == ' ' || *p == ',' ) *p = 0; p++;
			}

			for ( char *q = compression; q < compression + l; q++ )
			{
				if ( !_stricmp( q, "deflate" ) )
				{
					supportsDeflate = true;
					q += 7;
				}

				if ( !_stricmp( q, "gzip" ) )
				{
					supportsGzip = true;
					q += 4;
				}
			}

			free( compression );

			int windowBits = 0;

			if ( supportsGzip )
			{
				windowBits = 15 + 16; /* +16 for gzip	*/
				compression = _strdup( "gzip" );
			}
			else if ( supportsDeflate )
			{
				windowBits = 15;
				compression = _strdup( "deflate" );
			}

			if ( windowBits )
			{
				con->zlibStream = (z_stream *)malloc( sizeof( z_stream ) );
				con->zlibStream->next_in = Z_NULL;
				con->zlibStream->avail_in = Z_NULL;
				con->zlibStream->next_out = Z_NULL;
				con->zlibStream->avail_out = Z_NULL;
				con->zlibStream->zalloc = (alloc_func)0;
				con->zlibStream->zfree = (free_func)0;
				con->zlibStream->opaque = 0;

				int z_err = deflateInit2( con->zlibStream,
										  Z_DEFAULT_COMPRESSION,
										  Z_DEFLATED,
										  15 + 16 /* +16 for gzip */,
										  8,
										  Z_DEFAULT_STRATEGY );

				if ( z_err != Z_OK )
				{
					free( con->zlibStream );
					con->zlibStream = 0;
				}

				char temp_header[ 1024 ] = "Content-Encoding:";
				StringCchCatA( temp_header, 1024, compression );
				con->m_serv.set_reply_header( temp_header );
			}
		}

		return RUN_CONNECTION_CONTINUE;
	}

	if ( s < 4 )
	{
		if ( !con->m_pagegen )
		{
			return con->m_serv.bytes_inqueue() ? RUN_CONNECTION_CONTINUE : RUN_CONNECTION_DONE;
		}

		int l = con->m_serv.bytes_cansend();
		if ( l > 0 )
		{
			if ( con->zlibStream && extraDeflateBufferSize )
			{
				int len = MIN( extraDeflateBufferSize - extraDeflateBufferUsed, l );
				con->m_serv.write_bytes( extraDeflateBuffer + extraDeflateBufferUsed, len );
				extraDeflateBufferUsed += len;
				if ( extraDeflateBufferUsed == extraDeflateBufferSize )
				{
					extraDeflateBufferUsed = 0;
					extraDeflateBufferSize = 0;

					free( extraDeflateBuffer );
					extraDeflateBuffer = NULL;
				}

				l -= len;
				if ( l == 0 )
					return con->m_serv.bytes_inqueue() ? RUN_CONNECTION_CONTINUE : RUN_CONNECTION_DONE;
			}

			char buf[ PACKET_SIZE ] = { 0 };
			if ( l > sizeof( buf ) )
				l = sizeof( buf );

			l = con->m_pagegen->GetData( buf, l );
			if ( l < ( con->m_pagegen->IsNonBlocking() ? 0 : 1 ) ) // if nonblocking, this is l < 0, otherwise it's l<1
			{
				if ( con->zlibStream )
				{
					deflatehelper( con, 0, 0, true );
					/*
					char temp[8192] = {0};
					con->zlibStream->next_in = 0;
					con->zlibStream->avail_in = 0;
					con->zlibStream->avail_out = 0;
					while(!con->zlibStream->avail_out)
					{
						con->zlibStream->next_out = (Bytef *)temp;
						con->zlibStream->avail_out = 8192;
						int r = deflate(con->zlibStream, Z_FINISH);
						con->m_serv.write_bytes(temp, 8192 - con->zlibStream->avail_out);
						if(r != Z_OK) break;
					}*/
				}

				return con->m_serv.bytes_inqueue() ? RUN_CONNECTION_CONTINUE : RUN_CONNECTION_DONE;
			}

			if ( l > 0 )
			{
				if ( con->zlibStream )
					deflatehelper( con, buf, l, false );
				else
					con->m_serv.write_bytes( buf, l );
			}
		}

		return RUN_CONNECTION_CONTINUE;
	}

	return RUN_CONNECTION_DONE; // we're done by this point
}

void WebServerBaseClass::url_encode( char *in, char *out, int max_out )
{
	while ( in && *in && max_out > 4 )
	{
		if ( ( *in >= 'A' && *in <= 'Z' ) ||
			 ( *in >= 'a' && *in <= 'z' ) ||
			 ( *in >= '0' && *in <= '9' ) || *in == '.' || *in == '_' || *in == '-' )
		{
			*out++ = *in++;
			max_out--;
		}
		else
		{
			int i = *in++;
			*out++ = '%';

			int b = ( i >> 4 ) & 15;
			if ( b < 10 )
				*out++ = '0' + b;
			else
				*out++ = 'A' + b - 10;

			b = i & 15;
			if ( b < 10 )
				*out++ = '0' + b;
			else
				*out++ = 'A' + b - 10;

			max_out -= 3;

		}
	}

	*out = 0;
}

void WebServerBaseClass::url_decode( char *in, char *out, int maxlen )
{
	while ( in && *in && maxlen > 1 )
	{
		if ( *in == '+' )
		{
			in++;
			*out++ = ' ';
		}
		else if ( *in == '%' && in[ 1 ] != '%' && in[ 1 ] )
		{
			int a = 0;
			int b = 0;
			for ( b = 0; b < 2; b++ )
			{
				int r = in[ 1 + b ];
				if ( r >= '0' && r <= '9' )
					r -= '0';
				else if ( r >= 'a' && r <= 'z' )
					r -= 'a' - 10;
				else if ( r >= 'A' && r <= 'Z' )
					r -= 'A' - 10;
				else
					break;

				a *= 16;
				a += r;
			}

			if ( b < 2 )
				*out++ = *in++;
			else
			{
				*out++ = a;
				in += 3;
			}
		}
		else
			*out++ = *in++;

		maxlen--;
	}

	*out = 0;
}

void WebServerBaseClass::base64decode( char *src, char *dest, int destsize )
{
	int accum = 0;
	int nbits = 0;
	while ( src && *src )
	{
		int x = 0;
		char c = *src++;
		if ( c >= 'A' && c <= 'Z' )
			x = c - 'A';
		else if ( c >= 'a' && c <= 'z' )
			x = c - 'a' + 26;
		else if ( c >= '0' && c <= '9' )
			x = c - '0' + 52;
		else if ( c == '+' )
			x = 62;
		else if ( c == '/' )
			x = 63;
		else
			break;

		accum <<= 6;
		accum |= x;
		nbits += 6;

		while ( nbits >= 8 )
		{
			if ( --destsize <= 0 )
				break;

			nbits -= 8;
			*dest++ = (char)( ( accum >> nbits ) & 0xff );
		}
	}

	*dest = 0;
}

void WebServerBaseClass::base64encode( char *in, char *out )
{
	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int shift = 0;
	int accum = 0;

	while ( in && *in )
	{
		if ( *in )
		{
			accum <<= 8;
			shift += 8;
			accum |= *in++;
		}

		while ( shift >= 6 )
		{
			shift -= 6;
			*out++ = alphabet[ ( accum >> shift ) & 0x3F ];
		}
	}

	if ( shift == 4 )
	{
		*out++ = alphabet[ ( accum & 0xF ) << 2 ];
		*out++ = '=';
	}
	else if ( shift == 2 )
	{
		*out++ = alphabet[ ( accum & 0x3 ) << 4 ];
		*out++ = '=';
		*out++ = '=';
	}

	*out++ = 0;
}

int WebServerBaseClass::parseAuth( char *auth_header, char *out, int out_len ) //returns 0 on unknown auth, 1 on basic
{
	char *authstr = auth_header;
	*out = 0;
	if ( !auth_header || !*auth_header )
		return 0;

	while ( authstr && *authstr && *authstr == ' ' )
	{
		authstr++;
	}

	if ( _strnicmp( authstr, "basic ", 6 ) )
		return 0;

	authstr += 6;

	while ( authstr && *authstr && *authstr == ' ' )
		authstr++;

	base64decode( authstr, out, out_len );

	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WebServerBaseClass
START_DISPATCH
VCB( API_WEBSERV_RUN, run )
VCB( API_WEBSERV_SETMAXCONNECTIONS, setMaxConnections )
VCB( API_WEBSERV_SETREQUESTTIMEOUT, setRequestTimeout )
CB( API_WEBSERV_ADDLISTENPORT, addListenPort )
CB( API_WEBSERV_GETLISTENPORT, getListenPort )
VCB( API_WEBSERV_REMOVELISTENPORT, removeListenPort )
VCB( API_WEBSERV_REMOVELISTENIDX, removeListenIdx )
VCB( API_WEBSERV_ATTACHCONNECTION, attachConnection )
VCB( API_WEBSERV_URLENCODE, wasabi_url_encode )
VCB( API_WEBSERV_URLDECODE, wasabi_url_decode )
VCB( API_WEBSERV_BASE64ENCODE, wasabi_base64encode )
VCB( API_WEBSERV_BASE64DECODE, wasabi_base64decode )
CB( API_WEBSERV_PARSEAUTH, wasabi_parseAuth )
VCB( API_WEBSERV_SETCONNECTIONCB, SetConnectionCallback );
VCB( API_WEBSERV_ALLOWCOMPRESSION, AllowCompression )
END_DISPATCH