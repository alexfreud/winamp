/* $Header: /cvs/root/winamp/vlb/datastream.cpp,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: datastream.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: data stream element
 *
\***************************************************************************/

#include "datastream.h"

CDataStream::CDataStream (CDolbyBitStream &bs)
 : m_bs (bs),
   m_DataByteAlignFlag (1),
   m_Count (8),
   m_EscCount (8)
{
}

CDataStream::~CDataStream ()
{
}

void CDataStream::Read (void)
{
  m_DataByteAlignFlag.Read (m_bs) ;

  m_Count.Read (m_bs) ;

  if (m_Count == 255)
  {
    m_Count += m_EscCount.Read (m_bs) ;
  }

  if (m_DataByteAlignFlag)
  {
    m_bs.ByteAlign () ;
  }

  for (int i = 0 ; i < m_Count ; i++)
  {
    m_DataStreamByte [i].Read (m_bs, 8) ;
  }
}
