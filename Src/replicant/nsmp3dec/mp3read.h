/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3read.h
 *   project : ISO/MPEG-Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: mp3 read-functions: sideinfo, main data,
 *                                             scalefactors
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:04 $
 * $Id: mp3read.h,v 1.1 2010/11/17 20:46:04 audiodsp Exp $
 */

/*-------------------------------------------------------------------------*/

#ifndef __MP3READ_H__
#define __MP3READ_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"

/* ------------------------------------------------------------------------*/

class CBitStream;

/* ------------------------------------------------------------------------*/

bool mp3SideInfoRead(CBitStream &Bs, MP3SI &Si, const MPEG_INFO &Info, int crc_check);

bool mp3MainDataRead
    (
    CBitStream      &Bs, // bitstream
    CBitStream      &Db, // dynamic buffer
    const MP3SI     &Si,
    const MPEG_INFO &Info
    );

void mp3ScaleFactorRead
    (
    CBitStream      &Bs,
    MP3SI_GRCH      &SiGrCh,
    MP3SCF          &ScaleFac,
    const MPEG_INFO &Info,
    const int       *pScfsi,
    int              gr,
    int              ch
    );

/*-------------------------------------------------------------------------*/

#endif
