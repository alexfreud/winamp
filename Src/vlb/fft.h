/* $Header: /cvs/root/winamp/vlb/fft.h,v 1.1 2009/04/28 20:21:09 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: fft.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: FFT include file
 *
 * $Header: /cvs/root/winamp/vlb/fft.h,v 1.1 2009/04/28 20:21:09 audiodsp Exp $
 *
\***************************************************************************/

#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#ifndef FFTH
#define FFTH

#ifndef pi
#define pi	3.1415926535897932384626434f
#endif

void fftl(float* ,float* ,int );

#endif
