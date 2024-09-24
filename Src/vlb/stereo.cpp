/* $Header: /cvs/root/winamp/vlb/stereo.cpp,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: stereo.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: mid/side and intensity stereo processing
 *
\***************************************************************************/

#include <math.h>

#include "stereo.h"
#include "bitstream.h"
#include "channelinfo.h"
#include "block.h"

CJointStereo::CJointStereo ()
 : m_MsMaskPresent (2)
{
}

CJointStereo::~CJointStereo ()
{
}

void CJointStereo::Read (const CChannelInfo &info, CDolbyBitStream &bs)
{
  m_MsMaskPresent.Read (bs) ;

  int group, band ;

  switch (m_MsMaskPresent)
  {
    case 0 : // no M/S

      for (group = 0 ; group < info.GetWindowGroups () ; group++)
      {
        for (band = 0 ; band < info.GetScaleFactorBandsTransmitted () ; band++)
        {
          m_MsUsed [group][band] = false ;
        }
      }

      break ;

    case 1 : // read ms_used

      for (group = 0 ; group < info.GetWindowGroups () ; group++)
      {
        for (band = 0 ; band < info.GetScaleFactorBandsTransmitted () ; band++)
        {
          m_MsUsed [group][band] = bs.Get (1) ? true : false ;
        }
      }

      break ;

    case 2 : // full spectrum M/S

      for (group = 0 ; group < info.GetWindowGroups () ; group++)
      {
        for (band = 0 ; band < info.GetScaleFactorBandsTransmitted () ; band++)
        {
          m_MsUsed [group][band] = true ;
        }
      }

      break ;
  }
}

// mid / side

void CJointStereo::ApplyMS (const CChannelInfo &info, CBlock &left, CBlock &right)
{
  const int *BandOffsets = info.GetScaleFactorBandOffsets () ;

  for (int window = 0, group = 0 ; group < info.GetWindowGroups () ; group++)
  {
    for (int groupwin = 0 ; groupwin < info.GetWindowGroupLength (group) ; groupwin++, window++)
    {
      float *LeftSpectrum = left.AccessSpectralData (window) ;
      float *RightSpectrum = right.AccessSpectralData (window) ;

      for (int band = 0 ; band < info.GetScaleFactorBandsTransmitted () ; band++)
      {
        if (m_MsUsed [group][band] == true)
        {
          for (int index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
          {
            float LeftCoefficient  = LeftSpectrum [index] ;
            float RightCoefficient = RightSpectrum [index] ;
            
            LeftSpectrum [index]  = LeftCoefficient + RightCoefficient ;
            RightSpectrum [index] = LeftCoefficient - RightCoefficient ;
          }
        }
      }
    }
  }
}

// intensity

void CJointStereo::ApplyIS (const CChannelInfo &info, CBlock &left, CBlock &right)
{
  const int *BandOffsets = info.GetScaleFactorBandOffsets () ;

  for (int window = 0, group = 0 ; group < info.GetWindowGroups () ; group++)
  {
    int *CodeBook = right.AccessCodeBooks (group) ;
    int *ScaleFactor = right.AccessScaleFactors (group) ;

    for (int groupwin = 0 ; groupwin < info.GetWindowGroupLength (group) ; groupwin++, window++)
    {
      float *LeftSpectrum = left.AccessSpectralData (window) ;
      float *RightSpectrum = right.AccessSpectralData (window) ;

      for (int band = 0 ; band < info.GetScaleFactorBandsTransmitted () ; band++)
      {
        if ((CodeBook [band] == CBlock::INTENSITY_HCB) ||
            (CodeBook [band] == CBlock::INTENSITY_HCB2)
           )
        {
          float scale = (float) pow (0.5F, 0.25F * (ScaleFactor [band] + 100)) ;

          if (m_MsUsed [group][band])
          {
            if (CodeBook [band] == CBlock::INTENSITY_HCB) // _NOT_ in-phase
            {
              scale *= -1 ;
            }
          }
          else
          {
            if (CodeBook [band] == CBlock::INTENSITY_HCB2) // out-of-phase
            {
              scale *= -1 ;
            }
          }
          
          for (int index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
          {
            RightSpectrum [index] = LeftSpectrum [index] * scale ;
          }
        }
      }
    }
  }
}
