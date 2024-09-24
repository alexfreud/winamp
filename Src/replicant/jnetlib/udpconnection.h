/*
** JNetLib
** Copyright (C) 2000-2001 Nullsoft, Inc.
** Author: Justin Frankel
** File: udpconnection.h - JNL UDP connection interface
** License: see jnetlib.h
**
** Usage:
**   1. Create a JNL_Connection object, optionally specifying a JNL_AsyncDNS
**      object to use (or NULL for none, or WAC_NETWORK_CONNECTION_AUTODNS for auto),
**      and the send and receive buffer sizes.
**   2. Call connect() to have it connect to a host/port (the hostname will be 
**      resolved if possible).
**   3. call run() with the maximum send/recv amounts, and optionally parameters
**      so you can tell how much has been send/received. You want to do this a lot, while:
**   4. check get_state() to check the state of the connection. The states are:
**        JNL_Connection::STATE_ERROR
**          - an error has occured on the connection. the connection has closed,
**            and you can no longer write to the socket (there still might be 
**            data in the receive buffer - use recv_bytes_available()). 
**        JNL_Connection::STATE_NOCONNECTION
**          - no connection has been made yet. call connect() already! :)
**        JNL_Connection::STATE_RESOLVING
**          - the connection is still waiting for a JNL_AsycnDNS to resolve the
**            host. 
**        JNL_Connection::STATE_CONNECTING
**          - the asynchronous call to connect() is still running.
**        JNL_Connection::STATE_CONNECTED
**          - the connection has connected, all is well.
**        JNL_Connection::STATE_CLOSING
**          - the connection is closing. This happens after a call to close,
**            without the quick parameter set. This means that the connection
**            will close once the data in the send buffer is sent (data could
**            still be being received when it would be closed). After it is 
**            closed, the state will transition to:
**        JNL_Connection::STATE_CLOSED
**          - the connection has closed, generally without error. There still
**            might be data in the receieve buffer, use recv_bytes_available().
**   5. Use send() and send_string() to send data. You can use 
**      send_bytes_in_queue() to see how much has yet to go out, or 
**      send_bytes_available() to see how much you can write. If you use send()
**      or send_string() and not enough room is available, both functions will 
**      return error ( < 0)
**   6. Use recv() and recv_line() to get data. If you want to see how much data 
**      there is, use recv_bytes_available() and recv_lines_available(). If you 
**      call recv() and not enough data is available, recv() will return how much
**      data was actually read. See comments at the function defs.
**
**   7. To close, call close(1) for a quick close, or close() for a close that will
**      make the socket close after sending all the data sent. 
**  
**   8. delete ye' ol' object.
*/

#ifndef _UDPCONNECTION_H_
#define _UDPCONNECTION_H_

#include "asyncdns.h"
#include "nu/RingBuffer.h"
#include "jnetlib_defines.h"

#define JNL_DEFAULT_BUFFER_SIZE 8192

class JNL_UDPConnection : private Drainer, private Filler
{
public:
    typedef enum
    {
		STATE_ERROR        = JNL_CONNECTION_STATE_ERROR,
        STATE_NOCONNECTION = JNL_CONNECTION_STATE_NOCONNECTION,
        STATE_RESOLVING    = JNL_CONNECTION_STATE_RESOLVING,
        STATE_CONNECTING   = JNL_CONNECTION_STATE_CONNECTING,
        STATE_CONNECTED    = JNL_CONNECTION_STATE_CONNECTED,
        STATE_CLOSING      = JNL_CONNECTION_STATE_CLOSING,
        STATE_CLOSED       = JNL_CONNECTION_STATE_CLOSED,
        STATE_RESOLVED     = JNL_CONNECTION_STATE_RESOLVED,
    } state;

    JNL_UDPConnection();
    JNL_UDPConnection( unsigned short port, JNL_AsyncDNS *dns, int sendbufsize = JNL_DEFAULT_BUFFER_SIZE, int recvbufsize = JNL_DEFAULT_BUFFER_SIZE );
    ~JNL_UDPConnection();

    void         open( JNL_AsyncDNS *dns = JNL_AUTODNS, size_t sendbufsize = JNL_DEFAULT_BUFFER_SIZE, size_t recvbufsize = JNL_DEFAULT_BUFFER_SIZE );
    void         open( int incoming_socket, JNL_AsyncDNS *dns = JNL_AUTODNS, size_t sendbufsize = JNL_DEFAULT_BUFFER_SIZE, size_t recvbufsize = JNL_DEFAULT_BUFFER_SIZE );

    void         setpeer( const char *hostname, int port );
    void         setpeer( sockaddr *addr, socklen_t length /* of addr */ );

    void         run( size_t max_send_bytes = -1, size_t max_recv_bytes = -1, size_t *bytes_sent = NULL, size_t *bytes_rcvd = NULL );

    int          get_state()                                                 { return m_state; }
    const char  *get_errstr()                                                { return m_errorstr; }

    void         close( int quick = 0 );
    void         flush_send( void )                                          { send_buffer.clear(); }

    size_t       send_bytes_in_queue( void );
    size_t       send_bytes_available( void );
    int          send( const void *data, size_t length );                    // returns -1 if not enough room
    int          send_string( const char *line );                            // returns -1 if not enough room


    size_t       recv_bytes_available( void );
    size_t       recv_bytes( void *data, size_t maxlength );                 // returns actual bytes read
    unsigned int recv_int( void );
    int          recv_lines_available( void );
    int          recv_line( char *line, size_t maxlength );                  // returns 0 if the line was terminated with a \r or \n, 1 if not.
                                                                             // (i.e. if you specify maxlength=10, and the line is 12 bytes long
                                                                             // it will return 1. or if there is no \r or \n and that's all the data
                                                                             // the connection has.)
    size_t       peek_bytes( void *data, size_t maxlength );                 // returns bytes peeked

    int          get_interface( sockaddr *sin, socklen_t *sin_length );      // this returns the interface the connection is on
    short        get_remote_port( void )                                     { return m_remote_port; } // this returns the remote port of connection

    void         get_last_recv_msg_addr( sockaddr **addr, socklen_t *len )   { *addr = (sockaddr *)&m_last_addr; *len = m_last_addr_len; }

    void         set_ttl( uint8_t new_ttl );

protected:
    uint8_t           ttl;
    SOCKET            m_socket;
    unsigned short    m_remote_port;
    RingBuffer        recv_buffer;
    RingBuffer        send_buffer;

    sockaddr         *address;
    socklen_t         address_len;

    sockaddr_storage  m_last_addr;
    socklen_t         m_last_addr_len;
    addrinfo         *saddr;

    char             *m_host;

    JNL_AsyncDNS     *m_dns;
    int               m_dns_owned;

    state             m_state;
    const char       *m_errorstr;

private:
    void init(); // constructor helper function

    // functions for RingBuffer
    size_t Read( void *dest, size_t len );
    size_t Write( const void *dest, size_t len );

};

#endif // _UDPConnection_H_
