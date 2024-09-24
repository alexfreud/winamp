/* $Header: /cvs/root/winamp/vlb/audio_io.h,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: audio_io.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Audio i/o header file
 *
\***************************************************************************/

#include	<stdlib.h>

//Base Audio Device Errors:
#ifndef AUDIO_ERROR
#define AUDIO_ERROR

#define AUDIO_ERROR_NONE			0
#define AUDIO_ERROR_ENDOF			1
#define AUDIO_ERROR_NO_DEVICE		-1
#define AUDIO_ERROR_DEVICE_FAIL		-2

#endif
//Known Audio Formats:
#ifndef AUDIO_FORMAT
#define AUDIO_FORMAT

#define AUDIO_FORMAT_PRIVATE		0
#define AUDIO_FORMAT_WAV			1
#define AUDIO_FORMAT_AU				2
#define AUDIO_FORMAT_IMA_ADPCM		3
#define AUDIO_FORMAT_WAVMAPPING		4
#define AUDIO_FORMAT_DIRECTSOUND	5
#define AUDIO_FORMAT_CD_AUDIO		6

#define AUDIO_FORMAT_GENERIC_FILE	97
#define AUDIO_FORMAT_UNKNOWN		100

#endif

//Audio Format Info Structure:
#ifndef AUDIO_FORMAT_INFO
#define AUDIO_FORMAT_INFO

#define AUDIO_INPUT			0
#define AUDIO_OUTPUT		1

class AUDIO_FORMATINFO{
	public:
	unsigned int 	uiSampleRate;
	unsigned char	ucNChannels;
};

#endif

#ifndef AUDIO_IO_CONTROL
#define AUDIO_IO_CONTROL

class AudioIOControl{
	private:
		AUDIO_FORMATINFO 		*psFormatInfo;
	public:
		AudioIOControl();
		AudioIOControl(AudioIOControl&);
		AudioIOControl(int _iNChannels, unsigned int _uiSampleRate);
		~AudioIOControl();

		virtual AudioIOControl& operator=(AudioIOControl&);

		virtual int SetFormatInfo(AUDIO_FORMATINFO*);

		const AUDIO_FORMATINFO *GetFormatInfo() const;
		
		//virtual int IO(unsigned char**,int)=0;
		//virtual int IO(short**,int)=0;
		virtual int IO(float**,int)=0;

	protected:
		int		iError;
		short	*pshOutBuf;
};

#endif /* AUDIO_IO_CONTROL */



