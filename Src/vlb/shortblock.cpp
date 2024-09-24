/* $Header: /cvs/root/winamp/vlb/shortblock.cpp,v 1.2 2011/06/13 02:06:03 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: shortblock.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: eight short window sequence object
 *
\***************************************************************************/

#include <math.h> // pow()

#include "block.h"
#include "bitstream.h"
#include "channelinfo.h"
#include "overlapadd.h"

#ifdef MAIN_PROFILE
#include "prediction.h"
#endif
// ctor/dtor

CShortBlock::CShortBlock (CChannelInfo &info)
 : CBlock (info)
{
  m_SectBits = 3 ;
}

CShortBlock::~CShortBlock ()
{
}

// low-level access

float *CShortBlock::AccessSpectralData (int window)
{
  return m_SpectralCoefficient [window] ;
}

int *CShortBlock::AccessCodeBooks (int group)
{
  return m_CodeBook [group] ;
}

int *CShortBlock::AccessScaleFactors (int group)
{
  return m_ScaleFactor [group] ;
}

// readers

void CShortBlock::ReadSectionData (CDolbyBitStream &bs)
{
  CVLBBitSequence sect_cb (4) ;
  CVLBBitSequence sect_len_incr (m_SectBits) ;

  int sect_esc_val = (1 << m_SectBits) - 1 ;

  //Section Information:
  int iNumberOfSections;

  for (int group = 0 ; group < m_IcsInfo.GetWindowGroups () ; group++)
  {
    int band ; // msdev not ansi

	// initialize first group's section info
	iNumberOfSections=0;
	sSectionInfoStruct.aiSectionCount[group]=0;
	sSectionInfoStruct.aaiSectionStart[group][0]=0;

    for (band = 0 ; band < m_IcsInfo.GetScaleFactorBandsTransmitted () ; )
    {
      sect_cb.Read (bs) ;
      sect_len_incr.Read (bs) ;

      int sect_len = 0 ;

      while (sect_len_incr == sect_esc_val)
      {
        sect_len += sect_esc_val ;
        sect_len_incr.Read (bs) ;
      }

      sect_len += sect_len_incr ;

      for (int top = band + sect_len ; band < top ; band++)
      {
        m_CodeBook [group][band] = sect_cb ;

        if ((m_CodeBook [group][band] == BOOKSCL) || 
            (m_CodeBook [group][band] == RESERVED_HCB)
           )
        {
          throw EInvalidCodeBook () ;
        }
      }

	  sSectionInfoStruct.aaiSectionCodebooks[group][iNumberOfSections]=sect_cb;
	  sSectionInfoStruct.aaiSectionStart[group][iNumberOfSections+1]=band;
	  sSectionInfoStruct.aaiSectionEnd[group][iNumberOfSections]=band;
	  sSectionInfoStruct.aiSectionCount[group]++;
	  iNumberOfSections++;

    } // for band

	if(band < m_IcsInfo.GetScaleFactorBandsTotal() && m_IcsInfo.GetScaleFactorBandsTransmitted()){
		sSectionInfoStruct.aaiSectionCodebooks[group][iNumberOfSections]=ZERO_HCB;
		sSectionInfoStruct.aaiSectionEnd[group][iNumberOfSections]= m_IcsInfo.GetScaleFactorBandsTotal ();
		sSectionInfoStruct.aiSectionCount[group]++;
		iNumberOfSections++;
	}

    for ( ; band < m_IcsInfo.GetScaleFactorBandsTotal () ; band++)
    {
      m_CodeBook [group][band] = ZERO_HCB ;
    }
  } // for group..
}

void CShortBlock::ReadScaleFactorData (CDolbyBitStream &bs, const int global_gain)
{
  const CodeBookDescription *hcb = &HuffmanCodeBooks [BOOKSCL] ;

  int factor = global_gain ;
  int position = 0 ;

  int temp ;

  for (int group = 0 ; group < m_IcsInfo.GetWindowGroups () ; group++)
  {
    for (int band = 0 ; band < m_IcsInfo.GetScaleFactorBandsTransmitted () ; band++)
    {
      switch (m_CodeBook [group][band])
      {
        case ZERO_HCB : // zero book

          m_ScaleFactor [group][band] = 0 ;        
          break ;

        default : // decode scale factor 

          temp = DecodeHuffmanWord (bs, hcb->CodeBook) ;
          factor += temp - 60 ;  // MIDFAC 1.5 dB 
        
          m_ScaleFactor [group][band] = factor - 100 ;
          break ;

        case INTENSITY_HCB : // intensity steering
        case INTENSITY_HCB2 :

          temp = DecodeHuffmanWord (bs, hcb->CodeBook) ;
          position += temp - 60 ;

          m_ScaleFactor [group][band] = position - 100 ;
          break ;
      }
    }
  }
}

void CShortBlock::ReadSpectralData (CDolbyBitStream &bs)
{
  const int *BandOffsets = m_IcsInfo.GetScaleFactorBandOffsets () ;

  int QuantizedCoef [MaximumWindows][MaximumBins] ;

  int window, group ;

  // // // clear coeffs

  for (window = 0 ; window < MaximumWindows ; window++)
  {
    for (int index = 0 ; index < MaximumBins ; index++)
    {
      QuantizedCoef [window][index] = 0 ;
    }
  }

  // // // read interleaved coeffs

  int groupoffset = 0 ;

  for (group = 0 ; group < m_IcsInfo.GetWindowGroups () ; group++)
  {
    for (int band = 0 ; band < m_IcsInfo.GetScaleFactorBandsTransmitted () ; band++)
    {
      const CodeBookDescription *hcb = &HuffmanCodeBooks [m_CodeBook [group][band]] ;

      for (int groupwin = 0 ; groupwin < m_IcsInfo.GetWindowGroupLength (group) ; groupwin++)
      {
        if ((m_CodeBook [group][band] == ZERO_HCB) ||
            (m_CodeBook [group][band] == INTENSITY_HCB) ||
            (m_CodeBook [group][band] == INTENSITY_HCB2)
           )
          continue ;

        int window = groupoffset + groupwin ;
        int step = 0 ;

        for (int index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index += step)
        {
          step = UnpackIndex (DecodeHuffmanWord (bs, hcb->CodeBook), &QuantizedCoef [window][index], hcb) ;
  
          if (!hcb->IsSigned)
          {
            for (int i = 0 ; i < step ; i++)
            {
              if (QuantizedCoef [window][index + i])
              {
                if (bs.Get (1)) // sign bit
                {
                  QuantizedCoef [window][index + i] = -QuantizedCoef [window][index + i] ;
                }
              }
            }
          }

          if (m_CodeBook [group][band] == ESCBOOK)
          {
            QuantizedCoef [window][index] = GetEscape (bs, QuantizedCoef [window][index]) ;
            QuantizedCoef [window][index + 1] = GetEscape (bs, QuantizedCoef [window][index + 1]) ;
          }
        }
      }
    }

    groupoffset += m_IcsInfo.GetWindowGroupLength (group) ;
  }

  // // //

  for (window = 0, group = 0 ; group < m_IcsInfo.GetWindowGroups () ; group++)
  {
    for (int groupwin = 0 ; groupwin < m_IcsInfo.GetWindowGroupLength (group) ; groupwin++, window++)
    {
      // dequantize

      for (int index = 0 ; index < MaximumBins ; index++)
      {
        m_SpectralCoefficient [window][index] = InverseQuantize (QuantizedCoef [window][index]) ;
      }

      // apply scalefactors

      for (int band = 0 ; band < m_IcsInfo.GetScaleFactorBandsTransmitted () ; band++)
      {
        float factor = static_cast<float>(m_ScaleFactor [group][band]) ;

        if ((factor >= 0) && (factor < ExpTableSize))
        {
          factor = m_ExpTable [m_ScaleFactor [group][band]] ;
        }
        else
        {
          factor = static_cast<float>(pow (2.0F, 0.25F * factor)) ;
        }

        if (m_ScaleFactor [group][band] != -100)
        {
          for (int index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
          {
            m_SpectralCoefficient [window][index] *= factor ;
          }
        }
        else
        {
          for (int index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
          {
            m_SpectralCoefficient [window][index] = 0.0F ;
          }
        }
      }
    }
  }
}

void CShortBlock::TransformWindows (COverlapAddBuffer &Previous, float EightWindowsBuffer [])
{
  for (int i = 0 ; i < 2048 ; i++)
  {
    EightWindowsBuffer [i] = 0 ;
  }

  for (int window = 0 ; window < MaximumWindows ; window++)
  {
    float *theSpectrum = m_SpectralCoefficient [window] ;

    InverseTransform (theSpectrum) ;

    if (window == 0)
    {
      for (int i = 0 ; i < 128 ; i++)
      {
        theSpectrum [i] *= m_ShortWindow [Previous.GetWindowShape ()][i] ;
        theSpectrum [128 + i] *= m_ShortWindow [m_IcsInfo.GetWindowShape ()][127 - i] ;
      }
    }
    else
    {
      for (int i = 0 ; i < 128 ; i++)
      {
        theSpectrum [i] *= m_ShortWindow [m_IcsInfo.GetWindowShape ()][i] ;
        theSpectrum [128 + i] *= m_ShortWindow [m_IcsInfo.GetWindowShape ()][127 - i] ;
      }
    }

    // overlap add the 8 windows in this block

    for (int i = 0 ; i < 256 ; i++)
    {
      EightWindowsBuffer [448 + 128 * window + i] += theSpectrum [i] ;     
    }
  }

  Previous.SetWindowShape (m_IcsInfo.GetWindowShape ()) ;
}

/*
void CShortBlock::FrequencyToTime (COverlapAddBuffer &Previous, short Output [], const int stride)
{
  float EightWindowsBuffer [2048] ;

  TransformWindows (Previous, EightWindowsBuffer) ;

#if defined (WIN32) && defined (_M_IX86)

  PentiumOverlap (m_Output, EightWindowsBuffer, Previous.AccessBuffer (), stride) ;

#else

  for (int j = 0 ; j < 1024 ; j++)
  {
    // add first half and old data

    Output [j * stride] = FloatToShort (EightWindowsBuffer [j] + Previous [j]) ;
  
    // store second half as old data

    Previous [j] = EightWindowsBuffer [1024 + j] ;
  }

#endif
}
*/

void CShortBlock::FrequencyToTime_Fast (COverlapAddBuffer &Previous)
{
  float EightWindowsBuffer [2048] ;

  TransformWindows (Previous, EightWindowsBuffer) ;

#if defined (WIN32) && defined (_M_IX86)

  PentiumOverlap (m_Output, EightWindowsBuffer, Previous.AccessBuffer (), 1) ;

#else

  for (int j = 0 ; j < 1024 ; j++)
  {
    // add first half and old data

	m_Output [j] = EightWindowsBuffer [j] + Previous [j];

    // store second half as old data

    Previous [j] = EightWindowsBuffer [1024 + j] ;
  }

#endif
}

void CShortBlock::FrequencyToTime (COverlapAddBuffer &Previous, float Output [], const int stride)
{
  float EightWindowsBuffer [2048] ;

  TransformWindows (Previous, EightWindowsBuffer) ;

  for (int j = 0 ; j < 1024 ; j++)
  {
    // add first half and old data

    Output [j * stride] = EightWindowsBuffer [j] + Previous [j] ;
  
    // store second half as old data

    Previous [j] = EightWindowsBuffer [1024 + j] ;
  }
}

void CShortBlock::FrequencyToTime (COverlapAddBuffer &Previous)
{
  float EightWindowsBuffer [2048] ;
  TransformWindows (Previous, EightWindowsBuffer) ;
  for (int j = 0 ; j < 1024 ; j++)
  {
    m_Output [j] = EightWindowsBuffer [j] + Previous [j] ;
    Previous [j] = EightWindowsBuffer [1024 + j] ;
  }
}

void CShortBlock::ApplyEqualizationMask (float Mask [])
{
  for (int window = 0 ; window < MaximumWindows ; window++)
  {
    for (int i = 0 ; i < EqualizationMaskLength ; i++)
    {
      for (int j = 0 ; j < (MaximumBins / EqualizationMaskLength) ; j++)
      {
        m_SpectralCoefficient [window][(MaximumBins / EqualizationMaskLength) * i + j] *= Mask [i] ;
      }
    }
  }
}
