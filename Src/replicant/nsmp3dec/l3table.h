/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  � 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: l3table.h
 *   project : ISO/MPEG-Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: HEADER - tables for iso/mpeg-decoding (layer3)
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:02 $
 * $Id: l3table.h,v 1.1 2010/11/17 20:46:02 audiodsp Exp $
 */

/*-------------------------------------------------------------------------*/

#ifndef __L3TABLE_H__
#define __L3TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------ includes --------------------------------------*/

/* ------------------------------------------------------------------------*/

 struct SF_BAND_INDEX
{
  int l[23];
  int s[14];
};

/* ------------------------------------------------------------------------*/

extern const SF_BAND_INDEX sfBandIndex[3][3];

/*-------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif
