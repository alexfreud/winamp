#pragma once

typedef struct optimized_functions
{
	//void (*itrans4x4)(const h264_short_macroblock_t tblock, const h264_imgpel_macroblock_t mb_pred, h264_imgpel_macroblock_t mb_rec, int pos_x, int pos_y);
	void (*itrans8x8)(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x);

	void (*weighted_mc_prediction16x16)(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
	void (*weighted_mc_prediction16x8)(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
	void (*weighted_mc_prediction8x8)(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);

	void (*weighted_bi_prediction16x16)(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
	void (*weighted_bi_prediction16x8)(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
	void (*weighted_bi_prediction8x8)(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);

	void (*bi_prediction8x8)(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0);

	void (*copy_image_data_16x16_stride)(struct video_image *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source);
	int (*code_from_bitstream_2d_5_4)(SyntaxElement *sym, Bitstream *currStream, const uint8_t *lentab, const uint8_t *codtab, const uint8_t *masktab);
	int (*code_from_bitstream_2d_17_4)(SyntaxElement *sym, Bitstream *currStream, const uint16_t *lentab, const uint16_t *codtab, const uint16_t *masktab);
	int (*code_from_bitstream_2d_16_1)(Bitstream *currStream, const uint16_t *lentab,	const uint16_t *codtab, const uint16_t *masktab);
} OptimizedFunctions;

extern OptimizedFunctions opt;

/* define macros for these function calls.  this way we could do specific builds that call the functions directly, if we have the need */
#ifdef _DEBUG
#define opt_itrans4x4 (itrans4x4_c)
#else
#define opt_itrans4x4 (itrans4x4_mmx)
#endif
#define opt_itrans8x8 (opt.itrans8x8)

#define opt_weighted_mc_prediction16x16 (opt.weighted_mc_prediction16x16)
#define opt_weighted_mc_prediction16x8 (opt.weighted_mc_prediction16x8)
#define opt_weighted_mc_prediction8x8 (opt.weighted_mc_prediction8x8)

#define opt_weighted_bi_prediction16x16 (opt.weighted_bi_prediction16x16)
#define opt_weighted_bi_prediction16x8 (opt.weighted_bi_prediction16x8)
#define opt_weighted_bi_prediction8x8 (opt.weighted_bi_prediction8x8)

#define opt_bi_prediction8x8 (opt.bi_prediction8x8)
#define opt_copy_image_data_16x16_stride (opt.copy_image_data_16x16_stride)
#define opt_code_from_bitstream_2d_5_4 (opt.code_from_bitstream_2d_5_4)
#define opt_code_from_bitstream_2d_17_4 (opt.code_from_bitstream_2d_17_4)
#define opt_code_from_bitstream_2d_16_1 (opt.code_from_bitstream_2d_16_1)