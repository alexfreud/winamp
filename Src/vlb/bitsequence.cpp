/* $Header: /cvs/root/winamp/vlb/bitsequence.cpp,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

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

#include "bitsequence.h"
#include "bitstream.h"

CVLBBitSequence::CVLBBitSequence (const int length, const int value)
{
  m_Length = length ;
  m_Value  = value ;
}

CVLBBitSequence::CVLBBitSequence (const int length /* == 0 */)
{
  m_Length = length ;
  m_Value = 0 ;
}

CVLBBitSequence::~CVLBBitSequence ()
{
}

CVLBBitSequence &CVLBBitSequence::operator= (int value)
{
  m_Length = 0 ;
  m_Value = value ;
  return *this ;
}

CVLBBitSequence &CVLBBitSequence::operator+= (int value)
{
  m_Value += value ;
  return *this ;
}

int CVLBBitSequence::Read (CDolbyBitStream &bs, const int length)
{
  m_Length = length ;
  return Read (bs) ;
}
