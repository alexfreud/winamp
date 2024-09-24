#ifndef NULLSOFT_IN_MP3_ADTS_MPG123_H
#define NULLSOFT_IN_MP3_ADTS_MPG123_H

#include "../Plugins/Input/in_mp3/adts.h"
#include "api__mp3-mpg123.h"
#include <mpg123.h>

// {4192FE3F-E843-445c-8D62-51BE5EE5E68C}
static const GUID adts_mpg123_guid = 
{ 0x4192fe3f, 0xe843, 0x445c, { 0x8d, 0x62, 0x51, 0xbe, 0x5e, 0xe5, 0xe6, 0x8c } };

class ADTS_MPG123 : public adts
{
public:
	static const char *getServiceName() { return "MPG123 stream Decoder"; }
	static GUID getServiceGuid() { return adts_mpg123_guid; }
	ADTS_MPG123();
	~ADTS_MPG123();
	int Initialize(bool forceMono, bool reverse_stereo, bool allowSurround, int maxBits, bool allowRG, bool _useFloat = false, bool _useCRC = false);
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
	mpg123_handle *decoder;
	size_t bitsPerSample;
	double gain;
	bool allowRG;
	bool useFloat;

	int channels;
	int sampleRate;
	unsigned int decoderDelay;
	
	int _Read(ifc_mpeg_stream_reader *file);
	bool _UpdateProperties();
	float *decode_buffer;
	size_t decode_buffer_length;
};


#endif
