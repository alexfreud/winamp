/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: giobase.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-02-11
 *   contents/description: HEADER - basic I/O class for MPEG Decoder
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:02 $
 * $Id: giobase.h,v 1.1 2010/11/17 20:46:02 audiodsp Exp $
 */

#ifndef __GIOBASE_H__
#define __GIOBASE_H__

/* ------------------------ includes --------------------------------------*/

#include "mp3sscdef.h"

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

class CGioBase
{
public:
 
  virtual SSC  Read(void *pBuffer, int cbToRead, int *pcbRead) = 0;
  virtual bool IsEof() const = 0;

protected:
  ~CGioBase() {}
private:

};

/*-------------------------------------------------------------------------*/
#endif
