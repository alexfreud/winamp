/*
** JNetLib
** Copyright (C) 2000-2013 Nullsoft, Inc.
** Author: Justin Frankel, Ben Allison
** File: listen.cpp - JNL TCP listen implementation
** License: see jnetlib.h
*/

#include "foundation/error.h"

#include "netinc.h"
#include "util.h"

#include "wac_network_web_server_listen.h"


JNL_Listen::JNL_Listen()
{
	m_port = 0;
	m_socket = -1;
}

int JNL_Listen::Initialize( unsigned short port, sockaddr *which_interface, int family )
{
	m_port = port;

	char portString[ 32 ] = { 0 };
	sprintf( portString, "%d", (int)port );

	addrinfo *res;

	addrinfo hints;
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
	hints.ai_addr = which_interface ? which_interface : INADDR_ANY;

	if ( getaddrinfo( NULL, portString, &hints, &res ) == 0 )
	{
		int ret = Initialize( res, 0 );
		freeaddrinfo( res );

		return ret;
	}
	else
	{
		return NErr_Error;
	}
}

int JNL_Listen::Initialize( addrinfo *address, size_t index )
{
	addrinfo *res = address;

	while ( index-- )
	{
		res = res->ai_next;
		if ( !res )
			return NErr_EndOfEnumeration;
	}

	m_socket = ::socket( res->ai_family, res->ai_socktype, res->ai_protocol );
	if ( m_socket < 0 )
	{
		freeaddrinfo( res );
		return NErr_Error;
	}
	else
	{
		SET_SOCK_BLOCK( m_socket, 0 );

#ifndef _WIN32
		int bflag = 1;
		setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, &bflag, sizeof( bflag ) );
#endif

		if ( ::bind( m_socket, res->ai_addr, (int)res->ai_addrlen ) )
		{
			closesocket( m_socket );
			m_socket = -1;

			return NErr_Error;
		}
		else if ( ::listen( m_socket, 8 ) == -1 )
		{
			closesocket( m_socket );
			m_socket = -1;

			return NErr_Error;
		}
	}

	return NErr_Success;
}

JNL_Listen::~JNL_Listen()
{
	if ( m_socket >= 0 )
	{
		closesocket( m_socket );
	}
}

WAC_Network_Connection *JNL_Listen::get_connect( size_t sendbufsize, size_t recvbufsize )
{
	if ( m_socket < 0 )
		return NULL;

	sockaddr_storage saddr;
	socklen_t length = sizeof( saddr );

	SOCKET s = accept( m_socket, (sockaddr *)&saddr, &length );
	if ( s != -1 )
	{
		WAC_Network_Connection *c = new WAC_Network_Connection( NULL, sendbufsize, recvbufsize );
		c->connect( s, (sockaddr *)&saddr, length );

		return c;
	}

	return NULL;
}

socklen_t JNL_Listen::get_address( sockaddr *address, socklen_t *address_len )
{
	return getsockname( m_socket, address, address_len );
}

unsigned short JNL_Listen::get_port()
{
	if ( !m_port )
	{
		sockaddr_in address;
		socklen_t namelen = sizeof( address );
		if ( getsockname( m_socket, (sockaddr *)&address, &namelen ) == 0 )
			m_port = ntohs( address.sin_port );
	}

	return m_port;
}