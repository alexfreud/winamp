#include "LAMEinfo.h"
#include <windows.h>
#include <memory.h>
#include <math.h>
#include "api__in_mp3.h"
#include "resource.h"
#include "in2.h"
#pragma intrinsic(memcmp)

extern In_Module mod;

// Xing header -
// 4   Xing
// 4   flags
// 4   frames
// 4   bytes
// 100 toc
// 4   bytes VBR quality

// Lame tag
// 9 bytes - release name
// 11

// Lame extended info tag

// http://gabriel.mp3-tech.org/mp3infotag.html


/*-------------------------------------------------------------*/
static int32_t ExtractI4(unsigned char *buf)
{
	int x;
	// big endian extract

	x = buf[0];
	x <<= 8;
	x |= buf[1];
	x <<= 8;
	x |= buf[2];
	x <<= 8;
	x |= buf[3];

	return x;
}

static int16_t ExtractI2(unsigned char *buf)
{
	int x;
	// big endian extract

	x = buf[0];
	x <<= 8;
	x |= buf[1];

	return x;
}


const static int bitrateV1L3[] = { 0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 0};
const static int bitrateV1L1[] = { 0, 32000, 64000, 96000, 128000, 160000, 192000, 224000, 256000, 288000, 320000, 352000, 384000, 416000, 448000, 0};
const static int bitrateV1L2[] = { 0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 384000, 0};
const static int bitrateV2L1[] = { 0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000, 0};
const static int bitrateV2L2L3[] = { 0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 0};

const static int sampleRateV1[] = {44100, 48000, 32000, 0};
const static int sampleRateV2[] = {22050, 24000, 16000, 0};
const static int sampleRateV2_5[] = {11025, 12000, 8000, 0};

// [mpeg_version][layer]
static const int samples_per_frame[4][4] =
{
	//    Layer 3, Layer 2, Layer 1
	{  0, 576, 1152, 384}, // MPEG2.5
	{  0, },
	{  0, 576, 1152, 384}, // MPEG2
	{  0, 1152, 1152, 384}, // MPEG1
};

void MPEGFrame::ReadBuffer(const unsigned char *buffer)
	{
	sync = 	((unsigned short)buffer[0] << 3) | (buffer[1] >> 5);
	mpegVersion = (buffer[1] >> 3) & 3;
	layer = (buffer[1] >> 1) & 3;
	protection = (buffer[1]) & 1;
	bitrateIndex = (buffer[2] >> 4) & 0xF;
	sampleRateIndex = (buffer[2] >> 2) & 3;
	paddingBit = (buffer[2] >> 1) & 1;
	privateBit = buffer[2] & 1;
	channelMode = (buffer[3] >> 6) & 3;
	modeExtension = (buffer[3] >> 4) & 3;
	copyright = (buffer[3] >> 3) & 1;
	original = (buffer[3] >> 2) & 1;
	emphasis = (buffer[3]) & 3;
	}
	bool MPEGFrame::IsSync()
	{
		return sync == 0x07FF
      && layer != LayerError
      && mpegVersion != MPEG_Error
      && bitrateIndex != 15
      && bitrateIndex != 0
			&& sampleRateIndex != 3
      && !(mpegVersion == MPEG2 && layer != Layer3)
      && !(mpegVersion == MPEG2_5 && layer != Layer3);

	}
	int MPEGFrame::GetBitrate()
	{
		switch (mpegVersion)
		{
		case MPEG1:
			switch (layer)
			{
			case Layer1:
				return bitrateV1L1[bitrateIndex];
			case Layer2:
									return bitrateV1L2[bitrateIndex];
			case Layer3:
					return bitrateV1L3[bitrateIndex];
			}
			break;
		case MPEG2:
		case MPEG2_5:
			switch (layer)
			{
			case Layer1:
				return bitrateV2L1[bitrateIndex];
			case Layer2:
			case Layer3:
					return bitrateV2L2L3[bitrateIndex];
			}
			break;
		}
		
		return 0; // shouldn't get here
	}
	int MPEGFrame::GetPadding()
	{
		if (paddingBit == NotPadded)
			return 0;

		if (layer == Layer1)
			return 4;
		else
			return 1;
	}
	int MPEGFrame::HeaderSize()
	{
		if (protection == CRC)
			return 4 + 2; // 32bits frame header, 16bits CRC
		else
			return 4; // 32bits frame ehader
	}

	int MPEGFrame::GetSampleRate() const
	{
		switch(mpegVersion)
		{
		case MPEG1: return sampleRateV1[sampleRateIndex];
		case MPEG2:return sampleRateV2[sampleRateIndex];
		case MPEG2_5:return sampleRateV2_5[sampleRateIndex];
		default: return 99999999; // return something that will hopefully cause the framesize to be 0
		}

	}

	int MPEGFrame::GetSamplesPerFrame() const
	{
		return samples_per_frame[mpegVersion][layer];
	}

	bool MPEGFrame::IsCopyright()
	{
		return copyright == 1;
	}
	bool MPEGFrame::IsCRC()
	{
		return protection == CRC;
	}

	bool MPEGFrame::IsOriginal()
	{
		return original == 1;
	}

	const char *MPEGFrame::GetEmphasisString()
	{
		static char tempGE[32];
		switch (emphasis)
		{
			case Emphasis_None:
				return WASABI_API_LNGSTRING_BUF(IDS_NONE,tempGE,32);
			case Emphasis_50_15_ms:
				return WASABI_API_LNGSTRING_BUF(IDS_50_15_MICROSEC,tempGE,32);
			case Emphasis_reserved:
				return WASABI_API_LNGSTRING_BUF(IDS_INVALID,tempGE,32);
			case Emphasis_CCIT_J_17:
				return "CITT j.17";
			default:
				return WASABI_API_LNGSTRING_BUF(IDS_ERROR,tempGE,32);
		}
	}

	int MPEGFrame::FrameSize()
	{
		if (layer == Layer1)
		{
			return (int)floor((48.0f*(float)GetBitrate())/GetSampleRate()) + GetPadding();
		}
		else if (layer == Layer2 || layer == Layer3)
		{
			if (mpegVersion == MPEG1)
			return (int)floor((144.0f*(float)GetBitrate())/GetSampleRate()) + GetPadding(); 
			else
				return (int)floor((72.0f*(float)GetBitrate())/GetSampleRate()) + GetPadding(); 
		}
		return 0;
	}

	const char *MPEGFrame::GetMPEGVersionString()
	{
		switch(mpegVersion)
		{
		case MPEG1:
			return "MPEG-1";
		case MPEG2:
			return "MPEG-2";
		case MPEG2_5:
			return "MPEG-2.5";
		default:
			static char tempMF[16];
			return WASABI_API_LNGSTRING_BUF(IDS_ERROR,tempMF,16);
		}
	}

	const char *MPEGFrame::GetChannelModeString()
	{
		static char tempGC[32];
		switch(channelMode)
		{
			case Stereo:
				return WASABI_API_LNGSTRING_BUF(IDS_STEREO,tempGC,32);
			case JointStereo:
				return WASABI_API_LNGSTRING_BUF(IDS_JOINT_STEREO,tempGC,32);
			case DualChannel:
				return WASABI_API_LNGSTRING_BUF(IDS_2_CHANNEL,tempGC,32);
			case Mono:
				return WASABI_API_LNGSTRING_BUF(IDS_MONO,tempGC,32);
			default:
				return WASABI_API_LNGSTRING_BUF(IDS_ERROR,tempGC,32);
		}
	}

	int MPEGFrame::GetLayer()
	{
		switch(layer)
		{
		case Layer1:
			return 1;
		case Layer2:
			return 2;
		case Layer3:
			return 3;
		default:
			return 0;
		}
	}

	int MPEGFrame::GetNumChannels()
	{
		switch(channelMode)
		{
			case Stereo:
				return 2;
			case JointStereo:
				return 2;
			case DualChannel:
				return 2;
			case Mono:
				return 1;
			default:
				return 0;
		}
	}

int ReadLAMEinfo(unsigned char *buffer, LAMEinfo *lameInfo)
{
	int flags;
	MPEGFrame frame;
	frame.ReadBuffer(buffer);

	if (!frame.IsSync())
		return 0;
	
	lameInfo->h_id = frame.mpegVersion & 1;
	lameInfo->samprate = frame.GetSampleRate();
	// determine offset of header
	if (frame.mpegVersion == MPEGFrame::MPEG1) // MPEG 1
	{        
		if (frame.channelMode == MPEGFrame::Mono)
			buffer += (17 + 4);//frame.HeaderSize());
			
		else
			buffer += (32 + 4);//frame.HeaderSize());
	}
	else if (frame.mpegVersion == MPEGFrame::MPEG2) // MPEG 2
	{  
		if (frame.channelMode == MPEGFrame::Mono)
			buffer += (9 + 4);//frame.HeaderSize());
		else
			buffer += (17 + 4);//frame.HeaderSize());
	}
	else if (frame.mpegVersion == MPEGFrame::MPEG2_5) // MPEG 2
	{  
		if (frame.channelMode == MPEGFrame::Mono)
			buffer += (9 + 4);//frame.HeaderSize());
		else
			buffer += (17 + 4);//frame.HeaderSize());
	}

	if (!memcmp(buffer, "Info", 4))
		lameInfo->cbr=1;
	else if (memcmp(buffer, "Xing", 4) && memcmp(buffer, "Lame", 4)) 
		return 0;

	buffer += 4; // skip Xing tag
	flags = lameInfo->flags = ExtractI4(buffer);
	buffer += 4; // skip flags

	if (flags & FRAMES_FLAG)
	{
		lameInfo->frames = ExtractI4(buffer);
		buffer += 4; // skip frames
	}
	if (flags & BYTES_FLAG)
	{
		lameInfo->bytes = ExtractI4(buffer);
		buffer += 4;
	}
	if (flags & TOC_FLAG)
	{
		if (lameInfo->toc)
		{
			for (int i = 0;i < 100;i++)
				lameInfo->toc[i] = buffer[i];
		}
		buffer += 100;
	}

	lameInfo->vbr_scale = -1;
	if (flags & VBR_SCALE_FLAG)
	{
		lameInfo->vbr_scale = ExtractI4(buffer);
		buffer += 4;
	}

	if (!memcmp(buffer, "LAME", 4))
	{
	for (int i=0;i<9;i++)
		lameInfo->lameTag[i]=*buffer++;
	lameInfo->lameTag[9]=0; // null terminate in case tag used all 20 characters

	lameInfo->encodingMethod = (*buffer++)&0xF; // we'll grab the VBR method
	lameInfo->lowpass = (*buffer++)*100; // lowpass value
	lameInfo->peak=*((float *)buffer); // read peak value
	buffer+=4; // skip peak value

	// read track gain
	int16_t gain_word = ExtractI2(buffer);
	if ((gain_word & 0xFC00) == 0x2C00)
	{
		lameInfo->replaygain_track_gain = (float)(gain_word & 0x01FF);
		lameInfo->replaygain_track_gain /= 10;
		if (gain_word & 0x0200)
			lameInfo->replaygain_track_gain = -lameInfo->replaygain_track_gain;
	}
	buffer+=2; 

	// read album gain
	gain_word = ExtractI2(buffer);
	if ((gain_word & 0xFC00) == 0x4C00)
	{
		lameInfo->replaygain_album_gain = (float)(gain_word & 0x01FF);
		lameInfo->replaygain_album_gain /= 10;
		if (gain_word & 0x0200)
			lameInfo->replaygain_album_gain = -lameInfo->replaygain_album_gain;
	}
	buffer+=2; 

	buffer+=1; // skip encoding flags + ATH type
	buffer+=1; // skip bitrate

	// get the encoder delay and padding, annoyingly as 12 bit values packed into 3 bytes
	lameInfo->encoderDelay = ((unsigned short)buffer[0] << 4) | (buffer[1] >> 4);
	lameInfo->padding = ((unsigned short)(buffer[1]&0x0F) << 8) | (buffer[2]);
	}
	return frame.FrameSize();
}