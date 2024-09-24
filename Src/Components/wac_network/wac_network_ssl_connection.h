/*
** JNetLib
** Copyright (C) 2000-2006 CockOS, Inc.
** Author: Justin Frankel, Joshua Teitelbaum
** File: sslconnection.h - JNL SSL TCP connection interface
** License: see jnetlib.h
*/

#ifndef NULLSOFT_WAC_NETWORK_SSL_CONNECTION_H
#define NULLSOFT_WAC_NETWORK_SSL_CONNECTION_H

#include "netinc.h"
#include "util.h"

#include "wac_network_connection.h"

/*
**  AUTOLINK WITH THESE GUYS NOT IN PROJECT HEH HEH :)
**  Build and go, for you :)
*/

#include <openssl/ssl.h>

class JNL_SSL_Connection : public WAC_Network_Connection
{
protected:
	SSL *m_ssl;
	bool  m_bcontextowned;
	bool  m_bsslinit;

public:
	JNL_SSL_Connection();
	JNL_SSL_Connection( SSL *pssl, api_dns *dns, size_t sendbufsize, size_t recvbufsize );
	virtual ~JNL_SSL_Connection();

	virtual void connect( SOCKET sock, sockaddr *addr, socklen_t length /* of addr */ ); // used by the listen object, usually not needed by users.
	virtual void run( size_t max_send_bytes = -1, size_t max_recv_bytes = -1, size_t *bytes_sent = NULL, size_t *bytes_rcvd = NULL );

	/*
	**  Joshua Teitelbaum 1/27/2006 Adding new BSD socket analogues for SSL compatibility
	*/
protected:
	virtual void    socket_shutdown();
	virtual ssize_t socket_recv( char *buf, size_t len, int options );
	virtual ssize_t socket_send( const char *buf, size_t len, int options );
	virtual int     socket_connect();
	virtual void    on_socket_connected();
	/*
	**  init_ssl_connection:
	**  returns true if can continue onwards (could be error, in which case
	**  we want the error to cascade through).
	**  Else, false if connection was not made, and we have to call this
	**  again.
	*/
	bool init_ssl_connection();

	bool forceConnect;
};

extern SSL_CTX *sslContext;

#endif  //!NULLSOFT_WAC_NETWORK_SSL_CONNECTION_H
