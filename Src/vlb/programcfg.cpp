/* $Header: /cvs/root/winamp/vlb/programcfg.cpp,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: programcfg.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: program config element
 *
\***************************************************************************/

#include "programcfg.h"

CProgramConfig::CProgramConfig ()
 : m_ElementInstanceTag (4),
   m_Profile (2),
   m_SamplingFrequencyIndex (4), 
   m_NumLfeChannelElements (2),
   m_NumAssocDataElements (3),
   m_NumValidCcElements (4),

   m_MonoMixdownPresent (1),
   m_MonoMixdownElementNumber (4),

   m_StereoMixdownPresent (1), 
   m_StereoMixdownElementNumber (4),

   m_MatrixMixdownIndexPresent (1),
   m_MatrixMixdownIndex (2),
   m_PseudoSurroundEnable (1),

   m_CommentFieldBytes (8)
{
  m_ExplicitMapping = false ;
  m_NonMC_ConfigSet = false ;

  for (int row = FrontRow ; row < SpeakerRows ; row++)
  {
    m_NumberOfChannels [row] = 0 ;
    m_NumberOfChannelElements [row] = 0 ;
  }
}

CProgramConfig::~CProgramConfig ()
{
}

void CProgramConfig::ResetNonMCConfig(void)
{
	m_ExplicitMapping = false ;
	m_NonMC_ConfigSet = false ;
	
	for (int row = FrontRow ; row < SpeakerRows ; row++)
	{
		m_NumberOfChannels [row] = 0 ;
		m_NumberOfChannelElements [row] = 0 ;
	}
}

void CProgramConfig::Read (CDolbyBitStream &bs)
{
  int i, row;

  m_ElementInstanceTag.Read(bs) ;
  m_Profile.Read (bs) ;
  m_SamplingFrequencyIndex.Read (bs) ;

  for (row = FrontRow ; row < SpeakerRows ; row++)
    m_NumberOfChannelElements [row].Read (bs, 4) ;

  m_NumLfeChannelElements.Read (bs) ;
  m_NumAssocDataElements.Read (bs) ;
  m_NumValidCcElements.Read (bs) ;

  if (m_MonoMixdownPresent.Read (bs))
  {
    m_MonoMixdownElementNumber.Read (bs) ;
  }

  if (m_StereoMixdownPresent.Read (bs))
  {
    m_StereoMixdownElementNumber.Read (bs) ;
  }

  if (m_MatrixMixdownIndexPresent.Read (bs))
  {
    m_MatrixMixdownIndex.Read (bs) ;
    m_PseudoSurroundEnable.Read (bs) ;
  }

  m_NumChannels = 0;
  for (row = FrontRow ; row < SpeakerRows ; row++)
  {
    m_NumberOfChannels [row] = 0 ;

    for (i = 0 ; i < m_NumberOfChannelElements [row] ; i++)
    {
      m_ChannelElementIsCpe [row][i].Read (bs, 1) ;
      m_ChannelElementTagSelect [row][i].Read (bs, 4) ;

      m_NumberOfChannels [row] += m_ChannelElementIsCpe [row][i] ? 2 : 1 ;
	  m_NumChannels += m_NumberOfChannels [row];
    }
  }

  for (i = 0 ; i < m_NumLfeChannelElements ; i++)
  {
    m_LfeElementTagSelect [i].Read (bs, 4) ;
  }

  for (i = 0 ; i < m_NumAssocDataElements ; i++)
  {
    m_AssocDataElementTagSelect [i].Read (bs, 4) ;
  }

  for (i = 0 ; i < m_NumValidCcElements ; i++)
  {
    m_CcElementIsIndSw [i].Read (bs, 1) ;
    m_ValidCcElementTagSelect [i].Read (bs, 4) ;
  }

  // comment

  bs.ByteAlign () ;

  m_CommentFieldBytes.Read (bs) ;

  CVLBBitSequence text (8) ;

  for (i = 0 ; i < m_CommentFieldBytes ; i++)
  {
    text.Read (bs) ;

    if (i < MaximumCommentLength)
    {
      m_Comment [i] = text ;
    }
  }

  m_ExplicitMapping = true ;
}

// the decoder calls this method with every
// new element id found in the bitstream.

bool CProgramConfig::AddChannel (const int tag, const bool isCPE)
{
  if (!m_NonMC_ConfigSet)
  {
    m_NonMC_Tag = tag ;
    m_NonMC_IsCPE = isCPE ;
    m_NonMC_ConfigSet = true ;

    return true ;
  }
  else
  {
    return ((m_NonMC_IsCPE == isCPE) && (m_NonMC_Tag == tag)) ;
  }
}

bool CProgramConfig::AddSingleChannel (const int tag)
{
  if (m_ExplicitMapping)
  {
    for (int row = 0 ; row < SpeakerRows ; row++)
    {
      for (int i = 0 ; i < m_NumberOfChannelElements [row] ; i++)
      {
        if ((m_ChannelElementTagSelect [row][i] == tag) && !m_ChannelElementIsCpe [row][i])
          return true ;
      }
    }

    return false ;
  }

  return true ;
}

bool CProgramConfig::AddChannelPair (const int tag)
{
  if (m_ExplicitMapping)
  {
    for (int row = FrontRow ; row < SpeakerRows ; row++)
    {
      for (int i = 0 ; i < m_NumberOfChannelElements [row] ; i++)
      {
        if ((m_ChannelElementTagSelect [row][i] == tag) && m_ChannelElementIsCpe [row][i])
          return true ;
      }
    }

    return false ;
  }

  return true ;
}

bool CProgramConfig::AddCouplingChannel (const int tag)
{
  if (m_ExplicitMapping)
  {
    for (int i = 0 ; i < m_NumValidCcElements ; i++)
    {
      if (m_ValidCcElementTagSelect [i] == tag) 
	  {
		  // inc num_coupling_chan;
			  return true ;
	  }
    }
  
    return false ;
  }

  return true ;
}

bool CProgramConfig::AddLowFrequencyChannel (const int tag)
{
  if (m_ExplicitMapping)
  {
    for (int i = 0 ; i < m_NumLfeChannelElements ; i++)
    {
      if (m_LfeElementTagSelect [i] == tag) return true ;
    }

    return false ;
  }

  return true ;
}

bool CProgramConfig::IsChannelLocatedAt (const int row, const int tag)
{
  for (int i = 0 ; i < m_NumberOfChannelElements [row] ; i++)
  {
    if (!m_ChannelElementIsCpe [row][i] && (m_ChannelElementTagSelect [row][i] == tag))
      return true ;
  }

  return false ;
}

bool CProgramConfig::IsChannelPairLocatedAt (const int row, const int tag)
{
  for (int i = 0 ; i < m_NumberOfChannelElements [row] ; i++)
  {
    if (m_ChannelElementIsCpe [row][i] && (m_ChannelElementTagSelect [row][i] == tag))
      return true ;
  }

  return false ;
}

int CProgramConfig::GetNumberOfElements (const int row)
{
  return m_NumberOfChannelElements [row] ;
}

int CProgramConfig::GetNumberOfChannels (const int row)
{
  return m_NumberOfChannels [row] ;
}

int CProgramConfig::GetElementTag (const int row, const int index)
{
  return m_ChannelElementTagSelect [row][index] ;
}

bool CProgramConfig::IsChannelPair (const int row, const int index)
{
  return m_ChannelElementIsCpe [row][index] ? true : false ;
}

int CProgramConfig::GetNumberOfLowFrequencyChannels (void)
{
  return m_NumLfeChannelElements ;
}

int CProgramConfig::GetLowFrequencyChannelTag (const int index)
{
  return m_LfeElementTagSelect [index] ;
}


char* CProgramConfig::GetCommentFieldData (void)
{
	char* comment = new char[m_CommentFieldBytes];

	for (int i = 0; i < m_CommentFieldBytes; i++)
		comment[i] = m_Comment[i];

	return comment;
}

