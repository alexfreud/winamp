#pragma once
#include <bfc/platform/types.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	size_t frame_length;
	unsigned int buffer_fullness;
	unsigned int num_data_blocks;	
} ADTSHeader, *nsaac_adts_header_t;

/* must be 7 bytes */
int nsaac_adts_parse(nsaac_adts_header_t header, const uint8_t *buffer);
unsigned int nsaac_adts_get_samplerate(nsaac_adts_header_t header);
int nsaac_adts_match(nsaac_adts_header_t header1, nsaac_adts_header_t header2);
int nsaac_adts_get_channel_count(nsaac_adts_header_t header);
size_t nsaac_adts_get_header_size(nsaac_adts_header_t header);

#ifdef __cplusplus
}
#endif