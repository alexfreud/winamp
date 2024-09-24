/* $Header: /cvs/root/winamp/vlb/tns.cpp,v 1.1 2009/04/28 20:21:11 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: tns.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: temporal noise shaping
 *
\***************************************************************************/

#include <math.h>

#include "tns.h"
#include "channelinfo.h"
#include "block.h"

CTns::CTns ()
 : m_TnsDataPresent (1)
{
}

CTns::~CTns ()
{
}

void CTns::Read (const CChannelInfo &info, CDolbyBitStream &bs)
{
  m_TnsDataPresent.Read (bs) ;
  if (!m_TnsDataPresent) return ;

  CVLBBitSequence n_filt (info.IsLongBlock () ? 2 : 1) ;
  CVLBBitSequence length (info.IsLongBlock () ? 6 : 4) ;
  CVLBBitSequence order  (info.IsLongBlock () ? 5 : 3) ;

  CVLBBitSequence direction (1) ;
  CVLBBitSequence coef_compress (1) ;
  CVLBBitSequence coef_res (1) ;

  for (int window = 0 ; window < info.GetWindowsPerFrame () ; window++)
  {
    m_NumberOfFilters [window] = n_filt.Read (bs) ;

    if (n_filt)
    {
      coef_res.Read (bs) ;

      int nextstopband = info.GetScaleFactorBandsTotal () ;

      for (int index = 0 ; index < n_filt ; index++)
      {
        CFilter &filter = m_Filter [window][index] ;

        length.Read (bs) ;

        filter.m_StartBand = nextstopband - length ;
        filter.m_StopBand  = nextstopband ; 

        nextstopband = filter.m_StartBand ;

        filter.m_Order = order.Read (bs) ;

        if (order)
        {
          filter.m_Direction = direction.Read (bs) ? -1 : 1 ;

          coef_compress.Read (bs) ;

          filter.m_Resolution = coef_res + 3 ;

          static const int sgn_mask [] = {  0x2,  0x4,  0x8 } ;
          static const int neg_mask [] = { ~0x3, ~0x7, ~0xF } ;

          int s_mask = sgn_mask [coef_res + 1 - coef_compress] ;
          int n_mask = neg_mask [coef_res + 1 - coef_compress] ;

          for (int i = 0 ; i < order ; i++)
          {
            CVLBBitSequence coef (filter.m_Resolution - coef_compress) ;
            coef.Read (bs) ;

            filter.m_Coeff [i] = ((int) coef & s_mask) ? ((int) coef | n_mask) : (int) coef ;
          }
        }
      }
    }
  }
}

void CTns::Apply (const CChannelInfo &info, CBlock &spectrum)
{
  if (!m_TnsDataPresent) return ;

  float CoeffLPC [MaximumOrder + 1] ;

  for (int window = 0 ; window < info.GetWindowsPerFrame () ; window++)
  {
    for (int index = 0 ; index < m_NumberOfFilters [window] ; index++)
    {
      CFilter &filter = m_Filter [window][index] ;

      DecodeCoefficients (filter, CoeffLPC) ;

      int start = Minimum (filter.m_StartBand, info.GetMaximumTnsBands (), 
                           info.GetScaleFactorBandsTransmitted ()) ;

      start = info.GetScaleFactorBandOffsets () [start] ;

	    int stop = Minimum (filter.m_StopBand, info.GetMaximumTnsBands (),
                          info.GetScaleFactorBandsTransmitted ()) ;

      stop = info.GetScaleFactorBandOffsets () [stop] ;

      int size = stop - start ;

      if (size <= 0) continue ;

      Filter (&spectrum.AccessSpectralData (window) [start], size,
              filter.m_Direction, CoeffLPC, filter.m_Order) ;
    }
  }
}

void CTns::DecodeCoefficients (CFilter &filter, float *a)
{
  float tmp [MaximumOrder + 1], b [MaximumOrder + 1] ;
  
  // Inverse quantization

  const float pi2 = 3.14159265358979323846F / 2.0F ;

  float iqfac   = ((1 << (filter.m_Resolution - 1)) - 0.5F) / pi2 ;
  float iqfac_m = ((1 << (filter.m_Resolution - 1)) + 0.5F) / pi2 ;

  int i ;

  for (i = 0 ; i < filter.m_Order ; i++)
  {
    tmp [i + 1] = (float) sin (filter.m_Coeff [i] / ((filter.m_Coeff [i] >= 0) ? iqfac : iqfac_m)) ;
  }
  
  // Conversion to LPC coefficients - Markel and Gray,  pg. 95

  a [0] = 1.0F ;

  for (int m = 1 ; m <= filter.m_Order ; m++)
  {
    b [0] = a [0] ;

    for (i = 1 ; i < m ; i++)
    {
      b [i] = a [i] + tmp [m] * a [m - i] ;
    }

    b [m] = tmp [m] ;

    for (i = 0 ; i <= m ; i++)
    {
      a [i] = b [i] ;
    }
  }
}

void CTns::Filter (float *spec, int size, int inc, float *lpc, int order)
{
  // - Simple all-pole filter of order "order" defined by
  //  y(n) =  x(n) - a(2)*y(n-1) - ... - a(order+1)*y(n-order)
  //   
  // - The state variables of the filter are initialized to zero every time
  //   
  // - The output data is written over the input data ("in-place operation")
  //   
  // - An input vector of "size" samples is processed and the index increment
  // to the next data sample is given by "inc"

  int i, j;
  float y, state [MaximumOrder] ;
  
  for (i = 0 ; i < order ; i++)
  {
    state [i] = 0.0F ;
  }

  if (inc == -1)
  {
    spec += size - 1 ;
  }

  for (i = 0 ; i < size ; i++)
  {
    y = *spec;

    for (j = 0 ; j < order ; j++)
    {
      y -= lpc [j + 1] * state [j] ;
    }

    for (j = order - 1 ; j > 0 ; j--)
    {
      state [j] = state [j - 1] ;
    }

    state [0] = y ;
    *spec = y ;
    spec += inc ;
  }
}

