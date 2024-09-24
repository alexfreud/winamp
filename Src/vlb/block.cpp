/* $Header: /cvs/root/winamp/vlb/block.cpp,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: block.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: base class for CLongBlock, CShortBlock
 *
\***************************************************************************/

#include <math.h> // pow()

#include "block.h"
#include "bitstream.h"
#include "channelinfo.h"

CBlock::CBlock (CChannelInfo &info)
 : m_IcsInfo (info) 
{
  m_LongWindow  [0] = m_OnlyLongWindowSine ;
  m_ShortWindow [0] = m_OnlyShortWindowSine ;

#ifdef ONLY_SINE_WINDOW
  m_LongWindow  [1] = m_OnlyLongWindowSine ;
  m_ShortWindow [1] = m_OnlyShortWindowSine ;
#else
  m_LongWindow  [1] = m_OnlyLongWindowKBD ;
  m_ShortWindow [1] = m_OnlyShortWindowKBD ;
#endif
}

CBlock::~CBlock ()
{
}

int CBlock::DecodeHuffmanWord (CDolbyBitStream &bs, const unsigned int (*CodeBook) [HuffmanEntries])
{
  unsigned int val, len ;
  unsigned int index = 0, bits = HuffmanBits ;

  while (true)
  {
    val = CodeBook [index][bs.Get (HuffmanBits)] ;
    len = val >> 16 ;

    if (len == 0)
    {
      index = val & 0xFFFF ;
      bits += HuffmanBits ;
      continue ;
    }
    else
    {
      if (len != bits)
      {
        bs.PushBack (bits - len) ;
      }

      val &= 0xFFFF ;

      break ;
    }
  }

  return val ;
}

int CBlock::GetEscape (CDolbyBitStream &bs, const int q)
{
  int i, off, neg ;

  if (q < 0)
  {
    if (q != -16) return q ;

    neg = 1 ;
  }
  else
  {
    if (q != +16) return q ;

    neg = 0;
  }

  for (i = 4 ; ; i++)
  {
    if (bs.Get (1) == 0)
      break ;
  }

  if (i > 16)
  {
    off = bs.Get (i - 16) << 16 ;
    off |= bs.Get (16) ;
  }
  else
  {
    off = bs.Get (i) ;
  }

  i = off + (1 << i) ;

  if (neg) i = -i ;

  return i ;
}

int CBlock::UnpackIndex (int idx, int *qp, const CodeBookDescription *hcb)
{
  if (hcb->Dimension == 4)
  {
    int index = idx << 2 ;

    qp [0] = hcb->NTuples [index] ;
    qp [1] = hcb->NTuples [index + 1] ;
    qp [2] = hcb->NTuples [index + 2] ;
    qp [3] = hcb->NTuples [index + 3] ;
  }
  else
  {
    int index = idx << 1 ;

    qp [0] = hcb->NTuples [index] ;
    qp [1] = hcb->NTuples [index + 1] ;
  }

  return hcb->Dimension ;
}

float CBlock::InverseQuantize (int q)
{
  if (q > 0)
  {
    if (q < InverseQuantTableSize)
    {
      return (m_InverseQuantTable [q]) ;
    }
    else
    {
      return (static_cast<float>(pow (static_cast<float>(q), 4.0F / 3.0F))) ;
    }
  }
  else
  {
    q = -q ;

    if (q < InverseQuantTableSize)
    {
      return (-m_InverseQuantTable [q]) ;
    }
    else
    {
      return (static_cast<float>(-pow (static_cast<float>(q), 4.0F / 3.0F))) ;
    }
  }
}

void CBlock::Read (CDolbyBitStream &bs, const int global_gain) 
{
  ReadSectionData (bs) ;
  ReadScaleFactorData (bs, global_gain) ;

  m_PulseData.Read (bs) ;
  m_Tns.Read (m_IcsInfo, bs) ;

  CVLBBitSequence GainControlDataPresent (1) ;

  GainControlDataPresent.Read (bs) ;
  if (GainControlDataPresent)
  {
    throw EUnimplemented () ;
  }

  ReadSpectralData (bs) ;
}

void CBlock::ApplyTools (void)
{
  m_Tns.Apply (m_IcsInfo, *this) ;
}

CBlock &operator+= (CBlock &lhs, CBlock &rhs)
{
  // operator+= sums up the spectral bins of two blocks
  // in the frequency domain (must switch together)

  // add two long blocks

  if (lhs.m_IcsInfo.IsLongBlock () && rhs.m_IcsInfo.IsLongBlock ())
  {
    if (lhs.m_IcsInfo.GetWindowShape () == rhs.m_IcsInfo.GetWindowShape ())
    {
      for (int i = 0 ; i < CLongBlock::MaximumBins ; i++)
      {
        lhs.AccessSpectralData () [i] += rhs.AccessSpectralData () [i] ;
      }
    }
  }

  // add two short blocks

  else if (lhs.m_IcsInfo.IsShortBlock () && rhs.m_IcsInfo.IsShortBlock ())
  {
    if (lhs.m_IcsInfo.GetWindowShape () == rhs.m_IcsInfo.GetWindowShape ())
    {
      for (int win = 0 ; win < CShortBlock::MaximumWindows ; win++)
      {
        for (int i = 0 ; i < CShortBlock::MaximumBins ; i++)
        {
          lhs.AccessSpectralData (win) [i] += rhs.AccessSpectralData (win) [i] ;
        }
      }
    }
  }

  return lhs ;
}
