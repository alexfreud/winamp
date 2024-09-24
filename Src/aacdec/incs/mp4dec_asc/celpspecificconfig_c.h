/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2002)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/celpspecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: celp specific config interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __CELPSPECIFICCONFIG_C_H__
#define __CELPSPECIFICCONFIG_C_H__

#include "mp4dec_asc/audioobjecttypes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

enum {
  MPE     = 0,
  RPE     = 1,
  fs8KHz  = 0,
  fs16KHz = 1
};

typedef struct CSCelpSpecificConfig 
{
  int m_IsBaseLayer;
  int m_ExcitationMode;
  int m_SampleRateMode;
  int m_FineRateControl;
  int m_RPE_Configuration;
  int m_MPE_Configuration;
  int m_NumEnhLayers;
  int m_BandwidthScalabilityMode;
  int m_SilenceCompression;
  int m_BWS_configuration;
  int m_CelpBrsId;

  unsigned int m_isBWSLayer;

} CSCelpSpecificConfig, *CSCelpSpecificConfigPtr;

void CelpSpecificConfig_Parse(CSCelpSpecificConfigPtr self, CSCelpSpecificConfigPtr baselayer, const AUDIO_OBJECT_TYPE aot, struct CSBitStream *bs);
int  CelpSpecificConfig_GetSamplesPerFrame(CSCelpSpecificConfigPtr self);
int  CelpSpecificConfig_GetLayer(CSCelpSpecificConfigPtr csc, CSCelpSpecificConfigPtr csc_base);
void CelpSpecificConfig_Copy(CSCelpSpecificConfigPtr dst, const CSCelpSpecificConfigPtr src);
int  CelpSpecificConfig_Print(CSCelpSpecificConfigPtr self, char string[], const AUDIO_OBJECT_TYPE aot);

#ifdef __cplusplus
}
#endif

#endif /* __CELPSPECIFICCONFIG_H__ */
