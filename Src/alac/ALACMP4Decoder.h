/* copyright 2006 Ben Allison */
#ifndef NULLSOFT_ALAC_DECODER_H
#define NULLSOFT_ALAC_DECODER_H

#include "alac/ALACDecoder.h"
#include "../Plugins/Input/in_mp4/mpeg4audio.h"
#include "decomp.h"

class ALACMP4Decoder : public MP4AudioDecoder
{
public:
	ALACMP4Decoder() 
		: mpAlacDecoder(0), 
		  channels(0), 
		  bps(0), 
		  output_bits(0), 
		  use_rg(0), 
		  rg(0.0)
	{
	}
	int OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat);
	int GetCurrentBitrate(unsigned int *bitrate);
	int AudioSpecificConfiguration(void *buffer, size_t buffer_size); // reads ASC block from mp4 file
	void Flush();
	void Close();
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample);
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes); 
	int OutputFrameSize(size_t *frameSize);
	const char *GetCodecInfoString();
	int CanHandleCodec(const char *codecName);
	int SetGain(float gain);
	unsigned char setinfo_sample_size = 0x10;
private:
	ALACDecoder *mpAlacDecoder;
	int channels;
	int bps;
	int output_bits;
	bool use_rg;
	float rg;
protected:
	RECVS_DISPATCH;
};
#endif