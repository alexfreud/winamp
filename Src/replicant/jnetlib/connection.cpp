/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: connection.cpp - JNL TCP connection implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "connection.h"
#include "asyncdns.h"
#include "foundation\error.h"


#ifndef min
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

JNL_Connection::JNL_Connection()
{
	init();
}

JNL_Connection::JNL_Connection(JNL_AsyncDNS *dns, size_t sendbufsize, size_t recvbufsize)
{
	init();
	open(dns, sendbufsize, recvbufsize);
}


void JNL_Connection::init()
{
	m_errorstr="";
	address=0;
	m_dns=0;
	m_dns_owned=false;
	m_socket=-1;
	m_remote_port=0;
	m_state=STATE_NOCONNECTION;
	m_host[0]=0;
	saddr=0;
}

JNL_Connection::~JNL_Connection()
{
	/*
	**  Joshua Teitelbaum 1/27/2006
	**  virtualization for ssl, calling socket_shtudown()
	*/
	socket_shutdown();

	if (!saddr) // free it if it was passed to us (by JNL_Listen, presumably)
		free(address); // TODO: change this if we ever do round-robin DNS connecting or in any way change how we handle 'address'

	if (m_dns_owned) 
		delete m_dns;
}

void JNL_Connection::set_dns(JNL_AsyncDNS *dns)
{
	if (m_dns_owned)
		delete static_cast<JNL_AsyncDNS *>(m_dns);

	m_dns=dns;
	m_dns_owned=false;
}

void JNL_Connection::open(JNL_AsyncDNS *dns, size_t sendbufsize, size_t recvbufsize)
{
	if (dns != JNL_AUTODNS && dns)
	{
		m_dns=dns;
		m_dns_owned=false;
	}
	else if (!m_dns)
	{
		m_dns=new JNL_AsyncDNS;
		m_dns_owned=true;
	}

	recv_buffer.reserve(recvbufsize);
	send_buffer.reserve(sendbufsize);
}

void JNL_Connection::connect(SOCKET s, sockaddr *addr, socklen_t length)
{
	close(1);
	m_socket=s;
	address=(sockaddr *)malloc(length);
	memcpy(address, addr, length);

	m_remote_port=0;
	if (m_socket != -1)
	{
		SET_SOCK_BLOCK(m_socket,0);
		m_state=STATE_CONNECTED;
	}
	else 
	{
		m_errorstr="invalid socket passed to connect";
		m_state=STATE_ERROR;
	}

}

void JNL_Connection::connect(const char *hostname, int port)
{
	close(1);
	m_remote_port=(unsigned short)port;

#ifdef _WIN32
	lstrcpynA(m_host, hostname, sizeof(m_host));
#elif defined(__APPLE__)
	strlcpy(m_host, hostname, sizeof(m_host));
#else
	strncpy(m_host, hostname, sizeof(m_host)-1);
	m_host[sizeof(m_host)-1]=0;
#endif


	//memset(&m_saddr,0,sizeof(m_saddr));
	if (!m_host[0])
	{
		m_errorstr="empty hostname";
		m_state=STATE_ERROR;
	}
	else
	{
		m_state=STATE_RESOLVING;
	}
}

/*
**  Joshua Teitelbaum 1/27/2006
**  socket_shutdown
**  virtualization for ssl
*/
/* Virtual */ 
void JNL_Connection::socket_shutdown()
{
	if (m_socket >= 0)
	{
		::shutdown(m_socket, SHUT_RDWR);
		::closesocket(m_socket);

		m_socket=-1;
	}
}
/*
**  Joshua Teitelbaum 1/27/2006
**  socket_recv
**  virtualization for ssl
*/
/* Virtual */ 
ssize_t JNL_Connection::socket_recv(char *buf, size_t len, int options)
{
	return ::recv(m_socket,buf,(int)len,options);
}
/*
**  Joshua Teitelbaum 1/27/2006
**  socket_send
**  virtualization for ssl
*/
/* Virtual */ 
ssize_t JNL_Connection::socket_send(const char *buf, size_t len, int options)
{
	return ::send(m_socket,buf,(int)len,options);
}

int JNL_Connection::socket_connect()
{
	return ::connect(m_socket, saddr->ai_addr, (int)saddr->ai_addrlen);
}

void JNL_Connection::run(size_t max_send_bytes, size_t max_recv_bytes, size_t *bytes_sent, size_t *bytes_rcvd)
{
	socklen_t socket_buffer_size=0;
	socklen_t socket_buffer_size_len = sizeof(socket_buffer_size);
	socklen_t send_buffer_size;
	socklen_t recv_buffer_size;

	size_t bytes_allowed_to_send=(max_send_bytes==(size_t)-1)?send_buffer.size():max_send_bytes;
	size_t bytes_allowed_to_recv=(max_recv_bytes==(size_t)-1)?recv_buffer.avail():max_recv_bytes;

	if (bytes_sent) *bytes_sent=0;
	if (bytes_rcvd) *bytes_rcvd=0;

	switch (m_state)
	{
	case STATE_RESOLVING:
		if (saddr==0)
		{
			int a=m_dns->resolve(m_host, m_remote_port, &saddr, SOCK_STREAM);
			if (!a)
			{
				m_state=STATE_RESOLVED;
			}
			else if (a == 1)
			{
				m_state=STATE_RESOLVING; 
				break;
			}
			else
			{
				m_errorstr="resolving hostname"; 
				m_state=STATE_ERROR; 

				return;
			}
		}
		// fall through
	case STATE_RESOLVED:
		m_socket=::socket(saddr->ai_family, saddr->ai_socktype, saddr->ai_protocol);
		if (m_socket==-1)
		{
			m_errorstr="creating socket";
			m_state=STATE_ERROR;
		}
		else
		{
			SET_SOCK_BLOCK(m_socket,0);
		}

		socket_buffer_size=0;
		socket_buffer_size_len = sizeof(socket_buffer_size);
		getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)&socket_buffer_size, &socket_buffer_size_len);
		send_buffer_size = (int)(send_buffer.avail()+send_buffer.size());
		if (send_buffer_size > 65536)
			send_buffer_size=65536;
		if (socket_buffer_size < send_buffer_size)
			setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)&send_buffer_size, sizeof(send_buffer_size));
		getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)&socket_buffer_size, &socket_buffer_size_len);

		getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)&socket_buffer_size, &socket_buffer_size_len);
		recv_buffer_size = (int)recv_buffer.avail();
		if (recv_buffer_size > 65536)
			recv_buffer_size=65536;
		if (socket_buffer_size < recv_buffer_size)
			setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)&recv_buffer_size, sizeof(recv_buffer_size));
		getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)&socket_buffer_size, &socket_buffer_size_len);

		/*
		**  Joshua Teitelbaum 1/27/2006
		**  virtualization for ssl
		*/
		if(!socket_connect())
		{
			address=saddr->ai_addr;
			m_state=STATE_CONNECTED;

			on_socket_connected();
		}
		else if (ERRNO!=JNL_EINPROGRESS)
		{
			m_errorstr="Connecting to host";
			m_state=STATE_ERROR;
		}
		else
		{
			m_state=STATE_CONNECTING;
		}
		break;
	case STATE_CONNECTING:
		{		
			fd_set f[3];
			FD_ZERO(&f[0]);
			FD_ZERO(&f[1]);
			FD_ZERO(&f[2]);
			FD_SET(m_socket,&f[0]);
			FD_SET(m_socket,&f[1]);
			FD_SET(m_socket,&f[2]);
			struct timeval tv;
			memset(&tv,0,sizeof(tv));
			if (select((int)m_socket+1,&f[0],&f[1],&f[2],&tv)==-1)
			{
				m_errorstr="Connecting to host (calling select())";
				m_state=STATE_ERROR;
			}
			else if (FD_ISSET(m_socket,&f[1])) 
			{
				m_state=STATE_CONNECTED;
				on_socket_connected();
			}
			else if (FD_ISSET(m_socket,&f[2]))
			{
				m_errorstr="Connecting to host";
				m_state=STATE_ERROR;
			}
		}
		break;
	case STATE_CONNECTED:
	case STATE_CLOSING:
		/* --- send --- */		
		{
			size_t sent = send_buffer.drain(this, bytes_allowed_to_send);
			if (bytes_sent)
				*bytes_sent+=sent;

			if (m_state == STATE_CLOSED)
				break;

		/* --- receive --- */
			size_t received = recv_buffer.fill(this, bytes_allowed_to_recv);
			if (bytes_rcvd)
				*bytes_rcvd+=received;
		}

		if (m_state == STATE_CLOSING)
		{
			if (send_buffer.empty()) m_state = STATE_CLOSED;
		}
		break;
	default:
		break;
	}
}

void JNL_Connection::on_socket_connected(void)
{
	return;
}

void JNL_Connection::close(int quick)
{
	if (quick || m_state == STATE_RESOLVING || m_state == STATE_CONNECTING)
	{
		m_state=STATE_CLOSED;
		/*
		**  Joshua Teitelbaum 1/27/2006
		**  virualization for ssl
		*/
		socket_shutdown();

		m_socket=-1;

		recv_buffer.clear();
		send_buffer.clear();

		m_remote_port=0;
		m_host[0]=0;
		//memset(&m_saddr,0,sizeof(m_saddr));
	}
	else
	{
		if (m_state == STATE_CONNECTED)
			m_state=STATE_CLOSING;
	}
}

size_t JNL_Connection::send_bytes_in_queue(void)
{
	return send_buffer.size();
}

size_t JNL_Connection::send_bytes_available(void)
{
	return send_buffer.avail();
}

int JNL_Connection::send(const void *data, size_t length)
{
	if (length > send_bytes_available())
		return -1;

	send_buffer.write(data, length);
	return 0;
}

int JNL_Connection::send_string(const char *line)
{
	return send(line,strlen(line));
}

size_t JNL_Connection::recv_bytes_available(void)
{
	return recv_buffer.size();
}

size_t JNL_Connection::peek_bytes(void *data, size_t maxlength)
{
	if (data)
		return recv_buffer.peek(data, maxlength);
	else
		return min(maxlength, recv_bytes_available());
}

size_t JNL_Connection::recv_bytes(void *data, size_t maxlength)
{
	if (data)
		return recv_buffer.read(data, maxlength);
	else
		return recv_buffer.advance(maxlength);
}

int JNL_Connection::recv_lines_available(void)
{
	int l = (int)recv_bytes_available();
	int lcount = 0;
	int lastch = 0;
	
	for (int pos = 0; pos < l; pos ++)
	{
		char t;
		if (recv_buffer.at(pos, &t, 1) != 1)
			return lcount;
			
		if ((t=='\r' || t=='\n') &&( (lastch != '\r' && lastch != '\n') || lastch==t ))
			lcount++;

		lastch=t;
	}

	return lcount;
}

int JNL_Connection::recv_line(char *line, size_t maxlength)
{
	while (maxlength--)
	{
		char t;
		if (recv_buffer.read(&t, 1) == 0) 
		{
			*line=0;
			return 0;
		}

		if (t == '\r' || t == '\n')
		{
			char r;
			if (recv_buffer.peek(&r, 1) != 0)
			{
				if ((r == '\r' || r == '\n') && r != t)
					recv_buffer.advance(1);
			}

			*line=0;
			return 0;
			
		}

		*line++=t;
	}

	return 1;
}

unsigned long JNL_Connection::get_interface(void)
{
	if (m_socket==-1)
		return 0;

	struct sockaddr_in sin;
	memset(&sin,0,sizeof(sin));
	socklen_t len=sizeof(sin);

	if (::getsockname(m_socket,(struct sockaddr *)&sin,&len))
		return 0;

	return (unsigned long) sin.sin_addr.s_addr;
}

unsigned long JNL_Connection::get_remote()
{
	// TODO: IPv6
	if (address)
	{
		sockaddr_in *ipv4 = (sockaddr_in *)address;
		return ipv4->sin_addr.s_addr;
	}

	return 0;

}

unsigned short JNL_Connection::get_remote_port()
{
	return m_remote_port;
}

/* RingBuffer client function */
size_t JNL_Connection::Read(void *dest, size_t len)
{
	if (!len)
		return 0;

	int res=(int)socket_recv((char *)dest,len,0);

	if (res == 0 || (res < 0 && ERRNO != JNL_EWOULDBLOCK))
	{        
		m_state=STATE_CLOSED;
		return 0;
	}

	if (res > 0)
		return res;
	else
		return 0;
}

/* RingBuffer client function */
size_t JNL_Connection::Write(const void *dest, size_t len)
{
	if (!len)
		return 0;

	int res=(int)socket_send((const char *)dest,len,0);

	if (res==-1 && ERRNO != JNL_EWOULDBLOCK)
	{
		return 0;
		//              m_state=STATE_CLOSED;
	}

	if (res > 0)
		return res;
	else
		return 0;
}

int JNL_Connection::set_recv_buffer_size(size_t new_buffer_size)
{
	return recv_buffer.expand(new_buffer_size);
}

void JNL_Connection::reuse()
{
	if (m_state == STATE_CLOSED)
	{
		m_state = STATE_CONNECTED;
		recv_buffer.clear();
	}
}
