/* $Header: /cvs/root/winamp/vlb/longblock.cpp,v 1.2 2011/06/13 02:06:03 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: longblock.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: long window sequence object
 *
 * $Header: /cvs/root/winamp/vlb/longblock.cpp,v 1.2 2011/06/13 02:06:03 audiodsp Exp $
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

CLongBlock::CLongBlock (CChannelInfo &info)
 : CBlock (info)
{
  m_SectBits = 5 ;
}

CLongBlock::~CLongBlock ()
{
}

// low-level access

float *CLongBlock::AccessSpectralData (int /* window = 0 */)
{
  return m_SpectralCoefficient ;
}

int *CLongBlock::AccessCodeBooks (int /* group */)
{
  return m_CodeBook ;
}

int *CLongBlock::AccessScaleFactors (int /* group */)
{
  return m_ScaleFactor ;
}

// readers
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
void CLongBlock::ReadSectionData (CDolbyBitStream &bs)
{
  CVLBBitSequence sect_cb (4) ;
  CVLBBitSequence sect_len_incr (m_SectBits) ;

  int sect_esc_val = (1 << m_SectBits) - 1 ;

  int band ; // msdev not ansi
  
  //Section Information:
  int iNumberOfSections;
  iNumberOfSections=0;
  sSectionInfoStruct.aiSectionCount[0]=0;
  sSectionInfoStruct.aaiSectionStart[0][0]=0;
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
      m_CodeBook [band] = sect_cb ;

      if ((m_CodeBook [band] == BOOKSCL) || 
          (m_CodeBook [band] == RESERVED_HCB)
         )
      {
        throw EInvalidCodeBook () ;
      }
    }

	sSectionInfoStruct.aaiSectionCodebooks[0][iNumberOfSections]=sect_cb;
	sSectionInfoStruct.aaiSectionStart[0][iNumberOfSections+1]=band;
	sSectionInfoStruct.aaiSectionEnd[0][iNumberOfSections]=band;
	sSectionInfoStruct.aiSectionCount[0]++;
	iNumberOfSections++;
  }
  if(band < m_IcsInfo.GetScaleFactorBandsTotal() && m_IcsInfo.GetScaleFactorBandsTransmitted()){
	 sSectionInfoStruct.aaiSectionCodebooks[0][iNumberOfSections]=ZERO_HCB;
	 sSectionInfoStruct.aaiSectionEnd[0][iNumberOfSections]= m_IcsInfo.GetScaleFactorBandsTotal ();
	 sSectionInfoStruct.aiSectionCount[0]++;
	 iNumberOfSections++;
  }
  for ( ; band < m_IcsInfo.GetScaleFactorBandsTotal () ; band++)
  {
    m_CodeBook [band] = ZERO_HCB ;
  }
}

void CLongBlock::ReadScaleFactorData (CDolbyBitStream &bs, const int global_gain)
{
  const CodeBookDescription *hcb = &HuffmanCodeBooks [BOOKSCL] ;

  int factor = global_gain ;
  int position = 0 ;

  int temp ;

  for (int band = 0 ; band < m_IcsInfo.GetScaleFactorBandsTransmitted () ; band++)
  {
    switch (m_CodeBook [band])
    {
      case ZERO_HCB : // zero book

        m_ScaleFactor [band] = 0 ;        
        break ;

      default : // regular scale factor 

        temp = DecodeHuffmanWord (bs, hcb->CodeBook) ;
        factor += temp - 60 ;  // MIDFAC 1.5 dB 
      
        m_ScaleFactor [band] = factor - 100 ;
        break ;

      case INTENSITY_HCB : // intensity steering
      case INTENSITY_HCB2 :

        temp = DecodeHuffmanWord (bs, hcb->CodeBook) ;
        position += temp - 60 ;

        m_ScaleFactor [band] = position - 100 ;

#ifdef MAIN_PROFILE
        // use of intensity stereo overrides prediction_used mask
        m_IcsInfo.DeactivatePrediction (band) ;
#endif

        break ;
    }
  }
}

void CLongBlock::ReadSpectralData (CDolbyBitStream &bs)
{
  int QuantizedCoef [MaximumBins] ;

  int index, band ; // msdev for scoping not ansi

  for (index = 0 ; index < MaximumBins ; index++)
  {
    QuantizedCoef [index] = 0 ;
  }

  const int *BandOffsets = m_IcsInfo.GetScaleFactorBandOffsets () ;

  for (band = 0 ; band < m_IcsInfo.GetScaleFactorBandsTransmitted () ; band++)
  {
    if ((m_CodeBook [band] == ZERO_HCB) ||
        (m_CodeBook [band] == INTENSITY_HCB) ||
        (m_CodeBook [band] == INTENSITY_HCB2)
       )
      continue ;

    const CodeBookDescription *hcb = &HuffmanCodeBooks [m_CodeBook [band]] ;

    int step = 0 ;

    for (index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index += step)
    {
      step = UnpackIndex (DecodeHuffmanWord (bs, hcb->CodeBook), &QuantizedCoef [index], hcb) ;
    
      if (!hcb->IsSigned)
      {
        for (int i = 0 ; i < step ; i++)
        {
          if (QuantizedCoef [index + i])
          {
            if (bs.Get (1)) // sign bit
            {
              QuantizedCoef [index + i] = -QuantizedCoef [index + i] ;
            }
          }
        }
      }

      if (m_CodeBook [band] == ESCBOOK)
      {
        QuantizedCoef [index] = GetEscape (bs, QuantizedCoef [index]) ;
        QuantizedCoef [index + 1] = GetEscape (bs, QuantizedCoef [index + 1]) ;
      }
    }
  }

  m_PulseData.Apply (m_IcsInfo, QuantizedCoef) ;

  // dequantize and apply scalefactors

  for (index = 0, band = 0 ; band < m_IcsInfo.GetScaleFactorBandsTransmitted () ; band++)
  {
    float factor = static_cast<float>(m_ScaleFactor [band]) ;

    if ((factor >= 0) && (factor < ExpTableSize))
    {
      factor = m_ExpTable [m_ScaleFactor [band]] ;
    }
    else
    {
      if (m_ScaleFactor [band] == -100)
      {
        for (index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
        {
          m_SpectralCoefficient [index] = 0.0F ;
        }

        continue ;
      }

      factor = static_cast<float>(pow (2.0F, 0.25F * factor)) ;
    }

    for (index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
    {
      m_SpectralCoefficient [index] = InverseQuantize (QuantizedCoef [index]) * factor ;
    }
  }

  // zero out spectral data beyond max_sfb
  // index is now first bin of max_sfb+1

  for ( ; index < MaximumBins ; index++)
  {
    m_SpectralCoefficient [index] = 0.0F ;
  }
}

void CLongBlock::ApplyWindowFunction (COverlapAddBuffer &Previous)
{
  int i ;

  switch (m_IcsInfo.GetWindowSequence ())
  {
    case CChannelInfo::OnlyLongSequence :

#if defined (WIN32) && defined (_M_IX86)

      PentiumWindow (m_SpectralCoefficient, m_LongWindow [Previous.GetWindowShape ()], m_LongWindow [m_IcsInfo.GetWindowShape ()]) ;

#else
      
      for (i = 0 ; i < 1024 ; i++)
      {
        m_SpectralCoefficient [i] *= m_LongWindow [Previous.GetWindowShape ()][i] ;
        m_SpectralCoefficient [1024 + i] *= m_LongWindow [m_IcsInfo.GetWindowShape ()][1023 - i] ;
      }

#endif

      break ;

    case CChannelInfo::LongStartSequence :

      for (i = 0 ; i < 1024 ; i++)
      {
        m_SpectralCoefficient [i] *= m_LongWindow [Previous.GetWindowShape ()][i] ;
      }

      for (i = 0 ; i < 128 ; i++)
      {
        m_SpectralCoefficient [1472 + i] *= m_ShortWindow [m_IcsInfo.GetWindowShape ()][127 - i] ;
      }

      for (i = 1600 ; i < 2048 ; i++)
      {
        m_SpectralCoefficient [i] = 0.0F ;
      }

      break ;

    case CChannelInfo::LongStopSequence :

      for (i = 0 ; i < 448 ; i++)
      {
        m_SpectralCoefficient [i] = 0.0F ;
      }

      for (i = 0 ; i < 128 ; i++)
      {
        m_SpectralCoefficient [448 + i] *= m_ShortWindow [Previous.GetWindowShape ()][i] ;
      }

      for (i = 0 ; i < 1024 ; i++)
      {
        m_SpectralCoefficient [1024 + i] *= m_LongWindow [m_IcsInfo.GetWindowShape ()][1023 - i] ;
      }

      break ;
  }

  Previous.SetWindowShape (m_IcsInfo.GetWindowShape ()) ;
}

void CLongBlock::FrequencyToTime_Fast (COverlapAddBuffer &Previous)
{
  InverseTransform (m_SpectralCoefficient) ;
  ApplyWindowFunction (Previous) ;

#if defined (WIN32) && defined (_M_IX86)

  PentiumOverlap (m_Output, m_SpectralCoefficient, Previous.AccessBuffer (), 1) ;

#else

  for (int i = 0 ; i < 1024 ; i++)
  {
    // add first half and old data

    m_Output[i] = m_SpectralCoefficient [i] + Previous [i] ;
  
    // store second half as old data

    Previous [i] = m_SpectralCoefficient [1024 + i] ;
  }

#endif
}

void CLongBlock::FrequencyToTime (COverlapAddBuffer &Previous, float Output [], const int stride)
{
  InverseTransform (m_SpectralCoefficient) ;
  ApplyWindowFunction (Previous) ;

  for (int i = 0 ; i < 1024 ; i++)
  {
    // add first half and old data

    Output [i * stride] = m_SpectralCoefficient [i] + Previous [i] ;
  
    // store second half as old data

    Previous [i] = m_SpectralCoefficient [1024 + i] ;
  }
}

void CLongBlock::FrequencyToTime (COverlapAddBuffer &Previous)
{
  InverseTransform (m_SpectralCoefficient) ;
  ApplyWindowFunction (Previous) ;

  for (int i = 0 ; i < 1024 ; i++)
  {
    m_Output[i]=m_SpectralCoefficient [i] + Previous [i] ;
    Previous [i] = m_SpectralCoefficient [1024 + i] ;
  }
}

void CLongBlock::ApplyEqualizationMask (float Mask [])
{
  for (int i = 0 ; i < EqualizationMaskLength ; i++)
  {
    for (int j = 0 ; j < (MaximumBins / EqualizationMaskLength) ; j++)
    {
      m_SpectralCoefficient [(MaximumBins / EqualizationMaskLength) * i + j] *= Mask [i] ;
    }
  }
}

#if defined (WIN32) && defined (_M_IX86)
#pragma message (__FILE__": using x86 inline assembler") 

// this CBlock method goes here, because we'd prefer it to be inlined
// with CLongBlock. not all compilers support inter-module optimizations.

void CBlock::PentiumOverlap (float output [], float spec [], float prev [], unsigned int stride)
{
  const float minval = -32767.0F ;

  __asm
  {
    shl   stride, 2 // stride *= sizeof(float)

    mov   esi, dword ptr [output] // esi is pcm output pointer

    mov   ebx, 0x00 // ebx is buffer index

    mov   edx, dword ptr [spec]
    mov   ecx, dword ptr [prev]

entry:

    fld   dword ptr [edx + ebx*4]               // get: spec [i] 
    fadd  dword ptr [ecx + ebx*4]               // get: spec [i] + prev [i] 
    mov   eax, dword ptr [edx + ebx*4 + 0x1000] // get: spec [1024 + i] 
    mov   dword ptr [ecx + ebx*4],eax           // put: prev [i] = spec [1024 + i] 
	fst   dword ptr [esi]						// put: output [i] = spec [i] + prev [i]

    add   esi, stride

    fstp  st(0)

    inc   ebx
    cmp   ebx, 1024
    jne   entry

  }
}

void CLongBlock::PentiumWindow (float spec [], const float prev [], const float curr [])
{
  __asm
  {
    mov   ebx, 0x0100

    mov   edx, dword ptr [spec]
    mov   esi, dword ptr [curr]
    mov   ecx, dword ptr [prev]

    add   esi, 4 * 0x03FC

entry:

    // previous shape

    fldz

    fld   dword ptr [edx]
    fmul  dword ptr [ecx]  
    fxch  

    fld   dword ptr [edx + 0x04]
    fmul  dword ptr [ecx + 0x04]  
    fxch  

    fld   dword ptr [edx + 0x08]
    fmul  dword ptr [ecx + 0x08]  
    fxch  

    fld   dword ptr [edx + 0x0C]
    fmul  dword ptr [ecx + 0x0C]  
    fxch  st(4)

    fstp  dword ptr [edx]
    fxch  st(2)

    fstp  dword ptr [edx + 0x04]
    fstp  dword ptr [edx + 0x08]
    fxch

    fstp  dword ptr [edx + 0x0C]

    // current shape

    fld   dword ptr [edx + 0x1000]
    fmul  dword ptr [esi + 0x0C]  
    fxch  

    fld   dword ptr [edx + 0x1004]
    fmul  dword ptr [esi + 0x08]  
    fxch  

    fld   dword ptr [edx + 0x1008]
    fmul  dword ptr [esi + 0x04]  
    fxch  

    fld   dword ptr [edx + 0x100C]
    fmul  dword ptr [esi]  

    fxch  st(4)
    fstp  dword ptr [edx + 0x1000]

    fxch  st(2)
    fstp  dword ptr [edx + 0x1004]

    fstp  dword ptr [edx + 0x1008]

    fstp  st(0)
    fstp  dword ptr [edx + 0x100C]

    add   edx, 0x10
    sub   esi, 0x10
    add   ecx, 0x10

    dec   ebx
    jne   entry
  }
}

#endif
