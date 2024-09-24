/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (2001)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4AudioDecIfc.h,v 1.3 2012/05/08 20:16:49 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: interface to mpeg-4 audio decoder
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __MP4AUDIODECIFC_H__
#define __MP4AUDIODECIFC_H__


#include "mp4dec_helpers/err_code.h"

#include "mp4dec_asc/audiospecificconfig_c.h"
#include "mp4dec_helpers/usrparam.h"

#include "mp4dec_helpers/aubuffer_c.h"
#include "mp4dec_helpers/cubuffer_c.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(WIN32) || defined(WIN64)
  #pragma pack(push, 8)
#endif


/* data types */

typedef struct mp4AudioDecoder_VersionInfo {
  char dateTime[80];
  char versionNo[40];
  char options[1024];
  char options_ext[4096];
} mp4AudioDecoder_VersionInfo;


/* Opaque declaration of decoder handle  */
struct mp4AudioDecoder;
typedef struct mp4AudioDecoder* mp4AudioDecoderHandle;


/* mandatory decoder functions */

mp4AudioDecoderHandle MP4AUDIODECAPI mp4AudioDecoder_Create(
                                                            const struct CSAudioSpecificConfig * const asc[],
                                                            const unsigned int noOfLayers
                                                            );

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_SetParam(
                                                   const mp4AudioDecoderHandle self,
                                                   const unsigned int param,
                                                   const float value
                                                   );
  
MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_DecodeFrame(
                                                      mp4AudioDecoderHandle self, 
                                                      struct CAccessUnit* auBuffer[], 
                                                      struct CCompositionUnit* cuBuffer
                                                      ); 

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_Destroy(mp4AudioDecoderHandle* self);


/* utility functions */

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_Reset(
                                                mp4AudioDecoderHandle self,
                                                const unsigned int param,
                                                int value
                                                );
  
MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_Flush(
                                                mp4AudioDecoderHandle self,
                                                struct CCompositionUnit* cuBuffer
                                                );
  
MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_GetBufferFullness(
                                                            const mp4AudioDecoderHandle self, 
                                                            const unsigned int layer, 
                                                            unsigned int* bufferfullness
                                                            );

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_GetSamplesPerFrame(
                                                             const mp4AudioDecoderHandle self,
                                                             unsigned int* spf
                                                             );

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_SetOutputLayer(
                                                         const mp4AudioDecoderHandle self,
                                                         const unsigned int outputLayer
                                                         );

/* MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_SetSpeedPitch( */
/*                                                         const mp4AudioDecoderHandle self, */
/*                                                         const float speedChangeFactor, */
/*                                                         const float pitchChangeFactor */
/*                                                         ); */

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_GetLastError(const mp4AudioDecoderHandle self);
MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_GetLibraryVersion(mp4AudioDecoder_VersionInfo* versionInfo);

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_ascParse(
                                                   const unsigned char* decSpecificInfoBuf,
                                                   const unsigned int decSpecificInfoBuf_len,
                                                   struct CSAudioSpecificConfig* asc
                                                   );

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_ascParseStream(
                                                         const unsigned char* decSpecificInfoBuf,
                                                         struct CSAudioSpecificConfig* asc,
                                                         int* bitsRead
                                                         );

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_ascParseExt(
                                                      const unsigned char* const decSpecificInfoBuf[], 
                                                      const unsigned int   decSpecificInfoBuf_len[], 
                                                      const unsigned int   avgBitrate[],
                                                      const unsigned int   streams, 
                                                      unsigned int*  layers,                   /* out */
                                                      struct CSAudioSpecificConfig* asc[],     /* out */
                                                      unsigned int   streamsPerLayer[]         /* out */
                                                      );

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_ascPrint(
                                                   unsigned int                  layers,     /* in */
                                                   struct CSAudioSpecificConfig* asc[],      /* in */
                                                   unsigned int                  stringLen,  /* in */
                                                   unsigned char                 string[]    /* ptr in, content out */
                                                   );

MP4_RESULT MP4AUDIODECAPI mp4AudioDecoder_GetPcmWidth(
                                                      const unsigned char* const decSpecificInfoBuf[], 
                                                      const unsigned int   decSpecificInfoBuf_len[], 
                                                      const unsigned int   streams, 
                                                      unsigned int*        pcmwidth        /* out */
                                                      );


#if defined(WIN32) || defined(WIN64)
  #pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif  /*  __MP4AUDIODECIFC_H__   */
