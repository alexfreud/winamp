/* $Header: /cvs/root/winamp/vlb/datastream.h,v 1.1 2009/04/28 20:21:09 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: datastream.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: data stream element
 *
\***************************************************************************/

#ifndef __DATASTREAM_H__
#define __DATASTREAM_H__

#include "bitsequence.h"

/** DSE Data Stream Element.

    This class represents a data stream element, that may be used to transport
    additional user data embedded within the audio bitstream.
*/

class CDataStream
{
public :

  CDataStream (CDolbyBitStream &) ;
  ~CDataStream () ;

  void Read (void) ;

  int Length (void)
  {
    return m_Count ;
  }

protected :

  CDolbyBitStream &m_bs ;

  CVLBBitSequence m_DataByteAlignFlag ;
  CVLBBitSequence m_Count ;
  CVLBBitSequence m_EscCount ;

  enum
  {
    MaximumElementLength = 512
  } ;

  CVLBBitSequence m_DataStreamByte [MaximumElementLength] ;

} ;

#endif
