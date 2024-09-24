#ifndef NULLSOFT_LAMEINFOH
#define NULLSOFT_LAMEINFOH


#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008

#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)
#include <memory.h>
#pragma intrinsic(memset)
struct LAMEinfo
{
	LAMEinfo() 
	{
		memset(this, 0, sizeof(LAMEinfo));
	}

	int cbr; // set to 1 if the file is actually just CBR
	// Xing
	int h_id;
	int samprate;   // determined from MPEG header
    int flags;      // from Xing header data
    int frames;     // total bit stream frames from Xing header data
    int bytes;      // total bit stream bytes from Xing header data
    int vbr_scale;  // encoded vbr scale from Xing header data
    unsigned char *toc;  // pointer to unsigned char toc_buffer[100]
                         // may be NULL if toc not desired

		// LAME
	char lameTag[10]; // 9 characters, but we'll add an extra NULL just in case
	float peak;
	float replaygain_album_gain;
	float replaygain_track_gain;
	unsigned short lowpass;
	unsigned short encoderDelay;
	unsigned short padding;
	int encodingMethod;
};

enum
{
	ENCODING_METHOD_LAME = 0,
	ENCODING_METHOD_CBR = 1,
	ENCODING_METHOD_ABR = 2,
	ENCODING_METHOD_VBR1 = 3,
	ENCODING_METHOD_VBR2 = 4,
	ENCODING_METHOD_VBR3 = 5,
	ENCODING_METHOD_VBR4 = 6,
	ENCODING_METHOD_CBR_2PASS = 8,
	ENCODING_METHOD_ABR_2PASS = 9,
};

int ReadLAMEinfo(unsigned char *buffer, LAMEinfo *lameInfo);

class MPEGFrame
{
public:
	int GetNumChannels();

	void ReadBuffer(const unsigned char *buffer);
	bool IsSync();
	int GetBitrate();
	int GetPadding();
	int HeaderSize();
	int GetSampleRate() const;
	int FrameSize();
	const char *GetMPEGVersionString();
	const char *GetChannelModeString();
	const char *GetEmphasisString();
	int GetLayer();
	bool IsCRC();
	bool IsCopyright();
	bool IsOriginal();
	int MPEGFrame::GetSamplesPerFrame() const;
	
	enum
	{
		NotPadded=0,
		Padded=1,
		CRC = 0,
		NoProtection = 1,
		Stereo = 0,
		JointStereo = 1,
		DualChannel = 2,
		Mono = 3,
		MPEG1 = 3,
		MPEG2 = 2,
    MPEG_Error = 1,
		MPEG2_5 = 0,
		Layer1 = 3,
		Layer2 = 2,
		Layer3 = 1,
    LayerError = 0,
		Emphasis_None = 0,
		Emphasis_50_15_ms = 1,
		Emphasis_reserved = 2,
		Emphasis_CCIT_J_17 = 3,
	};

	unsigned int sync:11, 
mpegVersion:2, 
layer:2, 
protection:1,
bitrateIndex:4, 
paddingBit:1,
privateBit:1, 
channelMode:2, 
modeExtension:2, 
sampleRateIndex:2, 
copyright:1, 
original:1, 
emphasis:2;
};


#endif