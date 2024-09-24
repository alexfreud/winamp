/***************************************************************************\
*
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
*                        All Rights Reserved
*
*   filename: mp3decode.h
*   project : ISO/MPEG-Decoder
*   author  : Martin Sieler
*   date    : 1998-05-26
*   contents/description: MPEG Layer-3 decoder
*
*
\***************************************************************************/

/*
* $Date: 2011/01/28 21:45:29 $
* $Id: mp3decode.h,v 1.5 2011/01/28 21:45:29 audiodsp Exp $
*/

#ifndef __MP3DECODE_H__
#define __MP3DECODE_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"
#include "mpegbitstream.h"
#include "huffdec.h"
#include "mdct.h"
#include "polyphase.h"
#include "mp3ancofl.h"

#ifdef ERROR_CONCEALMENT
#include "conceal.h"
#endif

/*-------------------------------------------------------------------------*/

//
// MPEG Layer-3 decoding class.
//
//  This is the main MPEG Layer-3 decoder object.
//

class NALIGN(16) CMp3Decode
{
public:

	CMp3Decode(CMpegBitStream &_Bs, int _crc_check, DecoderHooks *_hooks=0);

	~CMp3Decode();

	void Init(bool fFullReset = true);

	// PcmFormat: 0: integer, 1: 32 bit float (IEEE)
	SSC Decode(float *pPcm,
		size_t cbPcm,
		size_t *pcbUsed,
		unsigned char *ancData,
		size_t *numAncBytes = 0,
		int oflOn = 0,
		unsigned int *startDelay = 0,
		unsigned int *totalLength = 0);


	SSC GetLastAncData(unsigned char* ancData, size_t *numAncBytes);

	SSC GetOflVersion(int* oflVersion);

protected:

	SSC  DecodeOnNoMainData(float *pPcm);
	SSC  DecodeNormal      (float *pPcm, bool fCrcOk);

	void PolyphaseReorder();
	void ZeroISpectrum();
	void ZeroSpectrum();
	void ZeroPolySpectrum();
	void SetInfo();

	CMp3Huffman       m_Mp3Huffman;  // huffman decoder
	CMdct             m_Mdct;        // mdct
	CPolyphase        m_Polyphase;   // polyphase
	CMp3AncOfl	    m_AncOfl;	   // ancillary data and ofl

#ifdef ERROR_CONCEALMENT
	CErrorConcealment m_Conceal;     // error concealment
#endif

	MPEG_INFO         m_Info;        // info structure
	CMpegBitStream   &m_Bs;          // bitstream
	CBitStream        m_Db;          // dynamic buffer
	MP3SI             m_Si;          // side info
	MP3SCF            m_ScaleFac[2]; // scalefactors

	int               m_ISpectrum[2][SSLIMIT*SBLIMIT]; // spectrum (integer)
	NALIGN(16) SPECTRUM          m_Spectrum;                      // spectrum (float)
	NALIGN(16) POLYSPECTRUM      m_PolySpectrum;                  // spectrum (post-mdct)

	int								m_crc_check;			// 0: no CRC check, 1: fail on CRC errors

protected:

	enum { dynBufSize = 2048 } ;

	unsigned char     m_dynBufMemory [dynBufSize] ;

private:
	DecoderHooks *hooks;
};

/*-------------------------------------------------------------------------*/
#endif
