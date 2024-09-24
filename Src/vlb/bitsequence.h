/* $Header: /cvs/root/winamp/vlb/bitsequence.h,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: bitsequence.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: bitstream helper object
 *
\***************************************************************************/

#ifndef __VLB_BITSEQUENCE_H__
#define __VLB_BITSEQUENCE_H__

#include "bitstream.h"

/** Bitstream Parser Utility.

    This helper class is basically a numerical value that can read itself from a 
    \Ref{CDolbyBitStream} interface for convenience. The decoder almost completely
    does the bitstream parsing through CVLBBitSequence rather than \Ref{CDolbyBitStream} directly.
*/

class CVLBBitSequence
{

public :

  CVLBBitSequence (const int length = 0) ;
  CVLBBitSequence (const int length, const int value) ;

  ~CVLBBitSequence () ;

  int Read (CDolbyBitStream& bs, const int length) ;

  int Read (CDolbyBitStream& bs)
  {
    m_Value = bs.Get (m_Length) ;
    return m_Value ;
  }

  operator int() const
  {
    return m_Value ;
  }

  CVLBBitSequence &operator=(int) ;
  CVLBBitSequence &operator+=(int) ;

protected :

  int m_Length ;
  int m_Value ;

} ;

#endif
