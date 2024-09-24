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


/*/////////////////////////////////////////////////////////////////////////
//
// dxlvinfd.c
//
// Purpose: A list of helper functions to the quick time codec code
//
///////////////////////////////////////////////////////////////////////*/

//#include <stdio.h>
//#include <math.h>
//#include <string.h>
#include "dxl_main.h"

struct DisplaySetting {
	long dotOne;
	long dotTwo;
	long dotThree;
	long dotFour;
	long dotFive;
};

static struct DisplaySetting id_RGB24 ={0x00000000,0x00000000,0xffffffff,0x00000000,0xffffffff}; 
static struct DisplaySetting id_RGB32 ={0x00000000,0x00000000,0x00000000,0x00000000,0xffffffff}; 
static struct DisplaySetting id_RGB555={0xffffffff,0x00000000,0xffffffff,0x00000000,0xffffffff}; 
static struct DisplaySetting id_RGB565={0xffffffff,0x00000000,0x00000000,0x00000000,0xffffffff}; 
static struct DisplaySetting id_UYVY  ={0xff80ff80,0x00800080,0xff80ff80,0x00800080,0x00800080}; 
static struct DisplaySetting id_YUY2  ={0x80ff80ff,0x80008000,0x80008000,0x80008000,0x80008000}; 
static struct DisplaySetting id_YVU9  ={0x80008000,0x80008000,0xff80ff80,0xff80ff80,0xff80ff80}; 
static struct DisplaySetting id_RGB8  ={0x00000000,0xffffffff,0x00000000,0xffffffff,0x00000000}; 


static struct DisplaySetting id_STRETCH 		={0x00000000,0xffffffff,0x00000000,0x00000000,0x00000000}; 
static struct DisplaySetting id_STRETCH_BRIGHT ={0xffffffff,0xffffffff,0x00000000,0x00000000,0x00000000}; 
static struct DisplaySetting id_STRETCH_SAME   ={0xffffffff,0x00000000,0x00000000,0x00000000,0x00000000}; 

static struct DisplaySetting id_KEY 	= 	{0x00000000,0x00000000,0xffffffff,0x00000000,0x00000000}; 
static struct DisplaySetting id_NOTKEY 	=	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000}; 

static struct DisplaySetting id_CLEAR_ME 	=	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000}; 


static void OrSettings(struct DisplaySetting *src1,struct DisplaySetting *src2, struct DisplaySetting *dst)
{
	if (dst) {
		dst->dotOne = src1->dotOne | src2->dotOne;
		dst->dotTwo = src1->dotTwo | src2->dotTwo;
		dst->dotThree = src1->dotThree | src2->dotThree;
		dst->dotFour = src1->dotFour | src2->dotFour;
		dst->dotFive = src1->dotFive | src2->dotFive;
	}
}


static void SetSettings(struct DisplaySetting *dst,struct DisplaySetting *src)
{
	if (dst) {
		dst->dotOne = src->dotOne ;
		dst->dotTwo = src->dotTwo ;
		dst->dotThree = src->dotThree ;
		dst->dotFour = src->dotFour ;
		dst->dotFive = src->dotFive ;
	}
}

