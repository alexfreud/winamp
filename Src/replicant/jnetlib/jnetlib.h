/*
** JNetLib
** Copyright (C) 2000-2006 Nullsoft, Inc.
** Author: Justin Frankel
** File: jnetlib.h - JNL main include file (not really necessary).
**
** For documentation, look at the following files:
**  Generic network initialization: netinc.h
**  DNS: asyncdns.h
**  TCP connections: connection.h
**  HTTP GET connections: httpget.h
**  TCP listen: listen.h
**
**  license:
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
*/

#ifndef _JNETLIB_H_
#define _JNETLIB_H_

#include "netinc.h"
#include "../foundation/types.h"
#include "jnetlib_defines.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#ifdef JNETLIB_EXPORTS
#define JNL_API __declspec(dllexport)
#else
#define JNL_API __declspec(dllimport)
#endif
#elif defined(__ANDROID__) || defined(__APPLE__)
#define JNL_API __attribute__ ((visibility("default")))
#elif defined(__linux__)
#if __GNUC__ >= 4
#define JNL_API __attribute__ ((visibility ("default")))
#else
#define JNL_API
#endif
#else
#error port me
#endif

/* these are reference counted.  so make sure to match init/quit calls.  */
JNL_API int jnl_init();
JNL_API void jnl_quit();

/* ----- Connection ----- */
JNL_API jnl_connection_t jnl_connection_create( jnl_dns_t dns, size_t sendbufsize, size_t recvbufsize );
JNL_API jnl_connection_t jnl_sslconnection_create( jnl_dns_t dns, size_t sendbufsize, size_t recvbufsize );
JNL_API void jnl_connection_run( jnl_connection_t connection, size_t max_send_bytes, size_t max_receive_bytes, size_t *bytes_sent, size_t *bytes_received );
JNL_API int jnl_connection_get_state( jnl_connection_t connection );
JNL_API size_t jnl_connection_send_bytes_available( jnl_connection_t connection );
JNL_API size_t jnl_connection_receive_bytes_available( jnl_connection_t connection );
JNL_API int jnl_connection_receive_line( jnl_connection_t _connection, void *bytes, size_t size );
JNL_API int jnl_connection_send( jnl_connection_t connection, const void *bytes, size_t size );
JNL_API int jnl_connection_send_string( jnl_connection_t connection, const char *str );
JNL_API size_t jnl_connection_receive( jnl_connection_t connection, void *bytes, size_t size );
JNL_API size_t jnl_connection_send_bytes_in_queue( jnl_connection_t connection );
JNL_API void jnl_connection_release( jnl_connection_t connection );
JNL_API size_t jnl_connection_receive_lines_available( jnl_connection_t connection );
JNL_API void jnl_connection_close( jnl_connection_t _connection, int fast );
JNL_API void jnl_connection_connect( jnl_connection_t connection, const char *hostname, int port );
JNL_API const char *jnl_connection_get_error( jnl_connection_t connection );
JNL_API size_t jnl_connection_peek( jnl_connection_t _connection, void *bytes, size_t size );

/* ----- UDP ----- */
JNL_API int jnl_udp_create_multicast_listener( jnl_udp_t *connection, const char *mcast_ip, unsigned short port );
JNL_API void jnl_udp_release( jnl_udp_t connection );
JNL_API void jnl_udp_run( jnl_udp_t connection, size_t max_send_bytes, size_t max_recv_bytes, size_t *bytes_sent, size_t *bytes_rcvd );
JNL_API size_t jnl_udp_recv_bytes( jnl_udp_t connection, void *buf, size_t len );
JNL_API int jnl_udp_send( jnl_udp_t connection, const void *bytes, size_t size );
JNL_API void jnl_udp_set_peer( jnl_udp_t connection, const char *hostname, unsigned short port );
JNL_API void jnl_udp_set_peer_address( jnl_udp_t connection, struct sockaddr *addr, socklen_t length );
// gets the address of whomever sent the last message
JNL_API int jnl_udp_get_address( jnl_udp_t connection, struct sockaddr **addr, socklen_t *length );

/* ----- HTTP ----- */

/* creation/destruction */
JNL_API jnl_http_t jnl_http_create( int recvbufsize, int sendbufsize );
JNL_API int jnl_http_set_recv_buffer_size( jnl_http_t http, size_t new_size ); /* increases the receive buffer size */
JNL_API jnl_http_t jnl_http_retain( jnl_http_t http );
JNL_API void jnl_http_release( jnl_http_t http );
JNL_API jnl_connection_t jnl_http_get_connection( jnl_http_t http );
/* TODO: replace these with a jnl_http_configure(jnl_http_t http) function */
JNL_API void jnl_http_set_persistent( jnl_http_t http );
JNL_API void jnl_http_allow_compression( jnl_http_t http );
JNL_API void jnl_http_allow_accept_all_reply_codes( jnl_http_t http );

/* run & status stuff */
JNL_API void jnl_http_connect( jnl_http_t http, const char *url, int http_version, const char *method );
JNL_API int jnl_http_run( jnl_http_t http );
JNL_API int jnl_http_get_status( jnl_http_t http );
JNL_API int jnl_http_getreplycode( jnl_http_t http );
JNL_API const char *jnl_http_getreply( jnl_http_t http );

/* reading data */
JNL_API size_t jnl_http_get_bytes( jnl_http_t http, void *buf, size_t len );
JNL_API size_t jnl_http_peek_bytes( jnl_http_t http, void *buf, size_t len );
JNL_API size_t jnl_http_bytes_available( jnl_http_t http );
JNL_API uint64_t jnl_http_content_length( jnl_http_t http );

/* HTTP headers */
JNL_API const char *jnl_http_getheader( jnl_http_t http, const char *header );
JNL_API void jnl_http_addheader( jnl_http_t http, const char *header );
JNL_API void jnl_http_addheadervalue( jnl_http_t http, const char *header, const char *value );
JNL_API void jnl_http_reset_headers( jnl_http_t http );
JNL_API const char *jnl_http_get_all_headers( jnl_http_t http );

/* other information */
JNL_API const char *jnl_http_get_url( jnl_http_t http );
JNL_API void jnl_http_set_proxy( const char *proxy );

/* ----- HTTP Request Parsing ----- */
JNL_API int jnl_http_request_create( jnl_http_request_t *http, jnl_connection_t connection );
JNL_API void jnl_http_request_release( jnl_http_request_t http );
JNL_API int jnl_http_request_run( jnl_http_request_t http );
JNL_API int jnl_htt_request_get_keep_alive( jnl_http_request_t http );
JNL_API const char *jnl_http_request_get_header( jnl_http_request_t http, const char *header );
JNL_API void jnl_http_request_reset( jnl_http_request_t http );
JNL_API void jnl_http_request_addheader( jnl_http_request_t http, const char *header );
JNL_API void jnl_http_request_set_reply_string( jnl_http_request_t http, const char *reply );
JNL_API void jnl_http_request_send_reply( jnl_http_request_t http );
JNL_API const char *jnl_http_request_get_uri( jnl_http_request_t http );
JNL_API const char *jnl_http_request_get_parameter( jnl_http_request_t _http, const char *parameter );
JNL_API jnl_connection_t jnl_http_request_get_connection( jnl_http_request_t http );
JNL_API const char *jnl_http_request_get_method( jnl_http_request_t http );

/* ----- HTTPU Request Parsing ----- */
JNL_API int jnl_httpu_request_create( jnl_httpu_request_t *httpu );
JNL_API void jnl_httpu_request_release( jnl_httpu_request_t httpu );
JNL_API int jnl_httpu_request_process( jnl_httpu_request_t httpu, jnl_udp_t udp );
JNL_API const char *jnl_httpu_request_get_method( jnl_httpu_request_t httpu );
JNL_API const char *jnl_httpu_request_get_uri( jnl_httpu_request_t httpu );
JNL_API const char *jnl_httpu_request_get_header( jnl_httpu_request_t httpu, const char *header );

/* ----- DNS ------ */
JNL_API int jnl_dns_create( jnl_dns_t *dns );
JNL_API void jnl_dns_release( jnl_dns_t dns );
JNL_API int jnl_dns_resolve( jnl_dns_t dns, const char *hostname, unsigned short port, struct addrinfo **addr, int sockettype );
// when you call jnl_dns_resolve_now, you need to call jnl_dns_freeaddrinfo
JNL_API int jnl_dns_resolve_now( const char *hostname, unsigned short port, struct addrinfo **addr, int sockettype );
JNL_API void jnl_dns_freeaddrinfo( struct addrinfo *addr );
JNL_API void jnl_dns_gethostname( char *name, size_t cch );
JNL_API void jnl_dns_ntop( int af, const void *src, char *dst, socklen_t size );
/* listen */
JNL_API int jnl_listen_create( jnl_listen_t *listen, unsigned short port );
JNL_API int jnl_listen_create_from_address( jnl_listen_t *listen, struct addrinfo *addr, size_t index );
JNL_API void jnl_listen_release( jnl_listen_t listen );
JNL_API jnl_connection_t jnl_listen_get_connection( jnl_listen_t listen );
JNL_API unsigned short jnl_listen_get_port( jnl_listen_t listen );


#ifdef __cplusplus
}
#endif

#endif//_JNETLIB_H_
