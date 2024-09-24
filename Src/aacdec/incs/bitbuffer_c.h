/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   filename: bitbuffer_c.h
 *   project : MPEG-4 Audio Decoder
 *   author  : Stefan Gewinner gew@iis.fhg.de
 *   contents/description: memory input class with transport format
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
 * $Header: /cvs/root/winamp/aacdec/incs/bitbuffer_c.h,v 1.3 2012/05/08 20:16:49 audiodsp Exp $
 *
\***************************************************************************/

#ifndef __BITBUFFER_C_H__
#define __BITBUFFER_C_H__

#include "mp4dec_helpers/machine.h"
#include "mp4dec_helpers/bitstream_c.h"

struct CSStreamInfo;

enum
{
  CSBitBuffer_BufferSize = 8192,
  CSBitBuffer_BufferBits = CSBitBuffer_BufferSize*8,

  /* CBitBuffer_InvalidCrcValue = 0xFFFFFFFF, */
  CBitBuffer_MaximumMarkers = 25*4

  /* , CAverageNumber_MaximumSize = 16 */
};


typedef struct
{
  CSBitStream_MarkerPosition what ;

  UINT32 m_elementBits ;

  UINT32 m_ValidBits;
  UINT32 m_BitCnt;
  UINT32 m_BitNdx;

} CMarker, *CMarkerPtr ;

typedef struct CSBitBuffer
{
  CSBitStream base ;

  UINT32 m_ValidBits;
  UINT32 m_ReadOffset;
  UINT32 m_BitCnt;
  UINT32 m_BitNdx;

  UINT32 m_FramesLeftInPacket ;
  UINT32 m_FrameCrcValue ;

  INT32 m_isEOF ;

  UINT8 m_Buffer [CSBitBuffer_BufferSize] ;

  /* CAverageNumber m_ActualBitrate ; */

  UINT32 m_LastBufferFullness ;
  UINT32 m_LastFrameLength ;

  CMarker m_MarkerList [CBitBuffer_MaximumMarkers] ;
  UINT32 m_Markers ;

} CSBitBuffer, *CSBitBufferPtr ;

#ifdef __cplusplus
extern "C" {
#endif

void CSBitBuffer_Initialize(CSBitBufferPtr self);

void CSBitBuffer_Feed(CSBitBufferPtr self, const UINT8 pBuf[], const UINT32 cbSize, UINT32 *cbValid);

INT32 CSBitBuffer_IsDecodableFrame(CSBitBufferPtr self, struct CSStreamInfo *info);
INT32 CSBitBuffer_IsCrcConsistent(CSBitBufferPtr self) ;

void CSBitBuffer_SetEOF(CSBitBufferPtr self) ;

#ifdef __cplusplus
}
#endif

#endif
