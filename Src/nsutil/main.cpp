#ifdef _WIN64
#include "../tools/staticlib/ipp_m7.h"
#else 
#include "../tools/staticlib/ipp_px.h"
#endif

#include <ippi.h>
#include <ipps.h>
#include <ippcc.h>
#include "image.h"
#include "iir.h"
#include "resize.h"
#include "alpha.h"
#include "fft.h"
#include "window.h"
#include "pcm.h"
#include "stats.h"

/* naming convension stuff:

nsutil_<area>_FunctionName_variant()

U8 - uint8_t data

destination pointer comes first
*/

int nsutil_image_CopyFlipped_U8(uint8_t *destination_image, size_t destination_stride, const uint8_t *source_image, size_t source_stride, uint32_t width, uint32_t height)
{
	IppiSize roi = { width, height };
	ippiMirror_8u_C1R(source_image, source_stride, destination_image, destination_stride, roi, ippAxsHorizontal);
	return 0;
}

int nsutil_image_Copy_U8(uint8_t *destination_image, size_t destination_stride, const uint8_t *source_image, size_t source_stride, uint32_t width, uint32_t height)
{
	IppiSize roi = { width, height };
	ippiCopy_8u_C1R(source_image, source_stride, destination_image, destination_stride, roi);
	return 0;
}

int nsutil_image_Convert_RGB24_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height)
{
	IppiSize roi = { width, height };
	ippiCopy_8u_C3AC4R(source_image, source_stride, (Ipp8u *)destination_image, destination_stride, roi);
	return 0;
}

int nsutil_image_ConvertFlipped_RGB24_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height)
{
#if 1
		IppiSize roi = { width, height };
	ippiCopy_8u_C3AC4R(source_image, source_stride, (Ipp8u *)destination_image + destination_stride*(height-1), -destination_stride, roi);
	return 0;
#else
		IppiSize roi = { width, 1 };
		uint8_t *dest = (uint8_t *)destination_image;
		source_image += source_stride * (height-1);
		for (uint32_t i = 0;i != height;i++)
		{
			ippiCopy_8u_C3AC4R(source_image, source_stride, (Ipp8u *)dest, destination_stride, roi);
			source_image -= source_stride;
			dest += destination_stride;
		}

	return 0;
#endif
}

/*
struct nsutil_resize_context
{
	int buffer_size;
	void *buffer;
};
*/

static inline void MakeIppiRect(IppiRect &ippi_rect, const nsutil_rect *rect)
{
	ippi_rect.x = rect->left;
	ippi_rect.y = rect->top;
	ippi_rect.width = rect->right - rect->left;
	ippi_rect.height = rect->bottom - rect->top;
}
/*
int nsutil_resize_Init_RGB(nsutil_resize_t *_context, const nsutil_rect *destination_rect, const nsutil_rect *source_rect, int resize_algorithm)
{
	nsutil_resize_context *context = 0;
	if (_context)
	{
		context = (nsutil_resize_context *)(*_context);
	}
	if (!context)
	{
		context = (nsutil_resize_context *)calloc(1, sizeof(nsutil_resize_context));
	}

	IppiRect srcRoi, dstRoi;
	MakeIppiRect(srcRoi, source_rect);
	MakeIppiRect(dstRoi, destination_rect);
	int buffer_size=0;
	ippiResizeGetBufSize(srcRoi, dstRoi, 1, resize_algorithm, &buffer_size);
	if (buffer_size > context->buffer_size)
	{
		_aligned_free(context->buffer);
		context->buffer = _aligned_malloc(buffer_size, 32);
		context->buffer_size = buffer_size;
	}
	return 0;
}

int nsutil_resize_Filter_RGB(nsutil_resize_t _context, void *destination, size_t destination_stride, const nsutil_rect *destination_rect,
														 const void *source, size_t source_stride, int source_width, int source_height, const nsutil_rect *source_rect,
														 double dx, double dy, double x_offset, double y_offset, int resize_algorithm)
{
	nsutil_resize_context *context = (nsutil_resize_context *)_context;

	IppiSize srcSize = { source_width, source_height };
	IppiRect srcRoi, dstRoi;
	MakeIppiRect(srcRoi, source_rect);
	MakeIppiRect(dstRoi, destination_rect);
	ippiResizeSqrPixel_8u_C1R((const Ipp8u *)source, srcSize, source_stride, srcRoi, 
		(Ipp8u *)destination, destination_stride, dstRoi,
		dx, dy, x_offset, y_offset,
		resize_algorithm, (Ipp8u *)context->buffer);
	return 0;
}
*/

int nsutil_alpha_Premultiply_RGB32(void *image, size_t image_stride, int width, int height)
{
	IppiSize roiSize = { width, height };
	ippiAlphaPremul_8u_AC4IR((Ipp8u *)image, image_stride, roiSize);
	return 0;
}

int nsutil_alpha_PremultiplyValue_RGB8(void *image, size_t image_stride, int width, int height, uint8_t alpha)
{
	IppiSize roiSize = { width, height };
	ippiAlphaPremulC_8u_C1IR(alpha, (Ipp8u *)image, image_stride, roiSize);
	return 0;
}

struct nsutil_fft_struct_F32R
{
	IppsFFTSpec_R_32f *fft_spec;
	Ipp8u *work_buffer;
};
int nsutil_fft_Create_F32R(nsutil_fft_t *fft, int order, int accuracy)
{
	nsutil_fft_struct_F32R *ippi_fft = (nsutil_fft_struct_F32R *)calloc(1, sizeof(nsutil_fft_struct_F32R));
	
	ippsFFTInitAlloc_R_32f(&ippi_fft->fft_spec, order, IPP_FFT_NODIV_BY_ANY, (IppHintAlgorithm)accuracy);
	int work_buffer_size;
	ippsFFTGetBufSize_R_32f(ippi_fft->fft_spec, &work_buffer_size);
	ippi_fft->work_buffer = (Ipp8u *)_aligned_malloc(work_buffer_size, 32);
	*fft = ippi_fft;
	return 0;
}

int nsutil_fft_Forward_F32R_IP(nsutil_fft_t fft, float *signal)
{
	nsutil_fft_struct_F32R *ippi_fft = (nsutil_fft_struct_F32R *)fft;
	ippsFFTFwd_RToPerm_32f_I(signal, ippi_fft->fft_spec, ippi_fft->work_buffer);
	return 0;
}

int nsutil_fft_Destroy_F32R(nsutil_fft_t fft)
{
	nsutil_fft_struct_F32R *ippi_fft = (nsutil_fft_struct_F32R *)fft;
	ippsFFTFree_R_32f(ippi_fft->fft_spec);
	_aligned_free(ippi_fft->work_buffer);
	free(ippi_fft);
	return 0;
}

int nsutil_window_Hann_F32_IP(float *signal, size_t number_of_samples)
{
	ippsWinHann_32f_I(signal, number_of_samples);
	return 0;
}

int nsutil_window_FillHann_F32_IP(float *window, size_t number_of_samples)
{
	ippsSet_32f(1.0f, window, number_of_samples);
	ippsWinHann_32f_I(window, number_of_samples);
	return 0;
}

int nsutil_window_FillKaiser_F32_IP(float *window, float alpha, size_t number_of_samples)
{
	ippsSet_32f(1.0f, window, number_of_samples);
	ippsWinKaiser_32f_I(window, number_of_samples, alpha);
	return 0;
}

int nsutil_window_Multiply_F32_IP(float *signal, const float *window, size_t number_of_samples)
{
	ippsMul_32f_I(window, signal, number_of_samples);
	return 0;
}

int nsutil_image_Recolor_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, uint32_t R, uint32_t G, uint32_t B, uint32_t width, uint32_t height)
{
	const Ipp32f twist[3][4] = 
	{
		{(float)B / 65536.0f, 0, 0, 0},
		{0, (float)G / 65536.0f, 0, 0},
		{0, 0, (float)R / 65536.0f, 0}
	};
	IppiSize roiSize = { width, height };
	ippiColorTwist32f_8u_AC4IR((Ipp8u *)destination_image, destination_stride, roiSize, twist);
	return 0;
}

int nsutil_image_Palette_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height, const RGB32 *palette)
{
	IppiSize roiSize = { width, height };
	ippiLUTPalette_8u32u_C1R(source_image, source_stride, (Ipp32u *)destination_image, destination_stride, roiSize, (const Ipp32u *)palette, 8);
	return 0;
}

int nsutil_image_PaletteFlipped_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height, const RGB32 *palette)
{
#if 1
	IppiSize roiSize = { width, height };
	ippiLUTPalette_8u32u_C1R(source_image, source_stride, (Ipp32u *) ((uint8_t *)destination_image + destination_stride*(height-1)), -destination_stride, roiSize, (const Ipp32u *)palette, 8);
	return 0;
#else
	IppiSize roiSize = { width, 1 };
	uint8_t *dest = (uint8_t *)destination_image;
	source_image += source_stride * (height-1);
	for (uint32_t i = 0;i != height;i++)
	{
		ippiLUTPalette_8u32u_C1R(source_image, source_stride, (Ipp32u *)dest, destination_stride, roiSize, (const Ipp32u *)palette, 8);
		source_image -= source_stride;
		dest += destination_stride;
	}
	return 0;
#endif
}

int nsutil_image_FillRectAlpha_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, uint32_t width, uint32_t height, RGB32 color, int alpha)
{

	IppiSize roiSize = { width, height };

	if (alpha == 255)
	{
		uint8_t c[3] = { 
			(color&0xFF0000)  >> 16, 
			(color&0xFF00) >> 8,
			(color&0xFF)
		};
		ippiSet_8u_AC4R(c, (uint8_t *)destination_image, destination_stride, roiSize);
	}
	else
	{
		uint8_t c[3] = { 
			((color&0xFF0000) * alpha / 255) >> 16, 
			((color&0xFF00) * alpha / 255) >> 8,
			((color&0xFF) * alpha / 255)
		};

		ippiAlphaPremulC_8u_AC4IR((255-alpha), (uint8_t *)destination_image, destination_stride, roiSize);
		ippiAddC_8u_AC4IRSfs(c, (uint8_t *)destination_image, destination_stride, roiSize, 0);
	}
	return 0;
}

int nsutil_stats_RMS_F32(const float *buffer, size_t num_samples, float *rms)
{
	ippsNorm_L2_32f(buffer, num_samples, rms);
	return 0;
}

int nsutil_image_Convert_YUV420_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, uint32_t width, uint32_t height, const uint8_t *planes[3], const size_t strides[3])
{
	IppiSize roiSize = { width, height };
	int int_strides[3] = {strides[0], strides[1], strides[2] };
	ippiYUV420ToRGB_8u_P3AC4R(planes, int_strides, (Ipp8u *)destination_image, destination_stride, roiSize);
	return 0;
}