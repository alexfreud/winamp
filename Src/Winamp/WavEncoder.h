#ifndef NULLSOFT_WAVENCODERH
#define NULLSOFT_WAVENCODERH

#include <mmreg.h>
#include <msacm.h>
#include "../nsv/enc_if.h"

#define WFSIZ 0x800
 struct EXT_WFX
{
	WAVEFORMATEX wfx;
	BYTE crap[WFSIZ];
};
class WavEncoder : public AudioCoder
{
public:
	WavEncoder(int srate, int nch, int bps, int res_srate = 0, int res_bps = 0, int res_nch = 0);
	virtual ~WavEncoder();

	virtual int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail);
	void FinishAudio(HANDLE fh, WavEncoder *coder);

	int GetLastError();

	int m_did_header;
	int m_nch, m_srate, m_bps;
	int m_bytes_done;
	int m_error;
	int m_hlen;
	int m_nsam;


	EXT_WFX m_convert_wfx;

	WAVEFORMATEX m_wfx_src;
	HACMSTREAM hStream, hStreamResample;

	ACMSTREAMHEADER ahd, ahdResample;

	unsigned char *m_acm_buf, *m_acm_outbuf;
	int m_bytes_inbuf, m_bytes_outbuf;
	unsigned char *m_acm_resample_buf, *m_acm_resample_outbuf;
	int m_bytes_inbuf_resample, m_bytes_outbuf_resample;

};


#endif