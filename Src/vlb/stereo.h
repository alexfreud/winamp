/* $Header: /cvs/root/winamp/vlb/stereo.h,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: stereo.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: mid/side and intensity stereo processing
 *
\***************************************************************************/

#ifndef __STEREO_H__
#define __STEREO_H__

#include "bitsequence.h"

class CChannelInfo ;
class CBlock ;

/** Joint Stereo Processing.

    This class represents the stereo processing tool for decoding mid/side and 
    intensity coded bands of spectral coefficients.

    The Stereo Processing tool is not profile-dependent and the CJointStereo
    implementation follows the Read()/Apply() convention used for all tools
    as far as applicable.
*/

class CJointStereo
{
public :

  CJointStereo () ;
  ~CJointStereo () ;

  void Read (const CChannelInfo &info, CDolbyBitStream &bs) ;

  void ApplyMS (const CChannelInfo &info, CBlock &left, CBlock &right) ;
  void ApplyIS (const CChannelInfo &info, CBlock &left, CBlock &right) ;

protected :

  enum
  {
    MaximumGroups = 8,
    MaximumBands = 64 
  } ;

  bool m_MsUsed [CJointStereo::MaximumGroups][CJointStereo::MaximumBands] ;

  CVLBBitSequence m_MsMaskPresent ;

} ;

#endif
