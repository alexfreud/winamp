/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: bitsequence.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-23
 *   contents/description: HEADER - bitsequence object
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:01 $
 * $Id: bitsequence.h,v 1.1 2010/11/17 20:46:01 audiodsp Exp $
 */

#ifndef __BITSEQUENCE_H__
#define __BITSEQUENCE_H__

/* ------------------------ includes --------------------------------------*/

#include "bitstream.h"

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

//
// Bitstream parser class.
//
//  This helper class is basically a numerical value that can read itself from
//  a CBitStream interface for convenience. The decoder almost completely
//  does the bitstream parsing through CBitSequence rather than CBitStream
//  directly.
//

class CBitSequence
{
public:

  CBitSequence(int nBits = 0) { m_nBits = nBits; m_nValue = 0; }
  virtual ~CBitSequence() {}

  void SetNumberOfBits(int nBits) { m_nBits = nBits; }
  int  GetNumberOfBits() const    { return m_nBits; }

  bool ReadFrom(CBitStream &Bs) { m_nValue = Bs.GetBits(m_nBits); return true; }
  bool ReadFrom_Bit(CBitStream &Bs) { m_nValue = Bs.Get1Bit(); return true; }
  bool ReadFrom(CBitStream &Bs, int nBits) { SetNumberOfBits(nBits); return ReadFrom(Bs); }

  bool Equals(int nValue) const { return (m_nValue == nValue); }

  int  ToInt() const        { return m_nValue; }
  void FromInt(int nValue) { m_nValue = nValue; }

protected:

private:

  int m_nBits;
  int m_nValue;
};

/*-------------------------------------------------------------------------*/
#endif
