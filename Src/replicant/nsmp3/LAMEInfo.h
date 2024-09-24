#pragma once
#include "foundation/types.h"
#include "MPEGHeader.h"
#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008

#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)

struct LAMEInfo
{
	LAMEInfo();
	uint64_t GetSeekPoint(double percent) const; /* 0 <= percent <= 1.0 */
	int Read(const MPEGHeader &frame, const uint8_t *buffer, size_t bufferlen);
	double GetLengthSeconds() const;
	uint64_t GetSamples() const;
	uint32_t GetFrames() const;

	bool Flag(int flag) const;
	int GetGaps(size_t *pregap, size_t *postgap);
protected:
	int version;
	int sample_rate;
	int samples_per_frame;

	int cbr; // set to 1 if the file is actually just CBR
	// Xing
	int flags;        // from Xing header data
	uint32_t frames;  // total bit stream frames from Xing header data
	uint64_t bytes;   // total bit stream bytes from Xing header data
	int vbr_scale;    // encoded vbr scale from Xing header data
	uint8_t toc[100]; // pointer to unsigned char toc_buffer[100]
	// may be NULL if toc not desired

	// LAME
	char encoder[32]; // 9 characters, but we'll add an extra NULL just in case
	float peak;
	float replaygain_album_gain;
	float replaygain_track_gain;
	unsigned short lowpass;
	unsigned short encoder_delay;
	unsigned short padding;
	uint8_t encoding_method;
	uint8_t tag_revision;
	uint8_t abr_bitrate;
	uint32_t music_length;
	uint16_t music_crc;
	uint16_t tag_crc;
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

int ReadLAMEInfo(const MPEGHeader &frame, const uint8_t *buffer, LAMEInfo *lameInfo);

