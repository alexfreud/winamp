#include "foundation/error.h"
#include "foundation/types.h"

#include "headers.h"
#include "netinc.h"

#include <stdlib.h>
#include <string.h>

JNL_Headers::JNL_Headers()
{
	m_recvheaders      = NULL;
	m_recvheaders_size = 0;
}

JNL_Headers::~JNL_Headers()
{
	if ( m_recvheaders )
		free( m_recvheaders );
}

void JNL_Headers::Reset()
{
	if ( m_recvheaders )
		free( m_recvheaders );

	m_recvheaders      = NULL;
	m_recvheaders_size = 0;
}

const char *JNL_Headers::GetAllHeaders()
{
	// double null terminated, null delimited list
	if ( m_recvheaders )
		return m_recvheaders;
	else
		return "\0\0";
}

const char *JNL_Headers::GetHeader( const char *headername )
{
	char *ret = NULL;

	if ( headername[ 0 ] == 0 || !m_recvheaders )
		return NULL;

	size_t headername_size = strlen( headername );

	char *buf = (char *)malloc( headername_size + 2 );
	strcpy( buf, headername );

	if ( buf[ headername_size - 1 ] != ':' )
	{
		buf[ headername_size++ ] = ':';
		buf[ headername_size ]   = 0;
	}

	char *p = m_recvheaders;
	while ( p && *p )
	{
		if ( !strncasecmp( buf, p, headername_size ) )
		{
			ret = p + headername_size;
			while ( ret && *ret && *ret == ' ' )
				ret++;

			break;
		}

		p += strlen( p ) + 1;
	}

	free( buf );

	return ret;
}

int JNL_Headers::Add( const char *buf )
{
	if ( !m_recvheaders )
	{
		m_recvheaders_size = strlen( buf ) + 1;
		if ( m_recvheaders_size == 0 || m_recvheaders_size == (size_t)-1 ) // check for overflow
		{
			return NErr_OutOfMemory;
		}

		m_recvheaders = (char *)malloc( m_recvheaders_size + 1 );
		if ( m_recvheaders )
		{
			strcpy( m_recvheaders, buf ); // safe because we malloc'd specifically above
			m_recvheaders[ m_recvheaders_size ] = 0;
		}
		else
		{
			return NErr_OutOfMemory;
		}
	}
	else
	{
		size_t oldsize = m_recvheaders_size;
		m_recvheaders_size += strlen( buf ) + 1;
		if ( m_recvheaders_size + 1 < oldsize ) // check for overflow
		{
			return NErr_OutOfMemory;
		}

		char *n = (char *)realloc( m_recvheaders, m_recvheaders_size + 1 );
		if ( !n )
		{
			return NErr_OutOfMemory;
		}

		strcpy( n + oldsize, buf ); // safe because we malloc specifially for the size
		n[ m_recvheaders_size ] = 0; // double null terminate
		m_recvheaders = n;
	}

	return NErr_Success;
}
