/* $Header: /cvs/root/winamp/vlb/audio_io_dsp.cpp,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: audio_io.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Audio I/O include file
 *
\***************************************************************************/

#include "audio_io_dsp.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

#define		CLIP_DLY		1

static double scaletab[NSCALETAB];

static const double invsampratetab[NSAMPRATE] = 
{	0.00001041667,
	0.00001133787,
	0.00001562500,
	0.00002083333, 
	0.00002267574, 
	0.00003125000,
	0.00004166667,
	0.00004535147,
	0.00006250000,
	0.00008333333, 
	0.00009070295, 
	0.00012500000
};

static short limitcnttab[NSAMPRATE] = 
{
	96000/(CLIPDLY * XLIMDELAY), 
	88200/(CLIPDLY * XLIMDELAY), 
	64000/(CLIPDLY * XLIMDELAY), 
	48000/(CLIPDLY * XLIMDELAY), 
	44100/(CLIPDLY * XLIMDELAY), 
	32000/(CLIPDLY * XLIMDELAY),
	24000/(CLIPDLY * XLIMDELAY), 
	22050/(CLIPDLY * XLIMDELAY), 
	16000/(CLIPDLY * XLIMDELAY),
	12000/(CLIPDLY * XLIMDELAY),
	11025/(CLIPDLY * XLIMDELAY), 
	8000/(CLIPDLY * XLIMDELAY)
};


AudioIODSP::AudioIODSP(int _iNChannels, unsigned int _uiSampRate)
:	iNChannels(_iNChannels), uiSampRate(_uiSampRate)
{
	int chan, i;
	double upper, lower, limdelta, upperlim;

	hsClipDly = new DLY_VARS*[iNChannels];
	hsSmoothVars = new SMOOTH_VARS*[iNChannels];
	for (chan = 0; chan < iNChannels; chan++)
	{
		hsClipDly[chan] = new DLY_VARS;
		hsClipDly[chan]->bufptr = new float[DLYBUFSZ];
		for (i = 0; i < DLYBUFSZ; i++)
		{
			hsClipDly[chan]->bufptr[i] = 0.0f;
		}
		hsClipDly[chan]->bufsize = DLYBUFSZ;
		hsClipDly[chan]->delay = CLIPDLY;
		hsClipDly[chan]->index = 0;

		hsSmoothVars[chan] = new SMOOTH_VARS;
		hsSmoothVars[chan]->dcy_count = 0;
		hsSmoothVars[chan]->dcy_samp = 0.0;
	}

	pshOutBuf = new short[iNChannels*DLYBUFSZ];
	for (i = 0; i < iNChannels*DLYBUFSZ; i++)
	{
		pshOutBuf[i] = 0;
	}
	pfDlyBuf = new float[DLYBUFSZ];

	bHardClip = false;

	for (chan = 0; chan < MAXCHANNELS; chan++)
	{
		fPrevClipScale[chan] = 1.0;
		for (i = 0; i < CLIPDLY; i++)
		{
			fPrevDlyBuf[chan][i] = 0.0;
		}
	}

	for (i = 0; i < DLYBUFSZ; i++)
	{
		pfDlyBuf[i] = 0.0;
	}

	upper = PEAKLIMIT;
	lower = PMAXF;
	limdelta = (upper - lower) / (NSCALETAB - 1);
	for (i = 0; i < NSCALETAB; i++)
	{	
		upperlim = lower + i * limdelta;
		scaletab[i] = (float)(lower / upperlim - INTERP_CORRECTION_FACT);
	}
}


AudioIODSP::~AudioIODSP()
{
	int chan;

	if (hsClipDly != NULL)
	{
		for (chan = 0; chan < iNChannels; chan++)
		{
			delete[] hsClipDly[chan]->bufptr;
			delete hsClipDly[chan];
		}
        delete [] hsClipDly;
	}

    if (hsSmoothVars != NULL)
    {
		for (chan = 0; chan < iNChannels; chan++)
		{
            delete hsSmoothVars[chan];
        }
        delete [] hsSmoothVars;
    }

	if (pshOutBuf != NULL)
	{
		delete[] pshOutBuf;
	}
	if (pfDlyBuf != NULL)
	{
		delete[] pfDlyBuf;
	}
}

void AudioIODSP::SetSamplingRate(unsigned int _uiSampRate)
{
	int		iSRateIndex;
	float	invSamp;

	this->uiSampRate = _uiSampRate;

	/* Set up limiter decay/delay params. */

	switch (uiSampRate)
	{
		case (8000):
			iSRateIndex = 0;
			break;
		case (11025):
			iSRateIndex = 1;
			break;
		case (12000):
			iSRateIndex = 2;
			break;
		case (16000):
			iSRateIndex = 3;
			break;
		case (22050):
			iSRateIndex = 4;
			break;
		case (24000):
			iSRateIndex = 5;
			break;
		case (32000):
			iSRateIndex = 6;
			break;
		case (44100):
			iSRateIndex = 7;
			break;
		case (48000):
			iSRateIndex = 8;
			break;
		case (64000):
			iSRateIndex = 9;
			break;
		case (88200):
			iSRateIndex = 10;
			break;
		case (96000):
			iSRateIndex = 11;
			break;
		default:
			bHardClip = true;		/* Non-standard sample rate.  Enable hard limiter in this case. */
	}

	invSamp = (float)invsampratetab[iSRateIndex];			/* inverse of samp rate*/
	initLimitCnt = limitcnttab[iSRateIndex];
	dcyDelta = (float)(1.0 - invSamp * 2.0 * CLIPDLY);
}

void AudioIODSP::Float_to_Int(float **fInput, int *iOutput, int iLength, int grpIndex, int bitResolution)
{
	short sTemp, outNdx, chan, samp;
	int iPWordLengthLim, iNWordLengthLim;
	float fSamp;
	short interleaveOut = iNChannels;
	
	for (chan = 0; chan < iNChannels; chan++)
	{
		outNdx = chan;
		for (samp = 0; samp < iLength; samp++)
		{
			fSamp = fInput[chan][grpIndex * iLength + samp];
			iPWordLengthLim = (1 << (bitResolution - 1)) - 1;
			iNWordLengthLim = -iPWordLengthLim - 1;

			fSamp=(fSamp > (float)iPWordLengthLim) ? (float)iPWordLengthLim : fSamp;
			fSamp=(fSamp < (float)iNWordLengthLim) ? (float)iNWordLengthLim : fSamp;		

	 		sTemp = (fSamp >= 0.0f) ? (short)(fSamp + 0.5f) : (short)(fSamp - 0.5f);
			iOutput[outNdx] = (int)(sTemp >> bitResolution);
			outNdx += interleaveOut;
		} // and samp loop
	} // end chan loop
}


void AudioIODSP::Float_to_Short_Clip(float **fInput, short *sOutput, int iLength, int grpIndex)
{
	short samp;
	short chan;
	short interleaveIn = 1;
	short interleaveOut = iNChannels;
	float fSamp;
	int outNdx;

#ifdef CLIP_DLY
	short dlysamp;
	float fClipScale;
	float limitAmount;
	float fLookAhead;
	float *pfDlyBufW = pfDlyBuf + CLIPDLY;
	float *pfDlyBuf_LookAhead, *fPrevDlyPtr;
	float currLimitAmount;
#endif
	
	assert(iLength >= CLIPDLY);

	for (chan = 0; chan < iNChannels; chan++)
	{

#ifdef CLIP_DLY
		fPrevDlyPtr = fPrevDlyBuf[chan];
		for (samp = 0; samp < CLIPDLY; samp++)
		{
			pfDlyBuf[samp] = fPrevDlyPtr[samp];
		}

/* !!!	The following line may cause a compile time warning or BoundsChecker error !!!
		This is a speed optimization and is intended to work this way. fPrevDlyPtr will point 
		to a location outside of the buffer. The  code that followswill not write to this location. */
		fPrevDlyPtr -= (iLength - CLIPDLY);		/* cue up the Prev buffer to allow samp to begin indexing
												   at position zero when samp = iLength - CLIPDLY */

		outNdx = chan;
		pfDlyBuf_LookAhead = fInput[chan] + grpIndex * iLength;
		for (samp = 0; samp < iLength; samp++)
		{
/* Call CalcAtten() to compute the amount of gain attenuation to apply to bring the current sample
   below the clip threshold.  Note that CalcAtten contains a delay and decay factor for each channel
   to govern transitioning out of limiting.  Thus even if the current sample does not clip, the value
   fClipScale could be < 1.0 due to the hysteresis of the limiter in CalcAtten. */

			fLookAhead = (float)fabs(fInput[chan][samp + grpIndex * iLength]);
			fClipScale = CalcAtten(hsSmoothVars[chan], fLookAhead, PMAXF);

/* Limit current sample to not clip, as determined by the return value from CalcAtten() */

			currLimitAmount = pfDlyBuf_LookAhead[samp] * fClipScale;
			(samp < iLength - CLIPDLY) ? (pfDlyBufW[samp] = currLimitAmount) :
										 (fPrevDlyPtr[samp] = currLimitAmount);
			limitAmount = fPrevClipScale[chan] - fClipScale;  /* limitAmount > 0:  going further into limiting */
															  /* limitAmount < 0:  coming out of limiting */

/* if difference between limit amount from previous sample, and current limit limit amount is non-zero,
   proceed back through previous samples to apply more (or less) clipping.  Delta clipping amount is 
   difference between current and previous clip amount.  Note that the delta clipping amount slopes 
   linearly to zero additional clipping as dlysamp advances towards CLIPDLY samples from the current sample.*/

			if (limitAmount > 0.0)
			{
				for (dlysamp = -1; dlysamp > -CLIPDLY; dlysamp--)
				{
					currLimitAmount = (float)(1.0 - limitAmount * (CLIPDLY + dlysamp) * CLIPNORM);
					(samp + dlysamp < iLength - CLIPDLY) ? (pfDlyBufW[samp + dlysamp] *= currLimitAmount) :
														   (fPrevDlyPtr[samp + dlysamp] *= currLimitAmount);
				}
			}

			fPrevClipScale[chan] = fClipScale;
		}

		for (samp = 0; samp < iLength; samp++)
		{
			fSamp = pfDlyBuf[samp];
			if (bHardClip)
			{
			   	fSamp=(fSamp > 32767.0f) ? 32767.0f : fSamp;
			   	fSamp=(fSamp < -32768.0f) ? -32768.0f : fSamp;
			}
			else
			{
				assert(fSamp <= 32767.0);
				assert(fSamp >= -32768.0);
			}
			sOutput[outNdx] = (fSamp >= 0.0f) ? (short)(fSamp + 0.5f) : (short)(fSamp - 0.5f);
			outNdx += interleaveOut;
		}

#else
		outNdx = chan;
		for (samp = 0; samp < iLength; samp++)
		{
			fSamp = fInput[chan][grpIndex * iLength + samp];
		   	fSamp=(fSamp>32767.0f)?32767.0f:fSamp;
		   	fSamp=(fSamp<-32768.0f)?-32768.0f:fSamp;
		   	sOutput[outNdx]=(fSamp>=0.0f)?(short)(fSamp+0.5f):(short)(fSamp-0.5f);
			outNdx += interleaveOut;
		} /* end samp loop */
#endif
	} /* end chan loop */
}


///////////////////////////////////////////////////////////////////
//  AudioIODSP::Limiter
//
//  Input:	fBuffer[chans][samples]	- data to be limited
//			iLength					- # of samples to be limited
//			grpIndex				- which chunk of data to limit	
//
//  Output: fBuffer[chans][samples] - limited data is written over 
//									  input data
///////////////////////////////////////////////////////////////////
void AudioIODSP::ApplyLimiter(float **fBuffer, int iLength, int grpIndex)
{
	short samp;
	short chan;
	float fSamp;

#ifdef CLIP_DLY
	short dlysamp;
	float fClipScale;
	float limitAmount;
	float fLookAhead;
	float *pfDlyBufW = pfDlyBuf + CLIPDLY;
	float *pfDlyBuf_LookAhead, *fPrevDlyPtr;
	float currLimitAmount;
#endif
	
	assert(iLength >= CLIPDLY);

	for (chan = 0; chan < iNChannels; chan++)
	{

#ifdef CLIP_DLY
		fPrevDlyPtr = fPrevDlyBuf[chan];
		for (samp = 0; samp < CLIPDLY; samp++)
		{
			pfDlyBuf[samp] = fPrevDlyPtr[samp];
		}

/* !!!	The following line may cause a compile time warning or BoundsChecker error !!!
		This is a speed optimization and is intended to work this way. fPrevDlyPtr will point 
		to a location outside of the buffer. The  code that followswill not write to this location. */
		fPrevDlyPtr -= (iLength - CLIPDLY);		/* cue up the Prev buffer to allow samp to begin indexing
												   at position zero when samp = iLength - CLIPDLY */

		pfDlyBuf_LookAhead = fBuffer[chan] + grpIndex * iLength;
		for (samp = 0; samp < iLength; samp++)
		{
/* Call CalcAtten() to compute the amount of gain attenuation to apply to bring the current sample
   below the clip threshold.  Note that CalcAtten contains a delay and decay factor for each channel
   to govern transitioning out of limiting.  Thus even if the current sample does not clip, the value
   fClipScale could be < 1.0 due to the hysteresis of the limiter in CalcAtten. */

			fLookAhead = (float)fabs(fBuffer[chan][samp + grpIndex * iLength]);
			fClipScale = CalcAtten(hsSmoothVars[chan], fLookAhead, PMAXF);

/* Limit current sample to not clip, as determined by the return value from CalcAtten() */

			currLimitAmount = pfDlyBuf_LookAhead[samp] * fClipScale;
			(samp < iLength - CLIPDLY) ? (pfDlyBufW[samp] = currLimitAmount) :
										 (fPrevDlyPtr[samp] = currLimitAmount);
			limitAmount = fPrevClipScale[chan] - fClipScale;  /* limitAmount > 0:  going further into limiting */
															  /* limitAmount < 0:  coming out of limiting */

/* if difference between limit amount from previous sample, and current limit limit amount is non-zero,
   proceed back through previous samples to apply more (or less) clipping.  Delta clipping amount is 
   difference between current and previous clip amount.  Note that the delta clipping amount slopes 
   linearly to zero additional clipping as dlysamp advances towards CLIPDLY samples from the current sample.*/

			if (limitAmount > 0.0)
			{
				for (dlysamp = -1; dlysamp > -CLIPDLY; dlysamp--)
				{
					currLimitAmount = (float)(1.0 - limitAmount * (CLIPDLY + dlysamp) * CLIPNORM);
					(samp + dlysamp < iLength - CLIPDLY) ? (pfDlyBufW[samp + dlysamp] *= currLimitAmount) :
														   (fPrevDlyPtr[samp + dlysamp] *= currLimitAmount);
				}
			}

			fPrevClipScale[chan] = fClipScale;
		}

		for (samp = 0; samp < iLength; samp++)
		{
			fSamp = pfDlyBuf[samp];
			if (bHardClip)
			{
			   	fSamp=(fSamp > 32767.0f) ? 32767.0f : fSamp;
			   	fSamp=(fSamp < -32768.0f) ? -32768.0f : fSamp;
			}
			else
			{
				assert(fSamp <= 32767.0);
				assert(fSamp >= -32768.0);
			}
			fBuffer[chan][samp + grpIndex * iLength] = (fSamp >= 0.0f) ? (fSamp + 0.5f) : (fSamp - 0.5f);
		}

#else
		for (samp = 0; samp < iLength; samp++)
		{
			fSamp = fBuffer[chan][grpIndex * iLength + samp];
		   	fSamp=(fSamp>32767.0f)?32767.0f:fSamp;
		   	fSamp=(fSamp<-32768.0f)?-32768.0f:fSamp;
		   	fBuffer[chan][grpIndex * iLength + samp] = (fSamp>=0.0f)?(fSamp+0.5f):(fSamp-0.5f);
		} /* end samp loop */
#endif
	} /* end chan loop */
}



/****
calcatten()

This routine calculates the amount of attenuation that must be applied to 
prevent clipping beyond PMAXF (32768.0f).  The output scale value varies 
between thresh-6 dB and thresh.  The input value is the maximum sample.  If the
max sample is greater than thresh, then compute a scalefactor that:

  1.	Is proportional to the amount fMaxSamp exceeds thresh.
  2.	Reflects recent history of clipping by applying a delay and decay
		in transitioning out of clipping.
****/

inline float AudioIODSP::CalcAtten(SMOOTH_VARS *smoothvars, float fMaxSamp, float thresh)
{
	float scale;
	float interp;
	short index;
	float limvalscl;

	if ((fMaxSamp > thresh) || (smoothvars->dcy_samp > thresh))
	{
		if (fMaxSamp > PEAKLIMITM1)		/* Hard limit in case fSample is 10 dB above full scale */
		{
			scale = (float)PMAXF / fMaxSamp;		
			smoothvars->dcy_samp = (float)PEAKLIMITM1;
			smoothvars->dcy_count = initLimitCnt;
		}
		else
		{
/**** 
If the input maximum sample is greater than the previous maximum sample, then 
assign the new maximum as the sample to be delayed and decayed.  On each 
subsequent call to CalcAtten(), if there are no samples greater than dcy_samp,
then dcy_samp will hold its value for initLimitCnt calls to CalcAtten() before
sloping down to thresh.
****/
			if (fMaxSamp > smoothvars->dcy_samp)
			{
				smoothvars->dcy_samp = fMaxSamp;
				smoothvars->dcy_count = initLimitCnt;
			}
			else
			{
				if (smoothvars->dcy_count != 0)	/* If delay before decay onset, decrement */
				{
					smoothvars->dcy_count--;
				}
				else if (smoothvars->dcy_samp > thresh)
				{
					smoothvars->dcy_samp = smoothvars->dcy_samp * dcyDelta;
				}
/* else smoothvars->dcy_samp < thresh which will cause scale = 1.0 and smoothvars->dcy_samp = 0.0
   for subsequent samples. */
			}

/* This finds the scaled inverse of limval using a lookup table.  Note that 
 since limval should always be less than 1.0, the index will always be less 
 than 32 and scaletab[index+1] will not go beyond the bounds of its array.*/

			if (smoothvars->dcy_samp > thresh)
			{
				limvalscl = (float)((smoothvars->dcy_samp - thresh) * CX_LIMSCALE);
				index = (short)limvalscl;
				interp = limvalscl - index;		

				scale = (float)(scaletab[index] + interp * (scaletab[index+1] - scaletab[index]));
			}
			else  /* This else case is only entered if dcyDelta scales dcy_samp to be less than thresh */
			{
				scale = 1.0;
				smoothvars->dcy_samp = 0.0;
			}
		}
	}
	else
	{
		scale = 1.0;
		smoothvars->dcy_samp = 0.0;
	}

	return (scale);
}

