/* $Header: /cvs/root/winamp/vlb/plainfile.h,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: plainfile.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: tiny file reader input class
 *
\***************************************************************************/

#ifndef __PLAINFILE_H__
#define __PLAINFILE_H__

#include <stdio.h>

#include "bitstream.h"
#include "DataIO.h"

/** A Simple File Reading Input Class.

    This class is a simple input class adhering to the CDolbyBitStream interface. It can
    be used to read raw and ADIF-style bitstreams.
*/

class CPlainFile : public CDolbyBitStream
{

public :

  CPlainFile (DataIOControl *aacData) ;
  virtual ~CPlainFile () ;

  // // //

  virtual void ByteAlign (void) ;
  virtual long Get (int n) ;

  virtual void PushBack (int n) ;

  /** Raw Block Read Method.

      This method sequentially reads chunks of data from the input file.

      @param pData The buffer where the data chunk should be stored.
      @param cBytes The number of bytes to read.
      @return The number of bytes actually read.
  */

  int Read (void *pData, int cBytes) ;

  int LockAndLoad();

protected :

  long GetShort (void) ;

  DataIOControl *m_InputData ;

  int m_ValidBits ;
  unsigned long m_Data ;

  static const unsigned long m_ValidMask [32] ;

  char m_tmpadifbits[4]; //CT
  int m_sendadifbits;
} ;

#endif
