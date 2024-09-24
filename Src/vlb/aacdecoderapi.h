/* $Header: /cvs/root/winamp/vlb/aacdecoderapi.h,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: aacdecoderapi.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: AAC decoder API
 *
\***************************************************************************/

#ifndef __AACDECODERAPI_H__
#define __AACDECODERAPI_H__

#include <stdio.h>
#include "DataIO.h"
#include "audio_io.h"

/** AAC defines */
#define	AAC_MAX_PROGRAMS	16

#define ADTS		0
#define ADIF		1
#define RIFFADTS	2

/** API return value defines */
#define ERR_NO_ERROR			(0)

/* Fatal errors (must re-sync) */
#define ERR_INVALID_BITSTREAM	(1)
#define ERR_INVALID_ADIF_HEADER	(2)
#define ERR_SYNC_ERROR			(3)
#define ERR_SUBROUTINE_ERROR	(4)
#define ERR_END_OF_FILE			(5)
#define ERR_INVALID_ADTS_HEADER	(6)
#define ERR_INPUT_BUFFER_ERROR	(7)
#define ERR_OUTPUT_BUFFER_ERROR	(8)
#define ERR_ILLEGAL_PROFILE		(9)
#define ERR_ILLEGAL_SAMP_RATE	(10)
#define	ERR_ILLEGAL_DATA_RATE	(11)
#define ERR_ILLEGAL_CHANNELS	(12)
#define ERR_CANNOT_SYNC_TO_ADIF (13)
#define ERR_UNEXPECTED_ERROR	(14)
#define ERR_USER_ABORT			(15)
#define MAX_EXEC_ERRORS			(16)//Insert new errors before this

/* Warnings (keep decoding) */
#define WARN_CRC_FAILED			 (-1)
#define WARN_STREAM_PARAM_CHANGE (-2) 

/** structure defines for MPEG-2 AAC API Object */

typedef struct {
	long	sampling_frequency;
	long	num_channels;
	long	bitrate;
	short	stream_format;
	long	frame_length;
	short	protection_absent;
	short	copyright;
	short	original_copy;

} AACStreamParameters;


/* Forward declarations */
class CAacDecoder;
class CPlainFile;
class CDolbyBitBuffer;
class CStreamInfo;
class CAacException;



/** MPEG-2 AAC API Object.

    This is the API object that interfaces with the application code.
    It wraps the bitstream element objects and calls their individual decoding methods.
    To indicate unexpected events and error conditions that prevent correct operation,
    an exception derived from \Ref{CAacException} will be thrown that should be caught
    by the calling application's decoding loop.

*/

class CAacDecoderApi
{
	enum
	{
		cbSize = 2048
	};

	CAacDecoder	*decoder;
	CPlainFile	*input;
	CDolbyBitBuffer  *buffer;
	CStreamInfo	*info;
		
	unsigned char readbuf[cbSize];

	unsigned int	cbValid, cbActual;
	const static unsigned int sfTab[16];

	long TranslateException(CAacException& e);
	void VerifySync(void);
	bool FillStreamParameters(AACStreamParameters *paacStreamParameters);

public :

  /** Object Constructor.

      Instantiates a decoder object capable of decoding one stream of
      MPEG-2 AAC Audio Data.

      bs The \Ref{CBitStream} object that the decoder will read from.
  */

  CAacDecoderApi(DataIOControl *paacInput);

  /** Object Destructor.
  */

  ~CAacDecoderApi();



  /**	AAC API Synchronize Method
		Synchronizes to stream
  */	

  long Synchronize(AACStreamParameters *paacStreamParameters);


  /** AAC Decoding Method.

      This method delivers one frame of decoded pcm audio data.

      info		The configuration information.
  */

  long DecodeFrame (	AudioIOControl *ppcmOutput,
						AACStreamParameters *paacStreamParameters) ;

} ;

#endif
