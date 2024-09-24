#include "MPEGHeader.h"
#include <math.h>

// [mpeg_version][layer][index]
static const int bitrates[4][4][15] =
{
	{
		// MPEG-2.5
		{ 0,},
		{  0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000}, // Layer 3
		{  0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000}, // Layer 2
		{  0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000}, // Layer 1
	},

	{
		// invalid
		{ 0, },
		{ 0, },
		{ 0, },
		{ 0, },
	},

	{
		// MPEG-2
		{ 0,},
		{  0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000}, // Layer 3
		{  0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000}, // Layer 2
		{  0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000}, // Layer 1
	},

	{
		// MPEG-1
		{ 0,},
		{  0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000}, // Layer 3
		{  0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 384000}, // Layer 2
		{  0, 32000, 64000, 96000, 128000, 160000, 192000, 224000, 256000, 288000, 320000, 352000, 384000, 416000, 448000}, // Layer 1
	},
};

// [mpeg_version][index]
static const int sample_rates[4][4] =
{
	{11025, 12000,  8000, 0}, // MPEG-2.5
	{0, },
	{22050, 24000, 16000, 0}, // MPEG-2
  {44100, 48000, 32000, 0}, // MPEG-1  
};

// [mpeg_version][layer]
static const int samples_per_frame[4][4] =
{
	//    Layer 3, Layer 2, Layer 1
	{  0, 576, 1152, 384}, // MPEG2.5
	{  0, },
	{  0, 576, 1152, 384}, // MPEG2
	{  0, 1152, 1152, 384}, // MPEG1
};

// [layer]
static const int bits_per_slot[4] = { 0, 8, 8, 32 };

void MPEGHeader::ReadBuffer(const uint8_t *buffer)
{
	sync = 	((uint16_t)buffer[0] << 3) | (buffer[1] >> 5);
	mpeg_version = (buffer[1] >> 3) & 3;
	layer = (buffer[1] >> 1) & 3;
	protection = (buffer[1]) & 1;
	bitrate_index = (buffer[2] >> 4) & 0xF;
	sample_rate_index = (buffer[2] >> 2) & 3;
	padding_bit = (buffer[2] >> 1) & 1;
	private_bit = buffer[2] & 1;
	channel_mode = (buffer[3] >> 6) & 3;
	mode_extension = (buffer[3] >> 4) & 3;
	copyright = (buffer[3] >> 3) & 1;
	original = (buffer[3] >> 2) & 1;
	emphasis = (buffer[3]) & 3;
}

bool MPEGHeader::IsSync() const
{
	return sync == 0x07FF
		&& layer != LayerError
		&& mpeg_version != MPEG_Error
		&& bitrate_index != 15
		&& bitrate_index != 0
		&& sample_rate_index != 3
		&& !(mpeg_version == MPEG2 && layer != Layer3)
		&& !(mpeg_version == MPEG2_5 && layer != Layer3);
}

int MPEGHeader::GetBitrate() const
{
	return bitrates[mpeg_version][layer][bitrate_index];
}

int MPEGHeader::HeaderSize() const
{
	if (protection == CRC)
		return 4 + 2; // 32bits frame header, 16bits CRC
	else
		return 4; // 32bits frame ehader
}

int MPEGHeader::GetSampleRate() const
{
	return sample_rates[mpeg_version][sample_rate_index];
}

bool MPEGHeader::IsCopyright() const
{
	return copyright == 1;
}
bool MPEGHeader::IsCRC() const
{
	return protection == CRC;
}

bool MPEGHeader::IsOriginal() const
{
	return original == 1;
}

int MPEGHeader::GetSamplesPerFrame() const
{
	return samples_per_frame[mpeg_version][layer];
}

int MPEGHeader::FrameSize() const
{
	int nBitsPerSlot;
  int nAvgSlotsPerFrame;

  nBitsPerSlot     = bits_per_slot[layer];

  nAvgSlotsPerFrame = (GetSamplesPerFrame() * (bitrates[mpeg_version][layer][bitrate_index] / nBitsPerSlot)) / sample_rates[mpeg_version][sample_rate_index];

  return (nAvgSlotsPerFrame + padding_bit) * nBitsPerSlot / 8;
}

int MPEGHeader::GetLayer() const
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

int MPEGHeader::GetNumChannels() const
{
	switch(channel_mode)
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
