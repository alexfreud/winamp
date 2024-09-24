/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/gaspecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: global audio specific config interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __GASPECIFICCONFIG_C_H__
#define __GASPECIFICCONFIG_C_H__

#include "mp4dec_asc/audioobjecttypes.h"
#include "mp4dec_asc/programcfg_c.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

typedef struct CSGaSpecificConfig {
  unsigned int m_frameLengthFlag;
  unsigned int m_dependsOnCoreCoder;
  unsigned int m_coreCoderDelay;

  unsigned int m_extensionFlag;
  unsigned int m_extensionFlag3;

  unsigned int m_layer;
  unsigned int m_numOfSubFrame;
  unsigned int m_layerLength;

  unsigned int m_vcb11Flag;
  unsigned int m_rvlcFlag;
  unsigned int m_hcrFlag;

  CSProgramConfig m_progrConfigElement;

} CSGaSpecificConfig, *CSGaSpecificConfigPtr;

void GaSpecificConfig_Parse(CSGaSpecificConfigPtr self, struct CSBitStream *bs, const int channelConfiguration, const AUDIO_OBJECT_TYPE aot);
void GaSpecificConfig_Copy(CSGaSpecificConfigPtr self, const CSGaSpecificConfigPtr gasc);
int  GaSpecificConfig_Compare(const CSGaSpecificConfigPtr self, const CSGaSpecificConfigPtr gasc, const AUDIO_OBJECT_TYPE aot);
#ifdef ASC_PRINT
int  GaSpecificConfig_Print(const CSGaSpecificConfigPtr gasc, char string[], const int channelConfiguration, const AUDIO_OBJECT_TYPE aot);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __GASPECIFICCONFIG_H__ */
