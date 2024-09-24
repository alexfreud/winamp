#pragma once
#include "../foundation/types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Connection */
typedef struct jnl_connection_struct jnl_connection_struct;
typedef jnl_connection_struct *jnl_connection_t;

enum
{
	JNL_CONNECTION_STATE_ERROR        = 0,
	JNL_CONNECTION_STATE_NOCONNECTION = 1,
	JNL_CONNECTION_STATE_RESOLVING    = 2,
	JNL_CONNECTION_STATE_CONNECTING   = 3,
	JNL_CONNECTION_STATE_CONNECTED    = 4,
	JNL_CONNECTION_STATE_CLOSING      = 5,
	JNL_CONNECTION_STATE_CLOSED       = 6,
	JNL_CONNECTION_STATE_RESOLVED     = 7, // happens after RESOLVING, but going here for compatability
};

/* UDP */
typedef struct jnl_udp_struct jnl_udp_struct;
typedef jnl_udp_struct *jnl_udp_t;

/* HTTP  */
typedef struct jnl_http_struct jnl_http_struct;
typedef jnl_http_struct *jnl_http_t;

enum
{
	HTTPGET_STATUS_ERROR            = -1,
	JNL_HTTP_STATUS_ERROR           = HTTPGET_STATUS_ERROR,
	HTTPGET_STATUS_CONNECTING       = 0,
	JNL_HTTP_STATUS_CONNECTING      = HTTPGET_STATUS_CONNECTING,
	HTTPGET_STATUS_READING_HEADERS  = 1,
	JNL_HTTP_STATUS_READING_HEADERS = HTTPGET_STATUS_READING_HEADERS,
	HTTPGET_STATUS_READING_CONTENT  = 2,
	JNL_HTTP_STATUS_READING_CONTENT = HTTPGET_STATUS_READING_CONTENT,
};

enum
{
	HTTPGET_RUN_ERROR               = -1,
	HTTPGET_RUN_OK                  = 0,
	JNL_HTTP_RUN_OK                 = HTTPGET_RUN_OK,
	HTTPGET_RUN_CONNECTION_CLOSED   = 1,
};

/* DNS */
typedef struct jnl_dns_struct jnl_dns_struct;
typedef jnl_dns_struct *jnl_dns_t;

typedef struct jnl_httpu_request_struct jnl_httpu_request_struct;
typedef jnl_httpu_request_struct *jnl_httpu_request_t;

typedef struct jnl_http_request_struct jnl_http_request_struct;
typedef jnl_http_request_struct *jnl_http_request_t;

typedef struct jnl_listen_struct jnl_listen_struct;
typedef jnl_listen_struct *jnl_listen_t;

#ifdef __cplusplus
}
#endif