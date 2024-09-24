/* $Header: /cvs/root/winamp/vlb/pulsedata.cpp,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

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

#include "pulsedata.h"
#include "bitstream.h"
#include "channelinfo.h"

CPulseData::CPulseData ()
 : m_PulseDataPresent (1),
   m_NumberPulse (2),
   m_PulseStartBand (6)
{
}

CPulseData::~CPulseData ()
{
}

void CPulseData::Read (CDolbyBitStream &bs)
{
  if (m_PulseDataPresent.Read (bs))
  {
    m_NumberPulse.Read (bs) ;
    m_PulseStartBand.Read (bs) ;

    for (int i = 0 ; i <= m_NumberPulse ; i++)
    {
      m_PulseOffset [i].Read (bs, 5) ;
      m_PulseAmp [i].Read (bs, 4) ;
    }
  }
}

void CPulseData::Apply (const CChannelInfo &info, int coef [])
{
  if (m_PulseDataPresent)
  {
    int k = info.GetScaleFactorBandOffsets () [(int) m_PulseStartBand] ;
  
    for (int i = 0 ; i <= m_NumberPulse ; i++)
    {
      k += m_PulseOffset [i] ;

      if (coef [k] > 0) coef [k] += m_PulseAmp [i] ;
      else              coef [k] -= m_PulseAmp [i] ;
    }
  }
}
