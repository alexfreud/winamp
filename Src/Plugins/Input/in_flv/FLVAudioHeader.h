#ifndef NULLSOFT_FLVAUDIOHEADER_H
#define NULLSOFT_FLVAUDIOHEADER_H

namespace FLV
{
	const int AUDIO_FORMAT_PCM = 0;
	const int AUDIO_FORMAT_ADPCM = 1;
	const int AUDIO_FORMAT_MP3 = 2;
	const int AUDIO_FORMAT_PCM_LE = 3; // little endian
	const int AUDIO_FORMAT_NELLYMOSER_16KHZ = 4;
	const int AUDIO_FORMAT_NELLYMOSER_8KHZ = 5;
	const int AUDIO_FORMAT_NELLYMOSER = 6;
	const int AUDIO_FORMAT_A_LAW = 7;
	const int AUDIO_FORMAT_MU_LAW = 8;
	const int AUDIO_FORMAT_AAC = 10;
	const int AUDIO_FORMAT_MP3_8KHZ = 14; 
	
};

class FLVAudioHeader
{
public:
	bool Read(unsigned __int8 *data, size_t size); // size must be >=1, returns "true" if this was a valid header

	// attributes, consider these read-only
	bool stereo;
	int bits;
	int sampleRate;
	int format;
};
#endif