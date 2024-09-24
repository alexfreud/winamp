#pragma once
#pragma once
#include <bfc/platform/types.h>
#include <bfc/platform/export.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NSUTIL_EXPORTS
#define NSUTIL_EXPORT __declspec(dllexport)
#else
#define NSUTIL_EXPORT __declspec(dllimport)
#endif

NSUTIL_EXPORT int nsutil_window_Hann_F32_IP(float *signal, size_t number_of_samples);
// fills a buffer with multiplier values for a Hann window
NSUTIL_EXPORT int nsutil_window_FillHann_F32_IP(float *window, size_t number_of_samples);
NSUTIL_EXPORT int nsutil_window_FillKaiser_F32_IP(float *window, float alpha, size_t number_of_samples);
NSUTIL_EXPORT int nsutil_window_Multiply_F32_IP(float *signal, const float *window, size_t number_of_samples);

#ifdef __cplusplus
}
#endif