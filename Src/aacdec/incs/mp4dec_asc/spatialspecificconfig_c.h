/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2007)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/spatialspecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: spatial specific config interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __SPATIALSPECIFICCONFIG_C_H__
#define __SPATIALSPECIFICCONFIG_C_H__

#include "mp4dec_asc/audioobjecttypes.h"
#include "sac_dec_ssc_struct.h"

#define MAX_SSC_SIZE  (256)

#ifdef __cplusplus
extern "C" {
#endif

struct CSBitStream;

typedef struct CSSpatialSpecificConfig {
  unsigned int            m_sacPayloadEmbedding;
  unsigned char           m_sscbinarybuffer[MAX_SSC_SIZE];
  unsigned int            m_ssclen;
  SPATIAL_SPECIFIC_CONFIG m_sscstruct;
  unsigned int            m_isLdmps;
} CSSpatialSpecificConfig, *CSSpatialSpecificConfigPtr;

  int SpatialSpecificConfig_Parse(CSSpatialSpecificConfigPtr self, struct CSBitStream *bs, unsigned int isLdmps);
/* void SpatialSpecificConfig_Set(CSSpatialSpecificConfigPtr self, const CSSpatialSpecificConfigPtr mpegssc, const AUDIO_OBJECT_TYPE aot); */
/* int SpatialSpecificConfig_Compare(const CSSpatialSpecificConfigPtr self, const CSSpatialSpecificConfigPtr mpegssc, const AUDIO_OBJECT_TYPE aot); */
int SpatialSpecificConfig_Print(CSSpatialSpecificConfigPtr self, char string[]);

#ifdef __cplusplus
}
#endif

#endif /* __SPATIALSPECIFICCONFIG_H__ */
