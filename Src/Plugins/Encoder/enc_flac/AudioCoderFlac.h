#pragma once
#include "../nsv/enc_if.h"
#include <FLAC/stream_encoder.h>
#include "StreamFileWin32.h"

typedef struct {
	unsigned int compression; // 0-8
} configtype;

class AudioCoderFlac : public AudioCoder 
{
public:
	AudioCoderFlac(unsigned int nch, unsigned int bps, unsigned int samplerate, unsigned int compression);
	~AudioCoderFlac();
	int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail); //returns bytes in out
	void PrepareToFinish();
	void Finish(const wchar_t *destination);
	bool OK();

private:
	FLAC__StreamEncoder *encoder;
	FLAC__StreamMetadata *padding;
	Win32_State win32State;
	unsigned int nch;
	unsigned int bps;
	wchar_t tempFile[MAX_PATH];
	bool finished;
	FLAC__uint64 finishedBytes;
};