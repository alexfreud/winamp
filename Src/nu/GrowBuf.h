#ifndef NULLSOFT_UTILITY_GROWBUF_H
#define NULLSOFT_UTILITY_GROWBUF_H

#include <memory.h>
#include <stdlib.h>
class GrowBuf
{
public:
	GrowBuf()                                                         {}

	~GrowBuf()
	{
		if ( m_s )
			free( m_s );

		m_s = NULL;
	}

	void reserve( size_t len )
	{
		if ( len > m_alloc )
		{
			void *ne;
			m_alloc = len;
			ne = realloc( m_s, m_alloc );
			if ( !ne )
			{
				ne = malloc( m_alloc );
				memcpy( ne, m_s, m_used );
				free( m_s );
			}
			m_s = ne;
		}
	}

	size_t add( void *data, size_t len )
	{
		if ( !len )
			return 0;

		resize( m_used + len );
		memcpy( (char *)get() + m_used - len, data, len );

		return m_used - len;
	}

	void set( void *data, size_t len )
	{
		resize( len );
		memcpy( (char *)get(), data, len );
	}

	void resize( size_t newlen )
	{
		m_used = newlen;
		if ( newlen > m_alloc )
		{
			m_alloc = newlen * 2;
			if ( m_alloc < 1024 )
				m_alloc = 1024;

			void *ne = realloc( m_s, m_alloc );
			if ( !ne )
			{
				ne = malloc( m_alloc );
				if ( !ne )
					*( (char *)ne ) = NULL;

				memcpy( ne, m_s, m_used );
				free( m_s );
			}

			m_s = ne;
		}
	}

	size_t getlen()
	{
		return m_used;
	}

	void *get()
	{
		return m_s;
	}

private:
	void   *m_s     = NULL;
	size_t  m_alloc = 0;
	size_t  m_used  = 0;
};

#endif