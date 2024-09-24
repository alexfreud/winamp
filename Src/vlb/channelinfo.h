/* $Header: /cvs/root/winamp/vlb/channelinfo.h,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: channelinfo.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: independent channel stream info object
 *
 * $Header: /cvs/root/winamp/vlb/channelinfo.h,v 1.1 2009/04/28 20:21:08 audiodsp Exp $
 *
\***************************************************************************/

#ifndef __CHANNELINFO_H__
#define __CHANNELINFO_H__

#include "bitsequence.h"
#include "exception.h"
#include "streaminfo.h"

class CDolbyBitStream ;
class CProgramConfig ;
class CStreamInfo ;

/** Channel Element Side Information.

    This class provides information essential to various parts of the decoder.
    It reads the ics_info() bitstream elements and provides accordingly mapped
    access to some static tables, first of all scale factor band widths.
*/

class CChannelInfo
{

public :

  // // //

  enum
  {
    OnlyLongSequence = 0,
    LongStartSequence,
    EightShortSequence,
    LongStopSequence,

#ifdef MAIN_PROFILE
    PredictionBandsTotal = 42,
#endif

    ProfileMain = 0,
    ProfileLowComplexity,
    ProfileSSR,
    ProfileReserved

  } ;

  // // //

  CChannelInfo () ;
  ~CChannelInfo () ;

  void Read (CDolbyBitStream &bs) ;

  bool IsValid (void) const ;
  bool IsLongBlock (void) const ;
  bool IsShortBlock (void) const ;
  bool IsMainProfile (void) const ;

  void Reset (const CStreamInfo &) ;

  int GetProfile (void) const ;

  int GetWindowShape (void) const
  {
    return m_WindowShape ;
  }

  int GetWindowSequence (void) const ;
  int GetWindowsPerFrame (void) const ;

  int GetWindowGroups (void) const ;
  int GetWindowGroupLength (int index) const ;

  int GetScaleFactorBandsTransmitted (void) const
  {
    return m_MaxSfBands ;
  }

  int GetScaleFactorBandsTotal (void) const
  {
    return m_TotalSfBands ;
  }

  const int *GetScaleFactorBandOffsets (void) const ;
 
  int GetSamplingFrequency (void) const ;
  int GetMaximumTnsBands (void) const ;

#ifdef MAIN_PROFILE
  // // // Prediction

  int GetMaximumPredictionBands (void) const ;

  bool GetPredictorDataPresent (void) const
  {
    return m_PredictorDataPresent ? true : false ;
  }

  bool GetPredictionUsedForBand (int band) const
  {
    return m_PredictionUsed [band] ;
  }

  bool GetPerformPredictorReset (void) const
  {
    if (GetPredictorDataPresent ())
    {
      return m_PredictorReset ? true : false ;
    }
    
    return false ;
  }

  int GetPredictorResetGroupNumber (void) const
  {
    return m_PredictorResetGroupNumber ;
  }

  void DeactivatePrediction (int band) ;

  // // //
#endif

  static int SamplingRateFromIndex (int index) ;

  int GetSamplingIndex(){return m_SamplingRateIndex;}
  int GetLastBin() ;
  
protected :
  bool m_Valid ;

  CVLBBitSequence m_IcsReservedBit ;
  CVLBBitSequence m_WindowSequence ;
  CVLBBitSequence m_WindowShape ;
  CVLBBitSequence m_MaxSfBands ;
  CVLBBitSequence m_ScaleFactorGrouping ;

  // prediction

  CVLBBitSequence m_PredictorDataPresent ;
#ifdef MAIN_PROFILE
  CVLBBitSequence m_PredictorReset ;
  CVLBBitSequence m_PredictorResetGroupNumber ;

  bool m_PredictionUsed [CChannelInfo::PredictionBandsTotal] ;

  // // //
#endif

  int m_TotalSfBands ;
  int m_SamplingRateIndex, m_Profile ;

  int m_WindowGroups, m_WindowGroupLength [8] ;

  // // //

#ifdef MAIN_PROFILE
  DECLARE_EXCEPTION(EInvalidPredictorReset, AAC_INVALIDPREDICTORRESET, "Invalid Predictor Reset Group Indicated") ;
#endif

  DECLARE_EXCEPTION(EUnsupportedWindowShape, AAC_UNSUPPORTEDWINDOWSHAPE, "Unsupported Window Shape Used") ;
  DECLARE_EXCEPTION(EIllegalProfile, AAC_ILLEGAL_PROFILE, "Illegal Profile") ;

  // // //

  typedef struct
  {
    int	       SamplingFrequency ;
    int	       NumberOfScaleFactorBands_Long ;
    const int *ScaleFactorBands_Long ;
    int	       NumberOfScaleFactorBands_Short ;
    const int *ScaleFactorBands_Short ;
    int        MaximumPredictionBands ;
  } SamplingRateInfo ;

  static const SamplingRateInfo m_SamplingRateInfoTable [16] ;

} ;

#endif
