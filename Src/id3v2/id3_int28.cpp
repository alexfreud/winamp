// The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
// patent or other intellectual property protection in this work. This means that
// it may be modified, redistributed and used in commercial and non-commercial
// software and hardware without restrictions. ID3Lib is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
// Mon Nov 23 18:34:01 1998


#include "id3_int28.h"
#include <bfc/platform/types.h>

int28::int28(uint32_t val)
{
 set ( val );
}


int28::int28(const uchar *val)
{
 for ( int i = 0; i < sizeof ( uint32_t ); i++ )
 value[ i ] = val[ i ];
}


void int28::set(uint32_t val)
{
 for ( int i = 0; i < sizeof ( uint32_t ); i++ )
 value[ sizeof ( uint32_t ) - 1 - i ] = (uint8_t) ( ( val >> ( i * 7 ) ) & 127 ) & 0xFF;

 return;
}

int28 &int28::setFromFile(uint32_t val)
{
 for ( int i = 0; i < sizeof ( uint32_t ); i++ )
	value[sizeof(uint32_t) - 1 - i] = (uint8_t) (val >>(i*8)) & 0xFF;

 return *this;
}


luint int28::get ( void )
{
 luint newSize = 0L;
 uchar bytes [ 4 ];

 bytes[ 3 ] = value[ 3 ] | ( ( value[ 2 ] & 1 ) << 7 );
 bytes[ 2 ] = ( ( value[ 2 ] >> 1 ) & 63 ) | ( ( value[ 1 ] & 3 ) << 6 );
 bytes[ 1 ] = ( ( value[ 1 ] >> 2 ) & 31 ) | ( ( value[ 0 ] & 7 ) << 5 );
 bytes[ 0 ] = ( ( value[ 0 ] >> 3 ) & 15 );

 newSize = bytes[ 3 ] | ( (luint) bytes[ 2 ] << 8 ) | ( (luint) bytes[ 1 ] << 16 ) | ( (luint) bytes[ 0 ] << 24 );

 return newSize;
}


uchar int28::operator[] ( luint posn )
{
 return value[ posn ];
}

