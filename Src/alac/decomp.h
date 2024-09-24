/* copyright 2006 David Hammerton, Ben Allison */
#ifndef __ALAC__DECOMP_H
#define __ALAC__DECOMP_H

#include <bfc/platform/types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct alac_file
{
    unsigned char *input_buffer;
    int input_buffer_bitaccumulator; /* used so we can do arbitary
                                        bit reads */
		int samplesize;
		int numchannels;
    int bytespersample;

    /* buffers */
    int32_t *predicterror_buffer_a;
    int32_t *predicterror_buffer_b;

    int32_t *outputsamples_buffer_a;
    int32_t *outputsamples_buffer_b;

		int32_t *uncompressed_bytes_buffer_a;
		int32_t *uncompressed_bytes_buffer_b;


  /* stuff from setinfo */
  uint32_t setinfo_max_samples_per_frame; /* 0x1000 = 4096 */    /* max samples per frame? */
  uint8_t setinfo_7a; /* 0x00 */
  uint8_t setinfo_sample_size; /* 0x10 */ /* bits per sample */
  uint8_t setinfo_rice_historymult; /* 0x28 */
  uint8_t setinfo_rice_initialhistory; /* 0x0a */
  uint8_t setinfo_rice_kmodifier; /* 0x0e */
  uint8_t setinfo_7f; /* 0x02 */ /* channels? */
  uint16_t setinfo_80; /* 0x00ff, 255 */
  uint32_t setinfo_82; /* 0x000020e7, 8423 */ /* max sample size?? */
  uint32_t setinfo_86; /* 0x00069fe4, 434148 */ /* bit rate (avarge)?? */
  uint32_t setinfo_sample_rate; /* 0x0000ac44, 44100 */ /* samplerate? */
  /* end setinfo stuff */

	int32_t last_bitrate;

} alac_file;


alac_file *create_alac(int samplesize, int numchannels);
void decode_frame(alac_file *alac,
                  unsigned char *inbuffer,
                  void *outbuffer, int *outputsize);
void alac_set_info(alac_file *alac, char *inputbuffer);
void destroy_alac(alac_file *alac);

#ifdef __cplusplus
}
#endif

#endif /* __ALAC__DECOMP_H */

