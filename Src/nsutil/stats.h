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

NSUTIL_EXPORT int nsutil_stats_RMS_F32(const float *buffer, size_t num_samples, float *rms);

#ifdef __cplusplus
}
#endif