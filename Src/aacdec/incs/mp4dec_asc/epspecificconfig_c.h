/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2002)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/epspecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: EP specific config interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/


#ifndef __EPSPECIFICCONFIG_C_H__
#define __EPSPECIFICCONFIG_C_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

typedef struct CEpClassConfig {
  unsigned int m_LengthEsc;
  unsigned int m_RateEsc;
  unsigned int m_CrcLengthEsc;

  unsigned int m_ConcatenateFlag;
  unsigned int m_FecType;
  unsigned int m_TerminationSwitch;

  unsigned int m_InterleaveSwitch;
  unsigned int m_ClassOptional;
  
  unsigned int m_NoBitsForLength;
  unsigned int m_ClassLength;
  
  unsigned int m_ClassRate;
  unsigned int m_ClassCrcLength;
} CEpClassConfig, *CEpClassConfigPtr;

typedef struct CPredSetConfig {

  unsigned int   m_NoOfClasses;
  unsigned int   m_ClassReorderedOutput;
  unsigned int  *m_ClassOutputOrder;
  CEpClassConfig *m_EpClassConfig;
} CPredSetConfig, *CPredSetConfigPtr;


typedef struct CEPBuffer {
  unsigned char* m_pData;
  unsigned int   m_noBytes;
  unsigned int   m_noBits;
} CEPBuffer, *CEPBufferPtr;

typedef struct CEPSpecificConfig {
  
  unsigned int m_NoOfPredSets;
  unsigned int m_InterleaveType;
  unsigned int m_BitStuffing;
  unsigned int m_NoOfConcatenatedFrames;

  unsigned int m_HeaderProtection;
  unsigned int m_HeaderRate;
  unsigned int m_HeaderCrcLength;

  unsigned int m_RSFecCapability;

  CPredSetConfig *m_psc;
  CEPBuffer       m_epTmpBuffer;

} CEpSpecificConfig, *CEpSpecificConfigPtr;

int  EpSpecificConfig_Parse(CEpSpecificConfigPtr self, struct CSBitStream *bs);
int  EpSpecificConfig_Copy(CEpSpecificConfigPtr self, CEpSpecificConfigPtr epsc);
int  EpSpecificConfig_Compare(CEpSpecificConfigPtr self, CEpSpecificConfigPtr epsc);
void EpSpecificConfig_Free(CEpSpecificConfigPtr self);

#ifdef __cplusplus
}
#endif

#endif
