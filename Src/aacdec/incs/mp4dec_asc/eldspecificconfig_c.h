/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2007)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/eldspecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: eld specific specific config parser - interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __ELDPECIFICCONFIG_C_H__
#define __ELDPECIFICCONFIG_C_H__

#include "mp4dec_asc/audioobjecttypes.h"

#define MAX_SBR_HEADER_SIZE       4
#define MAX_ELD_SBR_ELEMENTS      8
#define MAX_ELD_EXTENSIONS        6
#define MAX_ELD_EXTENSIONS_LENGTH 32

#define ASC_ELD_ELDEXT_TERM     0x0000
#define ASC_ELD_ELDEXT_SAOC     0x0001
#define ASC_ELD_ELDEXT_LDSAC    0x0002
#define ASC_ELD_ELDEXT_PSEUDOMC 0x0003

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

typedef struct {
  int           tag;
  int           length;
  unsigned char config_payload[MAX_ELD_EXTENSIONS_LENGTH];      
} ELD_EXTENSION;

typedef struct CSEldSpecificConfig {
  unsigned int m_frameLengthFlag;

  unsigned int m_vcb11Flag;
  unsigned int m_rvlcFlag;
  unsigned int m_hcrFlag;

  unsigned int m_ldSbrPresentFlag;
  unsigned int m_ldSbrSamplingRateFlag;
  unsigned int m_ldSbrCrcFlag;

  unsigned int m_useLDQMFTimeAlignment;

  unsigned char m_ldSbrHeaderData[MAX_ELD_SBR_ELEMENTS][MAX_SBR_HEADER_SIZE];
  ELD_EXTENSION m_eldExtension[MAX_ELD_EXTENSIONS];

} CSEldSpecificConfig, *CSEldSpecificConfigPtr;

void EldSpecificConfig_Parse(CSEldSpecificConfigPtr self, struct CSBitStream *bs, const int channelConfiguration, const AUDIO_OBJECT_TYPE aot, int* ldmpspresent, int* saocpresent);
void EldSpecificConfig_Copy(CSEldSpecificConfigPtr self, const CSEldSpecificConfigPtr gasc);
int  EldSpecificConfig_Compare(const CSEldSpecificConfigPtr self, const CSEldSpecificConfigPtr eldsc, const AUDIO_OBJECT_TYPE aot);
#ifdef ASC_PRINT
int  EldSpecificConfig_Print(const CSEldSpecificConfigPtr eldsc, char string[], const int channelConfiguration, const AUDIO_OBJECT_TYPE aot);
#endif
ELD_EXTENSION* getEldExtension( int tag, const CSEldSpecificConfigPtr eldsc);

#ifdef __cplusplus
}
#endif

#endif /* __ELDPECIFICCONFIG_H__ */
