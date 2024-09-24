/*
** JNetLib
** Copyright (C) 2008 Nullsoft, Inc.
** Author: Ben Allison
** File: multicastlisten.cpp - JNL Multicast UDP listen implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "multicastlisten.h"
#include "foundation/error.h"
#include <new>

int CreateMulticastListener(JNL_UDPConnection **connection, const char *mcast_ip, unsigned short port, size_t sendbufsize, size_t recvbufsize)
{
	char portString[32] = {0};

	if (port)
		sprintf(portString, "%d", (int)port);

	addrinfo *res=0;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; /* IPv4 only for now until we get IPv6 multicast registration working */
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
	hints.ai_protocol = IPPROTO_UDP;

	if (getaddrinfo(NULL, port?portString:0, &hints, &res) == 0)
	{
		SOCKET m_socket = ::socket(res->ai_family,res->ai_socktype, res->ai_protocol);  
		if (m_socket < 0)
		{
			freeaddrinfo(res);
			return NErr_Error;
		}
		else
		{
			SET_SOCK_BLOCK(m_socket,0);

			int bflag = 1;
			if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&bflag, sizeof(bflag)))
			{
				/*int err = ERRNO;
				err=err;
				printf("SO_REUSEADDR error: %d\n", err);*/
			}
#if defined(__FreeBSD__) || defined(__APPLE__)
			bflag=1; // in case it magically got unset above
			setsockopt(m_socket, SOL_SOCKET, SO_REUSEPORT, (const char *)&bflag, sizeof(bflag));
#endif
			if (::bind(m_socket, res->ai_addr, (int)res->ai_addrlen))
			{
				closesocket(m_socket);
				return NErr_Error;
			}
			else
			{
				// TODO: ipv6 with IPV6_ADD_MEMBERSHIP and ipv6_mreq

				sockaddr_in *ipv4 = (sockaddr_in *)res->ai_addr;

				/* join multicast group */
				ip_mreq ssdpMcastAddr;
				memset(&ssdpMcastAddr, 0, sizeof(ssdpMcastAddr));
				ssdpMcastAddr.imr_interface = ipv4->sin_addr;
				ssdpMcastAddr.imr_multiaddr.s_addr = inet_addr(mcast_ip);
				if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&ssdpMcastAddr, sizeof(ssdpMcastAddr)))
				{
					closesocket(m_socket);
					freeaddrinfo(res);
					return NErr_Error;
				}

				/* Set multicast interface. */
				in_addr addr;
				memset(&addr, 0, sizeof(addr));
				addr = ipv4->sin_addr;
				if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_IF, (const char *)&addr, sizeof(addr))) 
				{
					/*int err = ERRNO;
					err=err;
					printf("IP_MULTICAST_IF error: %d\n", err);*/
					/* This is probably not a critical error, so let's continue. */
				}

				/* set TTL to 4 */
				uint8_t ttl=4;
				if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl)))
				{
					/*int err = ERRNO;
					err=err;
					printf("IP_MULTICAST_TTL error: %d\n", err);*/
					/* This is probably not a critical error, so let's continue. */
				}

				int option = 1;
				if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&option, sizeof(option)) != 0) 
				{
					closesocket(m_socket);
					freeaddrinfo(res);
					return NErr_Error;
				}
			}
		}
		freeaddrinfo(res);
		JNL_UDPConnection *c=new (std::nothrow) JNL_UDPConnection();
		if (!c)
		{
			closesocket(m_socket);
			return NErr_OutOfMemory;
		}
		c->open((int)m_socket, NULL, sendbufsize, recvbufsize);
		*connection = c;
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}
}

