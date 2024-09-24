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


#ifndef	ID3LIB_TYPES_28BITINT_H
#define	ID3LIB_TYPES_28BITINT_H


#include "id3_types.h"


class int28
{
public:
				int28( uint32_t val = 0 ); // used for int32 -> int28 conversion
				int28( const uchar *val ); // used for int28 -> int32 conversion
				int28 &setFromFile(uint32_t val); // used for int28 -> int32 conversion

uchar			operator[]						( size_t posn );
luint			get								( void );

// *** PRIVATE INTERNAL DATA - DO NOT USE *** PRIVATE INTERNAL DATA - DO NOT USE ***

protected:
void			set								( uint32_t val );
uchar			value[ sizeof ( luint ) ];		// the integer stored as a uchar array
};


#endif


