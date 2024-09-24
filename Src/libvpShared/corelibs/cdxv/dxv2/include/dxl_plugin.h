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


#ifndef _dxl_plugin_h
#define _dxl_plugin_h

#include "duck_dxl.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* max number of algorithms to be supported at one time */
#define NUM_ALG 16

typedef void * DXL_HANDLE;

typedef unsigned int DXL_OBJECT_VERSION;

typedef DXL_HANDLE (*CREATE_FUNC)(DXL_XIMAGE_HANDLE, unsigned char *);

typedef DXL_HANDLE (*RECREATE_FUNC)(DXL_XIMAGE_HANDLE,void *,int,int,int,int); 

typedef int (*DESTROY_FUNC)(DXL_XIMAGE_HANDLE); 

typedef int (*SEED_DATA_FUNC)(DXL_XIMAGE_HANDLE); 

typedef int (*DX_FUNC)(DXL_XIMAGE_HANDLE, DXL_VSCREEN_HANDLE); 

typedef void (*SET_PARAMETER_FUNC)(DXL_XIMAGE_HANDLE, int , unsigned int);
//typedef int (*SET_PARAMETER_FUNC)(DXL_XIMAGE_HANDLE, int , unsigned int);

typedef int (*GET_PARAMETER_FUNC)(DXL_XIMAGE_HANDLE, int , unsigned int);

typedef int (*SEND_VMSG_FUNC)(DXL_XIMAGE_HANDLE, void *, unsigned int);


int DXL_GetAlgHandle(unsigned int fourcc);
DXL_HANDLE DXL_GetAlgorithmBasePtr(DXL_XIMAGE_HANDLE src);

int DXL_RegisterXImage(CREATE_FUNC creator, unsigned int fourcc);
int DXL_RegisterXImageRecreate(DXL_XIMAGE_HANDLE src, RECREATE_FUNC thisFunc);
int DXL_RegisterXImageDestroy(DXL_XIMAGE_HANDLE src, DESTROY_FUNC thisFunc);
int DXL_RegisterXImageSeedData(DXL_XIMAGE_HANDLE src, SEED_DATA_FUNC thisFunc);
int DXL_RegisterXImageDx(DXL_XIMAGE_HANDLE src, DX_FUNC thisFunc);

int DXL_RegisterXImageSetParameter(DXL_XIMAGE_HANDLE src, SET_PARAMETER_FUNC thisFunc);
int DXL_RegisterXImageGetParameter(DXL_XIMAGE_HANDLE src, GET_PARAMETER_FUNC thisFunc);

int DXL_RegisterXImageSendVideoMessage(DXL_XIMAGE_HANDLE src, SEND_VMSG_FUNC thisFunc);


#define DXL_MKFOURCC( ch0, ch1, ch2, ch3 ) \
		( (unsigned int)(unsigned char)(ch0) | ( (unsigned int)(unsigned char)(ch1) << 8 ) |    \
		( (unsigned int)(unsigned char)(ch2) << 16 ) | ( (unsigned int)(unsigned char)(ch3) << 24 ) )

#if defined(__cplusplus)
}
#endif

#endif
