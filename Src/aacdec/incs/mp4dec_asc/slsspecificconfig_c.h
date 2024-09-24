/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2007)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/slsspecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: sls specific config interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __SLSSPECIFICCONFIG_C_H__
#define __SLSSPECIFICCONFIG_C_H__

#include "mp4dec_asc/audioobjecttypes.h"
#include "mp4dec_asc/programcfg_c.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

typedef struct CSSlsSpecificConfig {
  unsigned int m_pcmWordLength;
  unsigned int m_aacCorePresent;
  unsigned int m_lleMainStream;
  unsigned int m_reservedBit;
  unsigned int m_frameLength;

  CSProgramConfig m_progrConfigElement;
} CSSlsSpecificConfig, *CSSlsSpecificConfigPtr;

int getSlsFrameLen(int idx);

int SlsSpecificConfig_Parse(CSSlsSpecificConfigPtr self, struct CSBitStream *bs, const int channelConfiguration);
/* void SlsSpecificConfig_Set(CSSlsSpecificConfigPtr self, const CSSlsSpecificConfigPtr slssc, const AUDIO_OBJECT_TYPE aot); */
/* int SlsSpecificConfig_Compare(const CSSlsSpecificConfigPtr self, const CSSlsSpecificConfigPtr slssc, const AUDIO_OBJECT_TYPE aot); */
int SlsSpecificConfig_Print(CSSlsSpecificConfigPtr self, char string[]);

#ifdef __cplusplus
}
#endif

#endif /* __SLSSPECIFICCONFIG_H__ */
