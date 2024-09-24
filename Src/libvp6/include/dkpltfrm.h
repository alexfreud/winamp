//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------

#ifndef _dkpltfrm_h
#define _dkpltfrm_h
/********************************************************

		PC for Win95/DOS/etc...

********************************************************/

/*
#define DX_COUNTERS 0
*/


/* #define VOXWARE_WIN32 1 */


#define SUPPORT565 1
#define DX_TR20 1
#define TINKER 1
#define LARGECHUNKS 1

#define RGBORDER 0
#define BGRORDER 1


#define DKTRUE 1
#define DKFALSE !DKTRUE

#define TBLOFFSET 0
#define CENTER_TABLE 0

#define BLACK16X2 0x00000000

//#include "nofar.h"
#include "littlend.h"

#define LIMITREADS	/* limit reads average frame size */
#define LIMIT_1_5	/* limit reads to 1.5x the average frame size */
#define DISPLAYDIB 0

#define AUDIOINTERLEAVED 1
typedef int GfsHn;
#define slow_seek duck_seek
#define gooseCD(x,y)

#define COLORORDER RGBORDER

#define SWAPENDS 0      

#define HW_CD_BUFFER 0  
#define CD_ONLY 0

#define DX24BIT


#if !defined(UINT64)
typedef unsigned __int64 UINT64;
#endif



#endif /* include guards */
