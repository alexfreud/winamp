/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mdct.h
 *   project : ISO/MPEG-Decoder
 *   author  : Stefan Gewinner
 *   date    : 1998-05-26
 *   contents/description: mdct class - HEADER
 *
 *
 \***************************************************************************/

/*
 * $Date: 2011/01/18 18:22:03 $
 * $Id: mdct.h,v 1.4 2011/01/18 18:22:03 audiodsp Exp $
 */

#ifndef __MDCT_H__
#define __MDCT_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"
#include "foundation/align.h"
/*-------------------------------------------------------------------------*/

//
// MDCT class.
//
//  This object performs the frequency-to-time mapping.
//

class CMdct
{

public :

  CMdct(const MPEG_INFO &_info);
  ~CMdct() {}

  void Init();
  void Apply(int ch, const MP3SI_GRCH &SiGrCH, SPECTRUM &rs);

protected :

  void cos_t_h_long (float *prev,float *dest,const float *win);
  void cos_t_h_short(float *prev,float *dest,const float *win);
	
  float hybrid_res[36];
  NALIGN(16) float cost36_rese[9];
  NALIGN(16) float cost36_reso[9];

  const MPEG_INFO &info;
  SPECTRUM         prevblck;
};

/*-------------------------------------------------------------------------*/
#endif
