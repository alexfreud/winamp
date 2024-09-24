/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2002)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/hvxcspecificconfig_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: HVXC specific config interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __HVXCSPECIFICCONFIG_C_H__
#define __HVXCSPECIFICCONFIG_C_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CSBitStream;

typedef struct HvxcSpecificConfig {
  int m_isBaseLayer;
  int m_DPvarMode;
  int m_DPrateMode;
  int m_DPextensionFlag;
  int m_vrScalFlag;
} HvxcSpecificConfig, *HvxcSpecificConfigPtr;

void HvxcSpecificConfig_Parse( HvxcSpecificConfigPtr self, HvxcSpecificConfigPtr baselayer, struct CSBitStream *bs); 
void HvxcSpecificConfig_Copy(HvxcSpecificConfigPtr dst, const HvxcSpecificConfigPtr src);
int  HvxcSpecificConfig_Print( HvxcSpecificConfigPtr self, char string[]);

#ifdef __cplusplus
}
#endif

#endif /* __HVXCSPECIFICCONFIG_H__ */
