/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2001)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_helpers/cubuffer_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: composition unit module public interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __CUBUFFER_C_H__
#define __CUBUFFER_C_H__

#include "mp4dec_helpers/err_code.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CUBUFFER_MAXCHANNELS = 100,
  CUBUFFER_MAXSAMPLESPERCHANNEL = 8192,
  CUBUFFER_MAXANCBYTES = 65536
} __cubuffer_constants;


typedef enum {
  CUBUFFER_PCMTYPE_FLOAT = 0x55,
  CUBUFFER_PCMTYPE_INT32,
  CUBUFFER_PCMTYPE_INT24,
  /* CUBUFFER_PCMTYPE_INT20, */
  CUBUFFER_PCMTYPE_INT16
  /* ,CUBUFFER_PCMTYPE_INT8 */
} CUBUFFER_PCMTYPE;

typedef enum {
  ANCDATA_IS_INVALID = 0x00,       /* default after calloc */

  ANCDATA_IS_COMPLETE_MP2FRAME,
  ANCDATA_IS_MP2ANCILLARYDATA,
  ANCDATA_IS_MP3ANCILLARYDATA,
  ANCDATA_IS_MP3SCF,
  ANCDATA_IS_MPEGS_AU,

  ANCDATA_IS_AAC_EXT_DRC,
  ANCDATA_IS_AAC_EXT_SAC,
  ANCDATA_IS_AAC_EXT_LDSAC,
  ANCDATA_IS_AAC_EXT_SAOC,
  ANCDATA_IS_AAC_EXT_SCESBR,
  ANCDATA_IS_AAC_EXT_CPESBR,
  ANCDATA_IS_AAC_EXT_SCESBRCRC,
  ANCDATA_IS_AAC_EXT_CPESBRCRC,
  ANCDATA_IS_AAC_ELD_SCESBR,
  ANCDATA_IS_AAC_ELD_CPESBR,
  ANCDATA_IS_AAC_ELD_SCESBRCRC,
  ANCDATA_IS_AAC_ELD_CPESBRCRC,
  ANCDATA_IS_AAC_EXT_FILLDATA,  /* is always 10100101 */
  ANCDATA_IS_AAC_EXT_DATAELEMENT_ANCDATA,
  ANCDATA_IS_AAC_EXT_DATAELEMENT_DEFAULT,
  ANCDATA_IS_AAC_EXT_FIL,
  ANCDATA_IS_AAC_EXT_DEFAULT,

  ANCDATA_IS_AAC_DSE_TAG0 = 0x80,
  ANCDATA_IS_AAC_DSE_TAG1,
  ANCDATA_IS_AAC_DSE_TAG2,
  ANCDATA_IS_AAC_DSE_TAG3,
  ANCDATA_IS_AAC_DSE_TAG4,
  ANCDATA_IS_AAC_DSE_TAG5,
  ANCDATA_IS_AAC_DSE_TAG6,
  ANCDATA_IS_AAC_DSE_TAG7,
  ANCDATA_IS_AAC_DSE_TAG8,
  ANCDATA_IS_AAC_DSE_TAG9,
  ANCDATA_IS_AAC_DSE_TAG10,
  ANCDATA_IS_AAC_DSE_TAG11,
  ANCDATA_IS_AAC_DSE_TAG12,
  ANCDATA_IS_AAC_DSE_TAG13,
  ANCDATA_IS_AAC_DSE_TAG14,
  ANCDATA_IS_AAC_DSE_TAG15,

  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG0 = 0x90,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG1,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG2,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG3,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG4,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG5,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG6,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG7,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG8,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG9,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG10,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG11,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG12,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG13,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG14,
  ANCDATA_IS_AAC_PCE_COMMENTFIELD_TAG15,

  ANCDATA_IS_INCOMPLETE = 0x0fd,
  BUFFER_IS_TOOSMALL_WITH_CLIPPED_DATA = 0xfe,
  BUFFER_IS_TOOSMALL_NOMORE_DATA = 0x0ff
} __cubuffer_ancDataBufTags;


typedef enum {
  CUBUFFER_CHANNEL_IS_INVALID = 0x0000,      /* default after init/calloc */      /* todo: we shouldn't name this INVALID, but DEFAULT or NOT_MAPPED or so ... */

  CUBUFFER_CHANNEL_IS_NOTMAPPED = 0x0001,    /* a valid pcm channel is here, but could not be mapped to a valid position */
  CUBUFFER_CHANNEL_IS_ELSEWHERE,             /* position is out of z-plane */

  CUBUFFER_CHANNEL_IS_CENTER = 0x0008,
  CUBUFFER_CHANNEL_IS_REARCENTER,

  CUBUFFER_CHANNEL_IS_LEFT0 = 0x0010,
  CUBUFFER_CHANNEL_IS_LEFT1,
  CUBUFFER_CHANNEL_IS_LEFT2,
  CUBUFFER_CHANNEL_IS_LEFT3,
  CUBUFFER_CHANNEL_IS_LEFT4,
  CUBUFFER_CHANNEL_IS_LEFT5,
  CUBUFFER_CHANNEL_IS_LEFT6,
  CUBUFFER_CHANNEL_IS_LEFT7,
  CUBUFFER_CHANNEL_IS_LEFT8,
  CUBUFFER_CHANNEL_IS_LEFT9,
  CUBUFFER_CHANNEL_IS_LEFT10,
  CUBUFFER_CHANNEL_IS_LEFT11,
  CUBUFFER_CHANNEL_IS_LEFT12,
  CUBUFFER_CHANNEL_IS_LEFT13,
  CUBUFFER_CHANNEL_IS_LEFT14,
  CUBUFFER_CHANNEL_IS_LEFT15,

  CUBUFFER_CHANNEL_IS_LEFTSIDE0 = 0x0020,
  CUBUFFER_CHANNEL_IS_LEFTSIDE1,
  CUBUFFER_CHANNEL_IS_LEFTSIDE2,
  CUBUFFER_CHANNEL_IS_LEFTSIDE3,
  CUBUFFER_CHANNEL_IS_LEFTSIDE4,
  CUBUFFER_CHANNEL_IS_LEFTSIDE5,
  CUBUFFER_CHANNEL_IS_LEFTSIDE6,
  CUBUFFER_CHANNEL_IS_LEFTSIDE7,
  CUBUFFER_CHANNEL_IS_LEFTSIDE8,
  CUBUFFER_CHANNEL_IS_LEFTSIDE9,
  CUBUFFER_CHANNEL_IS_LEFTSIDE10,
  CUBUFFER_CHANNEL_IS_LEFTSIDE11,
  CUBUFFER_CHANNEL_IS_LEFTSIDE12,
  CUBUFFER_CHANNEL_IS_LEFTSIDE13,
  CUBUFFER_CHANNEL_IS_LEFTSIDE14,
  CUBUFFER_CHANNEL_IS_LEFTSIDE15,

  CUBUFFER_CHANNEL_IS_LEFTBACK0 = 0x0030,
  CUBUFFER_CHANNEL_IS_LEFTBACK1,
  CUBUFFER_CHANNEL_IS_LEFTBACK2,
  CUBUFFER_CHANNEL_IS_LEFTBACK3,
  CUBUFFER_CHANNEL_IS_LEFTBACK4,
  CUBUFFER_CHANNEL_IS_LEFTBACK5,
  CUBUFFER_CHANNEL_IS_LEFTBACK6,
  CUBUFFER_CHANNEL_IS_LEFTBACK7,
  CUBUFFER_CHANNEL_IS_LEFTBACK8,
  CUBUFFER_CHANNEL_IS_LEFTBACK9,
  CUBUFFER_CHANNEL_IS_LEFTBACK10,
  CUBUFFER_CHANNEL_IS_LEFTBACK11,
  CUBUFFER_CHANNEL_IS_LEFTBACK12,
  CUBUFFER_CHANNEL_IS_LEFTBACK13,
  CUBUFFER_CHANNEL_IS_LEFTBACK14,
  CUBUFFER_CHANNEL_IS_LEFTBACK15,

  CUBUFFER_CHANNEL_IS_RIGHT0 = 0x0050,
  CUBUFFER_CHANNEL_IS_RIGHT1,
  CUBUFFER_CHANNEL_IS_RIGHT2,
  CUBUFFER_CHANNEL_IS_RIGHT3,
  CUBUFFER_CHANNEL_IS_RIGHT4,
  CUBUFFER_CHANNEL_IS_RIGHT5,
  CUBUFFER_CHANNEL_IS_RIGHT6,
  CUBUFFER_CHANNEL_IS_RIGHT7,
  CUBUFFER_CHANNEL_IS_RIGHT8,
  CUBUFFER_CHANNEL_IS_RIGHT9,
  CUBUFFER_CHANNEL_IS_RIGHT10,
  CUBUFFER_CHANNEL_IS_RIGHT11,
  CUBUFFER_CHANNEL_IS_RIGHT12,
  CUBUFFER_CHANNEL_IS_RIGHT13,
  CUBUFFER_CHANNEL_IS_RIGHT14,
  CUBUFFER_CHANNEL_IS_RIGHT15,

  CUBUFFER_CHANNEL_IS_RIGHTSIDE0 = 0x0060,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE1,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE2,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE3,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE4,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE5,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE6,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE7,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE8,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE9,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE10,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE11,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE12,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE13,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE14,
  CUBUFFER_CHANNEL_IS_RIGHTSIDE15,

  CUBUFFER_CHANNEL_IS_RIGHTBACK0 = 0x0070,
  CUBUFFER_CHANNEL_IS_RIGHTBACK1,
  CUBUFFER_CHANNEL_IS_RIGHTBACK2,
  CUBUFFER_CHANNEL_IS_RIGHTBACK3,
  CUBUFFER_CHANNEL_IS_RIGHTBACK4,
  CUBUFFER_CHANNEL_IS_RIGHTBACK5,
  CUBUFFER_CHANNEL_IS_RIGHTBACK6,
  CUBUFFER_CHANNEL_IS_RIGHTBACK7,
  CUBUFFER_CHANNEL_IS_RIGHTBACK8,
  CUBUFFER_CHANNEL_IS_RIGHTBACK9,
  CUBUFFER_CHANNEL_IS_RIGHTBACK10,
  CUBUFFER_CHANNEL_IS_RIGHTBACK11,
  CUBUFFER_CHANNEL_IS_RIGHTBACK12,
  CUBUFFER_CHANNEL_IS_RIGHTBACK13,
  CUBUFFER_CHANNEL_IS_RIGHTBACK14,
  CUBUFFER_CHANNEL_IS_RIGHTBACK15,

  CUBUFFER_CHANNEL_IS_SUB0 = 0x0090,
  CUBUFFER_CHANNEL_IS_SUB1,
  CUBUFFER_CHANNEL_IS_SUB2,
  CUBUFFER_CHANNEL_IS_SUB3,
  CUBUFFER_CHANNEL_IS_SUB4,
  CUBUFFER_CHANNEL_IS_SUB5,
  CUBUFFER_CHANNEL_IS_SUB6,
  CUBUFFER_CHANNEL_IS_SUB7,
  CUBUFFER_CHANNEL_IS_SUB8,
  CUBUFFER_CHANNEL_IS_SUB9,
  CUBUFFER_CHANNEL_IS_SUB10,
  CUBUFFER_CHANNEL_IS_SUB11,
  CUBUFFER_CHANNEL_IS_SUB12,
  CUBUFFER_CHANNEL_IS_SUB13,
  CUBUFFER_CHANNEL_IS_SUB14,
  CUBUFFER_CHANNEL_IS_SUB15,

  CUBUFFER_CHANNELMAPPING_LAST_ELEMENT
} __cubuffer_channelMappingTagTypes;


typedef enum {
  CUBUFFER_INVALID_VALUE = -1,

  __cubuffer_flags_start = 10000,
  CUBUFFER_VALIDCHANNELS,
  CUBUFFER_CHANNELSMAPPED,
  CUBUFFER_VALIDSAMPLES,
  CUBUFFER_SAMPLERATE,
  CUBUFFER_NROFLAYERS,
  CUBUFFER_CURRENTBITRATE,
  CUBUFFER_AVGBITRATE,
  CUBUFFER_DECODERDELAY,
  /* CUBUFFER_TIMESTAMP, */
  CUBUFFER_PCMTYPE_PROP,

  CUBUFFER_HAS_SBR,
  CUBUFFER_HAS_PS,
  CUBUFFER_HAS_EBCC,
  CUBUFFER_HAS_MPEG_SURROUND,
  CUBUFFER_HAS_BLINDUPMIX,
  /* CUBUFFER_HAS_IMPLICIT, */

  /* CUBUFFER_CONFIGCHANGED, */
  CUBUFFER_WAS_CONCEALED,
  CUBUFFER_TDL_GAINREDUCTION,
  CUBUFFER_IS_LOSSLESS,
  CUBUFFER_HAS_BEEP,
  CUBUFFER_HAS_SBRSYNCPOINT,


  /* special property: warning/message. All warnings and messages are or'd with CUBUFFER_MESSAGE */
  CUBUFFER_MESSAGE = 0x8000000,

  __cubuffer_flags_end
} __cubuffer_properties;



#if defined(WIN32) || defined(WIN64)
  #pragma pack(push, 8)
#endif


/* Opaque declaration of cubuffer handle */
struct CCompositionUnit;
typedef struct CCompositionUnit* CCompositionUnitPtr;


/* mandatory interface */
CCompositionUnitPtr MP4AUDIODECAPI CCompositionUnit_Create(
                                                           const unsigned int     channels,
                                                           const unsigned int     samplesPerChannel,
                                                           const unsigned int     samplingRate,
                                                           const unsigned int     sizeAncData,
                                                           const CUBUFFER_PCMTYPE cutype
                                                           );

CCompositionUnitPtr MP4AUDIODECAPI CCompositionUnit_CreateExt(
                                                              const unsigned int     channels,
                                                              const unsigned int     samplesPerChannel,
                                                              const unsigned int     samplingRate,
                                                              const unsigned int     sizeAncData,
                                                              const CUBUFFER_PCMTYPE pcmtype,
                                                              void*                  pcmBuf,
                                                              const unsigned int     pcmBufSize
                                                              );

MP4_RESULT MP4AUDIODECAPI CCompositionUnit_Destroy(CCompositionUnitPtr *self);
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_Reset(CCompositionUnitPtr self);

/* for signalling channel mapping of the (interleaved) pcm output channels */
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetChannelMapping(CCompositionUnitPtr self, const unsigned int channelNr, unsigned int* tag);
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetInvChannelMapping(CCompositionUnitPtr self, const unsigned int tagType, unsigned int* channel);

/* anc data handling */
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetAncDataCount(CCompositionUnitPtr self, unsigned int* nFields, unsigned int* nBytes);
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetAncDataByPos(CCompositionUnitPtr self, const unsigned int nr, unsigned char** ptr, unsigned int* size, unsigned int* tag);

/* retrieve cu-buffer parameters */
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetChannels(CCompositionUnitPtr self, unsigned int* nchannels);
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetSamplesPerChannel(CCompositionUnitPtr self, unsigned int* samplesPerChannel);
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetSamplingRate(CCompositionUnitPtr self, unsigned int* fs);
MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetPcmPtr(CCompositionUnitPtr self, void* buf);

MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetProperty(CCompositionUnitPtr self, const unsigned int prop, int* value);

MP4_RESULT MP4AUDIODECAPI CCompositionUnit_GetNextMessage(CCompositionUnitPtr self, int* value);


#if defined(WIN32) || defined(WIN64)
  #pragma pack(pop)
#endif


#ifdef __cplusplus
}
#endif

#endif /* __CUBUFFER_C_H__ */
