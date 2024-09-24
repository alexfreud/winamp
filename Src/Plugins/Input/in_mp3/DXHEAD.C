
/*---- DXhead.c --------------------------------------------
 
 
decoder MPEG Layer III
 
handle Xing header
 
mod 12/7/98 add vbr scale
 
Copyright 1998 Xing Technology Corp.
-----------------------------------------------------------*/
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "dxhead.h" 


/*-------------------------------------------------------------*/
int SeekPoint(unsigned char TOC[100], int file_bytes, float percent)
{
	// interpolate in TOC to get file seek point in bytes
	int a, seekpoint;
	float fa, fb, fx;


	if (percent < 0.0f)
		percent = 0.0f;
	if (percent > 100.0f)
		percent = 100.0f;

	a = (int)percent;
	if (a > 99) a = 99;
	fa = TOC[a];
	if (a < 99)
	{
		fb = TOC[a + 1];
	}
	else
	{
		fb = 256.0f;
	}


	fx = fa + (fb - fa) * (percent - a);

	seekpoint = (int) ((1.0f / 256.0f) * fx * file_bytes);


	return seekpoint;
}
/*-------------------------------------------------------------*/
