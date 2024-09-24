/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3tools.h
 *   project : ISO/MPEG-Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: HEADER - layer III processing
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/13 22:43:21 $
 * $Id: mp3tools.h,v 1.2 2011/01/13 22:43:21 audiodsp Exp $
 */

/*-------------------------------------------------------------------------*/

#ifndef __MP3TOOLS_H__
#define __MP3TOOLS_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"

/* ------------------------------------------------------------------------*/

void mp3ScaleFactorUpdate
    (
    const MP3SI_GRCH &SiL,
    const MP3SI_GRCH &SiR,
    const MPEG_INFO  &Info,
    MP3SCF           &ScaleFac
    );

void mp3StereoProcessing
    (
    float            *pLeft,
    float            *pRight,
    MP3SI_GRCH       &SiL,
    MP3SI_GRCH       &SiR,
    const MP3SCF     &ScaleFac, /* right channel!! */
    const MPEG_INFO  &Info
    );

void mp3Reorder
    (
    float            *pData,
    const MP3SI_GRCH &Si,
    const MPEG_INFO  &Info
    );

void mp3Antialias
    (
    float            *pData,
    MP3SI_GRCH       &Si,
    const MPEG_INFO  &Info
    );

/*-------------------------------------------------------------------------*/

#endif
