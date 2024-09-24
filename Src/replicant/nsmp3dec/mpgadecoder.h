/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mpgadecoder.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: MPEG Decoder class - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/25 18:24:17 $
 * $Id: mpgadecoder.h,v 1.4 2011/01/25 18:24:17 audiodsp Exp $
 */

#ifndef __MPGADECODER_H__
#define __MPGADECODER_H__

/* ------------------------ includes --------------------------------------*/

#include "mp3sscdef.h"
#include "mp3streaminfo.h"
#include "mpegbitstream.h"
#include "mp3decode.h"
#include "mp2decode.h"


/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

//
// Mp3 Decoder Top Level Object.
//
// This is the main ISO/MPEG decoder object that interfaces with the
// application code.
//
// It is however recommended to use IMpgaDecoder (see mp3decifc.h) instead.
// Define USE_MP3DECIFC when planning to use IMpgaDecoder.
//

enum
{
	MPEGAUDIO_CRCCHECK_OFF = 0,
	MPEGAUDIO_CRCCHECK_ON = 1,
};

class CMpgaDecoder
{
public:
	CMpgaDecoder(int crcCheck = MPEGAUDIO_CRCCHECK_OFF);
	CMpgaDecoder(DecoderHooks *hooks, int crcCheck = MPEGAUDIO_CRCCHECK_OFF);
	CMpgaDecoder(unsigned char *pBuf, int cbSize, int crcCheck = MPEGAUDIO_CRCCHECK_OFF);
	~CMpgaDecoder();
	void *operator new(size_t stAllocateBlock);
	void operator delete(void *);

	void Reset();

	SSC  DecodeFrame(float *pPcm, size_t cbPcm, size_t *pcbUsed = 0, unsigned char *ancData = 0, size_t *numAncBytes = 0, int oflOn = 0, unsigned int *startDelay = 0, unsigned int *totalLength = 0);

	const CMp3StreamInfo *GetStreamInfo() const;
	
	void Connect(CGioBase *gf);
	int  Fill(const unsigned char *pBuffer, int cbBuffer);
	int  GetInputFree() const;
	int  GetInputLeft() const;
	void SetInputEof();
	bool IsEof() const;

#ifdef KSA_DRM
	int GetScfBuffer(const unsigned char** ppBuffer, unsigned int* pBufSize) const;
#endif

	SSC GetLastAncData(unsigned char* ancData = 0, size_t *numAncBytes = 0);

	  SSC GetOflVersion(int* oflVersion = 0);
//protected:

	void SetStreamInfo(SSC dwReturn);


	CMp3StreamInfo m_Info;
	CMpegBitStream m_Mbs;
	NALIGN(16) CMp3Decode m_Mp3Decode;
	NALIGN(16) CMp2Decode m_Mp2Decode;
	bool           m_IsEof;
	int			 m_Layer;

private:

};

/*-------------------------------------------------------------------------*/
#endif
