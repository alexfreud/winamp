
/*!
 ************************************************************************
 * \file block.h
 *
 * \brief
 *    definitions for block decoding functions
 *
 * \author
 *  Inge Lille-Langoy               <inge.lille-langoy@telenor.com>    \n
 *  Telenor Satellite Services                                         \n
 *  P.O.Box 6914 St.Olavs plass                                        \n
 *  N-0130 Oslo, Norway
 *
 ************************************************************************
 */

#ifndef _BLOCK_H_
#define _BLOCK_H_

#include "global.h"
#include "transform8x8.h"

static const byte QP_SCALE_CR[52]=
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
   12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
   28,29,29,30,31,32,32,33,34,34,35,35,36,36,37,37,
   37,38,38,38,39,39,39,39

};

//! look up tables for FRExt_chroma support
static const unsigned char subblk_offset_x[3][8][4] =
{
  {
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}, 
  },
  { 
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}, 
  },
  {
    {0, 4, 0, 4},
    {8,12, 8,12},
    {0, 4, 0, 4},
    {8,12, 8,12},
    {0, 4, 0, 4},
    {8,12, 8,12},
    {0, 4, 0, 4},
    {8,12, 8,12}  
  }
};


static const unsigned char subblk_offset_y[3][8][4] =
{
  {
    {0, 0, 4, 4},
    {0, 0, 4, 4},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { 
    {0, 0, 4, 4},
    {8, 8,12,12},
    {0, 0, 4, 4},
    {8, 8,12,12},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { 
    {0, 0, 4, 4},
    {0, 0, 4, 4},
    {8, 8,12,12},
    {8, 8,12,12},
    {0, 0, 4, 4},
    {0, 0, 4, 4},
    {8, 8,12,12},
    {8, 8,12,12}
  }
};

static const byte decode_block_scan[16] = {0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15};
static const int cof4_pos_to_subblock[4][4] =  {	 { 0, 1, 4, 5},	 {2,3,6,7},	 {8,9,12,13},	 {10,11,14,15} };

extern void iMBtrans4x4(Macroblock *currMB, ColorPlane pl, int smb);
extern void iMBtrans8x8(Macroblock *currMB, ColorPlane pl);

extern void itrans_sp_cr(Macroblock *currMB, int uv);

extern void intrapred_chroma      (Macroblock *currMB, int uv);

extern void Inv_Residual_trans_4x4(Macroblock *currMB, ColorPlane pl, int ioff, int joff);
extern void Inv_Residual_trans_8x8(Macroblock *currMB, ColorPlane pl, int ioff,int joff);

extern void itrans4x4_c(const h264_short_block_t tblock, const h264_imgpel_macroblock_t mb_pred, h264_imgpel_macroblock_t mb_rec, int pos_x, int pos_y);
extern void itrans4x4_mmx(const h264_short_block_t tblock, const h264_imgpel_macroblock_t mb_pred, h264_imgpel_macroblock_t mb_rec, int pos_x, int pos_y);
extern int  intrapred   (Macroblock *currMB, ColorPlane pl, int ioff,int joff,int i4,int j4);
extern void itrans_2    (Macroblock *currMB, ColorPlane pl);
extern void iTransform  (Macroblock *currMB, ColorPlane pl, int smb);

extern void copy_image_data       (imgpel  **imgBuf1, imgpel  **imgBuf2, int dest_x, int src_x, int width, int height);
extern void copy_image_data_16x16 (imgpel  **imgBuf1, imgpel  **imgBuf2, int dest_x, int src_x);

extern void copy_image_data_16x16_stride_c(struct video_image *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source);
extern void copy_image_data_16x16_stride_sse(struct video_image *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source);
extern void copy_image_data_8x8_stride  (struct video_image *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source);
extern void copy_image_data_8x8_stride2  (struct video_image *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source, int src_x, int src_y);
extern void copy_image_data_4x4_stride  (struct video_image *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source, int src_x, int src_y);
extern void copy_image_data_stride      (struct video_image *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source, int width, int height);

#endif

