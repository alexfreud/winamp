/* $Header: /cvs/root/winamp/vlb/tns.h,v 1.1 2009/04/28 20:21:11 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: tns.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: temporal noise shaping
 *
\***************************************************************************/

#ifndef __TNS_H__
#define __TNS_H__

#include "bitsequence.h"

class CChannelInfo ;
class CBlock ;

/** Temporal Noise Shaping.

    This class represents the temporal noise shaping tool for decoding and 
    applying tns filter data to the spectral coefficients of the current block.

    TNS is a profile-dependent tool and the CTns implementation follows the
    Read()/Apply() convention used for all tools.
*/

class CTns
{
public :

  CTns () ;
  ~CTns () ;

  void Read (const CChannelInfo &info, CDolbyBitStream &bs) ;
  void Apply (const CChannelInfo &info, CBlock &spectrum) ;

  enum
  {
    MaximumWindows = 8,
    MaximumBands = 49,
    MaximumOrder = 31,
    MaximumFilters = 3
  } ;

protected :

  CVLBBitSequence m_TnsDataPresent ;

  class CFilter
  {
    public : 

      int m_StartBand ;
      int m_StopBand ;

      int m_Direction ;
      int m_Resolution ;

      int m_Order ;
      int m_Coeff [MaximumOrder] ;

  } ;

  int m_NumberOfFilters [MaximumWindows] ;
  CFilter m_Filter [MaximumWindows][MaximumFilters] ;

  int Minimum (int a, int b, int c)
  {
    int t = (a < b ) ? a : b ;
    return (t < c) ? t : c ;
  }

  void DecodeCoefficients (CFilter &filter, float *a) ;
  void Filter (float *spec, int size, int inc, float *lpc, int order) ;

} ;

#endif
