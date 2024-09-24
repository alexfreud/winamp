/* $Header: /cvs/root/winamp/vlb/pulsedata.h,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: pulsedata.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: pulse data tool
 *
\***************************************************************************/

#ifndef __PULSEDATA_H__
#define __PULSEDATA_H__

#include "bitsequence.h"

class CChannelInfo ;

/** Pulse Data Processing.

    This class represents the pulse data noiseless coding tool for decoding and 
    applying pulse data to the spectral coefficients of the current block.

    The Pulse Data tool is not profile-dependent and the CPulseData implementation
    follows the Read()/Apply() convention used for all tools.

*/

class CPulseData
{
public :

  CPulseData () ;
  ~CPulseData () ;

  void Read (CDolbyBitStream &bs) ;
  void Apply (const CChannelInfo &info, int coef []) ;

  enum
  {
    MaximumLines = 4
  } ;

protected :

  CVLBBitSequence m_PulseDataPresent ;
  CVLBBitSequence m_NumberPulse ;
  CVLBBitSequence m_PulseStartBand ;

  CVLBBitSequence m_PulseOffset [MaximumLines] ;
  CVLBBitSequence m_PulseAmp [MaximumLines] ;

} ;

#endif
