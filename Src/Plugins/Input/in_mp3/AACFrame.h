#ifndef NULLSOFT_AACFRAME_H
#define NULLSOFT_AACFRAME_H

class AACFrame
{
public:
	void ReadBuffer(unsigned __int8 *buffer);
	bool OK();

	enum
	{
		NOT_PROTECTED=1,
		PROTECTED=0,
		SYNC = 0xFFF,
		MAIN = 0x00,
		LC = 0x01,
		SSR = 0x10,
		LTP = 0x11,
	};
	int GetNumChannels();
	int GetSampleRate();
	const wchar_t *GetProfileName();
	const wchar_t *GetChannelConfigurationName();
	int GetMPEGVersion(); // returns 2 or 4
public:
	int syncword;
	int layer;
	int id;
	int protection;
	int profile;
	int sampleRateIndex;
	int privateBit;
	int channelConfiguration;
	int original;
	int home;
	int frameLength;
	int bufferFullness;
	int numDataBlocks;

	
};

#endif