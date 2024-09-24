#if !defined(VP50_COMP_INTERFACE_H)
#define VP50_COMP_INTERFACE_H
/****************************************************************************
*
*   Module Title :     VFW_COMP_INTERFACE.H
*
*   Description  :     Interface to video codec demo compressor DLL
*
*   AUTHOR       :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*   1.04 JBB 26 AUG 00 JBB Added fixed q setting
*   1.03 PGW 07/12/99  Retro fit JBB changes
*   1.02 PGW 16/09/99  Interface changes to simplify things for command line 
*                      compressor.
*   1.01 PGW 07/07/99  Added COMP_CONFIG.
*   1.00 PGW 28/06/99  New configuration baseline
*
*****************************************************************************
*/

//  C4514  Unreferenced inline function has been removed
#ifndef MACPPC
#pragma warning(disable: 4514)
#endif

#include "codec_common_interface.h"
#include "type_aliases.h"

/* Command interface to compressor. */
/* Settings Control */

typedef struct
{
    UINT32 FrameSize;
    UINT32 TargetBitRate;
    UINT32 FrameRate;
    UINT32 KeyFrameFrequency;
    UINT32 KeyFrameDataTarget;
    UINT32 Quality;
    BOOL   AllowDF;
    BOOL   QuickCompress;
    BOOL   AutoKeyFrameEnabled;
    INT32  AutoKeyFrameThreshold;
    UINT32 MinimumDistanceToKeyFrame;
    INT32  ForceKeyFrameEvery;
    INT32  NoiseSensitivity;
    BOOL   AllowSpatialResampling;

    // The Intended Horizontal Scale 
    UINT32 HScale;
    UINT32 HRatio;

    // The Intended Vertical Scale 
    UINT32 VScale;
    UINT32 VRatio;

    // The way in which we intended
    UINT32 ScalingMode;

    // Interlaced (0) means no (1) means Yes
    UINT32 Interlaced;

    BOOL   FixedQ;

    INT32  StartingBufferLevel;             // The initial encoder buffer level
    INT32  OptimalBufferLevel;              // The buffer level target we strive to reach / maintain.
    INT32  DropFramesWaterMark;             // Buffer fullness watermark for forced drop frames.
    INT32  ResampleDownWaterMark;           // Buffer fullness watermark for downwards spacial re-sampling
    INT32  ResampleUpWaterMark;             // Buffer fullness watermark where returning to larger image size is consdered
	INT32  OutputFrameRate;
	INT32  Speed;

	BOOL   ErrorResilientMode;				// compress using a mode that won't completely fall apart if we decompress using
	                                        // the frame after a dropped frame

} COMP_CONFIG_VP5;

INLINE
void comp_config_default_vp5(COMP_CONFIG_VP5* pcc)
{
    pcc->FrameSize = 0;  //  No default value
    pcc->TargetBitRate = 300;
    pcc->FrameRate = 0;  //  No default value
    pcc->KeyFrameFrequency = 120;
    pcc->KeyFrameDataTarget = 0;  //  No default value
    pcc->Quality = 56;
    pcc->AllowDF = 0;
    pcc->QuickCompress = 1;
    pcc->AutoKeyFrameEnabled = 1;
    pcc->AutoKeyFrameThreshold = 80;
    pcc->MinimumDistanceToKeyFrame = 8;
    pcc->ForceKeyFrameEvery = 120;
    pcc->NoiseSensitivity = 0;
    pcc->AllowSpatialResampling = 0;
    pcc->HScale = 1;
    pcc->HRatio = 1;
    pcc->VScale = 1;
    pcc->VRatio = 1;
    pcc->ScalingMode = MAINTAIN_ASPECT_RATIO;
    pcc->Interlaced = 0;
    pcc->FixedQ = 0;

    pcc->StartingBufferLevel   = 6;
    pcc->OptimalBufferLevel    = 10;
    pcc->DropFramesWaterMark   = 20;
    pcc->ResampleDownWaterMark = 35;
    pcc->ResampleUpWaterMark   = 45;

	pcc->OutputFrameRate = 30;
	pcc->Speed = 12;
	pcc->ErrorResilientMode = FALSE;

    return;
}

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

#endif 
