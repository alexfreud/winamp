/****************************************************************************
*
*   Module Title :     vp60_comp_interface.h
*
*   Description  :     Interface to VP60 compressor.
*
****************************************************************************/
#ifndef __INC_VP60_COMP_INTERFACE_H
#define __INC_VP60_COMP_INTERFACE_H

#ifdef _MSC_VER
#pragma warning(disable: 4514) // Disable warning 4514: Unreferenced inline function
#endif

/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common_interface.h"
#include "type_aliases.h"
#include "string.h"

#define DEFAULT_VALUE -999
/****************************************************************************
*  Typedefs
****************************************************************************/
typedef enum
{
    USAGE_STREAM_FROM_SERVER    = 0x0,  //  INTER prediction, (0,0) motion vector implied.
    USAGE_LOCAL_FILE_PLAYBACK   = 0x1  //  INTER prediction, (0,0) motion vector implied.
} END_USAGE;


typedef enum
{
    MODE_REALTIME       = 0x0,
    MODE_GOODQUALITY    = 0x1,
    MODE_BESTQUALITY    = 0x2,
    MODE_FIRSTPASS      = 0x3,
    MODE_SECONDPASS     = 0x4,
    MODE_SECONDPASS_BEST= 0x5
} MODE;



/* Command interface to compressor */
typedef struct
{
    //UINT32 FourCC;
    //UINT32 ConfigVersion;
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

    BOOL   ErrorResilientMode;              // compress using a mode that won't completely fall apart if we decompress using
                                            // the frame after a dropped frame
    INT32  Profile;

    BOOL   DisableGolden;                   // disable golden frame updates
    BOOL   VBMode;                          // run in variable bandwidth 1 pass mode
    UINT32 BestAllowedQ;                    // best allowed quality ( save bits by disallowings frames that are too high quality )
    INT32  UnderShootPct;                   // target a percentage of the actual frame to allow for sections that go over

    INT32  MaxAllowedDatarate;              // maximum the datarate is allowed to go.
    INT32  MaximumBufferSize;               // maximum buffer size.

    BOOL   TwoPassVBREnabled;               // two pass variable bandwidth enabled
    INT32  TwoPassVBRBias;                  // how variable do we want to target?
    INT32  TwoPassVBRMaxSection;            // maximum
    INT32  TwoPassVBRMinSection;            // minimum
    INT32  Pass;                            // which pass of the compression are we running.

    MODE   Mode;
    END_USAGE  EndUsage;

    char   FirstPassFile[512];
    char   SettingsFile[512];
    char   RootDirectory[512];

	INT32  PlaceHolder;
    INT32  DeleteFirstPassFile;
	INT32  Sharpness;

} COMP_CONFIG_VP6;


typedef struct
{
    int   YWidth;
    int   YHeight;
    int   YStride;

    int   UVWidth;
    int   UVHeight;
    int   UVStride;

    char *YBuffer;
    char *UBuffer;
    char *VBuffer;

} YUV_INPUT_BUFFER_CONFIG;

/****************************************************************************
*  Functions
****************************************************************************/
#ifdef _MSC_VER
_inline 
void comp_config_default_vp6 ( COMP_CONFIG_VP6* pcc )
{


    pcc->FrameSize = 0;             //  No default value
    pcc->TargetBitRate = 300;
    pcc->FrameRate = 0;             //  No default value
    pcc->KeyFrameFrequency = 120;
    pcc->KeyFrameDataTarget = 0;    //  No default value
    pcc->Quality = 56;
    pcc->AllowDF = 0;
    pcc->AutoKeyFrameEnabled = 1;
    pcc->AutoKeyFrameThreshold = 80;
    pcc->MinimumDistanceToKeyFrame = 4;
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

    pcc->StartingBufferLevel   = 4;
    pcc->OptimalBufferLevel    = 5;
    pcc->DropFramesWaterMark   = 20;
    pcc->ResampleDownWaterMark = 35;
    pcc->ResampleUpWaterMark   = 45;

    pcc->OutputFrameRate = 30;
    pcc->Speed = 4;
    pcc->ErrorResilientMode = FALSE;

    pcc->Profile = 0;

    pcc->DisableGolden = 0;
    pcc->BestAllowedQ = 4;
    pcc->UnderShootPct = 90;

    pcc->MaxAllowedDatarate = 100;
    pcc->MaximumBufferSize = 6;

    pcc->TwoPassVBRBias = 70;
    pcc->TwoPassVBRMaxSection = 400;
    pcc->TwoPassVBRMinSection = 40;


    pcc->Mode = MODE_GOODQUALITY;
    pcc->EndUsage = USAGE_STREAM_FROM_SERVER;

    // DEFAULT means default value as determined by mode and endusage
    pcc->QuickCompress = DEFAULT_VALUE;
    pcc->Pass = DEFAULT_VALUE;
    pcc->VBMode = DEFAULT_VALUE;
    pcc->TwoPassVBREnabled = DEFAULT_VALUE;

    pcc->SettingsFile[0] = 0;
    pcc->RootDirectory[0] = 0;
	pcc->Sharpness = 5;

    strncpy(pcc->FirstPassFile,"firstpass.fpf",512);
    //pcc->FourCC = '06PV';
    //pcc->ConfigVersion = 4;

    return;
}
#endif
#endif
