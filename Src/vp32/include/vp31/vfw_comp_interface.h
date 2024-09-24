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


#if !defined(VFW_COMP_INTERFACE_H)
#define VFW_COMP_INTERFACE_H
/****************************************************************************
*
*   Module Title :     VFW_COMP_INTERFACE.H
*
*   Description  :     Interface to video codec demo compressor DLL
*
*
*****************************************************************************
*/

#include "codec_common_interface.h"
#include "type_aliases.h"

/* Command interface to compressor. */
/*	Settings Control	*/

typedef enum
{	
    C_SET_KEY_FRAME,
    C_SET_FIXED_Q,
    C_SET_FIRSTPASS_FILE,
	C_SET_NODROPS

} C_SETTING;

typedef enum
{
    MAINTAIN_ASPECT_RATIO   = 0x0,
    SCALE_TO_FIT            = 0x1,
    CENTER                  = 0x2,
    OTHER                   = 0x3
} SCALE_MODE;

typedef struct
{
	UINT32	FrameSize;
	UINT32	TargetBitRate;
    UINT32  FrameRate;
    UINT32  KeyFrameFrequency;
    UINT32  KeyFrameDataTarget;
    UINT32  Quality;
    BOOL    AllowDF;
    BOOL    QuickCompress;
	BOOL    AutoKeyFrameEnabled;
	INT32   AutoKeyFrameThreshold;
	UINT32  MinimumDistanceToKeyFrame;
	INT32   ForceKeyFrameEvery;
	INT32	NoiseSensitivity;

} COMP_CONFIG;

#ifndef YUVINPUTBUFFERCONFIG
#define YUVINPUTBUFFERCONFIG
typedef struct
{
    int     YWidth;
    int     YHeight;
    int     YStride;

    int     UVWidth;
    int     UVHeight;
    int     UVStride;

    char *  YBuffer;
    char *  UBuffer;
    char *  VBuffer;

} YUV_INPUT_BUFFER_CONFIG;
#endif

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct CP_INSTANCE * xCP_INST;
extern BOOL CCONV StartEncoder( xCP_INST *cpi, COMP_CONFIG * CompConfig );
extern void CCONV ChangeCompressorSetting ( xCP_INST cpi, C_SETTING Setting, int Value );
extern void CCONV ChangeEncoderConfig (  xCP_INST cpi, COMP_CONFIG * CompConfig );
extern UINT32 CCONV EncodeFrame(  xCP_INST cpi, unsigned char * InBmpIPtr, unsigned char * InBmpPtr, unsigned char * OutPutPtr, unsigned int * is_key );
extern UINT32 CCONV EncodeFrameYuv(  xCP_INST cpi, YUV_INPUT_BUFFER_CONFIG *  YuvInputData, unsigned char * OutPutPtr, unsigned int * is_key );
extern BOOL CCONV StopEncoder( xCP_INST *cpi);
extern void VPEInitLibrary(void);
extern void VPEDeInitLibrary(void);
extern const char * CCONV VP31E_GetVersionNumber(void);

#ifdef __cplusplus

}
#endif

#endif  //  VFW_COMP_INTERFACE_H
