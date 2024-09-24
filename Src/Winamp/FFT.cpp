/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#define _USE_MATH_DEFINES
#include <math.h>
#include "main.h"
#include "fft.h"
#include "../nsutil/fft.h"

static nsutil_fft_t fft9;


void fft_init()
{
	if (!fft9)
		nsutil_fft_Create_F32R(&fft9, 9, nsutil_fft_fast);
}

void fft_9(float wave[512])
{
	fft_init(); // getting crash reports of this not being created. strange
	nsutil_fft_Forward_F32R_IP(fft9, wave);
}


