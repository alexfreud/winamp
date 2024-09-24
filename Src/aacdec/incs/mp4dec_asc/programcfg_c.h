/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2002)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/programcfg_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: program config specific description
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/


#ifndef __PROGRAMCFGC_H__
#define __PROGRAMCFGC_H__

#include "mp4dec_helpers/cubuffer_c.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

enum
{
  /* PCE settings */
  CSProgramConfig_FrontRow             = 0,
  CSProgramConfig_SideRow              = 1,
  CSProgramConfig_BackRow              = 2,
  CSProgramConfig_SpeakerRows          = 3,

  CSProgramConfig_MaximumChannels      = 16,
  CSProgramConfig_MaximumCommentLength = 256,

  /* aac profile definitions */
  CAAC_ProfileMain = 0,
  CAAC_ProfileLowComplexity,
  CAAC_ProfileSSR,
  CAAC_ProfileLTP,
  CAAC_ProfileScalable,
  CAAC_ProfileReserved
};


typedef struct CSProgramConfig
{
  /* non-multichannel configuration */

  int           m_NonMC_ConfigSet;
  int           m_NonMC_IsCPE;

  int           m_NonMC_Tag;

  /* */

  int           m_ExplicitMapping;

  int           m_Profile;
  int           m_SamplingFrequencyIndex;

  int           m_NumLfeChannelElements;
  int           m_NumAssocDataElements;
  int           m_NumValidCcElements;

  int           m_MonoMixdownPresent;
  int           m_MonoMixdownElementNumber;

  int           m_StereoMixdownPresent;
  int           m_StereoMixdownElementNumber;

  int           m_MatrixMixdownIndexPresent;
  int           m_MatrixMixdownIndex;
  int           m_PseudoSurroundEnable;

  unsigned int  m_NumberOfChannels[CSProgramConfig_SpeakerRows];
  int           m_NumberOfChannelElements[CSProgramConfig_SpeakerRows];

  int           m_ChannelElementIsCpe[CSProgramConfig_SpeakerRows][CSProgramConfig_MaximumChannels];
  unsigned int  m_ChannelElementTagSelect[CSProgramConfig_SpeakerRows][CSProgramConfig_MaximumChannels];

  unsigned int  m_LfeElementTagSelect[CSProgramConfig_MaximumChannels];
  unsigned int  m_AssocDataElementTagSelect[CSProgramConfig_MaximumChannels];

  int           m_CcElementIsIndSw[CSProgramConfig_MaximumChannels];
  unsigned int  m_ValidCcElementTagSelect[CSProgramConfig_MaximumChannels];

  int           m_CommentFieldBytes;
  unsigned char m_Comment[CSProgramConfig_MaximumCommentLength];

} CSProgramConfig, *CSProgramConfigPtr;

/* methods */

void CSProgramConfig_Initialize(CSProgramConfigPtr self);
void CSProgramConfig_Read      (CSProgramConfigPtr self, struct CSBitStream *bs);
void CSProgramConfig_ReadExt   (CSProgramConfigPtr self, struct CSBitStream *bs, CCompositionUnitPtr cubuffer, const unsigned int tag);
void CSProgramConfig_Copy      (CSProgramConfigPtr dst, const CSProgramConfigPtr src);

int  CSProgramConfig_AddChannel(CSProgramConfigPtr self, const unsigned int tag, const unsigned int isCPE);


int  CSProgramConfig_AddSingleChannel      (CSProgramConfigPtr self, const unsigned int tag);
int  CSProgramConfig_AddChannelPair        (CSProgramConfigPtr self, const unsigned int tag);
int  CSProgramConfig_AddCouplingChannel    (CSProgramConfigPtr self, const unsigned int tag);
int  CSProgramConfig_AddLowFrequencyChannel(CSProgramConfigPtr self, const unsigned int tag);

int  CSProgramConfig_IsChannelLocatedAt    (CSProgramConfigPtr self, const unsigned int row, const unsigned int tag);
int  CSProgramConfig_IsChannelPairLocatedAt(CSProgramConfigPtr self, const unsigned int row, const unsigned int tag);

/* explicit query */


/* query by index */

int CSProgramConfig_GetNumberOfElements(CSProgramConfigPtr self, const unsigned int row);
int CSProgramConfig_GetNumberOfChannels(CSProgramConfigPtr self);
int CSProgramConfig_GetNumberOfChannelsPerRow(CSProgramConfigPtr self, const unsigned int row);

int CSProgramConfig_GetElementTag(CSProgramConfigPtr self, const unsigned int row, const unsigned int index);
int CSProgramConfig_IsChannelPair(CSProgramConfigPtr self, const unsigned int row, const unsigned int index);

int CSProgramConfig_GetNumberOfLowFrequencyChannels(CSProgramConfigPtr self);
int CSProgramConfig_GetLowFrequencyChannelTag      (CSProgramConfigPtr self, const unsigned int index);

/* */

int CSProgramConfig_GetSamplingRateIndex(CSProgramConfigPtr self);
int CSProgramConfig_GetProfile          (CSProgramConfigPtr self);

#ifdef __cplusplus
}
#endif

#endif
