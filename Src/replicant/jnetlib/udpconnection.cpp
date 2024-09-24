/*
** JNetLib
** Copyright (C) 2000-2001 Nullsoft, Inc.
** Author: Justin Frankel
** File: udpconnection.cpp - JNL UDP connection implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "udpconnection.h"

JNL_UDPConnection::JNL_UDPConnection(unsigned short incoming_port, JNL_AsyncDNS *dns, int sendbufsize, int recvbufsize)
{
	init();
	open(dns, sendbufsize, recvbufsize);

	m_socket=::socket(PF_INET,SOCK_DGRAM,0);
	if (m_socket==-1)
	{
		m_errorstr="creating socket";
		m_state=STATE_ERROR;
	}
	SET_SOCK_BLOCK(m_socket,0);
	sockaddr_in m_iaddr;
	memset(&m_iaddr,0,sizeof(struct sockaddr_in));
	m_iaddr.sin_family=AF_INET;
	m_iaddr.sin_port=htons(incoming_port);
	m_iaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	if(::bind(m_socket,(struct sockaddr *)&m_iaddr,sizeof(m_iaddr))==-1)
	{
		m_errorstr="binding socket";
		m_state=STATE_ERROR;
	}
	m_state=STATE_CONNECTED;
}

JNL_UDPConnection::JNL_UDPConnection()
{
	init();
}

void JNL_UDPConnection::init()
{
	m_errorstr="";
	address=0;
	address_len=0;
	m_dns=0;
	m_dns_owned=false;
	m_socket=-1;
	m_remote_port=0;
	m_state=STATE_NOCONNECTION;
	m_host=0;
	saddr=0;
	m_last_addr_len=0;
	ttl=0;
}

void JNL_UDPConnection::set_ttl(uint8_t new_ttl)
{
	ttl=new_ttl;
	setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl));
}

void JNL_UDPConnection::open(int incoming_socket, JNL_AsyncDNS *dns, size_t sendbufsize, size_t recvbufsize)
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

	m_socket=incoming_socket;
	m_state=STATE_CONNECTED;

}

void JNL_UDPConnection::open(JNL_AsyncDNS *dns, size_t sendbufsize, size_t recvbufsize)
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

void JNL_UDPConnection::setpeer(const char *hostname, int port)
{
	m_remote_port=(unsigned short)port;
	m_host = _strdup(hostname);

	if (!m_host || !m_host[0])
	{
		m_errorstr="empty hostname";
		m_state=STATE_ERROR;
	}
	else
	{
		m_state=STATE_RESOLVING;
	}
}

void JNL_UDPConnection::setpeer(sockaddr *addr, socklen_t length /* of addr */)
{
	//memcpy(&m_saddr, addr, sizeof(sockaddr));
	free(address);
	address_len=length;
	address=(sockaddr *)malloc(length);
	memcpy(address, addr, length);
}

JNL_UDPConnection::~JNL_UDPConnection()
{
	if (m_socket >= 0)
	{
		::closesocket(m_socket);
		m_socket=-1;
	}
	if (!saddr) // free it if it was passed to us (by JNL_Listen, presumably)
		free(address); // TODO: change this if we ever do round-robin DNS connecting or in any way change how we handle 'address'

	if (m_dns_owned) 
	{
		delete static_cast<JNL_AsyncDNS *>(m_dns);
	}
	free(m_host);
}

void JNL_UDPConnection::run(size_t max_send_bytes, size_t max_recv_bytes, size_t *bytes_sent, size_t *bytes_rcvd)
{
	size_t bytes_allowed_to_send=(max_send_bytes<0)?send_buffer.size():max_send_bytes;
	size_t bytes_allowed_to_recv=(max_recv_bytes<0)?recv_buffer.avail():max_recv_bytes;

	if (bytes_sent) *bytes_sent=0;
	if (bytes_rcvd) *bytes_rcvd=0;

	switch (m_state)
	{
	case STATE_RESOLVING:
		if (saddr == 0)
		{
			int a=m_dns->resolve(m_host, m_remote_port, &saddr, SOCK_DGRAM);
			if (!a) 
			{
				address=saddr->ai_addr;
				address_len=(socklen_t)saddr->ai_addrlen;
				m_state=STATE_CONNECTED; 
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
		break;
	case STATE_CONNECTED:
	case STATE_CLOSING:
		if (!send_buffer.empty() && bytes_allowed_to_send>0)
		{
			size_t sent = send_buffer.drain(this, bytes_allowed_to_send);
			if (bytes_sent)
				*bytes_sent+=sent;
		}
		/* only read from socket when buffer is empty 
		* otherwise we risk data loss
		* see "man 2 recvfrom" for details
		*/
		if (recv_buffer.empty() && bytes_allowed_to_recv)
		{
			/*
			* use LockBuffer()/UnlockBuffer() because
			* "wrap-around" reads can't be done
			* we might read data from two separate packets
			*/
			size_t len = recv_buffer.avail();
			if (bytes_allowed_to_recv < len)
				len = bytes_allowed_to_recv;

			recv_buffer.clear();
			void *buffer = recv_buffer.LockBuffer();
			m_last_addr_len = (int)sizeof(m_last_addr);
			int res=::recvfrom(m_socket,(char *)buffer,(int)len,0,(sockaddr *)&m_last_addr,&m_last_addr_len);
			if (res == 0 || (res < 0 && ERRNO != JNL_EWOULDBLOCK))
			{ 
				recv_buffer.UnlockBuffer(0);
				m_state=STATE_CLOSED;
				break;
			}
			if (res > 0)
			{
				if (bytes_rcvd) 
					*bytes_rcvd+=res;
				recv_buffer.UnlockBuffer(res);
			}
			else
				recv_buffer.UnlockBuffer(0);

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

/* RingBuffer client function */
size_t JNL_UDPConnection::Read(void *dest, size_t len)
{
	if (!len)
		return 0;

	m_last_addr_len = (int)sizeof(m_last_addr);
	int res=::recvfrom(m_socket, (char *)dest, (int)len, 0, (sockaddr *)&m_last_addr,&m_last_addr_len);
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
size_t JNL_UDPConnection::Write(const void *dest, size_t len)
{
	if (!len)
		return 0;
	int res=::sendto(m_socket, (const char *)dest, (int)len, 0, address, address_len);
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

void JNL_UDPConnection::close(int quick)
{
	if (quick || m_state == STATE_RESOLVING)
	{
		m_state=STATE_CLOSED;
		if (m_socket >= 0)
		{
			::closesocket(m_socket);
		}
		m_socket=-1;
		recv_buffer.clear();
		send_buffer.clear();
		m_remote_port=0;
		free(m_host);
		m_host=0;
		//memset(&m_saddr,0,sizeof(m_saddr));
	}
	else
	{
		if (m_state == STATE_CONNECTED) m_state=STATE_CLOSING;
	}
}

size_t JNL_UDPConnection::send_bytes_in_queue(void)
{
	return send_buffer.size();
}

size_t JNL_UDPConnection::send_bytes_available(void)
{
	return send_buffer.avail();
}

int JNL_UDPConnection::send(const void *data, size_t length)
{
	if (length > send_bytes_available())
	{
		return -1;
	}

	send_buffer.write(data, length);
	return 0;
}

int JNL_UDPConnection::send_string(const char *line)
{
	return send(line,strlen(line));
}

size_t JNL_UDPConnection::recv_bytes_available(void)
{
	return recv_buffer.size();
}

size_t JNL_UDPConnection::peek_bytes(void *data, size_t maxlength)
{
	return recv_buffer.peek(data, maxlength);
}

size_t JNL_UDPConnection::recv_bytes(void *data, size_t maxlength)
{
	return recv_buffer.read(data, maxlength);
}

int JNL_UDPConnection::recv_lines_available(void)
{
	int l=(int)recv_bytes_available();
	int lcount=0;
	int lastch=0;
	int pos;
	for (pos=0; pos < l; pos ++)
	{
		char t;
		if (recv_buffer.at(pos, &t, 1) != 1)
			return lcount;
		if ((t=='\r' || t=='\n') &&(
			(lastch != '\r' && lastch != '\n') || lastch==t
			)) lcount++;
		lastch=t;
	}
	return lcount;
}

int JNL_UDPConnection::recv_line(char *line, size_t maxlength)
{
	if (maxlength > recv_buffer.size()) maxlength=recv_buffer.size();
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

int JNL_UDPConnection::get_interface(sockaddr *sin, socklen_t *sin_length)
{
	if (m_socket==-1) return 1;
	/*memset(sin,0,sizeof(sockaddr_storage));
	*sin_length =sizeof(sockaddr_storage);*/
	if (::getsockname(m_socket,(sockaddr *)sin,sin_length)) 
		return 1;
	return 0;
}

