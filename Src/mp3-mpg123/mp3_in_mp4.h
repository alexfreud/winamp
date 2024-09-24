#ifndef NULLSOFT_MP3_MPEG4_H
#define NULLSOFT_MP3_MPEG4_H

// used to decode an MPEG-1 audio object in an MPEG-4 ISO Media file

#include "../Plugins/Input/in_mp4/mpeg4audio.h"
#include <mpg123.h>

class MPEG4_MP3 : public MP4AudioDecoder
{
public:
	MPEG4_MP3();
	~MPEG4_MP3();
	int Open();
	int OpenEx(size_t bits, size_t maxChannels, bool useFloat);
	//int AudioSpecificConfiguration(void *buffer, size_t buffer_size); // reads ASC block from mp4 file
	int GetCurrentBitrate(unsigned int *bitrate);
	int OutputFrameSize(size_t *frameSize);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample); 
	int GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat); 
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();
	const char *GetCodecInfoString();
	int CanHandleCodec(const char *codecName);
	int CanHandleType(unsigned __int8 type);
	int CanHandleMPEG4Type(unsigned __int8 type);
	int SetGain(float gain);
	
private:
	bool _UpdateProperties();
	mpg123_handle *decoder;
	
	int bits;
	int channels;
	bool floatingPoint;
	int sample_rate;
	RECVS_DISPATCH;
	float gain;
	int pregap;
};

#endif
