#pragma once
#ifndef _ADTS_HEADER_H
#define _ADTS_HEADER_H

#include "nmrCommon/intTypes.h"
#include "global.h"
#include "uvox2Common.h"


typedef struct ADTSHeader
{
	unsigned int syncword;
	unsigned int layer;
	unsigned int id;
	unsigned int protection;
	unsigned int profile;
	unsigned int sample_rate_index;
	unsigned int private_bit;
	unsigned int channel_configuration;
	unsigned int original;
	unsigned int home;
	int frame_length;
	unsigned int buffer_fullness;
	unsigned int num_data_blocks;	
} ADTSHeader, *aac_adts_header_t;

enum
{
	NErr_Success = 0,
	NErr_True = 0,
	NErr_False = 8,			// returned from a bool-like function to indicate "false" as opposed to "i had an error while figuring it out"
	NErr_UnsupportedFormat = 13,
	NErr_LostSynchronization = 17,
	NErr_WrongFormat = 24,		// data was understood but is indicating a different format than expected.  e.g. an layer 2 header being encountered by a layer 3 parser
	NErr_Reserved = 25			// typically returned when a parser encounters data with a reserved flag set to true
};

struct AAC_FrameInfo : public parserInfo
{
    int m_blocks;
    int m_aot;

    int verifyFrame (const unsigned char *buf, unsigned int len);
    int getUvoxType() { return AAC_LC_DATA; }
    const char *getVersionName() const;
    const char *getAOT() const;

    AAC_FrameInfo (unsigned long value = 0);
    AAC_FrameInfo (const unsigned char *p, unsigned int len);
};

int getAACFrameInfo (const unsigned char *hdr, unsigned int len, AAC_FrameInfo &info);

/* must be 7 bytes */
const int aac_adts_parse(const aac_adts_header_t header, const __uint8 *buffer);
const int aac_adts_get_channel_count(const aac_adts_header_t header);
#if 0
const int aac_adts_match(const aac_adts_header_t header1, const aac_adts_header_t header2);
#endif
const __uint16 getADTSFrameInfo(const char *hdr, unsigned int *samplerate, __uint8 *asc_header = 0);

#endif
