/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2000)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_helpers/bitstream_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: bitstream module interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __BITSTREAMC_H__
#define __BITSTREAMC_H__

#include "mp4dec_helpers/machine.h"

/* CRC Markers for SetPositionMarker(). */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CSBitStream_MarkerPosition
{
  CSBitStream_ChannelElementStart,
  CSBitStream_ChannelElementStop,
  CSBitStream_AncillaryElementStart,
  CSBitStream_AncillaryElementStop,
  CSBitStream_SecondIndividualChannelStart,
  CSBitStream_CurrentPosition

} CSBitStream_MarkerPosition;

typedef enum {
  NO_EPCONF = -1,
  EPCONF0   = 0,
  EPCONF1   = 1,
  EPCONF2   = 2,
  EPCONF3   = 3
} __epconf_types;

typedef struct CSBitStream
{
  void  (*ByteAlign) (struct CSBitStream *self);
  void  (*PushBack)  (struct CSBitStream *self, INT32 n);
  INT32 (*Get)       (struct CSBitStream *self, INT32 elemID, INT32 n);

  void  (*SetPositionMarker)    (struct CSBitStream *self, INT32 id);
  void  (*SetCRCPositionMarker) (struct CSBitStream *self, CSBitStream_MarkerPosition position) ;
  
  INT32 (*GetBitCount)     (struct CSBitStream *self);
  INT32 (*GetValidBits)    (struct CSBitStream *self);
  INT32 (*GetValidBitsEp1) (struct CSBitStream *self, INT32 err_sens_class);

  UINT8* (*GetDataPointer) (struct CSBitStream *self);

  UINT32   m_BitsInCache;
  UINT32   m_CacheWord;
  INT32    m_erFlag;

} CSBitStream, *CSBitStreamPtr;

INT32 __getBits              (CSBitStreamPtr self, INT32 elemID, INT32 nBits);
void  __pushBack             (CSBitStreamPtr self, INT32 nBits);
void  __byteAlign            (CSBitStreamPtr self);
void  __setPositionMarker    (CSBitStreamPtr self, INT32 id);
void  __setCRCPositionMarker (CSBitStreamPtr self, CSBitStream_MarkerPosition position) ;
INT32 __getBitCount          (CSBitStreamPtr self);
INT32 __getValidBits         (CSBitStreamPtr self);
INT32 __getValidBits_ep1     (CSBitStreamPtr self, INT32 err_sens_class);
UINT8* __getDataPointer      (CSBitStreamPtr self);

#ifdef __cplusplus
}
#endif

#endif
