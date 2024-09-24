/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp_quant.h
 *   project : ISO/MPEG-Decoder
 *   author  : Markus Werner, addings: Martin Sieler
 *   date    : 1995-07-07
 *   contents/description: HEADER - sample-dequantization
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:04 $
 * $Id: mp3quant.h,v 1.1 2010/11/17 20:46:04 audiodsp Exp $
 */

/*-------------------------------------------------------------------------*/

#ifndef __MP3QUANT_H__
#define __MP3QUANT_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"

/* ------------------------------------------------------------------------*/

void mp3DequantizeSpectrum
    (
    int        *pIData,
    float      *pFData,
    const MP3SI_GRCH &SiGrCh,
    const MP3SCF     &ScaleFac,
    const MPEG_INFO  &Info
    );

/*-------------------------------------------------------------------------*/

#endif
