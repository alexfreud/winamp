#pragma once
#include "../in_mp4/mpeg4audio.h"
#include "incs/mp4AudioDecIfc.h"

// {654B5212-018E-45f6-88CF-75862C85D99A}
static const GUID mp4_aac_guid = { 0x654b5212, 0x18e, 0x45f6, { 0x88, 0xcf, 0x75, 0x86, 0x2c, 0x85, 0xd9, 0x9a } };

class MP4AACDecoder : public MP4AudioDecoder
{
public:
	static const char *getServiceName() { return "AAC MP4 Decoder"; }
	static GUID getServiceGuid() { return mp4_aac_guid; }
	MP4AACDecoder();
	int Open();
	int OpenEx(size_t bits, size_t maxChannels, bool useFloat);
	void Close();
	int GetCurrentBitrate(unsigned int *bitrate);
	int AudioSpecificConfiguration(void *buffer, size_t buffer_size); // reads ASC block from mp4 file
	void Flush();
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample);
	int GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *useFloat);
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes); 
	int OutputFrameSize(size_t *frameSize);
	int CanHandleCodec(const char *codecName);
	int CanHandleType(uint8_t type);
	int CanHandleMPEG4Type(uint8_t type);
	int SetGain(float _gain) { gain=_gain; return MP4_SUCCESS; }
private:
	mp4AudioDecoderHandle decoder;
	CCompositionUnitPtr composition_unit; /* output */
	CAccessUnitPtr access_unit; /* input */

	size_t preDelay;
	unsigned int bitsPerSample;
	size_t maxChannels;
	bool isFloat;
	float gain;
protected:
	RECVS_DISPATCH;
};
