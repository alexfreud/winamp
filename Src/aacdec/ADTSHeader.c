#include "ADTSHeader.h"
#include <bfc/error.h>

enum
{
	ADTS_NOT_PROTECTED=1,
	ADTS_PROTECTED=0,
	ADTS_SYNC = 0xFFF,
	ADTS_MAIN = 0x00,
	ADTS_LC = 0x01,
	ADTS_SSR = 0x10,
	ADTS_LTP = 0x11,
};


int nsaac_adts_parse(nsaac_adts_header_t header, const uint8_t *buffer)
{
	unsigned int syncword = (buffer[0] << 4) | (buffer[1] >> 4);
	unsigned int layer;
	unsigned int sample_rate_index;
	if (syncword != ADTS_SYNC)
		return NErr_LostSynchronization;

	header->syncword = syncword;
	header->id = (buffer[1] >> 3) & 1;
	layer = (buffer[1] >> 1) & 3;
	if (layer != 0)
		return NErr_WrongFormat;
	header->layer = layer;	
	header->protection = (buffer[1]) & 1;
	header->profile = (buffer[2] >> 6) & 3;
	sample_rate_index = (buffer[2] >> 2) & 0xF;
	if (sample_rate_index == 15)
		return NErr_UnsupportedFormat; // might actually be OK if we can separately signal the sample rate

	if (sample_rate_index > 13)
		return NErr_Reserved;

	header->sample_rate_index = sample_rate_index;
	header->private_bit = (buffer[2] >> 1) & 1;
	header->channel_configuration = ((buffer[2] & 1) << 2) | ((buffer[3] >> 6) & 3);
	header->original = (buffer[3] >> 5) &1;
	header->home = (buffer[3] >> 4) &1;

	//copyright_identification_bit = (buffer[3] >> 3) & 1;
	//copyright_identification_start = (buffer[3] >> 2) & 1;
	header->frame_length = ((buffer[3] & 3) << 11) | (buffer[4]<<3) | ((buffer[5] >> 5) & 7);
	header->buffer_fullness = ((buffer[5] & 0x1F) << 6) | (buffer[6] >> 2);
	header->num_data_blocks = buffer[6]  & 3;
	return NErr_Success;
}


static const unsigned int aac_sratetab[] =
{
	96000,
	88200,
	64000,
	48000,
	44100,
	32000,
	24000,
	22050,
	16000,
	12000,
	11025,
	8000,
	7350,
};

unsigned int nsaac_adts_get_samplerate(nsaac_adts_header_t header)
{
	return aac_sratetab[header->sample_rate_index];
}

int nsaac_adts_match(nsaac_adts_header_t header1, nsaac_adts_header_t header2)
{
	if (header1->id != header2->id)
		return NErr_False;

	if (header1->profile != header2->profile)
		return NErr_False;

	if (header1->sample_rate_index != header2->sample_rate_index)
		return NErr_False;

	if (header1->channel_configuration != header2->channel_configuration)
		return NErr_False;

	return NErr_True;
}

int nsaac_adts_get_channel_count(nsaac_adts_header_t header)
{
	switch(header->channel_configuration)
	{
	case 7:
		return 8;
	default:
		return header->channel_configuration;
	}
}

size_t nsaac_adts_get_header_size(nsaac_adts_header_t header)
{
	if (header->protection == ADTS_PROTECTED)
		return 9;
	else
		return 7;
}