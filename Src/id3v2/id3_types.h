//  The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
//  patent or other intellectual property protection in this work.  This means that
//  it may be modified, redistributed and used in commercial and non-commercial
//  software and hardware without restrictions.  ID3Lib is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//  
//  The ID3Lib authors encourage improvements and optimisations to be sent to the
//  ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org).  Approved
//  submissions may be altered, and will be included and released under these terms.
//  
//  Mon Nov 23 18:34:01 1998


#ifndef	ID3LIB_TYPES_H
#define	ID3LIB_TYPES_H


#include <wchar.h>
#include <bfc/platform/types.h>


typedef	unsigned char		uchar;
typedef short signed int	ssint;
typedef short unsigned int	suint;
typedef long signed int		lsint;
typedef size_t luint;
typedef	long double			ldoub;
typedef long unsigned int *	bitset;

#define	BS_SET(v,x)			( (v)[ (x) / ( sizeof ( luint ) * 8 ) ] |=  ( 1 << ( (x) % ( sizeof ( luint ) * 8 ) ) ) )
#define	BS_CLEAR(v,x)		( (v)[ (x) / ( sizeof ( luint ) * 8 ) ] &= ~( 1 << ( (x) % ( sizeof ( luint ) * 8 ) ) ) )
#define	BS_ISSET(v,x)		( (v)[ (x) / ( sizeof ( luint ) * 8 ) ] &   ( 1 << ( (x) % ( sizeof ( luint ) * 8 ) ) ) )

#ifndef	NULL
#define	NULL	(0L)
#endif

#ifndef	MIN
inline lsint	MIN								( lsint x, lsint y )
{
	return x < y ? x : y;
}
#endif

#ifndef	MAX
inline lsint	MAX								( lsint x, lsint y )
{
	return x > y ? x : y;
}
#endif

// include other abstract types here because they
// may depend on the types defined above
#include "id3_int28.h"

#endif


