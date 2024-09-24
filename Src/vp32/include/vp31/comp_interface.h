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
*   Module Title :     COMP_INTERFACE.H
*
*   Description  :     Interface to video codec demo compressor DLL
*
*
*****************************************************************************
*/

#ifndef COMP_INTERFACE_H
#define COMP_INTERFACE_H

#define  INC_WIN_HEADER      1
#include <windows.h>

#include "codec_common_interface.h"
#include "type_aliases.h"

/* Command interface to compressor. */
/*	Settings Control	*/
typedef enum
{	C_FRAME_SIZE,                
	C_QUALITY,                  // Quality trade off value (0 best to 15 worst)
	C_DATA_RATE,
    C_FRAME_RATE,
	C_FORCE_BASE_FRAME,         // Forces a key frame
	C_RESYNCH_VIDEO,            // Forces video to re-synch
    C_SETUP_GRABBER,
    C_RESET_FRAME_COUNTER,      // Relates to stats - may be removed

    /* Specialist test facilities. */
    C_ON_LINE,                  // Test function soon to be removed

    C_INTER_PREDICTION,         // Test code soon to be removed
    C_MOTION_COMP,              // Test code soon to be removed
    C_EXT_SRC,                  // Test code soon to be removed

} C_SETTING;

typedef enum
{
    LOCAL_NORMAL,
    LOCAL_GF,
    
} LOCAL_DISP_MODE;

typedef struct
{
    UINT32 CompTime;
    UINT32 MeanCompTime;
    UINT32 PPTime;
    UINT32 MeanPPTime;

} COMP_TIMINGS;

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct CP_INSTANCE * xCP_INST;
extern BOOL CCONV StartEncoder( xCP_INST cpi, UINT32 FrameSize, UINT32 TargetRate, UINT32 Quality );
extern int CCONV GetCompressorSetting ( xCP_INST cpi, C_SETTING Setting );
extern void CCONV ChangeCompressorSetting ( xCP_INST cpi, C_SETTING Setting, int Value );
extern INT32 CCONV GetVideoData( xCP_INST cpi, UINT8 * VideoBufferPtr );
extern BOOL CCONV StopEncoder(xCP_INST cpi );
extern BOOL CCONV GetFrameDetails( xCP_INST cpi, UINT32 *  FrameNumber, UINT32 * FrameSize );
extern void CCONV GetStillFrame( xCP_INST cpi, BOOL FullStill );
extern void CCONV ResumeVideo(xCP_INST cpi);
extern xCP_INST CreateCPInstance(void);
extern void DeleteCPInstance(xCP_INST *cpi);

#ifdef _cplusplus
}
#endif

#endif