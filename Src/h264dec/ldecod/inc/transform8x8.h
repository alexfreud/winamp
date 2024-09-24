/*!
 ***************************************************************************
 *
 * \file transform8x8.h
 *
 * \brief
 *    prototypes of 8x8 transform functions
 *
 * \date
 *    9. October 2003
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Yuri Vatis
 **************************************************************************/

#ifndef _TRANSFORM8X8_H_
#define _TRANSFORM8X8_H_

extern void itrans8x8_mmx(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x);
extern void itrans8x8_sse2(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x);
extern void itrans8x8_c(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x);
extern void itrans8x8_lossless(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x);
#endif
