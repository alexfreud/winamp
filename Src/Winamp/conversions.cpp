#include "main.h"
#include <math.h>
#pragma intrinsic(fabs)

/*
#ifndef _WIN64
__inline static int lrint(double flt) 
{
	int intgr;

	_asm
	{
		fld flt
		fistp intgr
	}

	return intgr;
} 
#else
__inline static int lrint(double flt) 
{
	return (int)flt;
} 
#endif
*/

#define PA_CLIP_( val, min, max )\
    { val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

 void Float32_To_Int16_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count)
{
    float *src = (float*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;

    while( count-- )
    {
        long samp = lrint((*src * (32768.0)));

        PA_CLIP_( samp, -0x8000, 0x7FFF );
        *dest = (signed short) samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

inline static double clip(double x, double a, double b)
{
   double x1 = fabs (x-a);
   double x2 = fabs (x-b);
   x = x1 + (a+b);
   x -= x2;
   x *= 0.5;
	 return x;
}

/*
benski> this might be faster than what the compiler spits out for the above function,
but we should benchmark
inline static double clip(double x, double a, double b)
{
	const double zero_point_five = 0.5;
	__asm 
	{
		fld	x
		fld a
		fld b

		fld st(2)
		fsub st(0),st(2) // x-b
		fabs
		fadd st(0),st(2) 
		fadd st(0),st(1)

		fld st(3)
		fsub st(0), st(2)
		fabs
		fsubp st(1), st(0)
		fmul zero_point_five

		ffree st(4)
		ffree st(3)
		ffree st(2)
		ffree st(1)
	}
}
*/


void Float32_To_Int24_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count)
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;

    while( count-- )
    {
        /* convert to 32 bit and drop the low 8 bits */
        double scaled = *src * 0x7FFFFFFF;
        scaled=clip( scaled, -2147483648., 2147483647.  );
        signed long temp = (signed long) scaled;

        dest[0] = (unsigned char)(temp >> 8);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 24);
        src += sourceStride;
        dest += destinationStride * 3;
    }
}
