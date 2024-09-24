//#ifdef USE_SSL
//#include "netinc.h"
//#include "util.h"
//#include "connection.h"
#include "sslconnection.h"

SSL_CTX *sslContext = 0;

#ifdef _DEBUG

extern "C" void apps_ssl_info_callback (const SSL * s, int where, int ret)
{
	/*
	**  DEBUG INFO HERE
	*/
}
#endif

/*
**  Well, you should probably change this based on like...
**  well, you're level of trust heh
**  For now, this basically trusts all certs :)
**  
*/
#if 0
extern "C" int verify_callback(int ok, X509_STORE_CTX * ctx)
{
	/* For certificate verification */
	int verify_depth = 0;
	int verify_error = X509_V_OK;

	char buf[1024] = {0};
	X509 * err_cert = X509_STORE_CTX_get_current_cert(ctx);
	int err = X509_STORE_CTX_get_error(ctx);
	int depth = X509_STORE_CTX_get_error_depth(ctx);
	X509_NAME_oneline(X509_get_subject_name(err_cert), buf, sizeof(buf));

	if (!ok)
	{
		if (verify_depth >= depth)
		{
			ok = 1;
			verify_error = X509_V_OK;
		}
		else
		{
			ok = 0;
			verify_error = X509_V_ERR_CERT_CHAIN_TOO_LONG;
		}
	}
	switch (ctx->error)
	{
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
		X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert), buf, sizeof(buf));
		break;

	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		break;

	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		break;
	}
	return ok;
}
#endif

JNL_SSL_Connection::JNL_SSL_Connection() : forceConnect(false), m_ssl(0), m_bsslinit(false), m_bcontextowned(true)
{
	m_bsslinit = true;
}


JNL_SSL_Connection::JNL_SSL_Connection(SSL* pssl, JNL_AsyncDNS *dns, size_t sendbufsize, size_t recvbufsize) : JNL_Connection(dns, sendbufsize, recvbufsize),
		forceConnect(false)
{
	m_ssl = pssl;
	m_bsslinit = false;

	if (m_ssl)
	{
		m_bcontextowned = false;
	}
	else
	{
		m_bcontextowned = true;
	}

	if (m_bcontextowned == false)
	{
		return ;
	}

	m_bsslinit = true;


	/* See the SSL states in our own callback */
#ifdef _DEBUG 
	//	SSL_CTX_set_info_callback(m_app_ctx, apps_ssl_info_callback);
#endif

	/* Set the certificate verification callback */
	//SSL_CTX_set_verify(sslContext, SSL_VERIFY_PEER, verify_callback);

	/* Not sure what this does */
	//SSL_CTX_set_session_cache_mode(sslContext, SSL_SESS_CACHE_CLIENT);

	return ;
}

int JNL_SSL_Connection::socket_connect()
{
	int retval;

	if (m_bcontextowned == false)
	{
		/*
		** WTF?
		*/ 
		return -1;
	}
	if (m_ssl != NULL)
	{
		return -1;
	}

	retval = JNL_Connection::socket_connect();

	if (retval != 0)
	{
		if (ERRNO != JNL_EINPROGRESS)
		{
			return retval; // benski> if the underlying socket hasn't connected yet, then we can't start the SSL connection
			/*
			**  Joshua Teitelbaum 3/2/2006
			**  Fatal error here
			*/
		}
	}

	// moved from InitSSL() as no need to create this unless
	// we're actually going to use it which helps slow loads
	if (!sslContext)
	{
		sslContext = SSL_CTX_new(SSLv23_client_method());
		if (sslContext)
		{
			SSL_CTX_set_verify(sslContext, SSL_VERIFY_NONE, NULL);
		}
		else
		{
			return -1;
		}
	}

	m_ssl = SSL_new(sslContext);

	if (m_ssl == NULL)
	{
		return -1;
	}

	/* Tell that we are in connect mode */
	SSL_set_connect_state(m_ssl);

	/* Set socket descriptor with the socket we already have open */
	if(SSL_set_fd(m_ssl, m_socket) != 0)
	{
		return -1;
	}

	return retval;
}

void JNL_SSL_Connection::socket_shutdown()
{
	if (m_ssl)
		SSL_shutdown(m_ssl);
	JNL_Connection::socket_shutdown();

	if (m_ssl)
	{
		SSL_free(m_ssl);
		m_ssl = NULL;
	}
	return ;
}

void JNL_SSL_Connection::run(size_t max_send_bytes, size_t max_recv_bytes, size_t *bytes_sent, size_t *bytes_rcvd)
{
	if (!m_bsslinit)
	{
		int rval = SSL_accept(m_ssl);
		if (rval == -1)
		{
			int e = SSL_get_error(m_ssl, rval);

			if (!((e == SSL_ERROR_WANT_READ) || (e == SSL_ERROR_WANT_WRITE)))
			{
				m_state = STATE_ERROR;
			}

			return ;
		}
		else
		{
			m_bsslinit = true;
		}
	}

	/**
	 ** benski - march 2, 2006
	 **if the underlying socket didn't connected yet, we need to try the SSL connection again
 	 */
	if (forceConnect)
	{
		if(init_ssl_connection() == false)
		{
			return;
		}	
	}
	JNL_Connection::run(max_send_bytes, max_recv_bytes, bytes_sent, bytes_rcvd);
}

/*
**  init_ssl_connection:
**  Returns true, meaning can continue
**  Else, cannot continue with underlying run
**  side effects:
**  sets forceConnect
*/
bool JNL_SSL_Connection::init_ssl_connection()
{
	if(m_ssl == NULL)
	{
		/*
		**  WTF?
		**  cascade up.
		*/
		return true;
	}

	int retval = SSL_connect(m_ssl);

	if (retval < 0)
	{
		int err = SSL_get_error(m_ssl, retval);

		switch (err)
		{
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_CONNECT:
				forceConnect = true;
				break;
				// fall through
			default: // TODO: benski> MOST other errors are OK, (especially "want read" and "want write", but we need to think through all the scenarios here
				forceConnect=false;
		}
	}
	else if(retval)
	{
		/*
		**  success
		*/
		forceConnect = false;
	}

	/*
	**  If retval == 0
	**  socket is closed, or serious error occurred.
	**  cascade up.
	*/

	return forceConnect==false;
}

void JNL_SSL_Connection::on_socket_connected(void)
{
	init_ssl_connection();
}

void JNL_SSL_Connection::connect(SOCKET s, sockaddr *addr, socklen_t length)
{
	/*
	**  Joshua Teitelbaum 2/01/2006
	**  No need to close
	**  This is the reason for divergence as well as setting
	**  the connect state
	*/

	m_socket = s;
	address=(sockaddr *)malloc(length);
	memcpy(address, addr, length);

	m_remote_port = 0;
	if (m_socket != -1)
	{
		SET_SOCK_BLOCK(m_socket, 0);
		m_state = STATE_CONNECTED;
		SSL_set_fd(m_ssl, m_socket);
	}
	else
	{
		m_errorstr = "invalid socket passed to connect";
		m_state = STATE_ERROR;
	}
}

ssize_t JNL_SSL_Connection::socket_recv(char *buf, size_t len, int options)
{
	return SSL_read(m_ssl, buf, (int)len);
}

ssize_t JNL_SSL_Connection::socket_send(const char *buf, size_t len, int options)
{
	return SSL_write(m_ssl, buf, (int)len);
}

JNL_SSL_Connection::~JNL_SSL_Connection()
{
	if (m_ssl)
	{
		SSL_free(m_ssl);
		m_ssl = NULL;
	}
}
//#endif