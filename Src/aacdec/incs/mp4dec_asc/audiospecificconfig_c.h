/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/audiospecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: interface to audioSpecificConfig
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __AUDIOSPECIFICCONFIG_C_H__
#define __AUDIOSPECIFICCONFIG_C_H__

#include "mp4dec_asc/audioobjecttypes.h"
#include "mp4dec_asc/gaspecificconfig_c.h"
#include "mp4dec_asc/celpspecificconfig_c.h"
#include "mp4dec_asc/hvxcspecificconfig_c.h"
#include "mp4dec_asc/spatialspecificconfig_c.h"
#include "mp4dec_asc/slsspecificconfig_c.h"
#include "mp4dec_asc/mpeg12specificconfig_c.h"
#include "mp4dec_asc/epspecificconfig_c.h"
#include "mp4dec_asc/eldspecificconfig_c.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

typedef struct CSAudioSpecificConfig {

  AUDIO_OBJECT_TYPE m_aot;
  AUDIO_OBJECT_TYPE m_origAot;
  unsigned int      m_samplingFrequencyIndex;
  unsigned int      m_samplingFrequency;
  int               m_channelConfiguration;
  int               m_epConfig;
  unsigned int      m_directMapping;

  /* SBR/PS extension */
  int               m_sbrPresentFlag;
  int               m_psPresentFlag;
  int               m_mpsPresentFlag;
  int               m_saocPresentFlag;
  int               m_ldmpsPresentFlag;
  AUDIO_OBJECT_TYPE m_extensionAudioObjectType;
  unsigned int      m_extensionSamplingFrequencyIndex;
  unsigned int      m_extensionSamplingFrequency;

  /* */
  unsigned int m_nrOfStreams;
  unsigned int m_avgBitRate;
  unsigned int m_layer;

  /* derived values */
  int          m_channels;
  unsigned int m_samplesPerFrame;

  /* aot-specific asc's */
  CSGaSpecificConfig      m_gaSpecificConfig;
  CSCelpSpecificConfig    m_celpSpecificConfig;
  HvxcSpecificConfig      m_hvxcSpecificConfig;
  CSSpatialSpecificConfig m_mpegsSpecificConfig;
  CSSpatialSpecificConfig m_saocSpecificConfig;
  CSSpatialSpecificConfig m_ldmpegsSpecificConfig;
  CSMpeg12SpecificConfig  m_mpeg12SpecificConfig;
  CSSlsSpecificConfig     m_slsSpecificConfig;
  CSEldSpecificConfig     m_eldSpecificConfig;
  CEpSpecificConfig       m_epSpecificConfig;

} CSAudioSpecificConfig, *CSAudioSpecificConfigPtr;

int AudioSpecificConfig_ParseLatm_amv0(
                                       CSAudioSpecificConfigPtr self, 
                                       struct CSBitStream *bs
                                       );

int AudioSpecificConfig_ParseLatm_amv1(
                                       CSAudioSpecificConfigPtr self, 
                                       struct CSBitStream *bs
                                       );

int AudioSpecificConfig_ParseExt(
                                 CSAudioSpecificConfigPtr self, 
                                 CSAudioSpecificConfigPtr baselayer, 
                                 struct CSBitStream       *bs, 
                                 unsigned int             streamsPerLayer, 
                                 unsigned int             avgBitrate,
                                 unsigned int             latm_flag
                                 );

void  AudioSpecificConfig_Copy(
                               CSAudioSpecificConfigPtr self, 
                               const CSAudioSpecificConfigPtr asc
                               );

int AudioSpecificConfig_Compare(
                                const CSAudioSpecificConfigPtr self, 
                                const CSAudioSpecificConfigPtr asc
                                );

#ifdef ASC_PRINT
int AudioSpecificConfig_Print(
                              const CSAudioSpecificConfigPtr asc,
                              char string[]
                              );
#endif

AUDIO_OBJECT_TYPE __GetAotFromAsc(CSAudioSpecificConfigPtr asc);
int __GetLayerFromAsc(
                      CSAudioSpecificConfigPtr asc, 
                      CSAudioSpecificConfigPtr asc_base
                      );
char* __GetAotString(AUDIO_OBJECT_TYPE aot);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIOSPECIFICCONFIG_H__ */ 
