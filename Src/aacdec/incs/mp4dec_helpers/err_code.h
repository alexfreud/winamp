/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_helpers/err_code.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: exported error codes
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __ERR_CODE_H__
#define __ERR_CODE_H__


typedef int MP4_RESULT;

#ifndef MP4AUDIODECAPI
  #if defined(WIN32) || defined(WIN64)
    #define MP4AUDIODECAPI __stdcall
  #else
    #define MP4AUDIODECAPI
  #endif
#endif


enum {
  MP4AUDIODEC_OK = 0,

  __exported_error_codes_start = -100,

  MP4AUDIODEC_NOT_OK = __exported_error_codes_start,
  MP4AUDIODEC_INVALID_HANDLE,
  MP4AUDIODEC_INVALID_POINTER,
  MP4AUDIODEC_INVALID_PARAMETER,
  MP4AUDIODEC_INVALID_VALUE,
  MP4AUDIODEC_INVALID_AUBUFFER,
  MP4AUDIODEC_INVALID_CUBUFFER,
  MP4AUDIODEC_INVALID_CHANNELCONFIG,
  MP4AUDIODEC_INVALID_NROFCHANNELS,
  MP4AUDIODEC_INVALID_SAMPLERATE,
  MP4AUDIODEC_INVALID_SAMPLESPERFRAME,
  MP4AUDIODEC_INVALID_EPCONFIG,
  MP4AUDIODEC_OUTOFMEMORY,
  MP4AUDIODEC_NOT_IMPLEMENTED,
  MP4AUDIODEC_AOT_NOT_SUPPORTED,
  MP4AUDIODEC_NOLAYERSTODECODE,
  MP4AUDIODEC_INVALID_FRAME_LENGTH,
  MP4AUDIODEC_TOOMANYLAYERS,
  MP4AUDIODEC_UNKNOWNERROR,

  MP4AUDIODEC_INVALID_CORECODER,
  MP4AUDIODEC_CELP_INVALIDCONFIG,

  MP4AUDIODEC_AUBUFFER_TOOMANYSUBFRAMES,
  MP4AUDIODEC_AUBUFFER_TOOSMALL,

  MP4AUDIODEC_CUBUFFER_TAGTYPE_NOTFOUND,
  MP4AUDIODEC_CUBUFFER_INVALIDPARAM,
  MP4AUDIODEC_CUBUFFER_NOMORE_ANCDATA,
  MP4AUDIODEC_CUBUFFER_PROPERTYNOTFOUND,
  MP4AUDIODEC_CUBUFFER_NOTENOUGHCHANNELS,
  MP4AUDIODEC_CUBUFFER_NOTENOUGHSAMPLES,
  MP4AUDIODEC_CUBUFFER_NOMORE_MESSAGES,
  MP4AUDIODEC_CUBUFFER_INVALIDPCMTYPE,

  MP4AUDIODEC_EP_PARSEERROR,
  MP4AUDIODEC_EP_PARSEERROR_DIRECTMAPPING,

  MP4AUDIODEC_SBRLIBERROR,
  MP4AUDIODEC_SBR_INVALID_ELEID,

  MP4AUDIODEC_MPEGS_ERROR,

  __exported_error_codes_end
};

#endif
