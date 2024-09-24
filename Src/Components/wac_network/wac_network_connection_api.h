#ifndef NULLSOFT_COMPONENT_WAC_NETWORK_CONNECTION_H
#define NULLSOFT_COMPONENT_WAC_NETWORK_CONNECTION_H

#define PACKET_SIZE 16384

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"

#include "wac_network_dns_api.h"

enum
{
	CONNECTION_STATE_ERROR,
	CONNECTION_STATE_NOCONNECTION,
	CONNECTION_STATE_RESOLVING,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTED,
	CONNECTION_STATE_CLOSING,
	CONNECTION_STATE_CLOSED,
	CONNECTION_STATE_RESOLVED, // happens after RESOLVING, but going here for compatability
};

class api_dns;

class NOVTABLE api_connection : public Dispatchable
{
protected:
	api_connection()                                                  {}
	~api_connection()                                                 {}

public:
	DISPATCH_CODES
	{
		API_CONNECTION_OPEN                     =  10,
		API_CONNECTION_CONNECT                  =  20,
		API_CONNECTION_RUN                      =  30,
		API_CONNECTION_GETSTATE                 =  40,
		API_CONNECTION_GETERROR                 =  50,
		API_CONNECTION_CLOSE                    =  60,
		API_CONNECTION_FLUSHSEND                =  70,
		API_CONNECTION_GETSENDBYTESINQUEUE      =  80,
		API_CONNECTION_GETSENDBYTESAVAILABLE    =  90,
		API_CONNECTION_SEND                     = 100,
		API_CONNECTION_SENDBYTES                = 110,
		API_CONNECTION_SENDSTRING               = 120,
		API_CONNECTION_GETRECEIVEBYTESAVAILABLE = 130,
		API_CONNECTION_RECEIVEBYTES             = 140,
//	    API_CONNECTION_RECEIVEINTEGER           = 150,
		API_CONNECTION_GETRECEIVELINESAVAILABLE = 160,
		API_CONNECTION_RECEIVELINE              = 170,
		API_CONNECTION_PEEKBYTES                = 180,
		API_CONNECTION_GETINTERFACE             = 190,
		API_CONNECTION_GETREMOTEADDRESS         = 200,
		API_CONNECTION_GETREMOTEPORT            = 210,
	};

	void          Open( api_dns *dns = API_DNS_AUTODNS, size_t sendbufsize = 8192, size_t recvbufsize = 8192 );
	void          connect( char *hostname, int port );
	void          run( int max_send_bytes = -1, int max_recv_bytes = -1, int *bytes_sent = NULL, int *bytes_rcvd = NULL );
	int           get_state();
	char         *GetError();
	void          Close( int quick = 0 );
	void          FlushSend( void );

	size_t        GetSendBytesInQueue( void );
	size_t        GetSendBytesAvailable( void );

	int           send( const void *data, int length );          // returns -1 if not enough room
	int           SendString( const char *line );                // returns -1 if not enough room

	size_t        GetReceiveBytesAvailable( void );
	size_t        recv_bytes( void *data, size_t maxlength );    // returns actual bytes read

	int           GetReceiveLinesAvailable( void );
	int           ReceiveLine( char *line, int maxlength );      // returns 0 if the line was terminated with a \r or \n, 1 if not.
	                                                             // (i.e. if you specify maxlength=10, and the line is 12 bytes long
	                                                             // it will return 1. or if there is no \r or \n and that's all the data
	                                                             // the connection has.)

	size_t PeekBytes( void *data, size_t maxlength );            // returns bytes peeked

	unsigned long GetInterface( void );                          // this returns the interface the connection is on
	unsigned long GetRemoteAddress( void );                      // remote host ip.
	short         GetRemotePort( void );                         // this returns the remote port of connection
};


inline void api_connection::Open( api_dns *dns, size_t sendbufsize, size_t recvbufsize )
{
	_voidcall( API_CONNECTION_OPEN, dns, sendbufsize, recvbufsize );
}

inline void api_connection::connect( char *hostname, int port )
{
	_voidcall( API_CONNECTION_CONNECT, hostname, port );
}

inline void api_connection::run( int max_send_bytes, int max_recv_bytes, int *bytes_sent, int *bytes_rcvd )
{
	_voidcall( API_CONNECTION_RUN, max_send_bytes, max_recv_bytes, bytes_sent, bytes_rcvd );
}

inline int api_connection::get_state()
{
	return _call( API_CONNECTION_GETSTATE, (int)0 );
}

inline char *api_connection::GetError()
{
	return _call( API_CONNECTION_GETERROR, (char *)NULL );
}

inline void api_connection::Close( int quick )
{
	_voidcall( API_CONNECTION_CLOSE, quick );
}

inline void api_connection::FlushSend( void )
{
	_voidcall( API_CONNECTION_FLUSHSEND );
}

inline size_t api_connection::GetSendBytesInQueue( void )
{
	return _call( API_CONNECTION_GETSENDBYTESINQUEUE, (size_t)0 );
}

inline size_t api_connection::GetSendBytesAvailable( void )
{
	return _call( API_CONNECTION_GETSENDBYTESAVAILABLE, (size_t)0 );
}

inline int api_connection::send( const void *data, int length )
{
	return _call( API_CONNECTION_SEND, (int)0, data, length );
}

inline int api_connection::SendString( const char *line )
{
	return _call( API_CONNECTION_SENDSTRING, (int)0, line );
}

inline size_t api_connection::GetReceiveBytesAvailable( void )
{
	return _call( API_CONNECTION_GETRECEIVEBYTESAVAILABLE, (size_t)0 );
}

inline size_t api_connection::recv_bytes( void *data, size_t maxlength )
{
	return _call( API_CONNECTION_RECEIVEBYTES, (size_t)0, data, maxlength );
}


inline int api_connection::GetReceiveLinesAvailable( void )
{
	return _call( API_CONNECTION_GETRECEIVELINESAVAILABLE, (int)0 );
}

inline int api_connection::ReceiveLine( char *line, int maxlength )
{
	return _call( API_CONNECTION_RECEIVELINE, (int)0, line, maxlength );
}

inline size_t api_connection::PeekBytes( void *data, size_t maxlength )
{
	return _call( API_CONNECTION_PEEKBYTES, (size_t)0, data, maxlength );
}

inline unsigned long api_connection::GetInterface( void )
{
	return _call( API_CONNECTION_GETINTERFACE, (unsigned long)0 );
}

inline unsigned long api_connection::GetRemoteAddress( void )
{
	return _call( API_CONNECTION_GETREMOTEADDRESS, (unsigned long)0 );
}

inline short api_connection::GetRemotePort( void )
{
	return _call( API_CONNECTION_GETREMOTEPORT, (short)0 );
}

// {049E1B1E-56DF-46e0-B88F-E7E319F131DD}
static const GUID connectionFactoryGUID =
{ 0x49e1b1e, 0x56df, 0x46e0, { 0xb8, 0x8f, 0xe7, 0xe3, 0x19, 0xf1, 0x31, 0xdd } };

// {8F0B5D22-699B-486b-B39D-650E5B09C1F1}
static const GUID sslConnectionFactoryGUID =
{ 0x8f0b5d22, 0x699b, 0x486b, { 0xb3, 0x9d, 0x65, 0xe, 0x5b, 0x9, 0xc1, 0xf1 } };

#endif // !NULLSOFT_COMPONENT_WAC_NETWORK_CONNECTION_H

