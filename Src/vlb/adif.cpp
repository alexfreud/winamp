/* $Header: /cvs/root/winamp/vlb/adif.cpp,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: adif.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: audio data interchange format header
 *
\***************************************************************************/

#include "adif.h"

// CAUTION: some elements read more than 16bits,
// this is not supported by CDolbyBitStream::Get().

CAdifHeader::CAdifHeader ()
 : m_CopyrightIdPresent (1),
   m_OriginalCopy (1),
   m_Home (1),
   m_BitstreamType (1),
   m_NumProgramConfigElements (4)
{
  m_BitRate = 0 ;
}

CAdifHeader::~CAdifHeader ()
{
}

#include<stdio.h>
void CAdifHeader::Read (CDolbyBitStream &bs)
{
  char A[4];
  A[0]=(char)(bs.Get (8));
  A[1]=(char)(bs.Get (8));
  A[2]=(char)(bs.Get (8));
  A[3]=(char)(bs.Get (8));
  if (A[0]!= 'A') throw ENoAdifHeader () ;
  if (A[1]!= 'D') throw ENoAdifHeader () ;
  if (A[2]!= 'I') throw ENoAdifHeader () ;
  if (A[3]!= 'F') throw ENoAdifHeader () ;

  if (m_CopyrightIdPresent.Read (bs))
  {
    for (int i = 0 ; i < 9 ; i++)
    {
      bs.Get (8) ; // CopyrightId
    }
  }

  m_OriginalCopy.Read (bs) ;
  m_Home.Read (bs) ;
  m_BitstreamType.Read (bs) ;

  m_BitRate = bs.Get (16) ;
  m_BitRate <<= 7 ;
  m_BitRate |= bs.Get (7) ;

  //

  m_NumProgramConfigElements.Read (bs) ;

  for (int i = 0 ; i <= m_NumProgramConfigElements ; i++)
  {
    if (m_BitstreamType == 0)
    {
      // we don't care much about adif_buffer_fullness

      bs.Get (16) ;
      bs.Get (4) ;
    }

    m_ProgramConfigElement[i].Read (bs);
  }
}

CProgramConfig &CAdifHeader::GetProgramConfig (int index)
{
  return m_ProgramConfigElement [index] ;
}

