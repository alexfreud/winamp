/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: httpget.h - JNL interface for doing HTTP GETs.
** License: see jnetlib.h
**
** Usage:
**   1. Create a JNL_HTTPGet object, optionally specifying a JNL_AsyncDNS
**      object to use (or NULL for none, or WAC_NETWORK_CONNECTION_AUTODNS for auto),
**      and the receive buffer size, and a string specifying proxy (or NULL 
**      for none). See note on proxy string below.
**   2. call addheader() to add whatever headers you want. It is recommended to
**      add at least the following two:
**        addheader("User-Agent:MyApp (Mozilla)");
*///      addheader("Accept:*/*");
/*         ( the comment weirdness is there so I Can do the star-slash :)
**   3. Call connect() with the URL you wish to GET (see URL string note below)
**   4. Call run() once in a while, checking to see if it returns -1 
**      (if it does return -1, call geterrorstr() to see what the error is).
**      (if it returns 1, no big deal, the connection has closed).
**   5. While you're at it, you can call bytes_available() to see if any data
**      from the http stream is available, or getheader() to see if any headers
**      are available, or getreply() to see the HTTP reply, or getallheaders() 
**      to get a double null terminated, null delimited list of headers returned.
**   6. If you want to read from the stream, call get_bytes (which returns how much
**      was actually read).
**   7. content_length() is a helper function that uses getheader() to check the
**      content-length header.
**   8. Delete ye' ol' object when done.
**
** Proxy String:
**   should be in the format of host:port, or user@host:port, or 
**   user:password@host:port. if port is not specified, 80 is assumed.
** URL String:
**   should be in the format of http://user:pass@host:port/requestwhatever
**   note that user, pass, port, and /requestwhatever are all optional :)
**   note that also, http:// is really not important. if you do poo://
**   or even leave out the http:// altogether, it will still work.
*/

#ifndef _HTTPGET_H_
#define _HTTPGET_H_

#include "connection.h"
#include "asyncdns.h"
#include "jnetlib_defines.h"
#ifdef _WIN32
#include "minizip/unzip.h"
#else
#include <zlib.h>
#endif
#include "nswasabi/ReferenceCounted.h"

class JNL_HTTPGet : public ReferenceCountedBase<JNL_HTTPGet>
{
public:
	JNL_HTTPGet(size_t recvbufsize=PACKET_SIZE, size_t sendbufsize=0);
	~JNL_HTTPGet();

	void set_sendbufsize( size_t sendbufsize = PACKET_SIZE ); // call if you're going to POST or do any kind of bidirectional communications
	int set_recv_buffer_size(size_t new_buffer_size);
	void addheader(const char *header);
	void addheadervalue(const char *header, const char *value);

	void connect(const char *url, int ver=0, const char *requestmethod="GET");

	int run(); // returns: 0 if all is OK. -1 if error (call geterrorstr()). 1 if connection closed.

	int   get_status(); // returns 0 if connecting, 1 if reading headers, 
                        // 2 if reading content, -1 if error.

    const char *getallheaders(); // double null terminated, null delimited list
    const char *getheader(const char *headername);
    const char *getreply() { return m_reply; }
    int   getreplycode(); // returns 0 if none yet, otherwise returns http reply code.

    const char *geterrorstr() { return m_errstr;}

    size_t bytes_available();
    size_t get_bytes(char *buf, size_t len);
    size_t peek_bytes(char *buf, size_t len);

    uint64_t content_length();

    JNL_Connection *get_con() { return m_con; }
	void AllowCompression();
	void reset_headers();

	void set_dns(JNL_AsyncDNS *dns);
	const char *get_url() { return m_http_url; }
	void set_accept_all_reply_codes(); // call this if you want to retrieve content even though a 404 (etc) was returned
	void set_persistent();
	static void set_proxy(const char *proxy);

protected:
	static char *get_proxy();
    void reinit();
    void deinit(bool full=true);
    void seterrstr(const char *str);

	void do_parse_url(const char *url, char **host, unsigned short*port, char **req, char **lp);
    void do_encode_mimestr(char *in, char *out);

    JNL_AsyncDNS *m_dns;
    JNL_Connection *m_con;
    size_t m_recvbufsize;

    int m_http_state;

    unsigned short m_http_port;
    char *m_http_url;
    char *m_http_host;
    char *m_http_lpinfo;
    char *m_http_request;

    char *m_http_proxylpinfo;
    char *m_http_proxyhost;
    unsigned short m_http_proxyport;

    char *m_sendheaders;
    char *m_recvheaders;
    size_t m_recvheaders_size;
    char *m_reply;

    char *m_errstr;
	bool allowCompression;

	size_t m_sendbufsize;
	/* gzip stuff */
	z_stream *zlibStream;

	bool accept_all_reply_codes;
	bool persistent;

	static char *g_proxy;
};

#endif // _HTTPGET_H_
