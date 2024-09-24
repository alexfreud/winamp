/*
** JNetLib
** Copyright (C) 2008 Nullsoft, Inc.
** Author: Ben Allison
** File: multicastlisten.h - JNL interface for opening a Multicast UDP listen
** License: see jnetlib.h
**
** Usage:
**   1. create a JNL_MulticastUDPListen object with the port and (optionally) the interface
**      to listen on.
**   2. call get_connect() to get the associated UDP connection (optionally specifying what
**      buffer sizes the connection should be created with).  Unlike TCP listen, there is only one listen object.
**   3. check is_error() to see if an error has occured
**   4. call port() if you forget what port the listener is on.
**
*/

#ifndef _MULTICASTLISTEN_H_
#define _MULTICASTLISTEN_H_

#include "udpconnection.h"


int CreateMulticastListener( JNL_UDPConnection **connection, const char *mcast_ip, unsigned short port, size_t sendbufsize = 8192, size_t recvbufsize = 8192 );

#endif //_LISTEN_H_
