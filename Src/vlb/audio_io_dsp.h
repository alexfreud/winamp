/* $Header: /cvs/root/winamp/vlb/audio_io_dsp.h,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: audio_io_dsp.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Audio DSP processing header file
 *
\***************************************************************************/

#ifndef __AUDIO_IO_DSP_H__
#define __AUDIO_IO_DSP_H__

#define CLIPDLY					10
#define CLIPNORM				(1.0/CLIPDLY)
#define DLYBUFSZ				128
#define PMAXF					32767.0f
#define PMAXFM1					(PMAXF-1)
#define NMAXF					-32768.0f
#define MAXCHANNELS				2
#define NSAMPRATE				12
#define XLIMDELAY				5			// delay is 1/5 second before decay stage of clip delay return segment.
#define	NSCALETAB				33
#define PEAKLIMIT				(3.16*PMAXF) // Allows for 10 dB of smoothed limiting.  Any overshoot greater 
											 // than 10 dB is hard limited.
#define PEAKLIMITM1				(PEAKLIMIT-1)
#define INTERP_CORRECTION_FACT	0.013		// compensates for error between actual inverse and estimated inverse
											// in scaletab table.
#define CX_LIMSCALE			((NSCALETAB-1)/(PEAKLIMIT-PMAXF))

typedef struct {
	short index;
	short delay;
	short bufsize;
	float *bufptr;
} DLY_VARS;

typedef struct {
	short dcy_count;
	float dcy_samp;
} SMOOTH_VARS;


class AudioIODSP{
	private:
		DLY_VARS		**hsClipDly;
		SMOOTH_VARS		**hsSmoothVars;
		int 			iNChannels;
		bool			bHardClip;
		float			*pfDlyBuf;
		float			fPrevDlyBuf[MAXCHANNELS][CLIPDLY];
		float			fPrevClipScale[MAXCHANNELS];
		short			initLimitCnt;	// Amount of time to delay before decaying the limiter scaling value.  
		float			dcyDelta;		// Dcy_delta is the amount to attenuate the limiter scaling value 
										// on each call to CalcAtten().
		unsigned int	uiSampRate;
		float AudioIODSP::CalcAtten(SMOOTH_VARS *smoothvars, float fMaxSamp, float thresh);

	public:
		short			*pshOutBuf;

		AudioIODSP(int _iNChannels, unsigned int _uiSampRate = 0);
		~AudioIODSP();
		void Float_to_Short_Clip(float **fInput, short *sOutput, int iLength, int grpIndex);
		void Float_to_Int(float **fInput, int *iOutput, int iLength, int grpIndex, 
			int bitResolution);

		void ApplyLimiter(float **fBuffer, int iLength, int grpIndex);
		void SetSamplingRate(unsigned int _uiSampRate);
};


#endif /* __AUDIO_IO_DSP_H__ */

