/* $Header: /cvs/root/winamp/vlb/adif.h,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: adif.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: audio data interchange format header
 *
\***************************************************************************/

#ifndef __ADIF_H__
#define __ADIF_H__

#include "bitsequence.h"
#include "programcfg.h"
#include "exception.h"

class CDolbyBitStream ;

/** Audio Data Interchange Format.

    This class is able to read the header information from an ADIF file
    and serves as a container for the Program Configuration Elements
    \Ref{CProgramConfig} found in the bitstream.
*/

class CAdifHeader
{

public :

  CAdifHeader () ;
  ~CAdifHeader () ;

  void Read (CDolbyBitStream &bs) ;

  CProgramConfig &GetProgramConfig (int index) ;

  unsigned int GetBitRate (void) const
  {
	return m_BitRate ;
  }

  unsigned int GetNumProgramConfigElements (void) const
  {
	  return m_NumProgramConfigElements;
  }

  unsigned int GetOriginalCopy (void) const
  {
	  return m_OriginalCopy;
  }
  
  unsigned int GetHome (void) const
  {
	  return m_Home;
  }

protected :

  enum
  {
    MaximumPCEs = 16
  } ;

  CVLBBitSequence m_CopyrightIdPresent ;
  CVLBBitSequence m_OriginalCopy ;
  CVLBBitSequence m_Home ;
  CVLBBitSequence m_BitstreamType ;
  CVLBBitSequence m_NumProgramConfigElements ;

  CProgramConfig m_ProgramConfigElement [MaximumPCEs] ;

  unsigned int m_BitRate ;

  DECLARE_EXCEPTION(ENoAdifHeader, AAC_NOTADIFHEADER, "Not Valid ADIF Header") ;

} ;

#endif
