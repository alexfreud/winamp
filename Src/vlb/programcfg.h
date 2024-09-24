/* $Header: /cvs/root/winamp/vlb/programcfg.h,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: programcfg.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: program config element
 *
\***************************************************************************/

#ifndef __PROGRAMCFG_H__
#define __PROGRAMCFG_H__

#include "bitsequence.h"

class CDolbyBitStream ;

/** PCE Program Configuration Element.

    This class holds the program configuration data read from the bitstream
    and maps the decoded channels according to the currently active program.
*/

class CProgramConfig
{

  enum
  {
    FrontRow = 0,
    SideRow = 1,
    BackRow = 2,
    SpeakerRows = 3,

    MaximumChannels = 16,
    MaximumCommentLength = 64
  } ;

  CVLBBitSequence m_ElementInstanceTag;
  CVLBBitSequence m_Profile ;
  CVLBBitSequence m_SamplingFrequencyIndex ;

  CVLBBitSequence m_NumLfeChannelElements ;
  CVLBBitSequence m_NumAssocDataElements ;
  CVLBBitSequence m_NumValidCcElements ;

  CVLBBitSequence m_MonoMixdownPresent ;
  CVLBBitSequence m_MonoMixdownElementNumber ;

  CVLBBitSequence m_StereoMixdownPresent ;
  CVLBBitSequence m_StereoMixdownElementNumber ;

  CVLBBitSequence m_MatrixMixdownIndexPresent ;
  CVLBBitSequence m_MatrixMixdownIndex ;
  CVLBBitSequence m_PseudoSurroundEnable ;

  CVLBBitSequence m_NumberOfChannelElements [SpeakerRows] ;

  CVLBBitSequence m_ChannelElementIsCpe [SpeakerRows][MaximumChannels] ;
  CVLBBitSequence m_ChannelElementTagSelect [SpeakerRows][MaximumChannels] ;

  CVLBBitSequence m_LfeElementTagSelect [MaximumChannels] ;
  CVLBBitSequence m_AssocDataElementTagSelect [MaximumChannels] ;

  CVLBBitSequence m_CcElementIsIndSw [MaximumChannels] ;
  CVLBBitSequence m_ValidCcElementTagSelect [MaximumChannels] ;

  CVLBBitSequence m_CommentFieldBytes ;
  char m_Comment [MaximumCommentLength] ;

  unsigned int m_NumChannels;

public :

  CProgramConfig () ;
  ~CProgramConfig () ;

  void ResetNonMCConfig(void);

  void Read (CDolbyBitStream &bs) ;

  //

  bool AddChannel (const int tag, const bool isCPE) ;

  //

  bool AddSingleChannel (const int tag) ;
  bool AddChannelPair (const int tag) ;
  bool AddCouplingChannel (const int tag) ;
  bool AddLowFrequencyChannel (const int tag) ;

  // explicit query

  bool IsFrontChannel (const int tag) { return IsChannelLocatedAt (FrontRow, tag) ; }
  bool IsFrontChannelPair (const int tag) { return IsChannelPairLocatedAt (FrontRow, tag) ; }

  bool IsBackChannel (const int tag) { return IsChannelLocatedAt (BackRow, tag) ; }
  bool IsBackChannelPair (const int tag) { return IsChannelPairLocatedAt (BackRow, tag) ; }

  // query by index

  int GetNumberOfElements (const int row) ;
  int GetNumberOfChannels (const int row) ;

  int GetElementTag (const int row, const int index) ;
  bool IsChannelPair (const int row, const int index) ;

  int GetNumberOfLowFrequencyChannels (void) ;
  int GetLowFrequencyChannelTag (const int index) ;

  //


  bool HasSpeakerMapping (void) const
  {
    return m_ExplicitMapping ;
  }

  unsigned int GetElementInstnaceTag (void) const
  {
    return m_ElementInstanceTag ;
  }

  unsigned int GetProfile (void) const
  {
    return m_Profile ;
  }

  unsigned int GetSamplingFrequencyIndex (void) const
  {
    return m_SamplingFrequencyIndex ;
  }

  unsigned int GetNumChannels (void) const
  {
    return m_NumChannels ;
  }

  unsigned int GetNumFrontChannels (void) const
  {
    return m_NumberOfChannelElements[FrontRow] ;
  }

  unsigned int GetNumSideChannels (void) const
  {
    return m_NumberOfChannelElements[SideRow] ;
  }

  unsigned int GetNumBackChannels (void) const
  {
    return m_NumberOfChannelElements[BackRow] ;
  }

  unsigned int GetNumLfeChannels (void) const
  {
    return m_NumLfeChannelElements ;
  }

  unsigned int GetNumCouplingChannels (void) const
  {
    return m_NumValidCcElements ;
  }

  unsigned int GetCouplingChannelTag (int index) const
  {
    return m_ValidCcElementTagSelect[index] ;
  }

  unsigned int GetMonoMixdownPresent (void) const
  {
    return m_MonoMixdownPresent ;
  }

  unsigned int GetStereoMixdownPresent (void) const
  {
    return m_StereoMixdownPresent ;
  }

  unsigned int GetMatrixMixdownPresent (void) const
  {
    return m_MatrixMixdownIndexPresent ;
  }

  unsigned int GetPseudoSurroundPresent (void) const
  {
    return m_PseudoSurroundEnable ;
  }

  char* GetCommentFieldData (void);




protected :

  bool IsChannelLocatedAt (const int row, const int tag) ;
  bool IsChannelPairLocatedAt (const int row, const int tag) ;

  // non-multichannel configuration

  bool m_NonMC_ConfigSet ;
  bool m_NonMC_IsCPE ;

  int m_NonMC_Tag ;

  //

  bool m_ExplicitMapping ;


  unsigned int m_NumberOfChannels [SpeakerRows] ;

} ;

#endif
