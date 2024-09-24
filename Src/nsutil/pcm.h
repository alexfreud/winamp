#pragma once
#pragma once
#include <bfc/platform/types.h>
#include <bfc/platform/export.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NSUTIL_EXPORTS
#define NSUTIL_EXPORT __declspec(dllexport)
#else
#define NSUTIL_EXPORT __declspec(dllimport)
#endif

	// this function is for when input buffer is in range from -1.0 to 1.0
NSUTIL_EXPORT int nsutil_pcm_FloatToInt_Interleaved_Gain(void *pcm, const float *input, int bps, size_t num_samples, float gain);
NSUTIL_EXPORT int nsutil_pcm_FloatToInt_Interleaved(void *pcm, const float *input, int bps, size_t num_samples);
NSUTIL_EXPORT int nsutil_pcm_IntToFloat_Interleaved(float *output, const void *pcm, int bps, size_t num_samples);
NSUTIL_EXPORT int nsutil_pcm_IntToFloat_Interleaved_Gain(float *output, const void *pcm, int bps, size_t num_samples, float gain);
NSUTIL_EXPORT int nsutil_pcm_S8ToS16_Interleaved(int16_t *output, const int8_t *pcm, size_t num_samples);
NSUTIL_EXPORT int nsutil_pcm_U8ToS16_Interleaved(int16_t *output, const uint8_t *pcm, size_t num_samples);

#ifdef __cplusplus
}
#endif