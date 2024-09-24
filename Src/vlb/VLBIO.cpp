#include "VLBIO.h"
#include "vlbout.h"
#include "api__vlb.h"

///////////////////////////////////////////////////////////////////////
//
// VLB Data Input Class
//
///////////////////////////////////////////////////////////////////////


void VLBIn::Fill( unsigned char *buf, int nbytes )
{
	int writepos = pos + size;

	if (writepos >= BUFSIZE) writepos -= BUFSIZE;

	if (writepos + nbytes > BUFSIZE)
	{
		int l = BUFSIZE - writepos;
		memcpy(data + writepos, buf, l);
		buf += l;
		writepos += l;
		size += l;
		nbytes -= l;
		if (writepos >= BUFSIZE) writepos -= BUFSIZE;
	}

	memcpy( data + writepos, buf, nbytes );
	size += nbytes;
}

int VLBIn::IO( void *buf, int s, int n )
{
	int nbytes = s * n;
	void *obuf = buf;
	if ( nbytes > size )
	{
		nbytes = size - nbytes % s;
	}
	if (!nbytes) return 0;

	int bleft = nbytes;

	if (pos + nbytes > BUFSIZE)
	{
		int l = BUFSIZE - pos;
		memcpy(buf, data + pos, l);
		bleft -= l;
		pos = 0;
		buf = (char *)buf + l;
	}

	memcpy(buf, data + pos, bleft);

	size -= nbytes;
	pos += bleft;
	if (pos >= BUFSIZE) pos = 0;

	return nbytes / s;
}

///////////////////////////////////////////////////////////////////////
//
// VLB Data Output Class
//
///////////////////////////////////////////////////////////////////////

#define ADD_SAMPLE(T,S) { *(T *)(data + ((size + pos) & (OBUFSIZE-1))) = S; size += sizeof(T); }

#if !defined(__alpha) && !defined(_WIN64)
static __inline long float_to_long(double t)
{
	long r;
	__asm fld t
	__asm fistp r
	return r;
}
#else
#define float_to_long(x) ((long)( x ))
#endif

int VLBOut::IO( float **buf, int samples )
{
	if ( iError ) return iError;

	long l;
	for ( int i = 0; i < samples; i++ )
		for ( int j = 0; j < format.ucNChannels; j++ )
		{
			l = float_to_long( buf[j][i] );
			if ( l > 32767 ) l = 32767;
			if ( l < -32768 ) l = -32768;
			ADD_SAMPLE(short, (short)l)
		}

	return iError;
}

void VLBOut::PullBytes( unsigned char *buf, int nbytes )
{
	if (pos + nbytes >= OBUFSIZE)
	{
		int l = OBUFSIZE - pos;
		memcpy(buf, data + pos, l);
		pos = 0;
		buf += l;
		nbytes -= l;
		size -= l;
	}

	memcpy( buf, data + pos, nbytes );
	size -= nbytes;
	pos += nbytes;
	if (pos >= OBUFSIZE) pos = 0;
}
