#include "precomp_wasabi_bfc.h"

#include "std_math.h"


void premultiplyARGB32(ARGB32 *words, int nwords) 
{
  for (; nwords > 0; nwords--, words++) 
	{
    unsigned char *pixel = (unsigned char *)words;
    unsigned int alpha = pixel[3];
    if (alpha == 255) continue;
    pixel[0] = (pixel[0] * alpha) >> 8;	// blue
    pixel[1] = (pixel[1] * alpha) >> 8;	// green
    pixel[2] = (pixel[2] * alpha) >> 8;	// red
  }
}

