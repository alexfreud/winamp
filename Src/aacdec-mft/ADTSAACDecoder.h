#pragma once
#include "../Plugins/Input/in_mp3/adts.h"
#include "MFTDecoder.h"

// {19450308-90D7-4E45-8A9D-DC71E67123E2}
static const GUID adts_aac_guid = 
{ 0x19450308, 0x90d7, 0x4e45, { 0x8a, 0x9d, 0xdc, 0x71, 0xe6, 0x71, 0x23, 0xe2 } };

class ADTSAACDecoder : public adts
{
public:
	static const char *getServiceName() { return "MFT AAC ADTS Decoder"; }
	static GUID getServiceGuid() { return adts_aac_guid; }
	ADTSAACDecoder();
	int Initialize(bool forceMono, bool reverse_stereo, bool allowSurround, int maxBits, bool allowRG, bool _useFloat, bool _useCRC);
	bool Open(ifc_mpeg_stream_reader *file);
	void Close();
	void GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate);
	void CalculateFrameSize(int *frameSize);
	void Flush(ifc_mpeg_stream_reader *file);
	size_t GetCurrentBitrate();
	size_t GetDecoderDelay();
	int Sync(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate);
	int Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut);
	int GetLayer();
	void Release();

private:
	int Internal_Decode(ifc_mpeg_stream_reader *file, const void *input, size_t input_length, unsigned __int8 *output, size_t outputSize, size_t *outputWritten);
	
	MFTDecoder decoder;

	int bitsPerSample;
	float gain;
	bool allowRG;
	bool useFloat;
	size_t predelay;

};