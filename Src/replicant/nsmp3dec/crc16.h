/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: crc16.h
 *   project : ISO/MPEG decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: functions to calculate a CRC-16
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:02 $
 * $Id: crc16.h,v 1.1 2010/11/17 20:46:02 audiodsp Exp $
 */

#ifndef __CRC16_H__
#define __CRC16_H__

/* ------------------------ includes --------------------------------------*/

/* ------------------------------------------------------------------------*/

class CBitStream;

/* ------------------------------------------------------------------------*/

unsigned int CalcCrc(CBitStream &Bs, int len, unsigned int start);

/*-------------------------------------------------------------------------*/
#endif
