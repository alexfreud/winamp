#ifndef __MP2DECODE_H__
#define __MP2DECODE_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"
#include "mpegbitstream.h"
#include "polyphase.h"

/*-------------------------------------------------------------------------*/

//
// MPEG Layer-2 decoding class.
//
//  This is the main MPEG Layer-2 decoder object.
//

class CMp2Decode
{
public:
  CMp2Decode(CMpegBitStream &_Bs, DecoderHooks *_hooks=0);
  ~CMp2Decode();

  void Init(bool fFullReset = true);

  SSC Decode(void *pPcm, size_t cbPcm, size_t *pcbUsed);

private:
  SSC Decode2(void *pPcm);
  SSC Decode1(void *pPcm);

  void ZeroPolySpectrum();
  void SetInfo();

  CPolyphase        m_Polyphase;   // polyphase

  MPEG_INFO         m_Info;        // info structure
  CMpegBitStream   &m_Bs;          // bitstream

  POLYSPECTRUM      m_PolySpectrum;                  // spectrum (post-mdct)

  char m_tab_3[32 * 3];
  char m_tab_5[128 * 3];
  char m_tab_9[1024 * 3];
  float m_scales[27][64];
	DecoderHooks *hooks;
};

/*-------------------------------------------------------------------------*/
#endif
