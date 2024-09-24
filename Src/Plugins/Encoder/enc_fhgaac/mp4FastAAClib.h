/*************************  Fast MPEG AAC Audio Encoder  **********************

                     (C) Copyright Fraunhofer IIS (2004-2010)
                                All Rights Reserved

   $Id: mp4FastAAClib.h,v 1.8 2013/10/29 00:56:15 dromagod Exp $
   Initial author:       M. Schug / A. Groeschel
   contents/description: Fast MPEG AAC Encoder Interface Library Functions

   This software and/or program is protected by copyright law and international 
   treaties. Any reproduction or distribution of this software and/or program, 
   or any portion of it, may result in severe civil and criminal penalties, and 
   will be prosecuted to the maximum extent possible under law.

******************************************************************************/

#ifndef _mp4FastAAClib_h_
#define _mp4FastAAClib_h_

#ifdef __cplusplus
extern "C"
{
#endif

/* ------------------------ structure alignment ---------------------------*/

#if defined(WIN32) || defined(WIN64)
#pragma pack(push, 1)
#endif

/*-------------------------- defines --------------------------------------*/

/*
 * calling convention
 */

#ifndef MPEG4ENCAPI
#if defined(WIN32) || defined(WIN64)
#define MPEG4ENCAPI __stdcall
#else
#define MPEG4ENCAPI
#endif
#endif

/*-------------------- enum definitions -----------------------------------*/

typedef enum 
{
  AUD_OBJ_TYP_LC              = 2,   /* AAC LC                                          */
  AUD_OBJ_TYP_LTP             = 4,   /* AAC LTP                                         */
  AUD_OBJ_TYP_HEAAC           = 5,   /* AAC LC + SBR                                    */
  AUD_OBJ_TYP_ER_LC           = 17,  /* ER AAC LC                                       */
  AUD_OBJ_TYP_ER_LTP          = 19,  /* ER AAC LTP                                      */
  AUD_OBJ_TYP_ER_SCAL         = 20,  /* ER AAC LC scalable                              */
  AUD_OBJ_TYP_PS              = 29,  /* AAC LC + SBR + PS                               */
  AUD_OBJ_TYP_MP2_LC          = 129, /* virtual AOT MP2 Low Complexity Profile          */
  AUD_OBJ_TYP_MP2_SBR         = 132, /* virtual AOT MP2 Low Complexity Profile with SBR */
  AUD_OBJ_TYP_SBR_DS          = 133, /* virtual AOT for downsampled SBR                 */
  AUD_OBJ_TYP_ER_SCAL_SBR     = 148, /* ER AAC LC scalable + SBR                        */
  AUD_OBJ_TYP_ER_SCAL_SBR_PS  = 157,  /* ER AAC LC scalable + SBR + PS                  */
  AUD_OBJ_TYP_MPS             =  30
} AUD_OBJ_TYP;

typedef enum {
  MP4_QUAL_FAST=0,
  MP4_QUAL_MEDIUM,
  MP4_QUAL_HIGH,
  MP4_QUAL_HIGHEST      /* Always resample to preferred sample rate */
} MPEG4ENC_QUALITY;

typedef enum {
  MP4_TT_RAW        = 0,
  MP4_TT_ADIF       = 1,
  MP4_TT_ADTS       = 2,
  MP4_TT_ADTSCRC    = 3,
  MP4_TT_LOAS       = 4,
  MP4_TT_LOAS_NOSMC = 5,
  MP4_TT_LATM       = 6,
  MP4_TT_LATM_NOSMC = 7,
  /* MP4_TT_LOAS_CRC = 8, */
  /* MP4_TT_LATM_CRC = 9, */
  ___mp4_tt_dummy
} MPEG4ENC_TRANSPORT_TYPE;

typedef enum {
  /*  These are the standard MPEG Channel mappings */
  MP4_CH_MODE_INVALID = 0,
  MP4_CH_MODE_MONO,         /* 1   channel mono   */
  MP4_CH_MODE_STEREO,       /* 2   channel stereo */
  MP4_CH_MODE_3,            /* 3   channel audio ( center + left/right front                       speaker       ) */
  MP4_CH_MODE_4,            /* 4   channel audio ( center + left/right front +     rear   surround speaker       ) */
  MP4_CH_MODE_5,            /* 5   channel audio ( center + left/right front + left/right surround speaker       ) */
  MP4_CH_MODE_5_1,          /* 5.1 channel audio ( center + left/right front + left/right surround speaker + LFE ) */
  MP4_CH_MODE_7_1,          /* 7.1 channel audio ( center + left/right front 
                                                      +  left/right outside front + left/right surround speaker + LFE ) */
  /* Channel mappings 8 to 15 are reserved */
  MP4_CH_MODE_6_1               = 11, /* 6.1 channel audio ( center + front left/right + surround left/right + rear surround center     + LFE ) */
  MP4_CH_MODE_7_1_REAR_SURROUND = 12, /* 7.1 channel audio ( center + front left/right + surround left/right + rear surround left/right + LFE ) */
  MP4_CH_MODE_7_1_TOP_FRONT     = 14, /* 7.1 channel audio ( center + front left/right + surround left/right + LFE  + + TOP front left/right  ) */

  /* Some non standard channel mappings */
  MP4_CH_MODE_DUAL_MONO = 16, /*  2 independent channels */
  MP4_CH_MODE_4TIMES1,        /*  4 independent channels */
  MP4_CH_MODE_6TIMES1,        /*  6 independent channels */
  MP4_CH_MODE_8TIMES1,        /*  8 independent channels */
  MP4_CH_MODE_12TIMES1,       /* 12 independent channels */
  MP4_CH_MODE_16TIMES1,
  MP4_CH_MODE_2TIMES2,        /*  2 stereo channel pairs */
  MP4_CH_MODE_3TIMES2,        /*  3 stereo channel pairs */
  MP4_CH_MODE_4TIMES2,        /*  4 stereo channel pairs */
  MP4_CH_MODE_6TIMES2,        /*  6 stereo channel pairs */

  MP4_CH_MODE_7_1_SIDE_CHANNEL = 32, /* 7.1 channel audio ( center + left/right front
                                                            + left/right side channels + left/right surround speakers + LFE ) */
  MP4_CH_MODE_7_1_FRONT_CENTER,      /* 7.1 channel audio ( center + left/right front
                                                            + left/right frontal center speakers + left/right surround speakers + LFE ) */

  /* Channel mapping for parametric stereo 
     (only works with AUD_OBJ_TYP_HEAAC, AUD_OBJ_TYP_PS)   */
  MP4_CH_MODE_PARAMETRIC_STEREO = 64,  /* 2 channel stereo input, transmit 1 channel mono + SBR + PS */
  MP4_CH_MODE_MPEGS_5x5         = 128, /* 6 channel input, transmit 1/2 channel(s) (+ SBR) + MPEGS Payload */
#ifdef SUPPORT_UPMIX
  MP4_CH_MODE_MPEGS_SXPRO_UPMIX,       /* 2 channel input, sxPro Upmix,  transmit 2 channel(s) (+ SBR) + MPEGS Payload */
#endif
#ifdef SUPPORT_MPS_7_X_7
  MP4_CH_MODE_MPEGS_7x7_REAR_SURROUND, /* 8 channel input (5.1 + left/right side speakers), transmit 2 channel(s) (+ SBR) + MPEGS Payload */ 
  /* 7.1 front center channel mapping is not yet supported! */
  MP4_CH_MODE_MPEGS_7x7_FRONT_CENTER,  /* 8 channel input (5.1 + left/right frontal center speakers), transmit 2 channel(s) (+ SBR) + MPEGS Payload */
#ifdef SUPPORT_MPS_7_5_7
  MP4_CH_MODE_MPEGS_757_FRONT_CENTER,  /* 8 channel input (5.1 + left/right frontal center speakers), transmit 5 channel(s) (+ SBR) + MPEGS Payload */
  MP4_CH_MODE_MPEGS_757_REAR_SURROUND, /* 8 channel input (5.1 + left/right side speakers), transmit 5 channel(s) (+ SBR) + MPEGS Payload */
#endif /* SUPPORT_MPS_7_5_7 */
#endif /* SUPPORT_MPS_7_X_7 */

  /* The following channel mappings are not yet supported! */
  MP4_CH_MODE_MPEGS_5x5_BLIND,
  MP4_CH_MODE_MPEGS_ARBITRARY_DOWNMIX_MONO,  /* 7 channel input, transmit 1 channel (+ SBR) + MPEGS Payload */
  MP4_CH_MODE_MPEGS_ARBITRARY_DOWNMIX_STEREO /* 8 channel input, transmit 2 channel(s) (+ SBR) + MPEGS Payload */

} MPEG4ENC_CH_MODE;

typedef enum {

  MP4_MPEGS_DOWNMIX_DEFAULT = 0, 
  /* The following config (FORCE_STEREO) is not yet supported! */
  MP4_MPEGS_DOWNMIX_FORCE_STEREO,
  MP4_MPEGS_DOWNMIX_MATRIX_COMPAT,
  /* The following configs are not yet supported! */
  MP4_MPEGS_DOWNMIX_ARBITRARY_MONO,
  MP4_MPEGS_DOWNMIX_ARBITRARY_STEREO
#ifdef SUPPORT_MPS_7_5_7
  , MP4_MPEGS_DOWNMIX_51
#endif /* SUPPORT_MPS_7_5_7 */

} MPEG4ENC_MPEGS_DOWNMIX_CONFIG;


typedef enum {
  MPEG4ENC_NO_ERROR = 0,
  MPEG4ENC_UNKNOWN_ERROR, 
  MPEG4ENC_PARAM_ERROR,
  MPEG4ENC_NOTIMPLEMENTED_ERROR,
  MPEG4ENC_MEMORY_ERROR,
  MPEG4ENC_INIT_ERROR,
  MPEG4ENC_FATAL_ERROR,
  MPEG4ENC_STACK_ALIGNMENT_ERROR,
  MPEG4ENC_METADATA_ERROR,
  MPEG4ENC_AOT_NOT_SUPPORTED = 64,
  MPEG4ENC_CHMODE_NOT_SUPPORTED,
  MPEG4ENC_BRMODE_NOT_SUPPORTED,
  MPEG4ENC_WARNING_MIN = 128,
  MPEG4ENC_WARNING_STACK_ALIGNMENT = MPEG4ENC_WARNING_MIN,
  MPEG4ENC_WARNING_METADATA,
  MPEG4ENC_WARNING_NOSYNC_TRIGGERED
} MPEG4ENC_ERROR;

typedef enum {
  MP4_SBRSIG_IMPLICIT  = 0, /* implicit signaling  (signaling 1) */
  MP4_SBRSIG_EXPL_BC   = 1, /* explicit backward compatible signaling (signaling 2.B.) */
  MP4_SBRSIG_EXPL_HIER = 2  /* explicit hierarchical signaling (signaling 2.A.) */
} MPEG4ENC_SIGNALING_MODE;

typedef enum {
  MP4_MPEGS_PAYLOAD_EMBED        = 0,   /* in case of MPEG-4 transportation, embed payload into AAC payload */
  MP4_MPEGS_NO_PAYLOAD_EMBED     = 1,   /* in case of MPEG-4 transportation, do *not* embed payload into AAC payload, but transport payload in extra stream */
  MP4_MPEGS_PAYLOAD_EMBED_ASCEXT = 2    /* M16117 */
} MPEG4ENC_MPEGS_PAYLOAD_MODE;

typedef enum {
  MP4_BR_MODE_CBR = 0, 
  MP4_BR_MODE_VBR_1   = 1,
  MP4_BR_MODE_VBR_2   = 2,
  MP4_BR_MODE_VBR_3   = 3,
  MP4_BR_MODE_VBR_4   = 4,
  MP4_BR_MODE_VBR_5   = 5,
  MP4_BR_MODE_VBR_6   = 6,
  MP4_BR_MODE_SFR     = 7, /* Superframing */
  MP4_BR_MODE_DABPLUS = 8, /* Superframing + DAB+ constraints */
  MP4_BR_MODE_DRMPLUS = 9, /* Superframing + DRM+ constraints */
  MP4_BR_MODE_DMB     = 10
} MPEG4ENC_BITRATE_MODE; 

typedef enum{
  MP4_GRANULE_960 = 960,
  MP4_GRANULE_1024 = 1024
} MPEG4ENC_GRANULE_LEN;

typedef enum {
  MP4_METADATA_NONE         = 0, /* do not embed any metadata */
  MP4_METADATA_MPEG,             /* embed MPEG defined metadata only */
  MP4_METADATA_MPEG_ETSI         /* embed all metadata */
} MPEG4ENC_METADATA_MODE;

typedef enum {
  MP4_METADATA_DRC_NONE          = 0,
  MP4_METADATA_DRC_FILMSTANDARD,
  MP4_METADATA_DRC_FILMLIGHT,
  MP4_METADATA_DRC_MUSICSTANDARD,
  MP4_METADATA_DRC_MUSICLIGHT,
  MP4_METADATA_DRC_SPEECH,
#ifdef SUPPORT_METADATA_DRC_MOBILE
  MP4_METADATA_DRC_MOBILE,
#endif
  MP4_METADATA_DRC_EMBED_EXTERN = -1,
  MP4_METADATA_DRC_NOT_PRESENT = -2
} MPEG4ENC_METADATA_DRC_PROFILE;

typedef enum {
  MPEG4ENC_METADATA_DMX_GAIN_0_dB    = 0,
  MPEG4ENC_METADATA_DMX_GAIN_1_5_dB  = 1,
  MPEG4ENC_METADATA_DMX_GAIN_3_dB    = 2,
  MPEG4ENC_METADATA_DMX_GAIN_4_5_dB  = 3,
  MPEG4ENC_METADATA_DMX_GAIN_6_dB    = 4,
  MPEG4ENC_METADATA_DMX_GAIN_7_5_dB  = 5,
  MPEG4ENC_METADATA_DMX_GAIN_9_dB    = 6,
  MPEG4ENC_METADATA_DMX_GAIN_INF     = 7,
} MPEG4ENC_METADATA_DMX_GAIN;

typedef enum {
  MP4_METADATA_DSUR_NOT_INDICATED  = 0, /* Dolby Surround mode not indicated */
  MP4_METADATA_DSUR_NOT_USED       = 1, /* 2-ch audio part is not Dolby surround encoded */
  MP4_METADATA_DSUR_IS_USED        = 2  /* 2-ch audio part is Dolby surround encoded */
} MPEG4ENC_METADATA_DSUR_IND;

typedef enum {                          /* see ETSI TS 101 154 V1.11.1, section C.5.2.2.3 and C.5.3 */
  MP4_METADATA_DRCPRESENTATION_NOT_INDICATED = 0,
  MP4_METADATA_DRCPRESENTATION_MODE_1 = 1,
  MP4_METADATA_DRCPRESENTATION_MODE_2 = 2
} MPEG4ENC_METADATA_DRCPRESENTATION;

typedef enum {
  MP4_MAX_ASC_SIZE = 64,
  MP4_MAX_SMC_SIZE = 256,
  MAX_DRC_BANDS = (1<<4),
  MP4_MAX_NUM_STREAMS =  2
} MPEG4ENC_DEFINES;   


typedef enum {
  MPEG4ENC_SYNCFRAME_STARTUP = 0,
  MPEG4ENC_SYNCFRAME_SWITCH,
  MPEG4ENC_SYNCFRAME_DASH
} MPEG4ENC_SYNCFRAME_TYPES;   

typedef enum {
  MP4_MPSDMXGAIN_INVALID =  -1,
  MP4_MPSDMXGAIN_0_dB    =   0,
  MP4_MPSDMXGAIN_1_5_dB  =   1,
  MP4_MPSDMXGAIN_3_dB    =   2,
  MP4_MPSDMXGAIN_4_5_dB  =   3,
  MP4_MPSDMXGAIN_6_dB    =   4,
  MP4_MPSDMXGAIN_7_5_dB  =   5,
  MP4_MPSDMXGAIN_9_dB    =   6,
  MP4_MPSDMXGAIN_12_dB   =   7
} MPEG4ENC_MPS_DMX_GAIN;

#ifdef SUPPORT_UPMIX
typedef enum {
  MP4_SXPRO_DEFAULT = 0,
  MP4_SXPRO_DRY,
  MP4_SXPRO_VIBRANT
} MP4_SXPRO_UPMIX_WORKMODE;

typedef enum {
  MP4_SXPRO_LFE_OFF = 0,
  MP4_SXPRO_LFE_ON
} MP4_SXPRO_UPMIX_LFE;
#endif

/*-------------------- structure definitions ------------------------------*/

typedef struct {
  AUD_OBJ_TYP               aot;
  int                       nBitRate;
  MPEG4ENC_BITRATE_MODE     bitrateMode;
  MPEG4ENC_QUALITY          quality;
  MPEG4ENC_CH_MODE          chMode;
  int                       nSampleRateIn;
  MPEG4ENC_TRANSPORT_TYPE   transportFormat;
  MPEG4ENC_SIGNALING_MODE   sbrSignaling;
  MPEG4ENC_GRANULE_LEN      nGranuleLength;
  MPEG4ENC_METADATA_MODE    metadataMode;
} MPEG4ENC_SETUP;

typedef enum{
  MP4_THREADING_MODE_SINGLE = 1,
  MP4_THREADING_MODE_MULTIPLE_BLOCKING,
  MP4_THREADING_MODE_MULTIPLE_NOBLOCKING
} MPEG4ENC_THREADING_MODE;


typedef MPEG4ENC_SETUP *HANDLE_MPEG4ENC_SETUP;

struct MPEG4ENC_ENCODER;
typedef struct MPEG4ENC_ENCODER * HANDLE_MPEG4ENC_ENCODER;

typedef struct
{
  int  nOutputStreams;                              /* number of output streams */
  int  nAccessUnitsPerStream[MP4_MAX_NUM_STREAMS];  /* number of AUs in bitstream buffer */
  int *pnAccessUnitOffset[MP4_MAX_NUM_STREAMS];     /* offset of AUs per stream, i.e. pnAccessUnitOffset[stream][numberAuPerStream] */
  int *pByteCnt[MP4_MAX_NUM_STREAMS];               /* lenght of each single AU in bitstream buffer */
  int *pIsSync[MP4_MAX_NUM_STREAMS];            /* flag, signaling if AU is self contained i.e. does not contain backward dependencies */
} MPEG4ENC_AUINFO;

typedef struct {
  int            nAscSizeBits;
  unsigned char  ascBuffer[MP4_MAX_ASC_SIZE];
} MPEG4ENC_ASCBUF;

typedef struct {
  int            nSmcSizeBits;
  unsigned char  smcBuffer[MP4_MAX_ASC_SIZE];
} MPEG4ENC_SMCBUF;

typedef struct 
{
  float fBandWidth;       /* audio bandwidth in Hz */
  int   nDelay;           /* encoder delay in units of sample frames */
  int   nDelayCore;       /* encoder delay in units of sample frames */
  int   nCbBufSizeMin;    /* minimum size of output buffer (bytes) */
  int   nSyncFrameDelay;

  int   nBitRate[MP4_MAX_NUM_STREAMS];
  int   nMaxBitRate[MP4_MAX_NUM_STREAMS];
  int   nBitResMax[MP4_MAX_NUM_STREAMS];
  int   nSamplingRate[MP4_MAX_NUM_STREAMS];
  int   nSamplesFrame[MP4_MAX_NUM_STREAMS];

  unsigned int   nAncBytesPerFrame;
  int   aot;
  int   nValidAsc;
  MPEG4ENC_ASCBUF ascBuf[MP4_MAX_NUM_STREAMS];
  MPEG4ENC_SMCBUF smcBuf;
  int   nProfLev;

  char  pVersion[50];
  char  pBuildDate[50];

} MPEG4ENC_INFO;

typedef struct MPEG4ENC_METADATA
{
  MPEG4ENC_METADATA_DRC_PROFILE drc_profile;  /* MPEG DRC compression profile */
  MPEG4ENC_METADATA_DRC_PROFILE comp_profile; /* ETSI heavy compression profile */

  float drc_TargetRefLevel;   /* used to define expected level to */
  float comp_TargetRefLevel;  /* adjust limiter to avoid overload */

  float drc_ext;  /* external feed DRC compression value */
  float comp_ext; /* external feed heavy compression value */

  int prog_ref_level_present;   /* flag, if prog_ref_level is present */
  float prog_ref_level;         /* Programme Reference Level = Dialogue Level: */
                                /*    -31.75dB .. 0 dB ; stepsize: 0.25dB      */

  int  PCE_mixdown_idx_present; /* flag, if dmx-idx should be written in programme config element */
  int  ETSI_DmxLvl_present;     /* flag, if dmx-lvl should be written in ETSI-ancData */
  MPEG4ENC_METADATA_DMX_GAIN  centerMixLevel;          /* center downmix level */
  MPEG4ENC_METADATA_DMX_GAIN  surroundMixLevel;        /* surround downmix level */

  MPEG4ENC_METADATA_DSUR_IND dolbySurroundMode;   /* Indication for Dolby Surround Encoding Mode */

  MPEG4ENC_METADATA_DRCPRESENTATION drcPresentationMode; /* DRC presentation mode (ETSI) */

  /* preprocessing */
  int dcFilter;             /* flag specifying if DC filtering is applied to input */
  int lfeLowpassFilter;     /* flag specifying if 120 Hz low-pass filter is applied to LFE channel */
  int surPhase90;           /* flag specifying if 90 degree phase shift is applied to surround channels */
  int surAtt3dB;            /* flag specifying if 3 dB attenuation is applied to surround channels */

} MPEG4ENC_METADATA;

typedef struct MPEG4ENC_EXTMETADATA
{
#if 1 /* ENABLE_ISO14496_3_2009_AMD4 */
  /* not fully supported yet */
  /* extended ancillary data */
  int pseudoSurroundEnable;     /* flag */
  int extAncDataEnable;         /* flag */
  int extDownmixLevelEnable;    /* flag */
  int extDownmixLevel_A;        /* downmix level index A (0...7, according to table) */
  int extDownmixLevel_B;        /* downmix level index B (0...7, according to table) */
  int dmxGainEnable;            /* flag */
  float dmxGain5;               /* gain factor for downmix to 5 channels */
  float dmxGain2;               /* gain factor for downmix to 2 channels */
  int lfeDmxEnable;             /* flag */
  int lfeDmxLevel;              /* downmix level index for LFE (0..15, according to table) */
#endif
} MPEG4ENC_EXTMETADATA;

typedef struct MPEG4ENC_METADATA *HANDLE_MPEG4ENC_METADATA;
typedef struct MPEG4ENC_EXTMETADATA *HANDLE_MPEG4ENC_EXTMETADATA;


/*-------------------- function prototypes --------------------------------*/


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_Configure
    description : fills encoder handle structure with default values
                  to be called before MPEG4ENC_Open                  
    returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_Configure (
                    HANDLE_MPEG4ENC_ENCODER     *phMp4Enc,  /* adress of encoder handle */
                    const HANDLE_MPEG4ENC_SETUP  hSetup     /* handle to filled setup stucture  */
                    );

/*---------------------------------------------------------------------------

    functionname:MPEG4ENC_GetVersionInfo
    description: get Version Number information about the encoding process 
    returns:     MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI
MPEG4ENC_GetVersionInfo(char *const pVersionInfo,
                        const int bufSize);

/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_Open
    description:  allocate and initialize a new encoder instance
                  samplesFirst holds the desired number of input
                  samples (of all channels) for the first frame
    returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

  ---------------------------------------------------------------------------*/

MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_Open(
              HANDLE_MPEG4ENC_ENCODER *phMp4Enc,     /* pointer to encoder handle, initialized on return */
              unsigned int* const      pSamplesFirst /* number of samples needed to encode the first frame */
              );

/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_Close
    description:  deallocate an encoder instance
    returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

  ---------------------------------------------------------------------------*/

MPEG4ENC_ERROR MPEG4ENCAPI
MPEG4ENC_Close (
                HANDLE_MPEG4ENC_ENCODER* phMp4Enc  /* pointer to an encoder handle */
                );

/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_Encode
    description:  encode the passed samples
    modifies:     pSamplesConsumed: number of used samples
                  pSamplesNext: number of samples needed to encode
                  the next frame
                  pOutputBytes: number of valid bytes in pOutput
    returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

  ---------------------------------------------------------------------------*/

MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_Encode(
                HANDLE_MPEG4ENC_ENCODER const hMp4Enc,          /* an encoder handle */
                const float* const            pSamples,         /* pointer to audio samples, interleaved*/
                const int                     nSamples,         /* number of samples
                                                                   must be a multiple of number of input channels */
                int*                          pSamplesConsumed, /* number of used input samples,
                                                                   will be a multiple of number of input channels */
                unsigned int* const           pSamplesNext,     /* number of desired samples for a complete frame */
                unsigned char* const          pOutput,          /* pointer to bitstream buffer */
                const int                     nOutputBufSize,   /* the size of the output buffer; 
                                                                   must be large enough to receive all data */
                int* const                    pOutputBytes,     /* number of bytes in bitstream buffer */
                MPEG4ENC_AUINFO             **ppAuInfo          /* */  
                );

/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_SRInfo
    description : returns the sample rate range and 
                  to be called before MPEG4ENC_Open                  
    returns:      MPEG4ENC_NO_ERROR on success,
                  MPEG4ENC_INIT_ERROR if the bitrate, channel, aot configuration 
                  is not supported

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SRInfo (

                 const int       bitRate,        /* the targeted bit rate            */
                 const MPEG4ENC_BITRATE_MODE  bitrateMode,    /* the bitrateMode */
                 const MPEG4ENC_CH_MODE       chMode,         /* the number of channels to encode */
                 const AUD_OBJ_TYP            aot,            /* the audio object type */
                 const MPEG4ENC_QUALITY       quality,        /* encoder quality       */
                       int *const             sampleRateMin,  /* lowest supported      */
                       int *const             sampleRateMax,  /* highest supported     */
                       int *const             sampleRatePref  /* preferred
                                                                 sampling frequency for the given 
                                                                 bitrate, channel, aot configuration */
                 );

/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_GetInfo
    description:  get information about the encoding process
    returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI
MPEG4ENC_GetInfo(const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                       MPEG4ENC_INFO * const   pInfo);

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetAncDataRate
  description:  Sets bitrate for ancillary data
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetAncDataRate(
                     HANDLE_MPEG4ENC_ENCODER hMp4Enc,                     
                     int                     nAncDataRate 
                     );

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetAncData
  description:  Passes ancillary data to encoder
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SetAncData(
                    HANDLE_MPEG4ENC_ENCODER    const hMp4Enc,    /* an encoder handle */
                    unsigned char*             pAncBytes,        /* ancillary data buffer */
                    unsigned int*              pNumAncBytes      /* ancillary data bytes left */
                    );

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetOffsets
  description:  changes mapping of input audio channels to AAC channels
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetOffsets(
                     const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                     const unsigned int            nChannels,
                     const unsigned int *const     channelOffset
                     );

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetSbrTransmissionConfig
  description:  changes signaling interval of SBR header, additional CRC bits 
                for SBR data
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetSbrTransmissionConfig(
                                  const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                                  const int                     bUseCRC,
                                  const float                   sendHeaderTimeInterval
                                  );


/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetInbandPceTimeInterval 
  description:  set update interval for explicit in band  PCE transmission 
                sendPceTimeInterval  > 0 -> regular time interval in seconds
                sendPceTimeInterval == 0 -> send no PCE (MPEG-2 only)
                sendPceTimeInterval  < 0 -> send PCE only the 1st frame
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetInbandPceTimeInterval(const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                                  const float                   sendPceTimeInterval);

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetAdtsPceTimeInterval 
  description:  set update interval for explicit channel signaling via PCE in
                case of ADTS transport stream, MPEG-2/4 AAC and 
                channel_configuration == 0
                sendPceTimeInterval  > 0 -> regular time interval in seconds
                sendPceTimeInterval == 0 -> send no PCE (MPEG-2 only)
                sendPceTimeInterval  < 0 -> send PCE only the 1st frame
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetAdtsPceTimeInterval(
				const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
				const float                   sendPceTimeInterval
				);

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_MpsSetSscTimeInterval 
  description:  set update interval for transmission of SpatialSpecificConfig
                (SSC) in case of encoding using MPEG Surround 
                sendSscTimeInterval  > 0 -> regular time interval in seconds
                sendSscTimeInterval == 0 -> send SSC every (MPEGS) frame
                sendSscTimeInterval  < 0 -> send SSC only the 1st frame
                - in combination with MPEGS only
                - MPEGS payload mode has to be MP4_MPEGS_PAYLOAD_EMBED, because 
                  otherwise the SSC is transmitted in a seperate ESD, which has 
                  to be handled by the user
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_MpsSetSscTimeInterval(
                               const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                               const float                   sendSscTimeInterval
                               );

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_MpsSetDownmixConfig
  description:  set MPEG Surround Downmix Configuration
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_MpsSetDownmixConfig(
                             const HANDLE_MPEG4ENC_ENCODER       hMp4Enc,
                             const MPEG4ENC_MPEGS_DOWNMIX_CONFIG mpegsDownmixCfg
                             );

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_MpsSetPayloadMode
  description:  set MPEG Surround Payload Transmission Mode 
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_MpsSetPayloadMode(
                           const HANDLE_MPEG4ENC_ENCODER       hMp4Enc,
                           const MPEG4ENC_MPEGS_PAYLOAD_MODE   mpegsPayloadMode
                           );

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetThreadingMode (deprecated)
  description:  sets threading mode to single threaded, multiple threaded
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

  Please note that this function is deprecated and should not be used any more.

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetThreadingMode(const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                          const MPEG4ENC_THREADING_MODE threadingMode);


/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_GetError
  description:  get error text
  returns:      pointer to an error text

 ------------------------------------------------------------------------------*/
char*  MPEG4ENCAPI 
MPEG4ENC_GetError(MPEG4ENC_ERROR error);

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetBandwidth
  description:  set bandwidth by user, returns with actual used bandwidth
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetBandwidth(const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                      const float proposedBandwidth,
                      float* usedBandwidth);

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetStereoPrePro
  description:  set bandwidth by user, returns with actual used bandwidth
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetStereoPrePro(const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                         const int enableStereoPrePro);


/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetLatmSmcTimeInterval 
  description:  set update interval for appearance of stream mux config in
                case of LOAS/LATM transport stream
                sendSmcTimeInterval  > 0 -> regular time interval (every n-th frame, default: 8)
                sendSmcTimeInterval == 0 -> send no inband StreamMuxConfig
                sendSmcTimeInterval  < 0 -> send StreamMuxConfig only the 1st frame
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetLatmSmcTimeInterval(
				const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
				const int                     sendSmcTimeInterval
				);

/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetLatmNrOfSubframes
  description:  set the nr of subframes per latm frame in
                case of LOAS/LATM transport stream
                nrOfSubframes < 1  -> reserved
                nrOfSubframes >= 1 -> use 'nrOfSubframes'

                optional, default is 1

  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetLatmNrOfSubframes(
                              const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                              const int                     nrOfSubframes
                              );


/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_GetLatmSmc
  description:  returns pointer to and size of LATM stream mux config

  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_GetLatmSmc(
                    const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                    unsigned char**               buffer,
                    int*                          nBits
                    );


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_Submit_Metadata
    description:  submit metadata
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_Submit_Metadata(
                         const HANDLE_MPEG4ENC_ENCODER  hMpeg4Enc,        /* an encoder handle */
                         const HANDLE_MPEG4ENC_METADATA pMetadata         /* pointer to metadata */
                        );
                        
                        
/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_Submit_ExtMetadata
    description:  submit metadata
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_Submit_ExtMetadata(
                         const HANDLE_MPEG4ENC_ENCODER  hMpeg4Enc,        /* an encoder handle */
                         const HANDLE_MPEG4ENC_EXTMETADATA pExtMetadata   /* pointer to extended metadata */
                        );



/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_BRInfo
    description : Provides the compatible bitrate range
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_BRInfo (
                 const AUD_OBJ_TYP        aot,
                 const MPEG4ENC_CH_MODE   chMode,
                 const int                samplingRate,
                 int*                     brMin,
                 int*                     brMax);


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_SetSbrSpeechConfig
    description : Sets SBR Speech config flag
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SetSbrSpeechConfig(
                            const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                            unsigned int                  flag
                           );


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_SetSbrTimeDiffCoding
    description : Sets SBR time differential coding (TDC); 
                    flag==0: Do not use TDC
                    flag==1: Use TDC
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SetSbrTimeDiffCoding(
                              const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                              unsigned int                  flag
                             );


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_SetUseIntensityStereo
    description : Sets intensity stereo coding (IS); 
                    flag==1: Use IS (default)
                    flag==0: Do not use IS
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SetUseIntensityStereo(
                               const HANDLE_MPEG4ENC_ENCODER hMp4Enc,
                               unsigned int                  flag
                              );


/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SendChCfgZero
  description:  will always use channel config zero + pce although a standard 
                channel config could be signalled
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SendChCfgZero(
                       const HANDLE_MPEG4ENC_ENCODER hMp4Enc
                       );



/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetSyncFrame
  description:  will generate a synchaable frame
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetSyncFrame(
                      const HANDLE_MPEG4ENC_ENCODER hMp4Enc
                      );


/*-----------------------------------------------------------------------------

  functionname: MPEG4ENC_SetSyncFrameWithType
  description:  will generate a synchaable frame
  returns:      MPEG4ENC_NO_ERROR on success, an appropriate error code else

 ------------------------------------------------------------------------------*/
MPEG4ENC_ERROR  MPEG4ENCAPI
MPEG4ENC_SetSyncFrameWithType(
                              const HANDLE_MPEG4ENC_ENCODER  hMp4Enc,
                              const MPEG4ENC_SYNCFRAME_TYPES syncType
                              );


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_InitDASH
    description : Configure encoder for DASH mode
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_InitDASH(
                  const HANDLE_MPEG4ENC_ENCODER   hMp4Enc
                  );


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_SetTransportType
    description : Reconfigure Transport Format
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SetTransportType(
                          const HANDLE_MPEG4ENC_ENCODER   hMp4Enc,
                          const MPEG4ENC_TRANSPORT_TYPE   transportType
                          );


/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_SetMPEG4Flag
    description : Reconfigure MPEG-2/4 compliance
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SetMPEG4Flag(
                      const HANDLE_MPEG4ENC_ENCODER   hMp4Enc,
                      const int                       mpeg4Flag
                      );


#ifdef SUPPORT_UPMIX
/*---------------------------------------------------------------------------

    functionname: MPEG4ENC_SetSXProUpmixParameter
    description : Sets SXPro parameters; 
                    umxMode: Upmix workmode
                    umxLFE:  Upmix LFE on/off
    returns:      MPEG4ENC_ERROR (error code)

  ---------------------------------------------------------------------------*/
MPEG4ENC_ERROR MPEG4ENCAPI 
MPEG4ENC_SetSXProUpmixParameter(
                                const HANDLE_MPEG4ENC_ENCODER   hMp4Enc,
                                const MP4_SXPRO_UPMIX_WORKMODE  umxMode,
                                const MP4_SXPRO_UPMIX_LFE       umxLFE
                               );
#endif


/*---------------------------------------------------------------------------*/
#if defined(WIN32) || defined(WIN64)
#pragma pack(pop)
#endif

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* _mp4FastAAClib_h_  */
