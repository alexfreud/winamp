#pragma once
#include "foundation/types.h"

class MPEGHeader
{
public:
	void ReadBuffer(const uint8_t *buffer);
	int GetNumChannels() const;
	bool IsSync() const;
	int GetBitrate() const;
	int HeaderSize() const;
	int GetSampleRate() const;
	int FrameSize() const;
	int GetLayer() const;
	bool IsCRC() const;
	bool IsCopyright() const;
	bool IsOriginal() const;
	int GetSamplesPerFrame() const;
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

	uint16_t sync;
	uint8_t mpeg_version, layer, protection, bitrate_index;
	uint8_t padding_bit, private_bit, channel_mode, mode_extension;
	uint8_t sample_rate_index, copyright, original, emphasis;
};
