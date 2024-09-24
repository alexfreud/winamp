/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: asyncdns.cpp - JNL portable asynchronous DNS implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"

#include "wac_network_dns.h"

#include <time.h>

#ifdef _WIN32
#include <strsafe.h>
#endif

enum
{
	MODE_RESOLVE = 0,
	MODE_REVERSE = 1,
};


struct  cache_entry
{
	time_t          last_used; // timestamp.
	bool            resolved;
	int             mode;      // 1=reverse
	unsigned short  port;
	char            hostname[ 256 ];
	addrinfo *addr;
	int             sockettype;
};

wa::Components::WAC_Network_AsyncDNS::WAC_Network_AsyncDNS( int max_cache_entries )
{
	m_thread_kill = 1;
	m_thread = 0;
	m_cache_size = max_cache_entries;
	m_cache = (cache_entry *)malloc( sizeof( cache_entry ) * m_cache_size );

	if ( m_cache )
		memset( m_cache, 0, sizeof( cache_entry ) * m_cache_size );
	else
		m_cache_size = 0;
}

wa::Components::WAC_Network_AsyncDNS::~WAC_Network_AsyncDNS()
{
	m_thread_kill = 1;

#ifdef _WIN32
	if ( m_thread )
	{
		WaitForSingleObject( m_thread, INFINITE );
		CloseHandle( m_thread );
	}
#else
	if ( m_thread )
	{
		void *p;
		pthread_join( m_thread, &p );
	}
#endif//!_WIN32

	// free all the addrinfo stuff
	for ( int x = 0; x < m_cache_size; x++ )
	{
		if ( m_cache[ x ].addr )
			freeaddrinfo( m_cache[ x ].addr );
	}

	free( m_cache );
}

int wa::Components::WAC_Network_AsyncDNS::resolvenow( const char *hostname, unsigned short port, addrinfo **addr, int sockettype )
{
	addrinfo hints;
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = PF_UNSPEC;

	if ( hostname )
		hints.ai_flags = AI_NUMERICHOST;
	else
		hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

	hints.ai_socktype = sockettype;

	char portString[ 32 ] = { 0 };
	sprintf( portString, "%u", (unsigned int)port );

	if ( getaddrinfo( hostname, portString, &hints, addr ) == 0 )
	{
		return 0;
	}
	else
	{
		hints.ai_flags = 0;
		if ( getaddrinfo( hostname, portString, &hints, addr ) == 0 )
			return 0;
		else
			return -1;
	}
}

#ifdef _WIN32
unsigned long WINAPI wa::Components::WAC_Network_AsyncDNS::_threadfunc( LPVOID _d )
#else
unsigned int WAC_Network_AsyncDNS::_threadfunc( void *_d )
#endif
{
	int nowinsock = JNL::open_socketlib();
	wa::Components::WAC_Network_AsyncDNS *_this = (WAC_Network_AsyncDNS *)_d;
	int x;
	for ( x = 0; x < _this->m_cache_size && !_this->m_thread_kill; x++ )
	{
		if ( _this->m_cache[ x ].last_used && !_this->m_cache[ x ].resolved )
		{
			if ( !nowinsock )
			{
				if ( _this->m_cache[ x ].mode == 0 )
				{
					addrinfo *res = 0;
					if ( resolvenow( _this->m_cache[ x ].hostname, _this->m_cache[ x ].port, &res, _this->m_cache[ x ].sockettype ) == 0 )
					{
						_this->m_cache[ x ].addr = res;
					}
					else
					{
						_this->m_cache[ x ].addr = 0; //INADDR_NONE;
					}
				}
				else if ( _this->m_cache[ x ].mode == 1 )
				{
					/*
					hostent *ent;
					// TODO: replace with getnameinfo for IPv6
					ent=gethostbyaddr((const char *)&_this->m_cache[x].addr,4,AF_INET);
					if (ent)
						lstrcpyn(_this->m_cache[x].hostname, ent->h_name, 256);
					else
						_this->m_cache[x].hostname[0]=0;
					*/
				}

				_this->m_cache[ x ].resolved = true;
			}
			else
			{
				if ( _this->m_cache[ x ].mode == 0 )
				{
					_this->m_cache[ x ].addr = 0;//INADDR_NONE;
					_this->m_cache[ x ].resolved = true;
				}
				else if ( _this->m_cache[ x ].mode == 1 )
				{
					_this->m_cache[ x ].hostname[ 0 ] = 0;
					_this->m_cache[ x ].resolved = true;
				}
			}
		}
	}

	if ( !nowinsock )
		JNL::close_socketlib();

	_this->m_thread_kill = 1;

	return 0;
}

int wa::Components::WAC_Network_AsyncDNS::resolve( const char *hostname, unsigned short port, addrinfo **addr, int sockettype )
{
	// return 0 on success, 1 on wait, -1 on unresolvable
	int x;

	for ( x = 0; x < m_cache_size; x++ )
	{
		if ( !strcasecmp( m_cache[ x ].hostname, hostname ) && port == m_cache[ x ].port && m_cache[ x ].mode == 0 && m_cache[ x ].sockettype == sockettype )
		{
			m_cache[ x ].last_used = time( 0 );
			if ( m_cache[ x ].resolved )
			{
				if ( m_cache[ x ].addr == 0 )//INADDR_NONE)
				{
					return DNS_RESOLVE_UNRESOLVABLE;
				}

				*addr = m_cache[ x ].addr;
				return DNS_RESOLVE_SUCCESS;
			}

			makesurethreadisrunning();
			return DNS_RESOLVE_WAIT;
		}
	}

	// add to resolve list
	int oi = -1;
	for ( x = 0; x < m_cache_size; x++ )
	{
		if ( !m_cache[ x ].last_used )
		{
			oi = x;
			break;
		}

		if ( ( oi == -1 || m_cache[ x ].last_used < m_cache[ oi ].last_used ) && m_cache[ x ].resolved )
		{
			oi = x;
		}
	}

	if ( oi == -1 )
	{
		return DNS_RESOLVE_UNRESOLVABLE;
	}

#ifdef _WIN32
	StringCchCopyA( m_cache[ oi ].hostname, 256, hostname );
#elif defined(__APPLE__)
	strlcpy( m_cache[ oi ].hostname, hostname, 255 );
#else
	strncpy( m_cache[ oi ].hostname, hostname, 255 );
	m_cache[ oi ].hostname[ 255 ] = 0;
#endif

	m_cache[ oi ].port       = port;
	m_cache[ oi ].mode       = 0;
	m_cache[ oi ].addr       = 0;//INADDR_NONE;
	m_cache[ oi ].resolved   = false;
	m_cache[ oi ].last_used  = time( 0 );
	m_cache[ oi ].sockettype = sockettype;

	makesurethreadisrunning();

	return DNS_RESOLVE_WAIT;
}

void wa::Components::WAC_Network_AsyncDNS::makesurethreadisrunning( void )
{
	if ( m_thread_kill )
	{
#ifdef _WIN32
		if ( m_thread )
		{
			WaitForSingleObject( m_thread, INFINITE );
			CloseHandle( m_thread );
		}
		DWORD id;
		m_thread_kill = 0;
		m_thread = CreateThread( NULL, 0, _threadfunc, (LPVOID)this, 0, &id );
		if ( !m_thread )
		{
#else
		if ( m_thread )
		{
			void *p;
			pthread_join( m_thread, &p );
		}
		m_thread_kill = 0;
		if ( pthread_create( &m_thread, NULL, ( void *( * ) ( void * ) )_threadfunc, (void *)this ) != 0 )
		{
#endif
			m_thread_kill = 1;
		}
		}
	}

#define CBCLASS wa::Components::WAC_Network_AsyncDNS
START_DISPATCH;
CB( API_DNS_RESOLVE, resolve );
END_DISPATCH;
#undef CBCLASS
