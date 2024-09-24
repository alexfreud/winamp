/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2003)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/mpeg12specificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: MP3OnMP4 specific config interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __MPEG12SPECIFICCONFIG_C_H__
#define __MPEG12SPECIFICCONFIG_C_H__

#include "mp4dec_asc/audioobjecttypes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

typedef struct CSMpeg12SpecificConfig {
  /* unsigned int m_ID; */
  /* unsigned int m_layer; */
  unsigned int m_reserved;
} CSMpeg12SpecificConfig, *CSMpeg12SpecificConfigPtr;

int Mpeg12SpecificConfig_Parse(CSMpeg12SpecificConfigPtr self, struct CSBitStream *bs);
/* void Mpeg12SpecificConfig_Set(CSMpeg12SpecificConfigPtr self, const CSMpeg12SpecificConfigPtr mp12sc, const AUDIO_OBJECT_TYPE aot); */
/* int Mpeg12SpecificConfig_Compare(const CSMpeg12SpecificConfigPtr self, const CSMpeg12SpecificConfigPtr mp12sc, const AUDIO_OBJECT_TYPE aot); */

#ifdef __cplusplus
}
#endif

#endif /* __MPEG12SPECIFICCONFIG_H__ */
