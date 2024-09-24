#include "jnetlib.h"
#include "httpget.h"
#include "sslconnection.h"
#include "asyncdns.h"
#include "util.h"
#include "httpserv.h"
#include "httpuserv.h"
#include "listen.h"
#include "multicastlisten.h"

#include "foundation/error.h"

#include <new>

int jnl_init()
{
	return JNL::open_socketlib();
}

void jnl_quit()
{
	JNL::close_socketlib();
}

/* --- Connection --- */
jnl_connection_t jnl_connection_create(jnl_dns_t dns, size_t sendbufsize, size_t recvbufsize)
{
	JNL_Connection *connection = new (std::nothrow) JNL_Connection((JNL_AsyncDNS *)dns, sendbufsize, recvbufsize);
	return (jnl_connection_t)connection;
}

jnl_connection_t jnl_sslconnection_create(jnl_dns_t dns, size_t sendbufsize, size_t recvbufsize)
{
	JNL_SSL_Connection *connection = new (std::nothrow) JNL_SSL_Connection(NULL, (JNL_AsyncDNS *)dns, sendbufsize, recvbufsize);
	return (jnl_connection_t)connection;
}

void jnl_connection_run(jnl_connection_t _connection, size_t max_send_bytes, size_t max_receive_bytes, size_t *bytes_sent, size_t *bytes_received)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	connection->run(max_send_bytes, max_receive_bytes, bytes_sent, bytes_received); 
}

int jnl_connection_get_state(jnl_connection_t _connection)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->get_state();
}

size_t jnl_connection_send_bytes_available(jnl_connection_t _connection)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->send_bytes_available();
}

size_t jnl_connection_receive_bytes_available(jnl_connection_t _connection)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->recv_bytes_available();
}

int jnl_connection_send(jnl_connection_t _connection, const void *bytes, size_t size)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->send(bytes, size);
}

JNL_API int jnl_connection_send_string(jnl_connection_t _connection, const char *str)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->send_string(str);
}

size_t jnl_connection_send_bytes_in_queue(jnl_connection_t _connection)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->send_bytes_in_queue();
}

size_t jnl_connection_receive(jnl_connection_t _connection, void *bytes, size_t size)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->recv_bytes(bytes, size);
}

size_t jnl_connection_peek(jnl_connection_t _connection, void *bytes, size_t size)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->peek_bytes(bytes, size);
}

void jnl_connection_release(jnl_connection_t _connection)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	if (connection)
		connection->Release();
}

int jnl_connection_receive_line(jnl_connection_t _connection, void *bytes, size_t size)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->recv_line((char *)bytes, size);
}

size_t jnl_connection_receive_lines_available(jnl_connection_t _connection)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->recv_lines_available();
}

void jnl_connection_close(jnl_connection_t _connection, int fast)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	connection->close(fast);
}

void jnl_connection_connect(jnl_connection_t _connection, const char *hostname, int port)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	connection->connect(hostname, port);
}

const char *jnl_connection_get_error(jnl_connection_t _connection)
{
	JNL_Connection *connection = (JNL_Connection *)_connection;
	return connection->get_errstr();
}

/* ---- UDP ----- */
int jnl_udp_create_multicast_listener(jnl_udp_t *connection, const char *mcast_ip, unsigned short port)
{
	JNL_UDPConnection *udp =  0;
	JNL::open_socketlib(); // TODO: cut

	int ret = CreateMulticastListener(&udp, mcast_ip, port);
	if (ret != NErr_Success)
	{
		JNL::close_socketlib(); // TODO: cut

		return ret;
	}
	*connection = (jnl_udp_t)udp;
	return NErr_Success;
}

void jnl_udp_release(jnl_udp_t _connection)
{
	JNL_UDPConnection *connection = (JNL_UDPConnection *)_connection;
	delete connection; // TODO: reference count
	JNL::close_socketlib(); // TODO: cut
}

void jnl_udp_run(jnl_udp_t _connection, size_t max_send_bytes, size_t max_recv_bytes, size_t *bytes_sent, size_t *bytes_rcvd)
{
	JNL_UDPConnection *connection = (JNL_UDPConnection *)_connection;
	if (connection)
	{
		connection->run(max_send_bytes, max_recv_bytes, bytes_sent, bytes_rcvd);
	}
}

size_t jnl_udp_recv_bytes(jnl_udp_t _connection, void *buf, size_t len)
{
	JNL_UDPConnection *connection = (JNL_UDPConnection *)_connection;
	if (connection)
	{
		return connection->recv_bytes(buf, len);
	}
	else
		return 0;
}

int jnl_udp_send(jnl_udp_t _connection, const void *bytes, size_t size)
{
	JNL_UDPConnection *connection = (JNL_UDPConnection *)_connection;
	return connection->send(bytes, size);
}

void jnl_udp_set_peer(jnl_udp_t _connection, const char *hostname, unsigned short port)
{
	JNL_UDPConnection *connection = (JNL_UDPConnection *)_connection;
	connection->setpeer(hostname, port);
}

void jnl_udp_set_peer_address(jnl_udp_t _connection, sockaddr *addr, socklen_t length)
{
	JNL_UDPConnection *connection = (JNL_UDPConnection *)_connection;
	connection->setpeer(addr, length);
}

int jnl_udp_get_address(jnl_udp_t _connection, sockaddr **addr, socklen_t *length)
{
	JNL_UDPConnection *connection = (JNL_UDPConnection *)_connection;
	connection->get_last_recv_msg_addr(addr, length);
	return NErr_Success;
}

/* ---- HTTP ---- */
jnl_http_t jnl_http_create(int recvbufsize, int sendbufsize)
{
	JNL_HTTPGet *http = new (std::nothrow) JNL_HTTPGet(recvbufsize, sendbufsize);
	return (jnl_http_t)http;
}

int jnl_http_set_recv_buffer_size(jnl_http_t _http, size_t new_size)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->set_recv_buffer_size(new_size);
}

int jnl_http_run(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->run();
}

size_t jnl_http_get_bytes(jnl_http_t _http, void *buf, size_t len)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->get_bytes(static_cast<char *>(buf), len);
}

size_t jnl_http_peek_bytes(jnl_http_t _http, void *buf, size_t len)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->peek_bytes(static_cast<char *>(buf), len);
}

size_t jnl_http_bytes_available(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->bytes_available();
}

uint64_t jnl_http_content_length(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->content_length();
}

int jnl_http_get_status(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->get_status();
}

int jnl_http_getreplycode(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->getreplycode();
}

const char *jnl_http_getreply(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->getreply();
}

const char *jnl_http_getheader(jnl_http_t _http, const char *header)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->getheader(header);
}

const char *jnl_http_get_all_headers(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->getallheaders();
}

void jnl_http_addheader(jnl_http_t _http, const char *header)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	http->addheader(header);
}

void jnl_http_addheadervalue(jnl_http_t _http, const char *header, const char *value)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	http->addheadervalue(header, value);
}

void jnl_http_connect(jnl_http_t _http, const char *url, int http_version, const char *method)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	http->connect(url, http_version, method);
}

void jnl_http_release(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;

	if (http)
		http->Release();
}

jnl_http_t jnl_http_retain(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	if (http)
		http->Retain();
	return _http;
}

const char *jnl_http_get_url(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return http->get_url();
}

void jnl_http_reset_headers(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	http->reset_headers();
}

void jnl_http_set_persistent(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	http->set_persistent();
}

void jnl_http_allow_compression(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	http->AllowCompression();
}

void jnl_http_allow_accept_all_reply_codes(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	http->set_accept_all_reply_codes();
}

jnl_connection_t jnl_http_get_connection(jnl_http_t _http)
{
	JNL_HTTPGet *http = (JNL_HTTPGet *)_http;
	return (jnl_connection_t)http->get_con();
}

void jnl_http_set_proxy(const char *proxy)
{
	JNL_HTTPGet::set_proxy(proxy);
}

/* ------- HTTP Request Parser ------- */
int jnl_http_request_create(jnl_http_request_t *_http, jnl_connection_t _connection)
{
	JNL_HTTPServ *http = new (std::nothrow) JNL_HTTPServ((JNL_Connection *)_connection);;
	if (!http)
		return NErr_OutOfMemory;

	*_http = (jnl_http_request_t) http;
	return NErr_Success;
}

void jnl_http_request_release(jnl_http_request_t _http)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	if (http)
		http->Release();
}

int jnl_http_request_run(jnl_http_request_t _http)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	return http->run();
}

int jnl_htt_request_get_keep_alive(jnl_http_request_t _http)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	return http->get_keep_alive();
}

const char *jnl_http_request_get_header(jnl_http_request_t _http, const char *header)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	return http->getheader(header);
}

void jnl_http_request_reset(jnl_http_request_t _http)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	http->reset();
}

void jnl_http_request_addheader(jnl_http_request_t _http, const char *header)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	http->add_reply_header(header);
}

void jnl_http_request_set_reply_string(jnl_http_request_t _http, const char *reply)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	http->set_reply_string(reply);
}

void jnl_http_request_send_reply(jnl_http_request_t _http)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	http->send_reply();
}

const char *jnl_http_request_get_uri(jnl_http_request_t _http)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	return http->get_request_file();
}

const char *jnl_http_request_get_parameter(jnl_http_request_t _http, const char *parameter)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	return http->get_request_parm(parameter);
}

jnl_connection_t jnl_http_request_get_connection(jnl_http_request_t _http)
{
		JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
		JNL_Connection *connection = http->get_con();
		if (connection)
		{
			connection->Retain();
			return (jnl_connection_t)connection;
		}
		else
			return 0;
}

const char *jnl_http_request_get_method(jnl_http_request_t _http)
{
	JNL_HTTPServ *http = (JNL_HTTPServ *)_http;
	return http->get_method();
}

/* ------- HTTPU Request Parser ------- */
int jnl_httpu_request_create(jnl_httpu_request_t *_httpu)
{
	JNL_HTTPUServ *httpu = new (std::nothrow) JNL_HTTPUServ;
	if (!httpu)
		return NErr_OutOfMemory;

	*_httpu = (jnl_httpu_request_t) httpu;
	return NErr_Success;
}

void jnl_httpu_request_release(jnl_httpu_request_t _httpu)
{
	JNL_HTTPUServ *httpu = (JNL_HTTPUServ *)_httpu;
	delete httpu; // TODO: reference count
}

int jnl_httpu_request_process(jnl_httpu_request_t _httpu, jnl_udp_t _udp)
{
	JNL_HTTPUServ *httpu = (JNL_HTTPUServ *)_httpu;
	return httpu->process((JNL_UDPConnection *)_udp);
}

const char *jnl_httpu_request_get_method(jnl_httpu_request_t _httpu)
{
	JNL_HTTPUServ *httpu = (JNL_HTTPUServ *)_httpu;
	return httpu->get_method();
}


const char *jnl_httpu_request_get_uri(jnl_httpu_request_t _httpu)
{
	JNL_HTTPUServ *httpu = (JNL_HTTPUServ *)_httpu;
	return httpu->get_request_uri();
}

const char *jnl_httpu_request_get_header(jnl_httpu_request_t _httpu, const char *header)
{
	JNL_HTTPUServ *httpu = (JNL_HTTPUServ *)_httpu;
	return httpu->getheader(header);
}

/* ----- DNS ----- */
int jnl_dns_create(jnl_dns_t *out_dns)
{
	JNL_AsyncDNS *dns = new (std::nothrow)JNL_AsyncDNS;
	if (!dns)
		return NErr_OutOfMemory;

	*out_dns = (jnl_dns_t)dns;
	return NErr_Success;
}

void jnl_dns_release(jnl_dns_t _dns)
{
	JNL_AsyncDNS *dns = (JNL_AsyncDNS *)_dns;
	delete dns; // TODO: reference counting
}

int jnl_dns_resolve(jnl_dns_t _dns, const char *hostname, unsigned short port, addrinfo **addr, int sockettype)
{
	JNL_AsyncDNS *dns = (JNL_AsyncDNS *)_dns;
	int ret = dns->resolve(hostname, port, addr, sockettype);
	if (ret == 0)
		return NErr_Success;
	else if (ret == -1)
		return NErr_TryAgain;
	else
		return NErr_Unknown;
}

int jnl_dns_resolve_now(const char *hostname, unsigned short port, addrinfo **addr, int sockettype)
{
	int ret = JNL_AsyncDNS::resolvenow(hostname, port, addr, sockettype);
	if (ret == 0)
		return NErr_Success;
	else
		return NErr_Unknown;
}


void jnl_dns_freeaddrinfo(addrinfo *addr)
{
	freeaddrinfo(addr);
}

void jnl_dns_gethostname(char *name, size_t cch)
{
	gethostname(name, (int)cch);
}

#ifdef _WIN32
PCSTR WSAAPI inet_ntop_xp(INT af, PVOID src, PSTR dst, size_t cnt)
{
	struct sockaddr_in srcaddr;

	memset(&srcaddr, 0, sizeof(struct sockaddr_in));
	memcpy(&(srcaddr.sin_addr), src, sizeof(srcaddr.sin_addr));

	srcaddr.sin_family = af;
	if (WSAAddressToStringA((struct sockaddr*) &srcaddr, sizeof(struct sockaddr_in), 0, dst, (LPDWORD) &cnt) != 0) {
		return NULL;
	}
	return dst;
}

PCSTR WSAAPI inet_ntop_win32(INT Family, PVOID pAddr, PSTR pStringBuf, size_t StringBufSize) {
    typedef PCSTR (WSAAPI * win32_inet_ntop)(INT, PVOID, PSTR, size_t);
    static win32_inet_ntop pwin32_inet_ntop = NULL;

	if (!pwin32_inet_ntop){
		HMODULE hlib = LoadLibrary(L"WS2_32.DLL");
		pwin32_inet_ntop = (win32_inet_ntop)GetProcAddress(hlib, "inet_ntop");
		if (!pwin32_inet_ntop) {
			pwin32_inet_ntop = inet_ntop_xp;
		}
	}

	return (*pwin32_inet_ntop)(Family, pAddr, pStringBuf, StringBufSize);
}

#endif

void jnl_dns_ntop(int af, const void *src, char *dst, socklen_t size)
{
#ifdef _WIN32
	// TODO need to revist this at a later date
	// [22:01:08] audiodsp: i will make a tweak for IPv6 compatability at some point
	// [22:01:49] audiodsp: just change references from sockaddr_in to sockaddr_storage
	// [22:02:24] audiodsp: keep it as is
	// [22:02:32] audiodsp: we're only using it in IPv4 mode at the moment
	// [22:02:40] audiodsp: i will fix when we need IPv6 server support
	// [22:03:58] audiodsp: the memcpy is what makes it non-trivial
	// [22:04:05] audiodsp: i have to switch on family and do different memcpy's accordingly
	// [22:04:16] audiodsp: or change the method to require a length
	// [22:04:19] audiodsp: which makes more sense
	// [22:04:29] audiodsp: and just not pass it to the linux function
	// [22:05:08] audiodsp: anyway not important. 
	inet_ntop_win32(af, (PVOID)src, dst, size);
#else
	inet_ntop(af, src, dst, size);
#endif
}
/* Listen */

int jnl_listen_create(jnl_listen_t *_listen, unsigned short port)
{
	JNL_Listen *l = new (std::nothrow) JNL_Listen();
	if (!l)
		return NErr_OutOfMemory;
	int ret = l->Initialize(port);
	if (ret != NErr_Success)
	{
		delete l;
		return ret;
	}
	*_listen = (jnl_listen_t)l;
	return NErr_Success;
}

int jnl_listen_create_from_address(jnl_listen_t *_listen, struct addrinfo *addr, size_t index)
{
	JNL_Listen *l = new (std::nothrow) JNL_Listen();
	if (!l)
		return NErr_OutOfMemory;
	int ret = l->Initialize(addr, index);
	if (ret != NErr_Success)
	{
		delete l;
		return ret;
	}
	*_listen = (jnl_listen_t)l;
	return NErr_Success;
}

jnl_connection_t jnl_listen_get_connection(jnl_listen_t _listen)
{
	JNL_Listen *listen = (JNL_Listen *)_listen;
	JNL_Connection *connection = listen->get_connect();
	return (jnl_connection_t)connection;
}

unsigned short jnl_listen_get_port(jnl_listen_t _listen)
{
	JNL_Listen *listen = (JNL_Listen *)_listen;
	return listen->get_port();
}

void jnl_listen_release(jnl_listen_t _listen)
{
	JNL_Listen *listen = (JNL_Listen *)_listen;
	if (listen)
		listen->Release();
}