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

typedef void *nsutil_iir_t;


NSUTIL_EXPORT int nsutil_iir_Create_F32(const float *coefficients, int order, nsutil_iir_t *out_iir);
NSUTIL_EXPORT int nsutil_iir_Reset_F32(nsutil_iir_t *iir);
NSUTIL_EXPORT int nsutil_iir_Filter_F32_IP(nsutil_iir_t *iir, float *samples, int num_samples);
NSUTIL_EXPORT int nsutil_iir_Filter_F32(nsutil_iir_t *iir, const float *input, float *output, int num_samples);
NSUTIL_EXPORT int nsutil_iir_Destroy_F32(nsutil_iir_t *iir);

NSUTIL_EXPORT int nsutil_iir_Create_F64(const double *coefficients, int order, nsutil_iir_t *out_iir);
NSUTIL_EXPORT int nsutil_iir_Reset_F64(nsutil_iir_t *iir);
NSUTIL_EXPORT int nsutil_iir_Filter_F64_IP(nsutil_iir_t *iir, double *samples, int num_samples);
NSUTIL_EXPORT int nsutil_iir_Filter_F64(nsutil_iir_t *iir, const double *input, double *output, int num_samples);
NSUTIL_EXPORT int nsutil_iir_Destroy_F64(nsutil_iir_t *iir);

#ifdef __cplusplus
}
#endif