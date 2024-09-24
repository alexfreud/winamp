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

	enum
	{
		nsutil_fft_none = 0,
		nsutil_fft_fast = 1,
		nsutil_fft_accurate = 2,
	};

typedef void *nsutil_fft_t;
NSUTIL_EXPORT int nsutil_fft_Create_F32R(nsutil_fft_t *fft, int order, int accuracy);
NSUTIL_EXPORT int nsutil_fft_Forward_F32R_IP(nsutil_fft_t fft, float *signal);
NSUTIL_EXPORT int nsutil_fft_Destroy_F32R(nsutil_fft_t *fft);

#ifdef __cplusplus
}
#endif