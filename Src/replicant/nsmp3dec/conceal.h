/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: conceal.h
 *   project : ISO/MPEG-Decoder
 *   author  : Stefan Gewinner
 *   date    : 1998-05-26
 *   contents/description: error concealment class - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:02 $
 * $Id: conceal.h,v 1.1 2010/11/17 20:46:02 audiodsp Exp $
 */

#ifndef __CONCEAL_H__
#define __CONCEAL_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"

/*-------------------------------------------------------------------------*/

//
// Error concealment class.
//
//  This object is used to apply error concealment to a spectrum in case of
//  CRC errors. CRC protection is optional for ISO/MPEG bitstreams.
//

class CErrorConcealment
{
public:

  CErrorConcealment();
  ~CErrorConcealment() {}

  void Init();

  void Apply
    (
    bool             fApply, // true: restore, false: store
    const MPEG_INFO &Info,
    MP3SI           &Si,
    float           *lpSpec,
    int              gr,
    int              ch
    );

  enum { MAX_SPECTRUM_DATA = 4 };

protected :

  //
  // structure to hold information for one granule
  //
  typedef struct tagGRAN_DATA
    {
    MP3SI_GRCH gr;                  /* side info                       */
    float      Rs[SBLIMIT*SSLIMIT]; /* line amplitudes                 */
    float      nrg[23];             /* sf-band energies                */
    int        nrgValid;            /* valid-flag for sf-band energies */

#ifdef DEBUG_CONCEALMENT
    long      frameNumber;
#endif
    } GRAN_DATA;

  //
  // structure for concealment data
  //
  typedef struct tagSPECTRUM_DATA
    {
    int       writeOffset;             /* place to store next valid granule */
    GRAN_DATA gran[MAX_SPECTRUM_DATA]; /* ring buffer */
    GRAN_DATA estGran;
    } SPECTRUM_DATA;

  SPECTRUM_DATA SpecDataBuffer[2]; /* one buffer for each channel */

  void Store
    (
    const MPEG_INFO &Info,
    const MP3SI     &Si,
    const float     *lpSpec,
    int              gr,
    int              ch
    );

  void Restore
    (
    const MPEG_INFO &Info,
    MP3SI           &Si,
    float           *lpSpec,
    int              gr,
    int              ch
    );

#ifdef USE_ENERGY_PREDICTION
  float predict(const float *hist, const float *coff, int n);
  void  adaptPredictor(const float *hist, float pwr, float *coff, float d, int n);
#endif

  void estimateBandEnergies(const MPEG_INFO &Info, GRAN_DATA *g);
  void predictEnergies(const MPEG_INFO &Info, SPECTRUM_DATA *s);


  //
  // random seeds for the float and bit random generators
  //
  float ranHigh1(float a);
  float ranHigh2(float a);
  float ranLow(float a);
  float ran3(long *idum);
  int   irbit2(unsigned long *iseed);

  int  inext;
  int  inextp;
  long ma [56];
  int  iff ;

  long          f_seed, w_seed ;
  unsigned long b_seed ;

#ifdef DEBUG_CONCEALMENT
  long currentFrame ;
#endif
};

/*-------------------------------------------------------------------------*/
#endif
