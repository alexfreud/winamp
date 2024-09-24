
/*!
*************************************************************************************
* \file mc_prediction.c
*
* \brief
*    Functions for motion compensated prediction
*
* \author
*      Main contributors (see contributors.h for copyright, 
*                         address and affiliation details)
*      - Alexis Michael Tourapis  <alexismt@ieee.org>
*
*************************************************************************************
*/
#include "global.h"
#include "block.h"
#include "mc_prediction.h"
#include "mbuffer.h"
#include "mb_access.h"
#include "macroblock.h"
#include "memalloc.h"
#include "optim.h"
#include <emmintrin.h>

static const int COEF[6] = { 1, -5, 20, 20, -5, 1 };
/*!
************************************************************************
* \brief
*    block single list prediction
************************************************************************
*/
static inline void mc_prediction(h264_imgpel_macroblock_t mb_pred,
																 int joff,
																 int ver_block_size, 
																 int hor_block_size,
																 int ioff,
																 const h264_imgpel_macroblock_t block)
{
	int jj;

	if (hor_block_size == MB_BLOCK_SIZE)
	{
		memcpy(&(mb_pred[joff][ioff]), &(block[0][0]), hor_block_size * ver_block_size * sizeof(imgpel));
	}
	else
	{
		h264_imgpel_macroblock_row_t *dest = (h264_imgpel_macroblock_row_t *)(mb_pred[joff]);
		for(jj = 0; jj < ver_block_size; jj++)
		{
			memcpy(&dest[jj][ioff], &(block[jj][0]), hor_block_size * sizeof(imgpel));
		}
	}
}

/*!
************************************************************************
* \brief
*    block single list weighted prediction
************************************************************************
*/
static inline void weighted_mc_prediction(h264_imgpel_macroblock_row_t *mb_pred,
																					int ver_block_size, 
																					int hor_block_size,
																					int wp_scale,
																					int wp_offset,
																					int weight_denom)
{
#ifdef H264_IPP
	IppiSize roi = {hor_block_size, ver_block_size};
	ippiUniDirWeightBlock_H264_8u_C1IR(mb_pred[0], sizeof(mb_pred[0]), weight_denom, wp_scale, wp_offset, roi);
#else
	int ii, jj;
	if (weight_denom > 0)
	{
		for(jj=0;jj<ver_block_size;jj++)
		{
			imgpel *row = mb_pred[jj];
			const imgpel *b0 = row;

			for(ii=0;ii<hor_block_size;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
	}
	else
	{
		weight_denom = -weight_denom;
		for(jj=0;jj<ver_block_size;jj++)
		{
			imgpel *row = mb_pred[jj];
			const imgpel *b0 = row;

			for(ii=0;ii<hor_block_size;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
	}
#endif
}


void weighted_mc_prediction16x16_sse2(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
		int jj;

		__m128i xmm_zero = _mm_setzero_si128();
		__m128i xmm_scale = _mm_set1_epi16(wp_scale);
		__m128i xmm_offset = _mm_set1_epi16(wp_offset);
		if (weight_denom > 0)
		{
			__m128i xmm_shift = _mm_cvtsi32_si128(weight_denom);
			__m128i xmm_add = _mm_set1_epi16((1<<(weight_denom-1)));

			for(jj = 0; jj < 16; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale);
				b0_low = _mm_add_epi16(b0_low, xmm_add);
				b0_high = _mm_add_epi16(b0_high, xmm_add);
				b0_low = _mm_sra_epi16(b0_low, xmm_shift);
				b0_high = _mm_sra_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);
				// (x + (1 << (a-1) )) >> a;
				//row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_pos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
			}
		}
		else
		{

			__m128i xmm_shift = _mm_cvtsi32_si128(-weight_denom);
			for(jj = 0; jj < 16; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale);
				b0_low = _mm_sll_epi16(b0_low, xmm_shift);
				b0_high = _mm_sll_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);
				//(x << a);
				//			row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_nonpos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
			}
		}
}

#ifdef H264_IPP
void weighted_mc_prediction16x16_ipp(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
	IppiSize roi = {16, 16};
	ippiUniDirWeightBlock_H264_8u_C1IR(mb_pred[0], sizeof(mb_pred[0]), weight_denom, wp_scale, wp_offset, roi);
}
#endif

void weighted_mc_prediction16x16_c(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
	int ii, jj;
		if (weight_denom > 0)
		{
		for(jj=0;jj<16;jj++)
		{
		imgpel *row = mb_pred[jj];
		const imgpel *b0 = row;

		for(ii=0;ii<16;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
		}
		else
		{
		weight_denom = -weight_denom;
		for(jj=0;jj<16;jj++)
		{
		imgpel *row = mb_pred[jj];
		const imgpel *b0 = row;

		for(ii=0;ii<16;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
		}
}


/* 16x8 */
void weighted_mc_prediction16x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
		int jj;

		__m128i xmm_zero = _mm_setzero_si128();
		__m128i xmm_scale = _mm_set1_epi16(wp_scale);
		__m128i xmm_offset = _mm_set1_epi16(wp_offset);
		if (weight_denom > 0)
		{
			__m128i xmm_shift = _mm_cvtsi32_si128(weight_denom);
			__m128i xmm_add = _mm_set1_epi16((1<<(weight_denom-1)));

			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale);
				b0_low = _mm_add_epi16(b0_low, xmm_add);
				b0_high = _mm_add_epi16(b0_high, xmm_add);
				b0_low = _mm_sra_epi16(b0_low, xmm_shift);
				b0_high = _mm_sra_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);
				// (x + (1 << (a-1) )) >> a;
				//row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_pos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
			}
		}
		else
		{

			__m128i xmm_shift = _mm_cvtsi32_si128(-weight_denom);
			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale);
				b0_low = _mm_sll_epi16(b0_low, xmm_shift);
				b0_high = _mm_sll_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);
				//(x << a);
				//			row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_nonpos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
			}
		}
}

#ifdef H264_IPP
void weighted_mc_prediction16x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
	IppiSize roi = {16, 8};
	ippiUniDirWeightBlock_H264_8u_C1IR(mb_pred[0], sizeof(mb_pred[0]), weight_denom, wp_scale, wp_offset, roi);
}
#endif

void weighted_mc_prediction16x8_c(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
	int ii, jj;
		if (weight_denom > 0)
		{
		for(jj=0;jj<8;jj++)
		{
		imgpel *row = mb_pred[jj];
		const imgpel *b0 = row;

		for(ii=0;ii<16;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
		}
		else
		{
		weight_denom = -weight_denom;
		for(jj=0;jj<8;jj++)
		{
		imgpel *row = mb_pred[jj];
		const imgpel *b0 = row;

		for(ii=0;ii<16;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
		}
}




#define LOAD_LINE_EPI16(reg, ptr) { reg = _mm_loadl_epi64((__m128i *)(ptr));	reg = _mm_unpacklo_epi8(reg, xmm_zero); }
void weighted_mc_prediction8x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
		int jj;

		__m128i xmm_zero = _mm_setzero_si128();
		__m128i xmm_scale = _mm_set1_epi16(wp_scale);
		__m128i xmm_offset = _mm_set1_epi16(wp_offset);
		if (weight_denom > 0)
		{
			__m128i xmm_shift = _mm_cvtsi32_si128(weight_denom);
			__m128i xmm_add = _mm_set1_epi16((1<<(weight_denom-1)));

			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0;
				LOAD_LINE_EPI16(b0, (__m128i *)mb_pred[jj]);
				b0 = _mm_mullo_epi16(b0, xmm_scale);
				b0 = _mm_add_epi16(b0, xmm_add);
				b0 = _mm_sra_epi16(b0, xmm_shift);
				b0 = _mm_add_epi16(b0, xmm_offset);

				b0 = _mm_packus_epi16(b0, b0); // convert back to epi8
				_mm_storel_epi64((__m128i *)mb_pred[jj], b0);
				// (x + (1 << (a-1) )) >> a;
				//row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_pos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
			}
		}
		else
		{

			__m128i xmm_shift = _mm_cvtsi32_si128(-weight_denom);
			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0;
				LOAD_LINE_EPI16(b0, (__m128i *)mb_pred[jj]);
				b0 = _mm_mullo_epi16(b0, xmm_scale);
				b0 = _mm_sll_epi16(b0, xmm_shift);
				b0 = _mm_add_epi16(b0, xmm_offset);

				b0 = _mm_packus_epi16(b0, b0); // convert back to epi8
				_mm_storel_epi64((__m128i *)mb_pred[jj], b0);
				//(x << a);
				//			row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_nonpos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
			}
		}
}

#ifdef H264_IPP
void weighted_mc_prediction8x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
			IppiSize roi = {8, 8};
		ippiUniDirWeightBlock_H264_8u_C1IR(mb_pred[0], sizeof(mb_pred[0]), weight_denom, wp_scale, wp_offset, roi);
}
#endif

void weighted_mc_prediction8x8_c(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom)
{
		int ii, jj;
		if (weight_denom > 0)
		{
		for(jj=0;jj<8;jj++)
		{
		imgpel *row = mb_pred[jj];
		const imgpel *b0 = row;

		for(ii=0;ii<8;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
		}
		else
		{
		weight_denom = -weight_denom;
		for(jj=0;jj<8;jj++)
		{
		imgpel *row = mb_pred[jj];
		const imgpel *b0 = row;

		for(ii=0;ii<8;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale * b0[ii]), weight_denom)  + wp_offset ));
		}
		}
}



/*!
************************************************************************
* \brief
*    block biprediction
************************************************************************
*/
static inline void bi_prediction(h264_imgpel_macroblock_row_t *mb_pred,
																 //int joff,
																 const h264_imgpel_macroblock_t block_l0, 
																 //const h264_imgpel_macroblock_t block_l1,
																 int ver_block_size, 
																 int hor_block_size
																 //int ioff
																 )
{

#ifdef H264_IPP
	ippiInterpolateBlock_H264_8u_P2P1R(block_l0[0], mb_pred[0], mb_pred[0], hor_block_size, ver_block_size, sizeof(mb_pred[0]));
#else
	int ii, jj;

	for(jj = 0;jj < ver_block_size;jj++)
	{
		const imgpel *b0  = block_l0[jj];
		imgpel *row = mb_pred[jj];
		const imgpel *b1  = row;

		for(ii = 0; ii < hor_block_size;ii++)
			row[ii] = (imgpel) rshift_rnd_sf((b0[ii] + b1[ii]), 1);
	}
#endif
}

static void bi_prediction4x4_mmx(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0)
{
	int jj;
	__m64 b0, b1;
	__m64 mmx_zero	= _mm_setzero_si64();
	__m64 mmx_one = _mm_set1_pi16(1);

	for(jj = 0;jj < 4;jj++)
	{
		b0 = _mm_cvtsi32_si64(*(int *)(&block_l0[jj]));        
		b0 = _mm_unpacklo_pi8(b0, mmx_zero);
		b1 = _mm_cvtsi32_si64(*(int *)(& mb_pred[jj]));
		b1 = _mm_unpacklo_pi8(b1, mmx_zero);
		b0 = _mm_add_pi16(b0, b1);
		b0 = _mm_add_pi16(b0, mmx_one);
		b0 = _mm_srai_pi16(b0, 1);
		b0 = _mm_packs_pu16(b0, b0); 
		*(int *)(&mb_pred[jj]) = _mm_cvtsi64_si32(b0);
	}
}

void bi_prediction8x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0)
{
	int jj;
	__m128i b0, b1;
	__m128i xmm_zero	= _mm_setzero_si128();
	__m128i xmm_one = _mm_set1_epi16(1);

	for(jj = 0;jj < 8;jj++)
	{
		LOAD_LINE_EPI16(b0, (__m128i *)block_l0[jj]);
		LOAD_LINE_EPI16(b1, (__m128i *)mb_pred[jj]);
		b0 = _mm_add_epi16(b0, b1);
		b0 = _mm_add_epi16(b0, xmm_one);
		b0 = _mm_srai_epi16(b0, 1);
		b0 = _mm_packus_epi16(b0, b0); 
		_mm_storel_epi64((__m128i *)mb_pred[jj], b0);
	}
}

#ifdef H264_IPP
void bi_prediction8x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0)
{
	ippiInterpolateBlock_H264_8u_P2P1R(block_l0[0], mb_pred[0], mb_pred[0], 8, 8, sizeof(mb_pred[0]));
}
#endif
/*!
************************************************************************
* \brief
*    block weighted biprediction
************************************************************************
*/
static inline void weighted_bi_prediction(h264_imgpel_macroblock_row_t *mb_pred,
																					const h264_imgpel_macroblock_t block_l0, 
																					int ver_block_size,  int hor_block_size,
																					int wp_scale_l0, int wp_scale_l1,
																					int wp_offset, int weight_denom)
{
#ifdef H264_IPP
	IppiSize roi = {hor_block_size, ver_block_size};
	ippiWeightedAverage_H264_8u_C1IR(block_l0[0], mb_pred[0], sizeof(mb_pred[0]), wp_scale_l0, wp_scale_l1, weight_denom, wp_offset, roi);
#else
	int ii, jj;

	if (weight_denom > 0)
	{
		for(jj = 0; jj < ver_block_size; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<hor_block_size;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
	else
	{
		weight_denom = -weight_denom;
		for(jj = 0; jj < ver_block_size; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<hor_block_size;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
#endif
}

void weighted_bi_prediction8x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
			int jj;

		__m128i xmm_zero = _mm_setzero_si128();
		__m128i xmm_scale_l0 = _mm_set1_epi16(wp_scale_l0);
		__m128i xmm_scale_l1 = _mm_set1_epi16(wp_scale_l1);	
		__m128i xmm_offset = _mm_set1_epi16(wp_offset);
		if (weight_denom > 0)
		{
			__m128i xmm_shift = _mm_cvtsi32_si128(weight_denom);
			__m128i xmm_add = _mm_set1_epi16((1<<(weight_denom-1)));

			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0, b1;
				LOAD_LINE_EPI16(b0, (__m128i *)block_l0[jj]);
				LOAD_LINE_EPI16(b1, (__m128i *)mb_pred[jj]);

				b0 = _mm_mullo_epi16(b0, xmm_scale_l0);
				b1 = _mm_mullo_epi16(b1, xmm_scale_l1);
				b0 = _mm_add_epi16(b0, b1);
				b0 = _mm_add_epi16(b0, xmm_add);
				b0 = _mm_sra_epi16(b0, xmm_shift);
				b0 = _mm_add_epi16(b0, xmm_offset);

				b0 = _mm_packus_epi16(b0, b0); // convert back to epi8
				_mm_storel_epi64((__m128i *)mb_pred[jj], b0);
				// (x + (1 << (a-1) )) >> a;
				// row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
			}
		}
		else
		{

			__m128i xmm_shift = _mm_cvtsi32_si128(-weight_denom);
			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0, b1;
				LOAD_LINE_EPI16(b0, (__m128i *)block_l0[jj]);
				LOAD_LINE_EPI16(b1, (__m128i *)mb_pred[jj]);

				b0 = _mm_mullo_epi16(b0, xmm_scale_l0);
				b1 = _mm_mullo_epi16(b1, xmm_scale_l1);
				b0 = _mm_add_epi16(b0, b1);
				b0 = _mm_sll_epi16(b0, xmm_shift);
				b0 = _mm_add_epi16(b0, xmm_offset);

				b0 = _mm_packus_epi16(b0, b0); // convert back to epi8
				_mm_storel_epi64((__m128i *)mb_pred[jj], b0);
				//(x << a);
				//		row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
			}
		}
}

#ifdef H264_IPP
void weighted_bi_prediction8x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
	IppiSize roi = {8, 8};
	ippiWeightedAverage_H264_8u_C1IR(block_l0[0], mb_pred[0], sizeof(mb_pred[0]), wp_scale_l0, wp_scale_l1, weight_denom, wp_offset, roi);
}
#endif

void weighted_bi_prediction8x8_c(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
	int ii, jj;

		if (weight_denom > 0)
		{
		for(jj = 0; jj < 8; jj++)
		{
		const imgpel *b0  = block_l0[jj];
		imgpel *row = mb_pred[jj];
		const imgpel *b1  = row;

		for(ii=0;ii<8;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
		}
		else
		{
		weight_denom = -weight_denom;
		for(jj = 0; jj < 8; jj++)
		{
		const imgpel *b0  = block_l0[jj];
		imgpel *row = mb_pred[jj];
		const imgpel *b1  = row;

		for(ii=0;ii<8;ii++)
		row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
		}
}

#if defined(_DEBUG) || !defined(_M_IX86)
static inline void weighted_bi_prediction4x4(h264_imgpel_macroblock_row_t *mb_pred,
																						 const h264_imgpel_macroblock_t block_l0, 
																						 uint16_t wp_scale_l0,
																						 uint16_t wp_scale_l1,
																						 uint16_t wp_offset,
																						 int weight_denom)
{
#ifdef H264_IPP
	IppiSize roi = {4, 4};
	ippiWeightedAverage_H264_8u_C1IR(block_l0[0], mb_pred[0], sizeof(mb_pred[0]), wp_scale_l0, wp_scale_l1, weight_denom, wp_offset, roi);
#else
	int ii, jj;

	if (weight_denom > 0)
	{
		for(jj = 0; jj < 4; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<4;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
	else
	{
		weight_denom = -weight_denom;
		for(jj = 0; jj < 4; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<4;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
#endif
}
#else
extern void weighted_bi_prediction4x4(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
#endif

void weighted_bi_prediction16x16_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
		int jj;

		__m128i xmm_zero = _mm_setzero_si128();
		__m128i xmm_scale_l0 = _mm_set1_epi16(wp_scale_l0);
		__m128i xmm_scale_l1 = _mm_set1_epi16(wp_scale_l1);	
		__m128i xmm_offset = _mm_set1_epi16(wp_offset);
		if (weight_denom > 0)
		{
			__m128i xmm_shift = _mm_cvtsi32_si128(weight_denom);
			__m128i xmm_add = _mm_set1_epi16((1<<(weight_denom-1)));

			for(jj = 0; jj < 16; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)block_l0[jj]);
				__m128i b1  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				__m128i b1_low = _mm_unpacklo_epi8(b1, xmm_zero);
				__m128i b1_high = _mm_unpackhi_epi8(b1, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale_l0);
				b1_low = _mm_mullo_epi16(b1_low, xmm_scale_l1);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale_l0);
				b1_high = _mm_mullo_epi16(b1_high, xmm_scale_l1);
				b0_low = _mm_add_epi16(b0_low, b1_low);
				b0_high = _mm_add_epi16(b0_high, b1_high);
				b0_low = _mm_add_epi16(b0_low, xmm_add);
				b0_high = _mm_add_epi16(b0_high, xmm_add);
				b0_low = _mm_sra_epi16(b0_low, xmm_shift);
				b0_high = _mm_sra_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);
				// (x + (1 << (a-1) )) >> a;
				// row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
			}
		}
		else
		{

			__m128i xmm_shift = _mm_cvtsi32_si128(-weight_denom);
			for(jj = 0; jj < 16; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)block_l0[jj]);
				__m128i b1  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				__m128i b1_low = _mm_unpacklo_epi8(b1, xmm_zero);
				__m128i b1_high = _mm_unpackhi_epi8(b1, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale_l0);
				b1_low = _mm_mullo_epi16(b1_low, xmm_scale_l1);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale_l0);
				b1_high = _mm_mullo_epi16(b1_high, xmm_scale_l1);
				b0_low = _mm_add_epi16(b0_low, b1_low);
				b0_high = _mm_add_epi16(b0_high, b1_high);
				b0_low = _mm_sll_epi16(b0_low, xmm_shift);
				b0_high = _mm_sll_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);

				//(x << a);
				//		row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
			}
		}
}

#ifdef H264_IPP
void weighted_bi_prediction16x16_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
	IppiSize roi = {16, 16};
	ippiWeightedAverage_H264_8u_C1IR(block_l0[0], mb_pred[0], sizeof(mb_pred[0]), wp_scale_l0, wp_scale_l1, weight_denom, wp_offset, roi);
}
#endif

void weighted_bi_prediction16x16_c(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
	int ii, jj;

	if (weight_denom > 0)
	{
		for(jj = 0; jj < 16; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<16;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
	else
	{
		weight_denom = -weight_denom;
		for(jj = 0; jj < 16; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<16;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
}

/* 16x8 */
void weighted_bi_prediction16x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
		int jj;

		__m128i xmm_zero = _mm_setzero_si128();
		__m128i xmm_scale_l0 = _mm_set1_epi16(wp_scale_l0);
		__m128i xmm_scale_l1 = _mm_set1_epi16(wp_scale_l1);	
		__m128i xmm_offset = _mm_set1_epi16(wp_offset);
		if (weight_denom > 0)
		{
			__m128i xmm_shift = _mm_cvtsi32_si128(weight_denom);
			__m128i xmm_add = _mm_set1_epi16((1<<(weight_denom-1)));

			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)block_l0[jj]);
				__m128i b1  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				__m128i b1_low = _mm_unpacklo_epi8(b1, xmm_zero);
				__m128i b1_high = _mm_unpackhi_epi8(b1, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale_l0);
				b1_low = _mm_mullo_epi16(b1_low, xmm_scale_l1);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale_l0);
				b1_high = _mm_mullo_epi16(b1_high, xmm_scale_l1);
				b0_low = _mm_add_epi16(b0_low, b1_low);
				b0_high = _mm_add_epi16(b0_high, b1_high);
				b0_low = _mm_add_epi16(b0_low, xmm_add);
				b0_high = _mm_add_epi16(b0_high, xmm_add);
				b0_low = _mm_sra_epi16(b0_low, xmm_shift);
				b0_high = _mm_sra_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);
				// (x + (1 << (a-1) )) >> a;
				// row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
			}
		}
		else
		{

			__m128i xmm_shift = _mm_cvtsi32_si128(-weight_denom);
			for(jj = 0; jj < 8; jj++)
			{
				__m128i b0  = _mm_load_si128((__m128i *)block_l0[jj]);
				__m128i b1  = _mm_load_si128((__m128i *)mb_pred[jj]);

				__m128i b0_low = _mm_unpacklo_epi8(b0, xmm_zero);
				__m128i b0_high = _mm_unpackhi_epi8(b0, xmm_zero);
				__m128i b1_low = _mm_unpacklo_epi8(b1, xmm_zero);
				__m128i b1_high = _mm_unpackhi_epi8(b1, xmm_zero);
				b0_low = _mm_mullo_epi16(b0_low, xmm_scale_l0);
				b1_low = _mm_mullo_epi16(b1_low, xmm_scale_l1);
				b0_high = _mm_mullo_epi16(b0_high, xmm_scale_l0);
				b1_high = _mm_mullo_epi16(b1_high, xmm_scale_l1);
				b0_low = _mm_add_epi16(b0_low, b1_low);
				b0_high = _mm_add_epi16(b0_high, b1_high);
				b0_low = _mm_sll_epi16(b0_low, xmm_shift);
				b0_high = _mm_sll_epi16(b0_high, xmm_shift);
				b0_low = _mm_add_epi16(b0_low, xmm_offset);
				b0_high = _mm_add_epi16(b0_high, xmm_offset);

				b0_low = _mm_packus_epi16(b0_low, b0_high); // convert back to epi8
				_mm_store_si128((__m128i *)mb_pred[jj], b0_low);

				//(x << a);
				//		row[ii] = (imgpel) iClip1(color_clip, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
			}
		}
}

#ifdef H264_IPP
void weighted_bi_prediction16x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
	IppiSize roi = {16, 8};
	ippiWeightedAverage_H264_8u_C1IR(block_l0[0], mb_pred[0], sizeof(mb_pred[0]), wp_scale_l0, wp_scale_l1, weight_denom, wp_offset, roi);
}
#endif

void weighted_bi_prediction16x8_c(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom)
{
	int ii, jj;

	if (weight_denom > 0)
	{
		for(jj = 0; jj < 8; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<16;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_pos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
	else
	{
		weight_denom = -weight_denom;
		for(jj = 0; jj < 8; jj++)
		{
			const imgpel *b0  = block_l0[jj];
			imgpel *row = mb_pred[jj];
			const imgpel *b1  = row;

			for(ii=0;ii<16;ii++)
				row[ii] = (imgpel) iClip1(255, (rshift_rnd_nonpos((wp_scale_l0 * b0[ii] + wp_scale_l1 * b1[ii]), weight_denom) + wp_offset));
		}
	}
}

/*!
************************************************************************
* \brief
*    No reference picture mc
************************************************************************
*/ 
static void get_data_no_ref(h264_imgpel_macroblock_row_t *block, int ver_block_size, int hor_block_size, imgpel med_imgpel_value)
{
	int i, j;
#ifdef _DEBUG
	printf("list[ref_frame] is equal to 'no reference picture' before RAP\n");
#endif

	/* fill the block with sample value middle value */
	for (j = 0; j < ver_block_size; j++)
		for (i = 0; i < hor_block_size; i++)
			block[j][i] = med_imgpel_value;
}

/*!
************************************************************************
* \brief
*    Interpolation of 1/4 subpixel
************************************************************************
*/ 
void get_block_luma(Macroblock *currMB, ColorPlane pl, StorablePicture *curr_ref, int x_pos, int y_pos, const short *motion_vector, int hor_block_size, int ver_block_size, h264_imgpel_macroblock_row_t *block)
{  
	VideoParameters *p_Vid = currMB->p_Vid;

	if (curr_ref == p_Vid->no_reference_picture && p_Vid->framepoc < p_Vid->recovery_poc)
	{
		get_data_no_ref(block, ver_block_size, hor_block_size, (imgpel) p_Vid->dc_pred_value_comp[pl]);
	}
	else
	{
		IppVCInterpolateBlock_8u block_data;
		StorablePicture *dec_picture = p_Vid->dec_picture;
		VideoImage *cur_imgY = curr_ref->imgY;

		if (IS_INDEPENDENT(p_Vid))
		{
			switch(p_Vid->colour_plane_id )
			{
			case    1:
				cur_imgY = curr_ref->imgUV[0];
				break;
			case    2:
				cur_imgY = curr_ref->imgUV[1];
				break;
			}
		}
		else if (pl!=PLANE_Y)
		{
			cur_imgY = curr_ref->imgUV[pl-1]; 
		}

		block_data.pSrc[0] = cur_imgY->base_address;
		block_data.srcStep = cur_imgY->stride;
		block_data.pDst[0] = block[0];
		block_data.dstStep = sizeof(block[0]);
		block_data.sizeFrame.width = dec_picture->size_x;
		block_data.sizeFrame.height = (dec_picture->motion.mb_field[currMB->mbAddrX]) ? (dec_picture->size_y >> 1): dec_picture->size_y;
		block_data.sizeBlock.width = hor_block_size;
		block_data.sizeBlock.height = ver_block_size;
		block_data.pointBlockPos.x = x_pos << 2;
		block_data.pointBlockPos.y = y_pos << 2;
		block_data.pointVector.x = motion_vector[0];
		block_data.pointVector.y = motion_vector[1];
		ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);

	}
}

/*!
************************************************************************
* \brief
*    Chroma (0,0)
************************************************************************
*/ 
static void get_chroma_00(h264_imgpel_macroblock_t block, const VideoImage *image, int ver_block_size, int hor_block_size, int x_pos, int y_pos)
{
	ptrdiff_t src_stride = image->stride; // in case the compiler doesn't optimize this
	imgpel *src = image->base_address + src_stride * y_pos + x_pos;

	int j, i;
	switch(hor_block_size) // basically just unrolling this
	{
	case 16:
		for (j = 0; j < ver_block_size; j++)
		{  
			imgpel *row = block[j];
			for (i = 0; i < 16; i++)
			{
				row[i] = src[i];
			}
			src+=src_stride;
		}
		break;
	case 8:
		for (j = 0; j < ver_block_size; j++)
		{  
			imgpel *row = block[j];
			for (i = 0; i < 8; i++)
			{
				row[i] = src[i];
			}
			src+=src_stride;
		}
		break;
	case 4:
		for (j = 0; j < ver_block_size; j++)
		{  
			imgpel *row = block[j];
			for (i = 0; i < 4; i++)
			{
				row[i] = src[i];
			}
			src+=src_stride;
		}
		break;
	case 2:
		for (j = 0; j < ver_block_size; j++)
		{  
			imgpel *row = block[j];
			for (i = 0; i < 2; i++)
			{
				row[i] = src[i];
			}
			src+=src_stride;
		}
		break;
	default: //degenerate case
		for (j = 0; j < ver_block_size; j++)
		{  
			imgpel *row = block[j];
			for (i = 0; i < hor_block_size; i++)
			{
				row[i] = src[i];
			}
			src+=src_stride;
		}
		break;
	}
}

static void get_block_chroma(Macroblock *currMB, StorablePicture *curr_ref, int x_pos, int y_pos, const short *motion_vector, int hor_block_size, int ver_block_size, h264_imgpel_macroblock_row_t *block0, h264_imgpel_macroblock_row_t *block1, int ioff, int joff)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	if (curr_ref == p_Vid->no_reference_picture && p_Vid->framepoc < p_Vid->recovery_poc)
	{
		get_data_no_ref(block0, ver_block_size, hor_block_size, (imgpel) p_Vid->dc_pred_value_comp[1]);
		get_data_no_ref(block1, ver_block_size, hor_block_size, (imgpel) p_Vid->dc_pred_value_comp[2]);
	}
	else
	{
		StorablePicture *dec_picture = p_Vid->dec_picture;
		IppVCInterpolateBlock_8u block_data;

		block_data.pSrc[0] = curr_ref->imgUV[0]->base_address;
		block_data.pSrc[1] = curr_ref->imgUV[1]->base_address;
		block_data.srcStep = curr_ref->imgUV[0]->stride;
		block_data.pDst[0] = &block0[joff][ioff];
		block_data.pDst[1] = &block1[joff][ioff];
		block_data.dstStep = sizeof(block0[0]);
		block_data.sizeFrame.width = dec_picture->size_x_cr;
		block_data.sizeFrame.height = (dec_picture->motion.mb_field[currMB->mbAddrX]) ? (dec_picture->size_y_cr >> 1): dec_picture->size_y_cr;
		block_data.sizeBlock.width = hor_block_size;
		block_data.sizeBlock.height = ver_block_size;
		if (dec_picture->chroma_format_idc == YUV444)
		{
			block_data.pointBlockPos.x = x_pos;
			block_data.pointVector.x = motion_vector[0] << 1; 
		}
		else
		{
			block_data.pointBlockPos.x = x_pos<<1;
			block_data.pointVector.x = motion_vector[0]; 
		}
		if (dec_picture->chroma_format_idc == YUV420)
		{
			block_data.pointVector.y = motion_vector[1];
			block_data.pointBlockPos.y = y_pos<<1;
		}
		else
		{
			block_data.pointBlockPos.y = y_pos;
			block_data.pointVector.y = motion_vector[1] << 1;
		}

		ippiInterpolateChromaBlock_H264_8u_P2R(&block_data);


	}
}


void intra_cr_decoding(Macroblock *currMB, int yuv)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	int uv;
	int b8,b4;
	int ioff, joff;
	// TODO: fix 4x4 lossless

	for(uv = 0; uv < 2; uv++)
	{
		int pl = uv + 1;
		const h264_short_block_t *blocks = currSlice->cof4[pl];
		const h264_imgpel_macroblock_row_t *mb_pred=currSlice->mb_pred[pl];
		h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];

		intrapred_chroma(currMB, uv);

		if ((!(currMB->mb_type == SI4MB) && (currMB->cbp >> 4)) )
		{
			if (yuv == YUV420-1)
			{
				opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
				opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);
				opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
				opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);
				copy_image_data_8x8_stride(dec_picture->imgUV[uv], currMB->pix_c_x, currMB->pix_c_y, mb_rec);
			}
			else
			{
				for (b8 = 0; b8 < (p_Vid->num_uv_blocks); b8++)
				{
					for(b4 = 0; b4 < 4; b4++)
					{
						joff = subblk_offset_y[yuv][b8][b4];          
						ioff = subblk_offset_x[yuv][b8][b4];          
			
						opt_itrans4x4(blocks[cof4_pos_to_subblock[joff>>2][ioff>>2]], mb_pred, mb_rec, ioff, joff);

						copy_image_data_4x4_stride(dec_picture->imgUV[uv], currMB->pix_c_x + ioff, currMB->pix_c_y + joff, mb_rec, ioff, joff);
					}
				}
			}
		}	
		else if (currMB->mb_type == SI4MB)
		{
			itrans_sp_cr(currMB, uv);

			opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
			copy_image_data_4x4_stride(dec_picture->imgUV[uv], currMB->pix_c_x + 0, currMB->pix_c_y + 0, mb_rec,  0, 0);
			opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);
			copy_image_data_4x4_stride(dec_picture->imgUV[uv], currMB->pix_c_x + 4, currMB->pix_c_y + 0, mb_rec,  4, 0);
			opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
			copy_image_data_4x4_stride(dec_picture->imgUV[uv], currMB->pix_c_x + 0, currMB->pix_c_y + 4, mb_rec,  0, 4);
			opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);
			copy_image_data_4x4_stride(dec_picture->imgUV[uv], currMB->pix_c_x + 4, currMB->pix_c_y + 4, mb_rec,  4, 4);
		}
		else
		{
			if (yuv == YUV420-1)
			{
				copy_image_data_8x8_stride(dec_picture->imgUV[uv], currMB->pix_c_x, currMB->pix_c_y, mb_pred);
			}
			else
			{
				for (b8 = 0; b8 < (p_Vid->num_uv_blocks); b8++)
				{
					for(b4 = 0; b4 < 4; b4++)
					{
						joff = subblk_offset_y[yuv][b8][b4];
						ioff = subblk_offset_x[yuv][b8][b4];          

						copy_image_data_4x4_stride(dec_picture->imgUV[uv], currMB->pix_c_x + ioff, currMB->pix_c_y + joff, mb_pred,  ioff, joff);
					}
				}
			}
		}
	}
}

void prepare_direct_params(Macroblock *currMB, StorablePicture *dec_picture, short pmvl0[2], short pmvl1[2],char *l0_rFrame, char *l1_rFrame)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;
	char l0_rFrameL, l0_rFrameU, l0_rFrameUR;
	char l1_rFrameL, l1_rFrameU, l1_rFrameUR;
	PicMotionParams *motion = &dec_picture->motion;

	PixelPos mb[4];

	get_neighbors0016(currMB, mb);

	if (!currSlice->mb_aff_frame_flag)
	{
		l0_rFrameL  = (char) (mb[0].available ? motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx : -1);
		l0_rFrameU  = (char) (mb[1].available ? motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx : -1);
		l0_rFrameUR = (char) (mb[2].available ? motion->motion[LIST_0][mb[2].pos_y][mb[2].pos_x].ref_idx : -1);

		l1_rFrameL  = (char) (mb[0].available ? motion->motion[LIST_1][mb[0].pos_y][mb[0].pos_x].ref_idx : -1);
		l1_rFrameU  = (char) (mb[1].available ? motion->motion[LIST_1][mb[1].pos_y][mb[1].pos_x].ref_idx : -1);
		l1_rFrameUR = (char) (mb[2].available ? motion->motion[LIST_1][mb[2].pos_y][mb[2].pos_x].ref_idx : -1);
	}
	else
	{
		if (currMB->mb_field)
		{
			l0_rFrameL = (char) (mb[0].available 
				? p_Vid->mb_data[mb[0].mb_addr].mb_field  || motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx < 0
				? motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx
			: motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx * 2: -1);

			l0_rFrameU = (char) (mb[1].available 
				? p_Vid->mb_data[mb[1].mb_addr].mb_field || motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx < 0
				? motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx 
			: motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx * 2: -1);

			l0_rFrameUR = (char) (mb[2].available 
				? p_Vid->mb_data[mb[2].mb_addr].mb_field || motion->motion[LIST_0][mb[2].pos_y][mb[2].pos_x].ref_idx < 0 
				? motion->motion[LIST_0][mb[2].pos_y][mb[2].pos_x].ref_idx
			: motion->motion[LIST_0][mb[2].pos_y][mb[2].pos_x].ref_idx * 2: -1);

			l1_rFrameL = (char) (mb[0].available 
				? p_Vid->mb_data[mb[0].mb_addr].mb_field || motion->motion[LIST_1][mb[0].pos_y][mb[0].pos_x].ref_idx  < 0 
				? motion->motion[LIST_1][mb[0].pos_y][mb[0].pos_x].ref_idx 
			: motion->motion[LIST_1][mb[0].pos_y][mb[0].pos_x].ref_idx * 2: -1);

			l1_rFrameU = (char) (mb[1].available 
				? p_Vid->mb_data[mb[1].mb_addr].mb_field || motion->motion[LIST_1][mb[1].pos_y][mb[1].pos_x].ref_idx  < 0 
				? motion->motion[LIST_1][mb[1].pos_y][mb[1].pos_x].ref_idx
			: motion->motion[LIST_1][mb[1].pos_y][mb[1].pos_x].ref_idx * 2: -1);

			l1_rFrameUR = (char) (mb[2].available 
				? p_Vid->mb_data[mb[2].mb_addr].mb_field || motion->motion[LIST_1][mb[2].pos_y][mb[2].pos_x].ref_idx < 0
				? motion->motion[LIST_1][mb[2].pos_y][mb[2].pos_x].ref_idx 
			: motion->motion[LIST_1][mb[2].pos_y][mb[2].pos_x].ref_idx * 2: -1);
		}
		else
		{
			l0_rFrameL = (char) (mb[0].available 
				? p_Vid->mb_data[mb[0].mb_addr].mb_field || motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx  < 0 
				? motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx >> 1 
				: motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx: -1);

			l0_rFrameU = (char) (mb[1].available 
				? p_Vid->mb_data[mb[1].mb_addr].mb_field || motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx < 0 
				? motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx >> 1 
				: motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx : -1);

			l0_rFrameUR = (char) (mb[2].available 
				? p_Vid->mb_data[mb[2].mb_addr].mb_field || motion->motion[LIST_0][mb[2].pos_y][mb[2].pos_x].ref_idx < 0 
				? motion->motion[LIST_0][mb[2].pos_y][mb[2].pos_x].ref_idx >> 1 
				: motion->motion[LIST_0][mb[2].pos_y][mb[2].pos_x].ref_idx : -1);

			l1_rFrameL = (char) (mb[0].available 
				? p_Vid->mb_data[mb[0].mb_addr].mb_field || motion->motion[LIST_1][mb[0].pos_y][mb[0].pos_x].ref_idx < 0 
				? motion->motion[LIST_1][mb[0].pos_y][mb[0].pos_x].ref_idx >> 1 
				: motion->motion[LIST_1][mb[0].pos_y][mb[0].pos_x].ref_idx : -1);

			l1_rFrameU = (char) (mb[1].available 
				? p_Vid->mb_data[mb[1].mb_addr].mb_field || motion->motion[LIST_1][mb[1].pos_y][mb[1].pos_x].ref_idx < 0 
				? motion->motion[LIST_1][mb[1].pos_y][mb[1].pos_x].ref_idx >> 1 
				: motion->motion[LIST_1][mb[1].pos_y][mb[1].pos_x].ref_idx : -1);

			l1_rFrameUR = (char) (mb[2].available 
				? p_Vid->mb_data[mb[2].mb_addr].mb_field || motion->motion[LIST_1][mb[2].pos_y][mb[2].pos_x].ref_idx < 0 
				? motion->motion[LIST_1][mb[2].pos_y][mb[2].pos_x].ref_idx >> 1
				: motion->motion[LIST_1][mb[2].pos_y][mb[2].pos_x].ref_idx : -1);
		}
	}

	*l0_rFrame = (char) ((l0_rFrameL >= 0 && l0_rFrameU >= 0)  ? imin(l0_rFrameL,l0_rFrameU) : imax(l0_rFrameL,l0_rFrameU));
	*l0_rFrame = (char) ((*l0_rFrame >= 0 && l0_rFrameUR >= 0) ? imin(*l0_rFrame,l0_rFrameUR): imax(*l0_rFrame,l0_rFrameUR));

	*l1_rFrame = (char) ((l1_rFrameL >= 0 && l1_rFrameU >= 0)  ? imin(l1_rFrameL,l1_rFrameU) : imax(l1_rFrameL,l1_rFrameU));
	*l1_rFrame = (char) ((*l1_rFrame >= 0 && l1_rFrameUR >= 0) ? imin(*l1_rFrame,l1_rFrameUR): imax(*l1_rFrame,l1_rFrameUR));

	if (*l0_rFrame >=0)
		currMB->GetMVPredictor (currMB, mb, pmvl0, *l0_rFrame, motion->motion[LIST_0], 0, 0, 16, 16);

	if (*l1_rFrame >=0)
		currMB->GetMVPredictor (currMB, mb, pmvl1, *l1_rFrame, motion->motion[LIST_1], 0, 0, 16, 16);
}

static void check_motion_vector_range(VideoParameters *p_Vid, short mv_x, short mv_y)
{
#ifdef _DEBUG
	if (mv_x > 8191 || mv_x < -8192)
	{
		fprintf(stderr,"WARNING! Horizontal motion vector %d is out of allowed range {-8192, 8191} in picture %d, macroblock %d\n", mv_x, p_Vid->number, p_Vid->current_mb_nr);
		//error("invalid stream: too big horizontal motion vector", 500);
	}

	if (mv_y > (p_Vid->max_mb_vmv_r - 1) || mv_y < (-p_Vid->max_mb_vmv_r))
	{
		fprintf(stderr,"WARNING! Vertical motion vector %d is out of allowed range {%d, %d} in picture %d, macroblock %d\n", mv_y, (-p_Vid->max_mb_vmv_r), (p_Vid->max_mb_vmv_r - 1), p_Vid->number, p_Vid->current_mb_nr);
		//error("invalid stream: too big vertical motion vector", 500);
	}
#endif
}

void perform_mc(Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset, int block_size_x, int block_size_y, int curr_mb_field)
{
	VideoParameters *p_Vid = currMB->p_Vid;  
	seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

	Slice *currSlice = currMB->p_Slice;

	static const int mv_mul = 16; // 4 * 4

	int i4   = currMB->block_x + i;
	int j4   = currMB->block_y + j;
	int ioff = (i << 2);
	int joff = (j << 2);         

	assert (pred_dir<=2);

	if (pred_dir != 2)
	{
		//===== Single List Prediction =====
		short       ref_idx = dec_picture->motion.motion[pred_dir][j4][i4].ref_idx;
		short       ref_idx_wp = ref_idx;
		short      *mv_array = dec_picture->motion.motion[pred_dir][j4][i4].mv;
		StorablePicture *list = p_Vid->listX[list_offset + pred_dir][ref_idx];

		check_motion_vector_range(p_Vid, mv_array[0], mv_array[1]);


		get_block_luma(currMB, pl, list, i4, currMB->block_y_aff + j, mv_array, block_size_x, block_size_y, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]); 

		if (currSlice->apply_weights)
		{
			int alpha_l0, wp_offset;
			if (curr_mb_field && ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))))
			{
				ref_idx_wp >>=1;
			}

			alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][0];
			wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][0];

			weighted_mc_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], block_size_y, block_size_x, alpha_l0, wp_offset, currSlice->luma_log2_weight_denom);
		}

		if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
		{ // YUV420 or YUV422
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = (p_Vid->mb_cr_size_y == MB_BLOCK_SIZE) ? joff : joff >> 1;
			int block_size_x_cr = block_size_x >> 1;
			int block_size_y_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? block_size_y : block_size_y >> 1;

			short mv_cr[2] = {mv_array[0], mv_array[1] +  + ((active_sps->chroma_format_idc == YUV420)? list->chroma_vector_adjustment : 0) };
			get_block_chroma(currMB, list, i4, currMB->block_y_aff + j, mv_cr, block_size_x_cr, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);
			for(uv=0;uv<2;uv++)
			{
				if (currSlice->apply_weights)
				{
					int alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][uv + 1];
					int wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][uv + 1];

					weighted_mc_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv + 1][joff_cr][ioff_cr], block_size_y_cr, block_size_x_cr, alpha_l0, wp_offset, currSlice->chroma_log2_weight_denom);
				}
			}
		}
	}
	else
	{
		//===== BI-PREDICTION =====
		__declspec(align(32)) h264_imgpel_macroblock_t tmp_block_l0[2];
		short *l0_mv_array = dec_picture->motion.motion[LIST_0][j4][i4].mv;
		short *l1_mv_array = dec_picture->motion.motion[LIST_1][j4][i4].mv;

		short l0_refframe = dec_picture->motion.motion[LIST_0][j4][i4].ref_idx;
		short l0_ref_idx  = l0_refframe;
		short l1_refframe = dec_picture->motion.motion[LIST_1][j4][i4].ref_idx;
		short l1_ref_idx  = l1_refframe;

		check_motion_vector_range(p_Vid, l0_mv_array[0], l0_mv_array[1]);
		check_motion_vector_range(p_Vid, l1_mv_array[0], l1_mv_array[1]);

		get_block_luma(currMB, pl, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, l0_mv_array, block_size_x, block_size_y, tmp_block_l0[0]);  
		get_block_luma(currMB, pl, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, l1_mv_array, block_size_x, block_size_y, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]);  

		if(currSlice->apply_weights)
		{
			int alpha_l0, alpha_l1, wp_offset;
			int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

			// This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
			// if (mv_mode==0 && currSlice->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
			if (((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))) && curr_mb_field)
			{
				l0_ref_idx >>=1;
				l1_ref_idx >>=1;
			}

			alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

			weighted_bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], block_size_y, block_size_x, alpha_l0, alpha_l1, wp_offset, (currSlice->luma_log2_weight_denom + 1));
		}
		else
		{ 
			bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], block_size_y, block_size_x); 
		}

		if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
		{ // YUV420 or YUV422
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? joff : joff >> 1;
			int block_size_x_cr = block_size_x >> 1;
			int block_size_y_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? block_size_y : block_size_y >> 1;

			int vec1_y_cr = currMB->block_y_aff + j + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0);
			int vec2_y_cr = currMB->block_y_aff + j + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0);
			short mv_cr1[2] = {l0_mv_array[0], l0_mv_array[1] +  ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0) };
			short mv_cr2[2] = {l1_mv_array[0], l1_mv_array[1] +  ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0) };

			get_block_chroma(currMB, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, mv_cr1, block_size_x_cr, block_size_y_cr, tmp_block_l0[0], tmp_block_l0[1], 0, 0);
			get_block_chroma(currMB, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, mv_cr2, block_size_x_cr, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);
			for(uv=0;uv<2;uv++)
			{
				if(currSlice->apply_weights)
				{
					int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

					int alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

					weighted_bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], block_size_y_cr, block_size_x_cr, alpha_l0, alpha_l1, wp_offset, (currSlice->chroma_log2_weight_denom + 1));
				}
				else
				{
					bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], block_size_y_cr, block_size_x_cr);
				}
			}
		}      
	}
}



void perform_mc8x16(Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset,  int curr_mb_field)
{
	VideoParameters *p_Vid = currMB->p_Vid;  
	seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
	const int block_size_x=8;
	const int block_size_y=16;
	Slice *currSlice = currMB->p_Slice;

	static const int mv_mul = 16; // 4 * 4

	int i4   = currMB->block_x + i;
	int j4   = currMB->block_y + j;
	int ioff = (i << 2);
	int joff = (j << 2);         

	assert (pred_dir<=2);

	if (pred_dir != 2)
	{
		//===== Single List Prediction =====
		short       ref_idx = dec_picture->motion.motion[pred_dir][j4][i4].ref_idx;
		short       ref_idx_wp = ref_idx;
		short      *mv_array = dec_picture->motion.motion[pred_dir][j4][i4].mv;
		StorablePicture *list = p_Vid->listX[list_offset + pred_dir][ref_idx];

		check_motion_vector_range(p_Vid, mv_array[0], mv_array[1]);

		get_block_luma(currMB, pl, list, i4, currMB->block_y_aff + j, mv_array, block_size_x, block_size_y, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]); 

		if (currSlice->apply_weights)
		{
			int alpha_l0, wp_offset;
			if (curr_mb_field && ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))))
			{
				ref_idx_wp >>=1;
			}

			alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][0];
			wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][0];

			weighted_mc_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], block_size_y, block_size_x, alpha_l0, wp_offset, currSlice->luma_log2_weight_denom);
		}

		if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
		{ // YUV420 or YUV422
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = (p_Vid->mb_cr_size_y == MB_BLOCK_SIZE) ? joff : joff >> 1;
			int block_size_x_cr = block_size_x >> 1;
			int block_size_y_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? block_size_y : block_size_y >> 1;

			short mv_cr[2] = {mv_array[0], mv_array[1] +  + ((active_sps->chroma_format_idc == YUV420)? list->chroma_vector_adjustment : 0) };
			get_block_chroma(currMB, list, i4, currMB->block_y_aff + j, mv_cr, block_size_x_cr, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);
			for(uv=0;uv<2;uv++)
			{
				if (currSlice->apply_weights)
				{
					int alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][uv + 1];
					int wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][uv + 1];

					weighted_mc_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv + 1][joff_cr][ioff_cr], block_size_y_cr, block_size_x_cr, alpha_l0, wp_offset, currSlice->chroma_log2_weight_denom);
				}
			}
		}
	}
	else
	{
		//===== BI-PREDICTION =====
		__declspec(align(32)) h264_imgpel_macroblock_t tmp_block_l0[2];
		short *l0_mv_array = dec_picture->motion.motion[LIST_0][j4][i4].mv;
		short *l1_mv_array = dec_picture->motion.motion[LIST_1][j4][i4].mv;

		short l0_refframe = dec_picture->motion.motion[LIST_0][j4][i4].ref_idx;
		short l0_ref_idx  = l0_refframe;
		short l1_refframe = dec_picture->motion.motion[LIST_1][j4][i4].ref_idx;
		short l1_ref_idx  = l1_refframe;

		check_motion_vector_range(p_Vid, l0_mv_array[0], l0_mv_array[1]);
		check_motion_vector_range(p_Vid, l1_mv_array[0], l1_mv_array[1]);

		get_block_luma(currMB, pl, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, l0_mv_array, block_size_x, block_size_y, tmp_block_l0[0]);  
		get_block_luma(currMB, pl, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, l1_mv_array, block_size_x, block_size_y, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]);  

		if(currSlice->apply_weights)
		{
			int alpha_l0, alpha_l1, wp_offset;
			int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

			// This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
			// if (mv_mode==0 && currSlice->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
			if (((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))) && curr_mb_field)
			{
				l0_ref_idx >>=1;
				l1_ref_idx >>=1;
			}

			alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

			weighted_bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], block_size_y, block_size_x, alpha_l0, alpha_l1, wp_offset, (currSlice->luma_log2_weight_denom + 1));
		}
		else
		{ 
			bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], block_size_y, block_size_x); 
		}

		if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
		{ // YUV420 or YUV422
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? joff : joff >> 1;
			int block_size_x_cr = block_size_x >> 1;
			int block_size_y_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? block_size_y : block_size_y >> 1;

			int vec1_y_cr = currMB->block_y_aff + j + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0);
			int vec2_y_cr = currMB->block_y_aff + j + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0);
			short mv_cr1[2] = {l0_mv_array[0], l0_mv_array[1] +  ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0) };
			short mv_cr2[2] = {l1_mv_array[0], l1_mv_array[1] +  ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0) };

			get_block_chroma(currMB, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, mv_cr1, block_size_x_cr, block_size_y_cr, tmp_block_l0[0], tmp_block_l0[1], 0, 0);
			get_block_chroma(currMB, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, mv_cr2, block_size_x_cr, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);
			for(uv=0;uv<2;uv++)
			{
				if(currSlice->apply_weights)
				{
					int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

					int alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

					weighted_bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], block_size_y_cr, block_size_x_cr, alpha_l0, alpha_l1, wp_offset, (currSlice->chroma_log2_weight_denom + 1));
				}
				else
				{
					bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], block_size_y_cr, block_size_x_cr);
				}
			}
		}      
	}
}

void perform_mc16x8(Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset, int curr_mb_field)
{
	VideoParameters *p_Vid = currMB->p_Vid;  
	seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

	Slice *currSlice = currMB->p_Slice;

	int i4   = currMB->block_x + i;
	int j4   = currMB->block_y + j;
	int ioff = (i << 2);
	int joff = (j << 2);

	assert (pred_dir<=2);

	if (pred_dir != 2)
	{
		//===== Single List Prediction =====
		short       ref_idx = dec_picture->motion.motion[pred_dir][j4][i4].ref_idx;
		short       ref_idx_wp = ref_idx;
		short      *mv_array = dec_picture->motion.motion[pred_dir][j4][i4].mv;
		StorablePicture *list = p_Vid->listX[list_offset + pred_dir][ref_idx];

		check_motion_vector_range(p_Vid, mv_array[0], mv_array[1]);

		get_block_luma(currMB, pl, list, i4, currMB->block_y_aff + j, mv_array, 16, 8, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]); 

		if (currSlice->apply_weights)
		{
			int alpha_l0, wp_offset;
			if (curr_mb_field && ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))))
			{
				ref_idx_wp >>=1;
			}

			alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][0];
			wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][0];

			opt_weighted_mc_prediction16x8((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], alpha_l0, wp_offset, currSlice->luma_log2_weight_denom);
		}

		if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
		{ // YUV420 or YUV422
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = (p_Vid->mb_cr_size_y == MB_BLOCK_SIZE) ? joff : joff >> 1;
			int block_size_x_cr = 16 >> 1;
			int block_size_y_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? 8 : 8 >> 1;

			short mv_cr[2] = {mv_array[0], mv_array[1] +  + ((active_sps->chroma_format_idc == YUV420)? list->chroma_vector_adjustment : 0) };
			get_block_chroma(currMB, list, i4, currMB->block_y_aff + j, mv_cr, block_size_x_cr, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);
			for(uv=0;uv<2;uv++)
			{
				if (currSlice->apply_weights)
				{
					int alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][uv + 1];
					int wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][uv + 1];

					weighted_mc_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv + 1][joff_cr][ioff_cr], block_size_y_cr, block_size_x_cr, alpha_l0, wp_offset, currSlice->chroma_log2_weight_denom);
				}
			}
		}
	}
	else
	{
		//===== BI-PREDICTION =====
		__declspec(align(32)) h264_imgpel_macroblock_t tmp_block_l0[2];
		short *l0_mv_array = dec_picture->motion.motion[LIST_0][j4][i4].mv;
		short *l1_mv_array = dec_picture->motion.motion[LIST_1][j4][i4].mv;

		short l0_refframe = dec_picture->motion.motion[LIST_0][j4][i4].ref_idx;
		short l0_ref_idx  = l0_refframe;
		short l1_refframe = dec_picture->motion.motion[LIST_1][j4][i4].ref_idx;
		short l1_ref_idx  = l1_refframe;

		check_motion_vector_range(p_Vid, l0_mv_array[0], l0_mv_array[1]);
		check_motion_vector_range(p_Vid, l1_mv_array[0], l1_mv_array[1]);

		get_block_luma(currMB, pl, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, l0_mv_array, 16, 8, tmp_block_l0[0]);  
		get_block_luma(currMB, pl, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, l1_mv_array, 16, 8, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]);  

		if(currSlice->apply_weights)
		{
			int alpha_l0, alpha_l1, wp_offset;
			int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

			// This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
			// if (mv_mode==0 && currSlice->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
			if (((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))) && curr_mb_field)
			{
				l0_ref_idx >>=1;
				l1_ref_idx >>=1;
			}

			alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

			opt_weighted_bi_prediction16x8((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], alpha_l0, alpha_l1, wp_offset, (currSlice->luma_log2_weight_denom + 1));
		}
		else
		{ 
			bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], 8, 16); 
		}

		if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
		{ // YUV420 or YUV422
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? joff : joff >> 1;
			int block_size_x_cr = 16 >> 1;
			int block_size_y_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? 8 : 8 >> 1;

			int vec1_y_cr = currMB->block_y_aff + j + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0);
			int vec2_y_cr = currMB->block_y_aff + j + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0);
			short mv_cr1[2] = {l0_mv_array[0], l0_mv_array[1] +  ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0) };
			short mv_cr2[2] = {l1_mv_array[0], l1_mv_array[1] +  ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0) };

			get_block_chroma(currMB, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, mv_cr1, block_size_x_cr, block_size_y_cr, tmp_block_l0[0], tmp_block_l0[1], 0, 0);
			get_block_chroma(currMB, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, mv_cr2, block_size_x_cr, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);
			for(uv=0;uv<2;uv++)
			{
				if(currSlice->apply_weights)
				{
					int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

					int alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

					weighted_bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], block_size_y_cr, block_size_x_cr, alpha_l0, alpha_l1, wp_offset, (currSlice->chroma_log2_weight_denom + 1));
				}
				else
				{
					bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], block_size_y_cr, block_size_x_cr);
				}
			}
		}      
	}
}


static void __forceinline perform_mc8x8_YUV420(Macroblock *currMB, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset, int curr_mb_field)
{
	VideoParameters *p_Vid = currMB->p_Vid;  

	Slice *currSlice = currMB->p_Slice;

	int i4   = currMB->block_x + i;
	int j4   = currMB->block_y + j;
	int ioff = (i << 2);
	int joff = (j << 2);         

	assert (pred_dir<=2);

	if (pred_dir != 2)
	{
		//===== Single List Prediction =====
		short       ref_idx = dec_picture->motion.motion[pred_dir][j4][i4].ref_idx;
		short       ref_idx_wp = ref_idx;
		short      *mv_array = dec_picture->motion.motion[pred_dir][j4][i4].mv;
		StorablePicture *list = p_Vid->listX[list_offset + pred_dir][ref_idx];

		check_motion_vector_range(p_Vid, mv_array[0], mv_array[1]);

		get_block_luma(currMB, PLANE_Y, list, i4, currMB->block_y_aff + j, mv_array, 8, 8, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[0][joff][ioff]); 

		if (currSlice->apply_weights)
		{
			int alpha_l0, wp_offset;
			if (curr_mb_field && ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))))
			{
				ref_idx_wp >>=1;
			}

			alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][0];
			wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][0];

			opt_weighted_mc_prediction8x8((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[0][joff][ioff], alpha_l0, wp_offset, currSlice->luma_log2_weight_denom);
		}

		{ 
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = joff >> 1;

			short mv_cr[2] = {mv_array[0], mv_array[1] + list->chroma_vector_adjustment };
			get_block_chroma(currMB, list, i4, currMB->block_y_aff + j, mv_cr, 4, 4, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);

			for(uv=0;uv<2;uv++)
			{

				if (currSlice->apply_weights)
				{
					int alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][uv + 1];
					int wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][uv + 1];

					weighted_mc_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv + 1][joff_cr][ioff_cr], 4, 4, alpha_l0, wp_offset, currSlice->chroma_log2_weight_denom);
				}
			}
		}
	}
	else
	{
		//===== BI-PREDICTION =====
		__declspec(align(32)) h264_imgpel_macroblock_t tmp_block_l0[2];
		short *l0_mv_array = dec_picture->motion.motion[LIST_0][j4][i4].mv;
		short *l1_mv_array = dec_picture->motion.motion[LIST_1][j4][i4].mv;

		short l0_ref_idx = dec_picture->motion.motion[LIST_0][j4][i4].ref_idx;
		short l1_ref_idx = dec_picture->motion.motion[LIST_1][j4][i4].ref_idx;
		
		StorablePicture *ref_image0 = p_Vid->listX[LIST_0 + list_offset][l0_ref_idx];
		StorablePicture *ref_image1 = p_Vid->listX[LIST_1 + list_offset][l1_ref_idx];

		check_motion_vector_range(p_Vid, l0_mv_array[0], l0_mv_array[1]);
		check_motion_vector_range(p_Vid, l1_mv_array[0], l1_mv_array[1]);

		if (p_Vid->framepoc < p_Vid->recovery_poc || IS_INDEPENDENT(p_Vid))
		{
			get_block_luma(currMB, PLANE_Y, ref_image0, i4, currMB->block_y_aff + j, l0_mv_array, 8, 8, tmp_block_l0[0]);  
			get_block_luma(currMB, PLANE_Y, ref_image1, i4, currMB->block_y_aff + j, l1_mv_array, 8, 8, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[0][joff][ioff]);  
		}
		else
		{
			IppVCInterpolateBlock_8u block_data;

			block_data.pSrc[0] = ref_image0->imgY->base_address;
			block_data.srcStep = ref_image0->imgY->stride;
			block_data.pDst[0] = (Ipp8u *)(tmp_block_l0[0]);
			block_data.dstStep = sizeof(tmp_block_l0[0][0]);
			block_data.sizeFrame.width = dec_picture->size_x;
			block_data.sizeFrame.height = (dec_picture->motion.mb_field[currMB->mbAddrX]) ? (dec_picture->size_y >> 1): dec_picture->size_y;
			block_data.sizeBlock.width = 8;
			block_data.sizeBlock.height = 8;
			block_data.pointBlockPos.x = i4 << 2;
			block_data.pointBlockPos.y = (currMB->block_y_aff + j) << 2;
			block_data.pointVector.x = l0_mv_array[0];
			block_data.pointVector.y = l0_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
			block_data.pSrc[0] = ref_image1->imgY->base_address;
			block_data.srcStep = ref_image1->imgY->stride;
			block_data.pDst[0] = &currSlice->mb_pred[0][joff][ioff];
			block_data.pointVector.x = l1_mv_array[0];
			block_data.pointVector.y = l1_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
		}

		if(currSlice->apply_weights)
		{
			int alpha_l0, alpha_l1, wp_offset;
			int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

			// This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
			// if (mv_mode==0 && currSlice->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
			if (((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))) && curr_mb_field)
			{
				l0_ref_idx >>=1;
				l1_ref_idx >>=1;
			}

			alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

			opt_weighted_bi_prediction8x8((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[0][joff][ioff], tmp_block_l0[0], alpha_l0, alpha_l1, wp_offset, (currSlice->luma_log2_weight_denom + 1));
		}
		else
		{ 
			bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[0][joff][ioff], tmp_block_l0[0], 8, 8); 
		}

		{
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = joff >> 1;

			short mv_cr1[2] = {l0_mv_array[0], l0_mv_array[1] +  ref_image0->chroma_vector_adjustment};
			short mv_cr2[2] = {l1_mv_array[0], l1_mv_array[1] +  ref_image1->chroma_vector_adjustment};

			if (p_Vid->framepoc < p_Vid->recovery_poc)
			{
				get_block_chroma(currMB, ref_image0, i4, currMB->block_y_aff + j, mv_cr1, 4, 4, tmp_block_l0[0], tmp_block_l0[1], 0, 0);
				get_block_chroma(currMB, ref_image1, i4, currMB->block_y_aff + j, mv_cr2, 4, 4, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);
			}
			else
			{
				IppVCInterpolateBlock_8u block_data;
				block_data.pSrc[0] = ref_image0->imgUV[0]->base_address;
				block_data.pSrc[1] = ref_image0->imgUV[1]->base_address;
				block_data.srcStep = ref_image0->imgUV[0]->stride;
				block_data.pDst[0] = (Ipp8u *)(tmp_block_l0[0]);
				block_data.pDst[1] = (Ipp8u *)(tmp_block_l0[1]);
				block_data.dstStep = sizeof(tmp_block_l0[0][0]);
				block_data.sizeFrame.width = dec_picture->size_x_cr;
				block_data.sizeFrame.height = (dec_picture->motion.mb_field[currMB->mbAddrX]) ? (dec_picture->size_y_cr >> 1): dec_picture->size_y_cr;
				block_data.sizeBlock.width = 4;
				block_data.sizeBlock.height = 4;
				block_data.pointBlockPos.x = i4<<1;
				block_data.pointVector.x = mv_cr1[0]; 
				block_data.pointVector.y = mv_cr1[1];
				block_data.pointBlockPos.y = (currMB->block_y_aff + j)<<1;
				ippiInterpolateChromaBlock_H264_8u_P2R(&block_data);
				block_data.pSrc[0] = ref_image1->imgUV[0]->base_address;
				block_data.pSrc[1] = ref_image1->imgUV[1]->base_address;
				block_data.srcStep = ref_image1->imgUV[0]->stride;
				block_data.pDst[0] = &currSlice->mb_pred[1][joff_cr][ioff_cr];
				block_data.pDst[1] = &currSlice->mb_pred[2][joff_cr][ioff_cr];
				block_data.pointVector.x = mv_cr2[0]; 
				block_data.pointVector.y = mv_cr2[1];
				ippiInterpolateChromaBlock_H264_8u_P2R(&block_data);
			}

			for(uv=0;uv<2;uv++)
			{
				if(currSlice->apply_weights)
				{
					int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

					int alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

					weighted_bi_prediction4x4((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], alpha_l0, alpha_l1, wp_offset, (currSlice->chroma_log2_weight_denom + 1));
				}
				else
				{
					bi_prediction4x4_mmx((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv]);
				}
			}
		}
	}
}

void perform_mc8x8(Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset, int curr_mb_field)
{
	if (dec_picture->chroma_format_idc == YUV420)
	{
		perform_mc8x8_YUV420(currMB, dec_picture, pred_dir, i, j, list_offset, curr_mb_field);
	}
	else
	{
	VideoParameters *p_Vid = currMB->p_Vid;  
	seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

	Slice *currSlice = currMB->p_Slice;

	static const int mv_mul = 16; // 4 * 4

	int i4   = currMB->block_x + i;
	int j4   = currMB->block_y + j;
	int ioff = (i << 2);
	int joff = (j << 2);         

	assert (pred_dir<=2);

	if (pred_dir != 2)
	{
		//===== Single List Prediction =====
		short       ref_idx = dec_picture->motion.motion[pred_dir][j4][i4].ref_idx;
		short       ref_idx_wp = ref_idx;
		short      *mv_array = dec_picture->motion.motion[pred_dir][j4][i4].mv;
		StorablePicture *list = p_Vid->listX[list_offset + pred_dir][ref_idx];

		check_motion_vector_range(p_Vid, mv_array[0], mv_array[1]);

		get_block_luma(currMB, pl, list, i4, currMB->block_y_aff + j, mv_array, 8, 8, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]); 

		if (currSlice->apply_weights)
		{
			int alpha_l0, wp_offset;
			if (curr_mb_field && ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))))
			{
				ref_idx_wp >>=1;
			}

			alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][0];
			wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][0];

			opt_weighted_mc_prediction8x8((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], alpha_l0, wp_offset, currSlice->luma_log2_weight_denom);
		}

		if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
		{ // YUV420 or YUV422
			int uv;

			int ioff_cr = ioff >> 1;
			int joff_cr = (p_Vid->mb_cr_size_y == MB_BLOCK_SIZE) ? joff : joff >> 1;
			int block_size_y_cr = p_Vid->mb_cr_size_y == MB_BLOCK_SIZE ? 8 : 4;

			short mv_cr[2] = {mv_array[0], mv_array[1] };
			get_block_chroma(currMB, list, i4, currMB->block_y_aff + j, mv_cr, 4, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);

			for(uv=0;uv<2;uv++)
			{

				if (currSlice->apply_weights)
				{
					int alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][uv + 1];
					int wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][uv + 1];

					weighted_mc_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv + 1][joff_cr][ioff_cr], block_size_y_cr, 4, alpha_l0, wp_offset, currSlice->chroma_log2_weight_denom);
				}
			}
		}
	}
	else
	{
		//===== BI-PREDICTION =====
		__declspec(align(32)) h264_imgpel_macroblock_t tmp_block_l0[2];
		short *l0_mv_array = dec_picture->motion.motion[LIST_0][j4][i4].mv;
		short *l1_mv_array = dec_picture->motion.motion[LIST_1][j4][i4].mv;

		short l0_refframe = dec_picture->motion.motion[LIST_0][j4][i4].ref_idx;
		short l0_ref_idx  = l0_refframe;
		short l1_refframe = dec_picture->motion.motion[LIST_1][j4][i4].ref_idx;
		short l1_ref_idx  = l1_refframe;
		
		check_motion_vector_range(p_Vid, l0_mv_array[0], l0_mv_array[1]);
		check_motion_vector_range(p_Vid, l1_mv_array[0], l1_mv_array[1]);

		if (p_Vid->framepoc < p_Vid->recovery_poc || IS_INDEPENDENT(p_Vid) || pl!=PLANE_Y)
		{
			get_block_luma(currMB, pl, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, l0_mv_array, 8, 8, tmp_block_l0[0]);  
			get_block_luma(currMB, pl, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, l1_mv_array, 8, 8, (h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff]);  
		}
		else
		{
			VideoImage *cur_imgY = p_Vid->listX[LIST_0 + list_offset][l0_refframe]->imgY;
			IppVCInterpolateBlock_8u block_data;

			block_data.pSrc[0] = cur_imgY->base_address;
			block_data.srcStep = cur_imgY->stride;
			block_data.pDst[0] = (Ipp8u *)(tmp_block_l0[0]);
			block_data.dstStep = sizeof(tmp_block_l0[0][0]);
			block_data.sizeFrame.width = dec_picture->size_x;
			block_data.sizeFrame.height = (dec_picture->motion.mb_field[currMB->mbAddrX]) ? (dec_picture->size_y >> 1): dec_picture->size_y;
			block_data.sizeBlock.width = 8;
			block_data.sizeBlock.height = 8;
			block_data.pointBlockPos.x = i4 << 2;
			block_data.pointBlockPos.y = (currMB->block_y_aff + j) << 2;
			block_data.pointVector.x = l0_mv_array[0];
			block_data.pointVector.y = l0_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
			cur_imgY = p_Vid->listX[LIST_1 + list_offset][l1_refframe]->imgY;
			block_data.pSrc[0] = cur_imgY->base_address;
			block_data.srcStep = cur_imgY->stride;
			block_data.pDst[0] = &currSlice->mb_pred[pl][joff][ioff];
			block_data.pointVector.x = l1_mv_array[0];
			block_data.pointVector.y = l1_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
		}

		if(currSlice->apply_weights)
		{
			int alpha_l0, alpha_l1, wp_offset;
			int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

			// This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
			// if (mv_mode==0 && currSlice->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
			if (((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))) && curr_mb_field)
			{
				l0_ref_idx >>=1;
				l1_ref_idx >>=1;
			}

			alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

			opt_weighted_bi_prediction8x8((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], alpha_l0, alpha_l1, wp_offset, (currSlice->luma_log2_weight_denom + 1));
		}
		else
		{ 
			bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[pl][joff][ioff], tmp_block_l0[0], 8, 8); 
		}

		if (dec_picture->chroma_format_idc == YUV422)
		{
			int uv;
			int ioff_cr = ioff >> 1;
			int joff_cr = joff;

			short mv_cr1[2] = {l0_mv_array[0], l0_mv_array[1]};
			short mv_cr2[2] = {l1_mv_array[0], l1_mv_array[1]};

			get_block_chroma(currMB, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff + j, mv_cr1, 4, 8, tmp_block_l0[0], tmp_block_l0[1], 0, 0);
			get_block_chroma(currMB, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff + j, mv_cr2, 4, 8, currSlice->mb_pred[1], currSlice->mb_pred[2], ioff_cr, joff_cr);

			for(uv=0;uv<2;uv++)
			{

				if(currSlice->apply_weights)
				{
					int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

					int alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

					weighted_bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], 8, 4, alpha_l0, alpha_l1, wp_offset, (currSlice->chroma_log2_weight_denom + 1));
				}
				else
				{
					bi_prediction((h264_imgpel_macroblock_row_t *)&currSlice->mb_pred[uv+1][joff_cr][ioff_cr], tmp_block_l0[uv], 8, 4);
				}
			}
		}
	}
	}
}


static void __forceinline perform_mc16x16_YUV420(Macroblock *currMB, StorablePicture *dec_picture, int pred_dir, int list_offset, int curr_mb_field)
{
	VideoParameters *p_Vid = currMB->p_Vid;  
	seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

	Slice *currSlice = currMB->p_Slice;

	static const int mv_mul = 16; // 4 * 4

	int i4   = currMB->block_x;
	int j4   = currMB->block_y;

	assert (pred_dir<=2);

	if (pred_dir != 2)
	{
		//===== Single List Prediction =====
		short       ref_idx = dec_picture->motion.motion[pred_dir][j4][i4].ref_idx;
		short       ref_idx_wp = ref_idx;
		short      *mv_array = dec_picture->motion.motion[pred_dir][j4][i4].mv;
		StorablePicture *list = p_Vid->listX[list_offset + pred_dir][ref_idx];

		check_motion_vector_range(p_Vid, mv_array[0], mv_array[1]);

		get_block_luma(currMB, PLANE_Y, list, i4, currMB->block_y_aff, mv_array, 16, 16, currSlice->mb_pred[0]); 

		if (currSlice->apply_weights)
		{
			int alpha_l0, wp_offset;
			if (curr_mb_field && ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))))
			{
				ref_idx_wp >>=1;
			}

			alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][0];
			wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][0];

			opt_weighted_mc_prediction16x16(currSlice->mb_pred[0], alpha_l0, wp_offset, currSlice->luma_log2_weight_denom);
		}
		{
			int uv;
			short mv_cr[2] = {mv_array[0], mv_array[1] +  list->chroma_vector_adjustment };
			get_block_chroma(currMB, list, i4, currMB->block_y_aff, mv_cr, 8, 8, currSlice->mb_pred[1], currSlice->mb_pred[2], 0, 0);

			for(uv=0;uv<2;uv++)
			{
				if (currSlice->apply_weights)
				{
					int alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][uv + 1];
					int wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][uv + 1];

					opt_weighted_mc_prediction8x8(currSlice->mb_pred[uv + 1], alpha_l0, wp_offset, currSlice->chroma_log2_weight_denom);
				}
			}
		}
		}
	else
	{
		//===== BI-PREDICTION =====
		__declspec(align(32)) h264_imgpel_macroblock_t tmp_block_l0[2];
		short *l0_mv_array = dec_picture->motion.motion[LIST_0][j4][i4].mv;
		short *l1_mv_array = dec_picture->motion.motion[LIST_1][j4][i4].mv;

		short l0_refframe = dec_picture->motion.motion[LIST_0][j4][i4].ref_idx;
		short l0_ref_idx  = l0_refframe;
		short l1_refframe = dec_picture->motion.motion[LIST_1][j4][i4].ref_idx;
		short l1_ref_idx  = l1_refframe;

		check_motion_vector_range(p_Vid, l0_mv_array[0], l0_mv_array[1]);
		check_motion_vector_range(p_Vid, l1_mv_array[0], l1_mv_array[1]);

		if (p_Vid->framepoc < p_Vid->recovery_poc || IS_INDEPENDENT(p_Vid))
		{
			get_block_luma(currMB, PLANE_Y, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff, l0_mv_array, 16, 16, tmp_block_l0[0]);  
			get_block_luma(currMB, PLANE_Y, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff, l1_mv_array, 16, 16, currSlice->mb_pred[0]);
		}
		else
		{
			VideoImage *cur_imgY = p_Vid->listX[LIST_0 + list_offset][l0_refframe]->imgY;
			IppVCInterpolateBlock_8u block_data;

			block_data.pSrc[0] = cur_imgY->base_address;
			block_data.srcStep = cur_imgY->stride;
			block_data.pDst[0] = (Ipp8u *)(tmp_block_l0[0]);
			block_data.dstStep = sizeof(tmp_block_l0[0][0]);
			block_data.sizeFrame.width = dec_picture->size_x;
			block_data.sizeFrame.height = (dec_picture->motion.mb_field[currMB->mbAddrX]) ? (dec_picture->size_y >> 1): dec_picture->size_y;
			block_data.sizeBlock.width = 16;
			block_data.sizeBlock.height = 16;
			block_data.pointBlockPos.x = i4 << 2;
			block_data.pointBlockPos.y = currMB->block_y_aff<< 2;
			block_data.pointVector.x = l0_mv_array[0];
			block_data.pointVector.y = l0_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
			cur_imgY = p_Vid->listX[LIST_1 + list_offset][l1_refframe]->imgY;
			block_data.pSrc[0] = cur_imgY->base_address;
			block_data.srcStep = cur_imgY->stride;
			block_data.pDst[0] = (Ipp8u *)(currSlice->mb_pred[0]);
			block_data.pointVector.x = l1_mv_array[0];
			block_data.pointVector.y = l1_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
		}

		if(currSlice->apply_weights)
		{
			int alpha_l0, alpha_l1, wp_offset;
			int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

			// This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
			// if (mv_mode==0 && currSlice->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
			if (((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))) && curr_mb_field)
			{
				l0_ref_idx >>=1;
				l1_ref_idx >>=1;
			}

			alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

			opt_weighted_bi_prediction16x16(currSlice->mb_pred[0], tmp_block_l0[0], alpha_l0, alpha_l1, wp_offset, (currSlice->luma_log2_weight_denom + 1));
		}
		else
		{ 
			bi_prediction(currSlice->mb_pred[0], tmp_block_l0[0], 16, 16); 
		}
		
		{ 
			int uv;

			short mv_cr1[2] = {l0_mv_array[0], l0_mv_array[1] + p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment };
			short mv_cr2[2] = {l1_mv_array[0], l1_mv_array[1] + p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment };

			get_block_chroma(currMB, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff , mv_cr1, 8, 8, tmp_block_l0[0], tmp_block_l0[1], 0, 0);
			get_block_chroma(currMB, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff , mv_cr2, 8, 8, currSlice->mb_pred[1], currSlice->mb_pred[2], 0, 0);

			for(uv=0;uv<2;uv++)
			{
				if(currSlice->apply_weights)
				{
					int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

					int alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

					opt_weighted_bi_prediction8x8(currSlice->mb_pred[uv+1], tmp_block_l0[uv], alpha_l0, alpha_l1, wp_offset, (currSlice->chroma_log2_weight_denom + 1));
				}
				else
				{
					bi_prediction(currSlice->mb_pred[uv+1], tmp_block_l0[uv], 8, 8);
				}
			}
		}      
		
	}
}



void perform_mc16x16(Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int list_offset, int curr_mb_field)
{
	if (dec_picture->chroma_format_idc == YUV420)
	{
		perform_mc16x16_YUV420(currMB, dec_picture, pred_dir, list_offset, curr_mb_field);
	}
	else
	{
	VideoParameters *p_Vid = currMB->p_Vid;  
	seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

	Slice *currSlice = currMB->p_Slice;

	static const int mv_mul = 16; // 4 * 4

	int i4   = currMB->block_x;
	int j4   = currMB->block_y;

	assert (pred_dir<=2);

	if (pred_dir != 2)
	{
		//===== Single List Prediction =====
		short       ref_idx = dec_picture->motion.motion[pred_dir][j4][i4].ref_idx;
		short       ref_idx_wp = ref_idx;
		short      *mv_array = dec_picture->motion.motion[pred_dir][j4][i4].mv;
		StorablePicture *list = p_Vid->listX[list_offset + pred_dir][ref_idx];

		check_motion_vector_range(p_Vid, mv_array[0], mv_array[1]);

		get_block_luma(currMB, pl, list, i4, currMB->block_y_aff, mv_array, 16, 16, currSlice->mb_pred[pl]); 

		if (currSlice->apply_weights)
		{
			int alpha_l0, wp_offset;
			if (curr_mb_field && ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))))
			{
				ref_idx_wp >>=1;
			}

			alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][0];
			wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][0];

			opt_weighted_mc_prediction16x16(currSlice->mb_pred[pl], alpha_l0, wp_offset, currSlice->luma_log2_weight_denom);
		}

		if (dec_picture->chroma_format_idc == YUV422)
		{
			int uv;
			short mv_cr[2] = {mv_array[0], mv_array[1]};
			get_block_chroma(currMB, list, i4, currMB->block_y_aff, mv_cr, 8, 16, currSlice->mb_pred[1], currSlice->mb_pred[2], 0, 0);

			for(uv=0;uv<2;uv++)
			{
				if (currSlice->apply_weights)
				{
					int alpha_l0  = currSlice->wp_weight[pred_dir][ref_idx_wp][uv + 1];
					int wp_offset = currSlice->wp_offset[pred_dir][ref_idx_wp][uv + 1];

					weighted_mc_prediction(currSlice->mb_pred[uv + 1], 16, 8, alpha_l0, wp_offset, currSlice->chroma_log2_weight_denom);
				}
			}
		}
	}
	else
	{
		//===== BI-PREDICTION =====
		__declspec(align(32)) h264_imgpel_macroblock_t tmp_block_l0[2];
		short *l0_mv_array = dec_picture->motion.motion[LIST_0][j4][i4].mv;
		short *l1_mv_array = dec_picture->motion.motion[LIST_1][j4][i4].mv;

		short l0_refframe = dec_picture->motion.motion[LIST_0][j4][i4].ref_idx;
		short l0_ref_idx  = l0_refframe;
		short l1_refframe = dec_picture->motion.motion[LIST_1][j4][i4].ref_idx;
		short l1_ref_idx  = l1_refframe;

		check_motion_vector_range(p_Vid, l0_mv_array[0], l0_mv_array[1]);
		check_motion_vector_range(p_Vid, l1_mv_array[0], l1_mv_array[1]);

		if (p_Vid->framepoc < p_Vid->recovery_poc || IS_INDEPENDENT(p_Vid) || pl!=PLANE_Y)
		{
			get_block_luma(currMB, pl, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff, l0_mv_array, 16, 16, tmp_block_l0[0]);  
			get_block_luma(currMB, pl, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff, l1_mv_array, 16, 16, currSlice->mb_pred[pl]);
		}
		else
		{
			VideoImage *cur_imgY = p_Vid->listX[LIST_0 + list_offset][l0_refframe]->imgY;
			IppVCInterpolateBlock_8u block_data;

			block_data.pSrc[0] = cur_imgY->base_address;
			block_data.srcStep = cur_imgY->stride;
			block_data.pDst[0] = (Ipp8u *)(tmp_block_l0[0]);
			block_data.dstStep = sizeof(tmp_block_l0[0][0]);
			block_data.sizeFrame.width = dec_picture->size_x;
			block_data.sizeFrame.height = (dec_picture->motion.mb_field[currMB->mbAddrX]) ? (dec_picture->size_y >> 1): dec_picture->size_y;
			block_data.sizeBlock.width = 16;
			block_data.sizeBlock.height = 16;
			block_data.pointBlockPos.x = i4 << 2;
			block_data.pointBlockPos.y = currMB->block_y_aff<< 2;
			block_data.pointVector.x = l0_mv_array[0];
			block_data.pointVector.y = l0_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
			cur_imgY = p_Vid->listX[LIST_1 + list_offset][l1_refframe]->imgY;
			block_data.pSrc[0] = cur_imgY->base_address;
			block_data.srcStep = cur_imgY->stride;
			block_data.pDst[0] = (Ipp8u *)(currSlice->mb_pred[pl]);
			block_data.pointVector.x = l1_mv_array[0];
			block_data.pointVector.y = l1_mv_array[1];
			ippiInterpolateLumaBlock_H264_8u_P1R(&block_data);
		}

		if(currSlice->apply_weights)
		{
			int alpha_l0, alpha_l1, wp_offset;
			int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

			// This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
			// if (mv_mode==0 && currSlice->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
			if (((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
				(p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE))) && curr_mb_field)
			{
				l0_ref_idx >>=1;
				l1_ref_idx >>=1;
			}

			alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
			wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

			opt_weighted_bi_prediction16x16(currSlice->mb_pred[pl], tmp_block_l0[0], alpha_l0, alpha_l1, wp_offset, (currSlice->luma_log2_weight_denom + 1));
		}
		else
		{ 
			bi_prediction(currSlice->mb_pred[pl], tmp_block_l0[0], 16, 16); 
		}

		if (dec_picture->chroma_format_idc == YUV422) 
		{ // YUV422
			int uv;

			int block_size_y_cr = p_Vid->mb_cr_size_y;

			short mv_cr1[2] = {l0_mv_array[0], l0_mv_array[1] + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0) };
			short mv_cr2[2] = {l1_mv_array[0], l1_mv_array[1] + ((active_sps->chroma_format_idc == 1)? p_Vid->listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0) };

			get_block_chroma(currMB, p_Vid->listX[LIST_0 + list_offset][l0_refframe], i4, currMB->block_y_aff , mv_cr1, 8, block_size_y_cr, tmp_block_l0[0], tmp_block_l0[1], 0, 0);
			get_block_chroma(currMB, p_Vid->listX[LIST_1 + list_offset][l1_refframe], i4, currMB->block_y_aff , mv_cr2, 8, block_size_y_cr, currSlice->mb_pred[1], currSlice->mb_pred[2], 0, 0);


			for(uv=0;uv<2;uv++)
			{
				if(currSlice->apply_weights)
				{
					int wt_list_offset = (p_Vid->active_pps->weighted_bipred_idc==2)? list_offset : 0;

					int alpha_l0  =   currSlice->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int alpha_l1  =   currSlice->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
					int wp_offset = ((currSlice->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + currSlice->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

					weighted_bi_prediction(currSlice->mb_pred[uv+1], tmp_block_l0[uv], block_size_y_cr, 8, alpha_l0, alpha_l1, wp_offset, (currSlice->chroma_log2_weight_denom + 1));
				}
				else
				{
					bi_prediction(currSlice->mb_pred[uv+1], tmp_block_l0[uv], block_size_y_cr, 8);
				}
			}
		}      
	}
	}
}

