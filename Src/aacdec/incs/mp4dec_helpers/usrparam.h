/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2001)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_helpers/usrparam.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: user changeable parameters, common struct
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __USERPARAM_H__
#define __USERPARAM_H__


typedef enum {
  MP4AUDIODECPARAM_DEFAULT = 0,
  MP4AUDIODECPARAM_ALL = MP4AUDIODECPARAM_DEFAULT
} __mp4AudioDecoder_ResetParam;


typedef enum {
  INVALID_PARAM = 0,

  __usrparam_param_start = 1000,
  /* postprocessor tools */
  DECODE_IMPLICIT,
  DECODE_SBR,
  DECODE_PS,
  DECODE_EBCC,
  DECODE_MPEGS,
  DECODE_DRC,

  /* mpeg surround params */
  MPEGS_UPMIX_TYPE,              /* for demo only */
  MPEGS_HRTF_MODEL,              /* for demo only */
  MPEGS_PART_COMPLEX,            /* for demo only */
  MPEGS_LEVEL,                   /* for demo only */
  MPEGS_BINAURAL_FRONT_ANGLE,    /* for demo only */
  MPEGS_BINAURAL_REAR_ANGLE,     /* for demo only */
  MPEGS_BINAURAL_DISTANCE,       /* for demo only */
  MPEGS_BINAURAL_DIALOG_CLARITY, /* for demo only */
  MPEGS_BINAURAL_QUALITY,        /* for demo only */
  MPEGS_BINAURAL_PRESET,

  /* DRC params */
  DRC_BOOST,
  DRC_COMPRESS,
  DRC_TARGET_REF,
  
  /* concealment params */
  CONCEALMENT_ENERGYINTERPOLATION,
  CONCEALMENT_TECHNIQUE,
  CONCEALMENT_ATTENUATION,

  /* time domain limiter */
  TDL_MODE,

  /* hvxc */
  HVXC_DELAYMODE,         /* for conformance test only */
  HVXC_TESTMODE,          /* for conformance test only */
  HVXC_PITCHFACTOR,       /* for conformance test only */
  HVXC_SPEEDFACTOR,       /* for conformance test only */

  /* sls */
  SLS_TRUNCATIONRATE,     /* max sls bitrate per channel to decode, for demo only */

  /* sbr */
  SBR_LOWPOWERMODE,       /* for demo only */

  /* scalable */
  SCAL_SETOUTPUTLAYER,

  /* advanced windowing - adjusts non-meaningful window sequence transitions */
  WINDOW_ADJUST_PARAM,

  /* PCM buffer re-shuffling */
  WAVE_REMAPPING,

  /* debugging only */
  VERBOSE_LEVEL,

  /* further params come here */

  __usrparam_param_dummy
} __mp4AudioDecoder_ConfigureParameters;


typedef enum {

  /* general values */
  SWITCH_OFF = 0,
  SWITCH_ON  = 1,
  SWITCH_DISABLED = 0,
  SWITCH_ENABLED  = 1,

  __usrparam_value_start = 10000,

  /** concealment settings **/

  /* preferred */
  CONCEALMENT_TECHNIQUE_NOISESUBST_FAST_STATIC_PRS = 7,

  /* for debugging and historic reasons */
  CONCEALMENT_TECHNIQUE_OFF = 0,
  CONCEALMENT_TECHNIQUE_MUTING = 1,
  CONCEALMENT_TECHNIQUE_REPETITION = 2,
  CONCEALMENT_TECHNIQUE_NOISESUBST_FAST = 3,
  CONCEALMENT_TECHNIQUE_NOISESUBST_IIR = 5,
  CONCEALMENT_TECHNIQUE_NOISESUBST_IIR_STATIC_PRS = 9,

  /* experimental - not recommended in production environments */
  CONCEALMENT_TECHNIQUE_NOISESUBST_FAST_PLUS_PRED = 4,
  CONCEALMENT_TECHNIQUE_NOISESUBST_IIR_PLUS_PRED = 6,
  CONCEALMENT_TECHNIQUE_NOISESUBST_FAST_STATIC_PRS_PLUS_PRED = 8,
  CONCEALMENT_TECHNIQUE_NOISESUBST_IIR_STATIC_PRS_PLUS_PRED = 10,

  /** advanced windowing **/
  WINDOW_ADJUST_PARAM_OFF = 0,
  WINDOW_ADJUST_PARAM_ON_ONLY_CURRENT = 1,
  WINDOW_ADJUST_PARAM_ON_PREF_LONG = 2,
  WINDOW_ADJUST_PARAM_ON_PREF_SHORT = 3,

  __usrparam_value_dummy
} __mp4AudioDecoder_ConfigureValues;


#define SLS_TRUNCATIONRATE_OFF             (-1.0f)
#define SLS_TRUNCATIONRATE_MIN             (32000.0f)
#define WINDOW_ADJUST_PARAM_DEFAULT        (WINDOW_ADJUST_PARAM_OFF)

#endif
