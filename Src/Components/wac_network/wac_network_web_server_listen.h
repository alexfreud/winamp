/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: wac_network_web_server_listen.h - JNL interface for opening a TCP listen
** License: see jnetlib.h
**
** Usage:
**   1. create a JNL_Listen object with the port and (optionally) the interface
**      to listen on.
**   2. call get_connect() to get any new connections (optionally specifying what
**      buffer sizes the connection should be created with)
**   3. check is_error() to see if an error has occured
**   4. call port() if you forget what port the listener is on.
**
*/

#ifndef NULLSOFT_WAC_NETWORK_WEB_SERVER_LISTEN_H
#define NULLSOFT_WAC_NETWORK_WEB_SERVER_LISTEN_H

#include "wac_network_connection.h"

class JNL_Listen
{
public:
    JNL_Listen();
    ~JNL_Listen();

    int Initialize( unsigned short port, sockaddr *which_interface = 0, int family = PF_UNSPEC );
    int Initialize( struct addrinfo *address, size_t index );

    WAC_Network_Connection *get_connect( size_t sendbufsize = 8192, size_t recvbufsize = 8192 );
    unsigned short get_port();
    int is_error( void )                                              { return ( m_socket < 0 ); }

    socklen_t get_address( sockaddr *address, socklen_t *address_len );

protected:
    SOCKET m_socket;
    unsigned short m_port;
};

#endif //!NULLSOFT_WAC_NETWORK_WEB_SERVER_LISTEN_H
