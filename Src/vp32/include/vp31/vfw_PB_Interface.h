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


/****************************************************************************
*
*   Module Title :     VFW_PB_INTERFACE.H
*
*   Description  :     Interface to video codec demo decompressor DLL
*
*
*****************************************************************************
*/

#include "codec_common_interface.h"

#include "type_aliases.h"
#ifndef XPB_DEFINED
#define XPB_DEFINED
typedef struct PB_INSTANCE * xPB_INST;
#endif
//#include "pbdll.h"

// YUV buffer configuration structure
typedef struct
{
    int     YWidth;
    int     YHeight;
    int     YStride;

    int     UVWidth;
    int     UVHeight;
    int     UVStride;

    unsigned char *  YBuffer;
    unsigned char *  UBuffer;
    unsigned char *  VBuffer;

} YUV_BUFFER_CONFIG;

//	Settings Control	
typedef enum
{
	PBC_SET_POSTPROC,
	PBC_SET_CPUFREE,
    PBC_MAX_PARAM
} PB_COMMAND_TYPE;


#ifdef __cplusplus
extern "C"
{
#endif

extern BOOL CCONV StartDecoder( xPB_INST *pbi, UINT32  ImageWidth, UINT32  ImageHeight );
extern void CCONV GetPbParam( xPB_INST, PB_COMMAND_TYPE Command, UINT32 *Parameter );
extern void CCONV SetPbParam( xPB_INST, PB_COMMAND_TYPE Command, UINT32 Parameter );
extern void CCONV GetYUVConfig( xPB_INST, YUV_BUFFER_CONFIG * YuvConfig );
extern const char * CCONV VP31D_GetVersionNumber(void);

extern int CCONV DecodeFrameToYUV( xPB_INST, char * VideoBufferPtr, unsigned int ByteCount,
                                    UINT32 ImageWidth, UINT32 ImageHeight );
extern BOOL CCONV StopDecoder(xPB_INST *pbi);

#ifdef __cplusplus
}
#endif



