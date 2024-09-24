/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_asc/audioobjecttypes.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   contents/description: the MPEG-4 Audio object types
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __AUDIOOBJECTTYPES_H__
#define __AUDIOOBJECTTYPES_H__

typedef enum _AUDIO_OBJECT_TYPE
{
  AOT_NULL_OBJECT      = 0,   /* Null Object (PCM or invalid)                           */
  AOT_AAC_MAIN         = 1,   /* AAC Main Object                                        */
  AOT_AAC_LC           = 2,   /* AAC LC Object                                          */
  AOT_AAC_SSR          = 3,   /* AAC SSR Object                                         */
  AOT_AAC_LTP          = 4,   /* AAC LTP Object                                         */
  AOT_SBR              = 5,   /* Meta: SBR, Spectral Band Replication                   */
  AOT_AAC_SCAL         = 6,   /* AAC Scalable Object                                    */
  AOT_TWIN_VQ          = 7,   /* TwinVQ Object                                          */
  AOT_CELP             = 8,   /* CELP Object                                            */
  AOT_HVXC             = 9,   /* HVXC Object                                            */
  AOT_RSVD_10          = 10,  /* (reserved)                                             */
  AOT_RSVD_11          = 11,  /* (reserved)                                             */
  AOT_TTSI             = 12,  /* TTSI Object                                            */
  AOT_MAIN_SYNTH       = 13,  /* Main Synthetic Object                                  */
  AOT_WAV_TAB_SYNTH    = 14,  /* Wavetable Synthesis Object                             */
  AOT_GEN_MIDI         = 15,  /* General MIDI Object                                    */
  AOT_ALG_SYNTH_AUD_FX = 16,  /* Algorithmic Synthesis and Audio FX Object              */
  AOT_ER_AAC_LC        = 17,  /* Error Resilient(ER) AAC LC Object                      */
  AOT_RSVD_18          = 18,  /* (reserved)                                             */
  AOT_ER_AAC_LTP       = 19,  /* Error Resilient(ER) AAC LTP Object                     */
  AOT_ER_AAC_SCAL      = 20,  /* Error Resilient(ER) AAC Scalable Object                */
  AOT_ER_TWIN_VQ       = 21,  /* Error Resilient(ER) TwinVQ Object                      */
  AOT_ER_BSAC          = 22,  /* Error Resilient(ER) BSAC Object                        */
  AOT_ER_AAC_LD        = 23,  /* Error Resilient(ER) AAC LD Object                      */
  AOT_ER_CELP          = 24,  /* Error Resilient(ER) CELP Object                        */
  AOT_ER_HVXC          = 25,  /* Error Resilient(ER) HVXC Object                        */
  AOT_ER_HILN          = 26,  /* Error Resilient(ER) HILN Object                        */
  AOT_ER_PARA          = 27,  /* Error Resilient(ER) Parametric Object                  */
  AOT_RSVD_28          = 28,  /* might become SSC                                       */
  AOT_PS               = 29,  /* Meta: PS, Parametric Stereo                            */
  AOT_MPEGS            = 30,  /* MPEG surround                                          */

  AOT_ESCAPE           = 31,  /* escape AOT for AOTs > 31                               */

  AOT_MP3ONMP4_L1      = 32,  /* MPEG-Layer-1 in MPEG-4                                 */
  AOT_MP3ONMP4_L2      = 33,  /* MPEG-Layer-2 in MPEG-4                                 */
  AOT_MP3ONMP4_L3      = 34,  /* MPEG-Layer-3 in MPEG-4                                 */
  AOT_RSVD_35          = 35,  /* DST                                                    */
  AOT_RSVD_36          = 36,  /* ALS                                                    */
  AOT_SLS              = 37,  /* SLS w/ core                                            */
  AOT_SLS_NC           = 38,  /* SLS non-core                                           */
  AOT_ER_AAC_ELD       = 39,  /* ER AAC LD with LD filterbank                           */

  AOT_RSVD_40          = 40,  /* reserved                                               */
  AOT_RSVD_41          = 41,  /* reserved                                               */
  AOT_RSVD_42          = 42,  /* reserved                                               */

  AOT_SAOC             = 43,  /* SAOC                                                   */
  AOT_LD_MPEGS         = 44,  /* Low Delay MPEG surround                                */

  AOT_LZM30            = 94, /* (reserved)                                              */
  AOT_LZM31            = 95, /* (reserved)                                              */

  AOT_MP2_AAC_MAIN     = 128, /* virtual AOT MP2 Main Profile                           */
  AOT_MP2_AAC_LC       = 129, /* virtual AOT MP2 Low Complexity Profile                 */
  AOT_MP2_AAC_SSR      = 130, /* virtual AOT MP2 Scalable Sampling Rate Profile         */

  AOT_PLAIN_MP1        = 131, /* virtual AOT for plain mp1 decoding                     */
  AOT_PLAIN_MP2        = 132, /* virtual AOT for plain mp2 decoding                     */
  AOT_PLAIN_MP3        = 133, /* virtual AOT for plain mp3 decoding                     */
  AOT_DAB              = 134, /* virtual AOT for DAB (Layer2 w/ scalefactor CRC)        */

  AOT_INVALID          = 555, /* dummy AOT for empty switch/case statements             */
  AOT_DUMMY            = 556  /* dummy AOT for empty switch/case statements             */

} AUDIO_OBJECT_TYPE;

#endif
