/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1996)
 *                        All Rights Reserved
 *
 *   filename: l3reg.h
 *   project : <none>
 *   author  : Martin Sieler
 *   date    : 1996-11-05
 *   contents/description: HEADER - registered types for MPEG Layer-3
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:02 $
 * $Header: /cvs/root/nullsoft/Replicant/jni/nsmp3/l3reg.h,v 1.1 2010/11/17 20:46:02 audiodsp Exp $
 */

#ifndef __L3REG_H__
#define __L3REG_H__
/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

#ifdef _MSC_VER
  #pragma pack(push, 1) /* assume byte packing throughout */
#endif

/*-------------------------------------------------------------------------*/

//==========================================================================;
//
// ISO/MPEG Layer3 Format Tag
//
#define WAVE_FORMAT_MPEGLAYER3 0x0055

//==========================================================================;
//
// Manufacturer ID and Product ID
//
#define MM_FRAUNHOFER_IIS     172
#define MM_FHGIIS_MPEGLAYER3   10

#define MM_FHGIIS_MPEGLAYER3_DECODE        9
#define MM_FHGIIS_MPEGLAYER3_LITE         10
#define MM_FHGIIS_MPEGLAYER3_BASIC        11
#define MM_FHGIIS_MPEGLAYER3_ADVANCED     12
#define MM_FHGIIS_MPEGLAYER3_PROFESSIONAL 13

#define MM_FHGIIS_MPEGLAYER3_ADVANCEDPLUS 14

//==========================================================================;
//
//
//
//==========================================================================;

#ifdef MPEGLAYER3_WFX_EXTRA_BYTES
  //
  // seems like the structure below is already defined
  //
#else

//==========================================================================;
//
// MPEG Layer3 WAVEFORMATEX structure
//
#define MPEGLAYER3_WFX_EXTRA_BYTES   12

// WAVE_FORMAT_MPEGLAYER3 format structure
//
typedef struct tagMPEGLAYER3WAVEFORMAT
  {
  WAVEFORMATEX	wfx;

  WORD          wID;
  DWORD         fdwFlags;
  WORD          nBlockSize;
  WORD          nFramesPerBlock;
  WORD          nCodecDelay;

  } MPEGLAYER3WAVEFORMAT;

typedef MPEGLAYER3WAVEFORMAT       * PMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT NEAR  *NPMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT FAR   *LPMPEGLAYER3WAVEFORMAT;

#endif

//==========================================================================;

#define MPEGLAYER3_ID_UNKNOWN            0
#define MPEGLAYER3_ID_MPEG               1
#define MPEGLAYER3_ID_CONSTANTFRAMESIZE  2

#define MPEGLAYER3_FLAG_PADDING_ISO      0x00000000
#define MPEGLAYER3_FLAG_PADDING_ON       0x00000001
#define MPEGLAYER3_FLAG_PADDING_OFF      0x00000002

#define MPEGLAYER3_FLAG_CRC_ON           0x00000010
#define MPEGLAYER3_FLAG_CRC_OFF          0x00000020

#define MPEGLAYER3_FLAG_VBR              0x00000100

/*-------------------------------------------------------------------------*/

#ifdef _MSC_VER
  #pragma pack(pop) /* revert to previous packing */
#endif

/*-------------------------------------------------------------------------*/
#endif
