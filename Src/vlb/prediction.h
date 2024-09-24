/* $Header: /cvs/root/winamp/vlb/prediction.h,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

#ifdef MAIN_PROFILE

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: prediction.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: prediction tool
 *
\***************************************************************************/

#ifndef __PREDICTION_H__
#define __PREDICTION_H__

#define DWORDCAST(x) (*(unsigned int*)&(x))
#define FLOATCAST(x) (*(float*)&(x))

class CChannelInfo ;
class CBlock ;

/** Intra Channel Prediction.

    This class represents the MPEG-2 AAC Main Profile intra channel prediction
    tool. It holds the state information for a second order backward-adaptive
    lattice structure predictor for each spectral component, and can be applied
    to the spectral coefficients in the main profile only.

    Prediction is a profile-dependent tool and the CPrediction implementation
    follows the Read()/Apply() convention used for all tools.
*/

class CPrediction
{
public :

  CPrediction () ;
  ~CPrediction () ;

  void Apply (const CChannelInfo &info, CBlock &spectrum) ;

  void FullReset (void) ;

  enum
  {
    MaximumPredictedBins = 672,
    Order = 2 // Order of the backward adaptive lattice filter
  } ;

protected :

  class CQuantizedFloat
  {
    public :

      operator float () const
      {
        return FLOATCAST(m_Value) ;
      }

      CQuantizedFloat &operator=(float assigned)
      {
        m_Value = DWORDCAST(assigned) & 0xFFFF0000 ;
        return *this ;
      }

      float GetBDivVar (void) ;
      
    protected :

      unsigned int m_Value ;

      static const float m_Mantissa [128] ;
      static const float m_Exponent [256] ;

  } ;

  // State variables of the backward adaptive predictor
  // (according to lattice structure in IS)

  class CPredictionState
  {
    public :

      CPredictionState ()
      {
        Reset () ;
      }

      ~CPredictionState () {}

      float GetPredictedValue (void) ;
      void Update (float value) ;

      void Reset (void)
      {
        for (int i = 0 ; i < Order ; i++)
        {
          m_cor [i] = 0.0F ;
          m_var [i] = 1.0F ;
          m_r   [i] = 0.0F ;
        }
      }

    protected :

      float FloatRoundInfinity (float x) ;

      CQuantizedFloat m_r   [Order] ; // r coefficient
      CQuantizedFloat m_cor [Order] ; // correlation coefficient
      CQuantizedFloat m_var [Order] ; // variance coefficient

  } ;

  CPredictionState m_State [MaximumPredictedBins] ;

} ;

#endif
#endif
