
/*!
 *************************************************************************************
 * \file mc_prediction.h
 *
 * \brief
 *    definitions for motion compensated prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */

#ifndef _MC_PREDICTION_H_
#define _MC_PREDICTION_H_

#include "global.h"
#include "mbuffer.h"

extern void get_block_luma  (Macroblock *currMB, ColorPlane pl, StorablePicture *list, int x_pos, int y_pos, const short *motion_vector, int ver_block_size, int hor_block_size, h264_imgpel_macroblock_t block);

extern void intra_cr_decoding    (Macroblock *currMB, int yuv);
extern void prepare_direct_params(Macroblock *currMB, StorablePicture *dec_picture, short pmvl0[2], short pmvl1[2],char *l0_rFrame, char *l1_rFrame);
extern void perform_mc           (Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset,   int block_size_x, int block_size_y, int curr_mb_field);
extern void perform_mc16x16      (Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int list_offset, int curr_mb_field);
extern void perform_mc16x8       (Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset,   int curr_mb_field);
extern void perform_mc8x8        (Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset,   int curr_mb_field);
extern void perform_mc8x16       (Macroblock *currMB, ColorPlane pl, StorablePicture *dec_picture, int pred_dir, int i, int j, int list_offset,   int curr_mb_field);

void weighted_mc_prediction16x16_sse2(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
void weighted_mc_prediction16x16_ipp(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
void weighted_mc_prediction16x16_c(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);

void weighted_mc_prediction16x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
void weighted_mc_prediction16x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
void weighted_mc_prediction16x8_c(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);

void weighted_mc_prediction8x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
void weighted_mc_prediction8x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);
void weighted_mc_prediction8x8_c(h264_imgpel_macroblock_row_t *mb_pred, int wp_scale, int wp_offset, int weight_denom);

void weighted_bi_prediction8x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
void weighted_bi_prediction8x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
void weighted_bi_prediction8x8_c(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0,  int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);

void weighted_bi_prediction16x16_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
void weighted_bi_prediction16x16_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
void weighted_bi_prediction16x16_c(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);

void weighted_bi_prediction16x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
void weighted_bi_prediction16x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);
void weighted_bi_prediction16x8_c(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0, int wp_scale_l0, int wp_scale_l1, int wp_offset, int weight_denom);

void bi_prediction8x8_sse2(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0);
void bi_prediction8x8_ipp(h264_imgpel_macroblock_row_t *mb_pred, const h264_imgpel_macroblock_t block_l0);

#endif

