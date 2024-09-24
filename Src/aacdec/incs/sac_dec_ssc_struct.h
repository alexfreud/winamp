/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2007)
 *                        All Rights Reserved
 *
 *   $Id: sac_dec_ssc_struct.h,v 1.3 2012/05/08 20:16:49 audiodsp Exp $
 *   project : MPEG surround decoder lib
 *   contents/description: interface - spatial specific config struct
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __SAC_DEC_SSC_STRUCT_H__
#define __SAC_DEC_SSC_STRUCT_H__


#define MAX_NUM_QMF_BANDS (128)
#define MAX_TIME_SLOTS     (72)
#define MAX_INPUT_CHANNELS  (6)
#define MAX_OUTPUT_CHANNELS (8)
#define MAX_NUM_OTT  (5)
#define MAX_NUM_TTT  (1)
#define MAX_NUM_EXT_TYPES               ( 8 )
#define MAX_PARAMETER_BANDS   (28)

#define MAX_ARBITRARY_TREE_LEVELS       ( 2 )
#define MAX_OUTPUT_CHANNELS_AT          ( MAX_OUTPUT_CHANNELS * (1<<MAX_ARBITRARY_TREE_LEVELS) )
#define MAX_ARBITRARY_TREE_INDEX        ( (1<<(MAX_ARBITRARY_TREE_LEVELS+1))-1 )



typedef enum {

  SPATIALDEC_FREQ_RES_40 = 40, 
  SPATIALDEC_FREQ_RES_28 = 28, 
  SPATIALDEC_FREQ_RES_23 = 23,
  SPATIALDEC_FREQ_RES_20 = 20,
  SPATIALDEC_FREQ_RES_15 = 15,
  SPATIALDEC_FREQ_RES_14 = 14,
  SPATIALDEC_FREQ_RES_10 = 10,
  SPATIALDEC_FREQ_RES_7  = 7,
  SPATIALDEC_FREQ_RES_5  = 5,
  SPATIALDEC_FREQ_RES_4  = 4

} SPATIALDEC_FREQ_RES;

typedef enum {

  SPATIALDEC_QUANT_FINE_DEF = 0, 
  SPATIALDEC_QUANT_EDQ1 = 1, 
  SPATIALDEC_QUANT_EDQ2 = 2,
  SPATIALDEC_QUANT_RSVD3 = 3,
  SPATIALDEC_QUANT_RSVD4 = 4,
  SPATIALDEC_QUANT_RSVD5 = 5,
  SPATIALDEC_QUANT_RSVD6 = 6,
  SPATIALDEC_QUANT_RSVD7 = 7

} SPATIALDEC_QUANT_MODE;

typedef enum {

  SPATIALDEC_MODE_5151 = 0, 
  SPATIALDEC_MODE_5152 = 1,
  SPATIALDEC_MODE_525  = 2,
  SPATIALDEC_MODE_RSVD3 = 3, 
  SPATIALDEC_MODE_RSVD4 = 4,
  SPATIALDEC_MODE_RSVD5 = 5,
  SPATIALDEC_MODE_RSVD6 = 6,
  SPATIALDEC_MODE_RSVD7 = 7,
  SPATIALDEC_MODE_RSVD8 = 8,
  SPATIALDEC_MODE_RSVD9 = 9,
  SPATIALDEC_MODE_RSVD10 = 10,
  SPATIALDEC_MODE_RSVD11 = 11,
  SPATIALDEC_MODE_RSVD12 = 12,
  SPATIALDEC_MODE_RSVD13 = 13,
  SPATIALDEC_MODE_RSVD14 = 14,
  SPATIALDEC_MODE_SIGNAL

} SPATIALDEC_TREE_CONFIG;



typedef enum {

  SPATIALDEC_GAIN_MODE0 = 0,
  SPATIALDEC_GAIN_RSVD1 = 1,
  SPATIALDEC_GAIN_RSVD2 = 2,
  SPATIALDEC_GAIN_RSVD3 = 3,
  SPATIALDEC_GAIN_RSVD4 = 4,
  SPATIALDEC_GAIN_RSVD5 = 5,
  SPATIALDEC_GAIN_RSVD6 = 6,
  SPATIALDEC_GAIN_RSVD7 = 7,
  SPATIALDEC_GAIN_RSVD8 = 8,
  SPATIALDEC_GAIN_RSVD9 = 9,
  SPATIALDEC_GAIN_RSVD10 = 10,
  SPATIALDEC_GAIN_RSVD11 = 11,
  SPATIALDEC_GAIN_RSVD12 = 12,
  SPATIALDEC_GAIN_RSVD13 = 13,
  SPATIALDEC_GAIN_RSVD14 = 14,
  SPATIALDEC_GAIN_RSVD15 = 15

} SPATIALDEC_FIXED_GAINS;


typedef enum {

  SPATIALDEC_TS_TPNOWHITE = 0,
  SPATIALDEC_TS_TPWHITE = 1,
  SPATIALDEC_TS_TES = 2,
  SPATIALDEC_TS_NOTS = 3,
  SPATIALDEC_TS_RSVD4 = 4,
  SPATIALDEC_TS_RSVD5 = 5,
  SPATIALDEC_TS_RSVD6 = 6,
  SPATIALDEC_TS_RSVD7 = 7,
  SPATIALDEC_TS_RSVD8 = 8,
  SPATIALDEC_TS_RSVD9 = 9,
  SPATIALDEC_TS_RSVD10 = 10,
  SPATIALDEC_TS_RSVD11 = 11,
  SPATIALDEC_TS_RSVD12 = 12,
  SPATIALDEC_TS_RSVD13 = 13,
  SPATIALDEC_TS_RSVD14 = 14,
  SPATIALDEC_TS_RSVD15 = 15

} SPATIALDEC_TS_CONF;


typedef enum {

  SPATIALDEC_DECORR_MODE0 = 0,
  SPATIALDEC_DECORR_MODE1 = 1,
  SPATIALDEC_DECORR_MODE2 = 2,
  SPATIALDEC_DECORR_RSVD3 = 3,
  SPATIALDEC_DECORR_RSVD4 = 4,
  SPATIALDEC_DECORR_RSVD5 = 5,
  SPATIALDEC_DECORR_RSVD6 = 6,
  SPATIALDEC_DECORR_RSVD7 = 7,
  SPATIALDEC_DECORR_RSVD8 = 8,
  SPATIALDEC_DECORR_RSVD9 = 9,
  SPATIALDEC_DECORR_RSVD10 = 10,
  SPATIALDEC_DECORR_RSVD11 = 11,
  SPATIALDEC_DECORR_RSVD12 = 12,
  SPATIALDEC_DECORR_RSVD13 = 13,
  SPATIALDEC_DECORR_RSVD14 = 14,
  SPATIALDEC_DECORR_RSVD15 = 15

} SPATIALDEC_DECORR_CONF;


typedef struct T_SPATIALDEC_TREE_DESC {

  /* tbd */
  int tmp;

} SPATIALDEC_TREE_DESC;


typedef struct T_SPATIALDEC_OTT_CONF {

  int nOttBands;

} SPATIALDEC_OTT_CONF;


typedef enum {

  SPATIALDEC_TTT_PRED_DECORR   = 0,
  SPATIALDEC_TTT_PRED_NODECORR = 1,
  SPATIALDEC_TTT_RSVD2         = 2,
  SPATIALDEC_TTT_ENERGY_SUB    = 3,
  SPATIALDEC_TTT_RSVD4         = 4,
  SPATIALDEC_TTT_ENERGY_NOSUB  = 5,
  SPATIALDEC_TTT_RSVD6         = 6,
  SPATIALDEC_TTT_RSVD7         = 7

} SPATIALDEC_TTT_MODE;


typedef struct T_SPATIALDEC_TTT_CONF {

  int                 bTttDualMode;
  SPATIALDEC_TTT_MODE tttModeLow;
  SPATIALDEC_TTT_MODE tttModeHigh;
  int                 nTttBandsLow;

} SPATIALDEC_TTT_CONF;


typedef struct T_SPATIALDEC_RESIDUAL_CONF {

  int bResidualPresent;
  int nResidualBands;

} SPATIALDEC_RESIDUAL_CONF;


typedef struct T_SPATIAL_SPECIFIC_CONFIG {

  int samplingFreq;
  int nTimeSlots;
  int LdMode;
  SPATIALDEC_FREQ_RES       freqRes;
  SPATIALDEC_TREE_CONFIG    treeConfig;
  SPATIALDEC_QUANT_MODE     quantMode;
  int                       bOneIcc;
  int                       bArbitraryDownmix;
  
  int arbitraryDownmixResidualSamplingFreq;
  int arbitraryDownmixResidualFramesPerSpatialFrame;
  int arbitraryDownmixResidualBands;
 
  int                       bResidualCoding;
  SPATIALDEC_FIXED_GAINS    bsFixedGainSur;
  SPATIALDEC_FIXED_GAINS    bsFixedGainLFE;
  SPATIALDEC_FIXED_GAINS    bsFixedGainDMX;


  int                       bMatrixMode;
  SPATIALDEC_TS_CONF        tempShapeConfig;
  SPATIALDEC_DECORR_CONF    decorrConfig;
  SPATIALDEC_TREE_DESC     *pTreeDesc;
  
  int                       nInputChannels;   /* derived from  treeConfig */
  int                       nOutputChannels;  /* derived from  treeConfig */


  /* ott config */
  int                       nOttBoxes;        /* derived from  treeConfig */
  SPATIALDEC_OTT_CONF       OttConfig[MAX_NUM_OTT]; /* dimension nOttBoxes */

  /* ttt config */
  int                       nTttBoxes;       /* derived from  treeConfig */ 
  SPATIALDEC_TTT_CONF       TttConfig[MAX_NUM_TTT]; /* dimension nTttBoxes */

  /* residual config */
  int                       residualSamplingFreq;
  int                       nResidualFramesPerSpatialFrame;
  SPATIALDEC_RESIDUAL_CONF  ResidualConfig[MAX_NUM_OTT+MAX_NUM_TTT]; /* dimension (nOttBoxes + nTttBoxes) */

  int sacExtCnt;
  int sacExtType[MAX_NUM_EXT_TYPES];
  int envQuantMode;

  int bArbitraryTree;
  int numOutChanAT;
  int numOttBoxesAT;
  int OutputChannelPosAT[MAX_OUTPUT_CHANNELS_AT];
  int OttBoxPresentAT[MAX_OUTPUT_CHANNELS][MAX_ARBITRARY_TREE_INDEX];
  int OttDefaultCldAT[MAX_OUTPUT_CHANNELS*MAX_ARBITRARY_TREE_INDEX];
  int OttModeLfeAT[MAX_OUTPUT_CHANNELS*MAX_ARBITRARY_TREE_INDEX];
  int OttBandsAT[MAX_OUTPUT_CHANNELS*MAX_ARBITRARY_TREE_INDEX];

  int b3DaudioMode;
  int b3DaudioHRTFset;
  int HRTFfreqRes;
  int HRTFnumBand;
  int HRTFnumChan;
  int HRTFasymmetric;
  int HRTFlevelLeft[MAX_OUTPUT_CHANNELS][MAX_PARAMETER_BANDS];
  int HRTFlevelRight[MAX_OUTPUT_CHANNELS][MAX_PARAMETER_BANDS];
  int HRTFphase[MAX_OUTPUT_CHANNELS];
  int HRTFphaseLR[MAX_OUTPUT_CHANNELS][MAX_PARAMETER_BANDS];
  int HRTFicc[MAX_OUTPUT_CHANNELS];
  int HRTFiccLR[MAX_OUTPUT_CHANNELS][MAX_PARAMETER_BANDS];

} SPATIAL_SPECIFIC_CONFIG;

#endif
