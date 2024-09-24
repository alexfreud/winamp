#ifndef NULLSOFT_ENC_ACM_ACMENCODER_H
#define NULLSOFT_ENC_ACM_ACMENCODER_H

#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include "../nsv/enc_if.h"
#include "Config.h"
#include "Finisher.h"

class ACMEncoder : public AudioCommon
{
public:
	ACMEncoder(int srate, int nch, int bps, ACMConfig *config);
	virtual ~ACMEncoder();

	virtual int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail);
	void FinishAudio(const wchar_t *filename);

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

	bool do_header;

};



#endif