#ifndef NULLSOFT_ENC_WAV_WAVENCODER_H
#define NULLSOFT_ENC_WAV_WAVENCODER_H


#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include "../nsv/enc_if.h"
#include "Config.h"
#include "Finisher.h"
class WAVEncoder : public AudioCommon
{
public:
	WAVEncoder(int nch, int srate, int bps, ACMConfig *config);
	int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail); //returns bytes in out
	void FinishAudio(const wchar_t *filename);
	void PrepareToFinish();

	WAVEFORMATEX inputFormat;
	size_t numBytes;
	int first;
	bool do_header;
};

#endif