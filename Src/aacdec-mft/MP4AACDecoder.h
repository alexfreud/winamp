#pragma once
#include "../Plugins/Input/in_mp4/mpeg4audio.h"
#include "MFTDecoder.h"

// {3C0B6E1B-0C21-4716-B8D6-C7665CBC90E9}
static const GUID mp4_aac_guid = 
{ 0x3c0b6e1b, 0xc21, 0x4716, { 0xb8, 0xd6, 0xc7, 0x66, 0x5c, 0xbc, 0x90, 0xe9 } };


class MP4AACDecoder : public MP4AudioDecoder
{
public:
	static const char *getServiceName() { return "MFT AAC MP4 Decoder"; }
	static GUID getServiceGuid() { return mp4_aac_guid; }
	MP4AACDecoder();
	~MP4AACDecoder();
	int OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat);
	void Close();
	int GetCurrentBitrate(unsigned int *bitrate);
	int AudioSpecificConfiguration(void *buffer, size_t buffer_size); // reads ASC block from mp4 file
	void Flush();
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *_bitsPerSample);
	int GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *useFloat);
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes); 
	int OutputFrameSize(size_t *frameSize);
	int CanHandleCodec(const char *codecName);
	int CanHandleType(uint8_t type);
	int CanHandleMPEG4Type(uint8_t type);
	int SetGain(float _gain) { gain=_gain; return MP4_SUCCESS; }
	void EndOfStream();
private:
	MFTDecoder decoder;

	unsigned int bitsPerSample;
	bool isFloat;
	float gain;
	unsigned int channels;
protected:
	RECVS_DISPATCH;
};
