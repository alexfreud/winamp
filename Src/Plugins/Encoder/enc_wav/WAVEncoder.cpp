#include "WAVEncoder.h"

#define rev32(X) ((((DWORD)(X)&0xFF)<<24)|(((DWORD)(X)&0xFF00)<<8)|(((DWORD)(X)&0xFF0000)>>8)|(((DWORD)(X)&0xFF000000)>>24))

WAVEncoder::WAVEncoder(int nch, int srate, int bps, ACMConfig *config)
{
	numBytes = 0;

	inputFormat.wFormatTag = WAVE_FORMAT_PCM;
	inputFormat.nChannels = nch;
	inputFormat.nSamplesPerSec = srate;
	inputFormat.nAvgBytesPerSec = 	srate * nch * (bps >> 3);
	inputFormat.nBlockAlign = nch * (bps >> 3);
	inputFormat.wBitsPerSample	= bps;
	inputFormat.cbSize = 0;

	do_header = config->header;
	if (do_header)
	first = 44;
}

int WAVEncoder::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	if (first)
	{
		int valid = min(first, out_avail);
		first -= valid;
		*in_used = 0;
		return valid;
	}

	int valid = min(in_avail, out_avail);

	memcpy(out, in, valid);
	*in_used = valid;
	numBytes += valid;

	return valid;
}

void WAVEncoder::PrepareToFinish()
{}

void WAVEncoder::FinishAudio(const wchar_t *filename)
{
	if (!do_header) return;
	// open old file
	HANDLE tempfile = CreateFileW(filename, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if (tempfile)
	{
		// rewrite initial 44 bytes
		DWORD bw = 0;
		unsigned __int32 t;
		t = rev32('RIFF');
		WriteFile(tempfile, &t, 4, &bw, 0); // RIFF (4 bytes)
		t = (unsigned __int32)numBytes + 36;
		bw = 0;WriteFile(tempfile, &t, 4, &bw, 0); // size of chunk (4 bytes)
		t = rev32('WAVE');
		bw = 0;WriteFile(tempfile, &t, 4, &bw, 0); // WAVE (4 bytes)
		t = rev32('fmt ');
		bw = 0;WriteFile(tempfile, &t, 4, &bw, 0);// fmt  (4 bytes)
		t = 16;
		bw = 0;WriteFile(tempfile, &t, 4, &bw, 0);// size of chunk (4 bytes)
		bw = 0;WriteFile(tempfile, &inputFormat, 16, &bw, 0); // write WAVEFORMAT out (16 bytes)
		t = rev32('data');
		bw = 0;WriteFile(tempfile, &t, 4, &bw, 0);// data  (4 bytes)
		bw = 0;WriteFile(tempfile, &numBytes, 4, &bw, 0);// size of chunk (4 bytes)

		CloseHandle(tempfile);
	}
}