/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3ssc.h
 *   project : ---
 *   author  : Martin Sieler
 *   date    : 1999-02-15
 *   contents/description: ssc helper class (Structured Status Code)
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:04 $
 * $Id: mp3ssc.h,v 1.1 2010/11/17 20:46:04 audiodsp Exp $
 */

#ifndef __MP3SSC_H__
#define __MP3SSC_H__

/* ------------------------ includes --------------------------------------*/

#include "mp3sscdef.h"

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

/** Helper class for more information about SSC codes.
*/
class CMp3Ssc
{
public:
  /** Object constructor

    @param An SSC staus code to initialize the object with.

  */
  CMp3Ssc(SSC ssc);
  ~CMp3Ssc() {}

  /** Operator for conversion to a text string.

    @return Textual description.

  */
  operator const char*();

private:
  SSC  m_ssc;
};

/*-------------------------------------------------------------------------*/
#endif
