/* $Header: /cvs/root/winamp/vlb/channelinfo.cpp,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: channelinfo.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: independent channel stream info object
 *
\***************************************************************************/

#include "channelinfo.h"
#include "streaminfo.h"

// // // CChannelInfo wraps ics_info()

CChannelInfo::CChannelInfo ()
 : m_IcsReservedBit (1),
   m_WindowSequence (2),
   m_WindowShape (1),
   m_ScaleFactorGrouping (7),
   m_PredictorDataPresent (1)

#ifdef MAIN_PROFILE   
   ,
   m_PredictorReset (1),
   m_PredictorResetGroupNumber (5)
#endif
{
  m_Valid = false ;
  m_TotalSfBands = 0 ;
}

CChannelInfo::~CChannelInfo ()
{
}

int CChannelInfo::GetProfile (void) const
{
  return m_Profile ;
}

bool CChannelInfo::IsValid (void) const
{
  return m_Valid ;
}

void CChannelInfo::Reset (const CStreamInfo &si)
{
  m_Valid = false ;
  m_TotalSfBands = 0 ;

  m_SamplingRateIndex = si.GetSamplingRateIndex () ;
  m_Profile = si.GetProfile () ;
}

bool CChannelInfo::IsLongBlock (void) const
{
  return (m_WindowSequence != EightShortSequence) ;
}

bool CChannelInfo::IsShortBlock (void) const
{
  return (m_WindowSequence == EightShortSequence) ;
}

bool CChannelInfo::IsMainProfile (void) const
{
  return (m_Profile == ProfileMain) ;
}

int CChannelInfo::GetWindowsPerFrame (void) const
{
  return (m_WindowSequence == EightShortSequence) ? 8 : 1 ;
}

int CChannelInfo::GetWindowSequence (void) const
{
  return m_WindowSequence ;
}

int CChannelInfo::GetWindowGroups (void) const
{
  return m_WindowGroups ;
}

int CChannelInfo::GetWindowGroupLength (int index) const
{
  return m_WindowGroupLength [index] ;
}

// scale factor band indices

const int *CChannelInfo::GetScaleFactorBandOffsets (void) const
{
  if (IsLongBlock ())
  {
    return m_SamplingRateInfoTable [m_SamplingRateIndex].ScaleFactorBands_Long ;
  }
  else
  {
    return m_SamplingRateInfoTable [m_SamplingRateIndex].ScaleFactorBands_Short ;
  }
}

int CChannelInfo::GetLastBin()
{
	if (IsLongBlock())
	{
		return m_SamplingRateInfoTable[m_SamplingRateIndex].ScaleFactorBands_Long[m_MaxSfBands];
	}
	else
	{
		return m_SamplingRateInfoTable[m_SamplingRateIndex].ScaleFactorBands_Short[m_MaxSfBands];
	}
}


int CChannelInfo::GetSamplingFrequency (void) const
{
  return SamplingRateFromIndex (m_SamplingRateIndex) ;
}

int CChannelInfo::SamplingRateFromIndex (int index)
{
  return m_SamplingRateInfoTable [index].SamplingFrequency ;
}

#ifdef MAIN_PROFILE
int CChannelInfo::GetMaximumPredictionBands (void) const
{
  return m_SamplingRateInfoTable [m_SamplingRateIndex].MaximumPredictionBands ;
}

void CChannelInfo::DeactivatePrediction (int band)
{
  if (band < GetMaximumPredictionBands ())
  {
    m_PredictionUsed [band] = false ;
  }
}
#endif

void CChannelInfo::Read (CDolbyBitStream &bs)
{
  m_IcsReservedBit.Read (bs) ;

  m_WindowSequence.Read (bs) ;
  m_WindowShape.Read (bs) ;

#ifdef ONLY_SINE_WINDOW
  if (m_WindowShape == 1)
    throw EUnsupportedWindowShape () ;
#endif

  if (IsLongBlock ())
  {
    m_TotalSfBands = m_SamplingRateInfoTable [m_SamplingRateIndex].NumberOfScaleFactorBands_Long ;

    m_MaxSfBands.Read (bs, 6) ; 
    
    if (m_PredictorDataPresent.Read (bs))
    {
#ifdef MAIN_PROFILE
      if (m_PredictorReset.Read (bs))
      {
        m_PredictorResetGroupNumber.Read (bs) ;

        if ((m_PredictorResetGroupNumber < 1) || (m_PredictorResetGroupNumber > 30))
        {
          throw EInvalidPredictorReset () ;
        }
      }

      int maxpred = (GetScaleFactorBandsTransmitted () < GetMaximumPredictionBands ()) ?
                     GetScaleFactorBandsTransmitted () : GetMaximumPredictionBands () ;

      for (int band = 0 ; band < maxpred ; band++)
      {
        m_PredictionUsed [band] = bs.Get (1) ? true : false ;
      }
#else
	  throw EIllegalProfile();
#endif
    }

    m_WindowGroups = 1 ;
    m_WindowGroupLength [0] = 1 ;
  }
  else
  {
    m_TotalSfBands = m_SamplingRateInfoTable [m_SamplingRateIndex].NumberOfScaleFactorBands_Short ;

    m_MaxSfBands.Read (bs, 4) ; 
    m_ScaleFactorGrouping.Read (bs) ;

    // // // expand group lengths

    m_WindowGroups = 0 ;

    for (int i = 0 ; i < 7 ; i++)
    {
      int mask = 1 << (6 - i) ;

      m_WindowGroupLength [i] = 1 ;

      if (m_ScaleFactorGrouping & mask)
      {
        m_WindowGroupLength [m_WindowGroups]++ ;
      }
      else
      {
        m_WindowGroups++ ;
      }
    }

    // loop runs to i < 7 only

    m_WindowGroupLength [7] = 1 ;
    m_WindowGroups++ ;
  }

  m_Valid = true ;
}

int CChannelInfo::GetMaximumTnsBands (void) const
{
  static const int tns_max_bands_tbl [12][4] =
  {
    /* entry for each sampling rate	
     * 1    Main/LC long window
     * 2    Main/LC short window
     * 3    SSR long window
     * 4    SSR short window
     */

    { 31,  9, 28,  7 },	    /* 96000 */
    { 31,  9, 28,  7 },	    /* 88200 */
    { 34, 10, 27,  7 },	    /* 64000 */
    { 40, 14, 26,  6 },	    /* 48000 */
    { 42, 14, 26,  6 },	    /* 44100 */
    { 51, 14, 26,  6 },	    /* 32000 */
    { 46, 14, 29,  7 },	    /* 24000 */
    { 46, 14, 29,  7 },	    /* 22050 */
    { 42, 14, 23,  8 },	    /* 16000 */
    { 42, 14, 23,  8 },	    /* 12000 */
    { 42, 14, 23,  8 },	    /* 11025 */
    { 39, 14, 19,  7 },	    /* 8000  */
  } ;

  int i = IsLongBlock () ? 0 : 1 ;
  i += (GetProfile () == ProfileSSR) ? 2 : 0 ;
    
  return tns_max_bands_tbl [m_SamplingRateIndex][i] ;
}
