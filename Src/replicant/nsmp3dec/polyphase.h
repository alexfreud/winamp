/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: polyphase.h
 *   project : ISO/MPEG-Decoder
 *   author  : Stefan Gewinner
 *   date    : 1998-05-26
 *   contents/description: polyphase class - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/02/14 14:48:56 $
 * $Id: polyphase.h,v 1.6 2011/02/14 14:48:56 bigg Exp $
 */

#pragma once
//#ifndef __POLYPHASE_H__
//#define __POLYPHASE_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"
#include "foundation/align.h"
/*-------------------------------------------------------------------------*/

#define HAN_SIZE 512

/*-------------------------------------------------------------------------*/

// Class for (inverse) Polyphase calculation.

class CPolyphase
{

public:

  CPolyphase(const MPEG_INFO &_info);

  ~CPolyphase() {}

  void Init();
  float *Apply(POLYSPECTRUM &sample, float *pPcm, int frms=18);
	static void Reorder(int channels, POLYSPECTRUM &output, const SPECTRUM &input);
protected:

  int   bufOffset;
	NALIGN(16) float syn_buf[2][HAN_SIZE];

  const MPEG_INFO &info ;      // info-structure

  void window_band_m(int bufOffset, float *out_samples) const;
  void window_band_s(int bufOffset, float *out_samples) const;
};

/*-------------------------------------------------------------------------*/
//#endif
