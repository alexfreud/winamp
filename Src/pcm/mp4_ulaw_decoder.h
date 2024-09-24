#pragma once

#include "../Plugins/Input/in_mp4/mpeg4audio.h"

// {F05D2AAA-6F21-4970-AE9C-8D9FFB497957}
static const GUID mp4_ulaw_guid = 
{ 0xf05d2aaa, 0x6f21, 0x4970, { 0xae, 0x9c, 0x8d, 0x9f, 0xfb, 0x49, 0x79, 0x57 } };


class MP4ulawDecoder : public MP4AudioDecoder
{
public:
	static const char *getServiceName() { return "ulaw MP4 Decoder"; }
	static GUID getServiceGuid() { return mp4_ulaw_guid; } 
	MP4ulawDecoder();
	int Open();
	void Close();
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample);
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes); 
	int CanHandleCodec(const char *codecName);
	//int SetGain(float _gain) { gain=_gain; return MP4_SUCCESS; }
	int RequireChunks(); // return 1 if your decoder wants to read whole chunks rather than samples

protected:
	RECVS_DISPATCH;
};