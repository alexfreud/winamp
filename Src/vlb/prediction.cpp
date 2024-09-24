/* $Header: /cvs/root/winamp/vlb/prediction.cpp,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

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
 *   filename: prediction.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: prediction tool
 *
\***************************************************************************/

#include "prediction.h"
#include "channelinfo.h"
#include "block.h"

//////////////////////////////////////////////////////////////////////////////
// CQuantizedFloat

const float CPrediction::CQuantizedFloat::m_Mantissa [128] =
{
  0.953125F, 0.945313F, 0.937500F, 0.929688F, 0.925781F, 0.917969F, 0.910156F, 0.902344F, 
  0.898438F, 0.890625F, 0.882813F, 0.878906F, 0.871094F, 0.867188F, 0.859375F, 0.851563F, 
  0.847656F, 0.839844F, 0.835938F, 0.828125F, 0.824219F, 0.820313F, 0.812500F, 0.808594F, 
  0.800781F, 0.796875F, 0.792969F, 0.785156F, 0.781250F, 0.777344F, 0.773438F, 0.765625F, 
  0.761719F, 0.757813F, 0.753906F, 0.750000F, 0.742188F, 0.738281F, 0.734375F, 0.730469F, 
  0.726563F, 0.722656F, 0.718750F, 0.714844F, 0.710938F, 0.707031F, 0.699219F, 0.695313F, 
  0.691406F, 0.687500F, 0.683594F, 0.679688F, 0.679688F, 0.675781F, 0.671875F, 0.667969F, 
  0.664063F, 0.660156F, 0.656250F, 0.652344F, 0.648438F, 0.644531F, 0.640625F, 0.640625F, 
  0.636719F, 0.632813F, 0.628906F, 0.625000F, 0.621094F, 0.621094F, 0.617188F, 0.613281F, 
  0.609375F, 0.605469F, 0.605469F, 0.601563F, 0.597656F, 0.593750F, 0.593750F, 0.589844F, 
  0.585938F, 0.582031F, 0.582031F, 0.578125F, 0.574219F, 0.574219F, 0.570313F, 0.566406F, 
  0.566406F, 0.562500F, 0.558594F, 0.558594F, 0.554688F, 0.550781F, 0.550781F, 0.546875F, 
  0.542969F, 0.542969F, 0.539063F, 0.539063F, 0.535156F, 0.531250F, 0.531250F, 0.527344F, 
  0.527344F, 0.523438F, 0.519531F, 0.519531F, 0.515625F, 0.515625F, 0.511719F, 0.511719F, 
  0.507813F, 0.507813F, 0.503906F, 0.503906F, 0.500000F, 0.498047F, 0.496094F, 0.494141F, 
  0.492188F, 0.490234F, 0.488281F, 0.486328F, 0.484375F, 0.482422F, 0.480469F, 0.478516F
} ;

const float CPrediction::CQuantizedFloat::m_Exponent [256] =
{
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 0.000000e+000F, 
  5.000000e-001F, 2.500000e-001F, 1.250000e-001F, 6.250000e-002F, 3.125000e-002F, 1.562500e-002F, 7.812500e-003F, 3.906250e-003F, 
  1.953125e-003F, 9.765625e-004F, 4.882813e-004F, 2.441406e-004F, 1.220703e-004F, 6.103516e-005F, 3.051758e-005F, 1.525879e-005F, 
  7.629395e-006F, 3.814697e-006F, 1.907349e-006F, 9.536743e-007F, 4.768372e-007F, 2.384186e-007F, 1.192093e-007F, 5.960464e-008F, 
  2.980232e-008F, 1.490116e-008F, 7.450581e-009F, 3.725290e-009F, 1.862645e-009F, 9.313226e-010F, 4.656613e-010F, 2.328306e-010F, 
  1.164153e-010F, 5.820766e-011F, 2.910383e-011F, 1.455192e-011F, 7.275958e-012F, 3.637979e-012F, 1.818989e-012F, 9.094947e-013F, 
  4.547474e-013F, 2.273737e-013F, 1.136868e-013F, 5.684342e-014F, 2.842171e-014F, 1.421085e-014F, 7.105427e-015F, 3.552714e-015F, 
  1.776357e-015F, 8.881784e-016F, 4.440892e-016F, 2.220446e-016F, 1.110223e-016F, 5.551115e-017F, 2.775558e-017F, 1.387779e-017F, 
  6.938894e-018F, 3.469447e-018F, 1.734723e-018F, 8.673617e-019F, 4.336809e-019F, 2.168404e-019F, 1.084202e-019F, 5.421011e-020F, 
  2.710505e-020F, 1.355253e-020F, 6.776264e-021F, 3.388132e-021F, 1.694066e-021F, 8.470329e-022F, 4.235165e-022F, 2.117582e-022F, 
  1.058791e-022F, 5.293956e-023F, 2.646978e-023F, 1.323489e-023F, 6.617445e-024F, 3.308722e-024F, 1.654361e-024F, 8.271806e-025F, 
  4.135903e-025F, 2.067952e-025F, 1.033976e-025F, 5.169879e-026F, 2.584939e-026F, 1.292470e-026F, 6.462349e-027F, 3.231174e-027F, 
  1.615587e-027F, 8.077936e-028F, 4.038968e-028F, 2.019484e-028F, 1.009742e-028F, 5.048710e-029F, 2.524355e-029F, 1.262177e-029F, 
  6.310887e-030F, 3.155444e-030F, 1.577722e-030F, 7.888609e-031F, 3.944305e-031F, 1.972152e-031F, 9.860761e-032F, 4.930381e-032F, 
  2.465190e-032F, 1.232595e-032F, 6.162976e-033F, 3.081488e-033F, 1.540744e-033F, 7.703720e-034F, 3.851860e-034F, 1.925930e-034F, 
  9.629650e-035F, 4.814825e-035F, 2.407412e-035F, 1.203706e-035F, 6.018531e-036F, 3.009266e-036F, 1.504633e-036F, 7.523164e-037F, 
  3.761582e-037F, 1.880791e-037F, 9.403955e-038F, 4.701977e-038F, 2.350989e-038F, 1.175494e-038F, 5.877472e-039F, 0.000000e+000F
} ;

float CPrediction::CQuantizedFloat::GetBDivVar (void)
{
  // description:  compute division of B/VAR by means of two look up tables
  // returns:      single precision float (B/VAR)

  unsigned int mntInd = m_Value & 0x007F0000 ; // extract the 7-bit mantissa index 
  unsigned int expInd = m_Value & 0x7F800000 ; // extract the 8-bit exponent index

  return m_Mantissa [mntInd >> 16] * m_Exponent [expInd >> 23] ;
}

//////////////////////////////////////////////////////////////////////////////
// CPredictionState

float CPrediction::CPredictionState::FloatRoundInfinity (float x)
{
  // description:  rounds 32 bit single precision number to 16 bit precision 
  //               float towards infinity (non IEEE comform)
  // returns:      rounded float
  // input:        32 bit single precision float

  unsigned int t    = DWORDCAST(x) ;
  unsigned int flag = t & 0x8000 ;

  t &= 0xFFFF0000L ; // zero out bits below LSB
  x = FLOATCAST(t) ;

  // if the below-LSB part of the mantissa is 0.5 or above, round to inf

  if (flag)
  {
    // we need to add 1 LSB. However, since the mantissa always includes
    // a hidden MSB of 1, i.e. 2^e * 1, we need to subtract that afterwards.

    unsigned int c1 = t  & 0xFF800000L ; // 2^e * 1
    unsigned int c2 = c1 | 0x00010000L ; // LSB

    x -= FLOATCAST(c1);
    x += FLOATCAST(c2);
  }

  return x ;
}

float CPrediction::CPredictionState::GetPredictedValue (void)
{
  // description:  performs prediction for one MDCT bin
  // returns:      predicted value
  // input:        state variable handle, prediction buffer handle

  float k [CPrediction::Order] ;
  
  for (int i = 0 ; i < CPrediction::Order ; i++)
  {
    // compute (b / VAR) * COR

    k [i] = m_var [i].GetBDivVar () * m_cor [i] ;
  }

  // compute the estimate, round it to nearest infinity and store it

  return FloatRoundInfinity (k [0] * m_r [0] + k [1] * m_r [1]) ;
}

void CPrediction::CPredictionState::Update (float value)
{ 
  const float ALPHA = 0.90625F ;
  const float A = 0.953125F ;

  float e [CPrediction::Order] ;

  float k0 = m_cor [0] * m_var [0].GetBDivVar () ;

  e [0] = value ;
  e [1] = e [0] - k0 * m_r [0] ;

  for (int i = 0 ; i < CPrediction::Order ; i++)
  {
    float r = m_r [i] ;

    m_var [i] = ALPHA * m_var [i] + (0.5F) * (r * r + e [i] * e [i]) ;
    m_cor [i] = ALPHA * m_cor [i] + e [i] * r ;
  }

  m_r [1] = A * (m_r [0] - k0 * e [0]) ;
  m_r [0] = A * e [0] ;
}

//////////////////////////////////////////////////////////////////////////////
// CPrediction

CPrediction::CPrediction ()
{
}

CPrediction::~CPrediction ()
{
}

void CPrediction::FullReset (void)
{
  for (int i = 0 ; i < CPrediction::MaximumPredictedBins ; i++)
  {
    m_State [i].Reset () ;
  }
}

void CPrediction::Apply (const CChannelInfo &info, CBlock &spectrum)
{
  // short blocks

  if (info.IsShortBlock ())
  {
    FullReset () ;
    return ;
  }

  // long blocks

  const int *BandOffsets = info.GetScaleFactorBandOffsets () ;

  // apply predicted value to transmitted error value

  if (info.GetPredictorDataPresent ())
  {
    for (int band = 0 ; band < info.GetMaximumPredictionBands () ; band++)
    { 
      if (info.GetPredictionUsedForBand (band))
      {
        for (int index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
        {
          spectrum.AccessSpectralData () [index] += m_State [index].GetPredictedValue () ;
        }
      }
    }
  }

  // update state

  for (int band = 0 ; band < info.GetMaximumPredictionBands () ; band++)
  { 
    for (int index = BandOffsets [band] ; index < BandOffsets [band + 1] ; index++)
    {
      m_State [index].Update (spectrum.AccessSpectralData () [index]) ;
    }
  }

  // perform cyclic reset

  if (info.GetPerformPredictorReset ())
  {
    int group = info.GetPredictorResetGroupNumber () - 1 ;
    int maxbin = BandOffsets [info.GetMaximumPredictionBands ()] ;

    for (int index = group ; index < maxbin ; index += 30)
    {
      m_State [index].Reset () ;
    }
  }
}

#endif
