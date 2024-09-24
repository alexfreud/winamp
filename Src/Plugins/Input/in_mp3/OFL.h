#pragma once
#include "LAMEInfo.h"
#include <bfc/platform/types.h>
class OFL
{
public:
	int Read(const MPEGFrame &header, const uint8_t *buffer, size_t buffer_len);
	double GetLengthSeconds() const;
	uint64_t GetSamples() const;
	uint32_t GetFrames() const;
	int GetGaps(size_t *pregap, size_t *postgap);

private:
	int samples_per_frame;
	uint32_t total_length;
	uint16_t codec_delay;
	uint16_t additional_delay;

	unsigned int sample_rate;
};

