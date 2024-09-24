/*
** JNetLib
** Copyright (C) 2012 Nullsoft, Inc.
** Author: Ben Allison
** File: httpuserv.h - JNL interface for doing HTTPU (HTTP over UDP)
** This is half-baked so far.  Need to think things through a touch more
*/

#pragma once

#include "udpconnection.h"
#include "headers.h"

class JNL_HTTPUServ
{
public:
    JNL_HTTPUServ();
    ~JNL_HTTPUServ();

    // pass this a connection that has just received a packet
    int         process( JNL_UDPConnection *m_con );

    const char *geterrorstr()                                         { return m_errstr; }

    // use these when state returned by run() is 2 
    const char *get_request_uri();                            // file portion of http request
    const char *get_request_parm( const char *parmname );     // parameter portion (after ?)
    const char *getallheaders()                                       { return recvheaders.GetAllHeaders(); } // double null terminated, null delimited list
    const char *getheader( const char *headername );
    const char *get_method()                                          { return m_method; }

    void        set_reply_string( const char *reply_string ); // should be HTTP/1.1 OK or the like
    void        set_reply_header( const char *header );       // i.e. "content-size: 12345"

    void        send_reply( JNL_UDPConnection *m_con );       // sends a reply to the given UDP socket.  it must have been setup beforehand with the appropriate peer

    void reset();                                             // prepare for another request 

    int get_http_version()                                            { return http_ver; }

protected:
    void seterrstr( const char *str )                                 { if ( m_errstr ) free( m_errstr ); m_errstr = _strdup( str ); }

    int          m_reply_ready;
    int          http_ver;

    char        *m_errstr;
    char        *m_reply_headers;
    char        *m_reply_string;
    JNL_Headers  recvheaders;
    char        *m_recv_request; // either double-null terminated, or may contain parameters after first null.
    char        *m_method;
};


