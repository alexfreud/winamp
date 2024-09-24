/*!
 *************************************************************************************
 * \file intra8x8_pred.c
 *
 * \brief
 *    Functions for intra 8x8 prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Yuri Vatis
 *      - Jan Muenster
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */
#include "global.h"
#include "intra8x8_pred.h"
#include "mb_access.h"
#include "image.h"

// Notation for comments regarding prediction and predictors.
// The pels of the 8x8 block are labeled a..p. The predictor pels above
// are labeled A..H, from the left I..P, and from above left X, as follows:
//
//  0   1   2   3   4   5  6  7  8  9  10  11  12  13   14  15  16
//  17  a1  b1  c1  d1  e1 f1 g1 h1
//  18  a2  b2  c2  d2  e2 f2 g2 h2
//  19  a3  b3  c3  d3  e3 f3 g3 h3
//  20  a4  b4  c4  d4  e4 f4 g4 h4
//  21  a5  b5  c5  d5  e5 f5 g5 h5
//  22  a6  b6  c6  d6  e6 f6 g6 h6
//  23  a7  b7  c7  d7  e7 f7 g7 h7
//  24  a8  b8  c8  d8  e8 f8 g8 h8


static void memset_8x8(h264_imgpel_macroblock_row_t *mb_pred, int offset_x, int pred)
{
#ifdef _M_IX86
	// benski> can't believe the shitty code that the compiler generated...  this code is better
	__m64 mmx_pred = _mm_set1_pi8(pred);
	mb_pred = (h264_imgpel_macroblock_row_t *)&mb_pred[0][offset_x];
	*(__m64 *)mb_pred[0] = mmx_pred;
	*(__m64 *)mb_pred[1] = mmx_pred;
	*(__m64 *)mb_pred[2] = mmx_pred;
	*(__m64 *)mb_pred[3] = mmx_pred;
	*(__m64 *)mb_pred[4] = mmx_pred;
	*(__m64 *)mb_pred[5] = mmx_pred;
	*(__m64 *)mb_pred[6] = mmx_pred;
	*(__m64 *)mb_pred[7] = mmx_pred;
#else
	int ii, jj;
	for (jj = 0; jj < BLOCK_SIZE_8x8; jj++)
	{
		for (ii = 0; ii < BLOCK_SIZE_8x8; ii++)
		{
			mb_pred[jj][offset_x+ii]=(imgpel) pred;
		}
	}
#endif
}

static void memset_8x8_row(h264_imgpel_macroblock_row_t *mb_pred, int offset_x, const imgpel row[8])
{
#ifdef _M_IX86
	// benski> can't believe the shitty code that the compiler generated...  this code is better
	__m64 mmx_pred = *(__m64 *)row;
	mb_pred = (h264_imgpel_macroblock_row_t *)&mb_pred[0][offset_x];
	*(__m64 *)mb_pred[0] = mmx_pred;
	*(__m64 *)mb_pred[1] = mmx_pred;
	*(__m64 *)mb_pred[2] = mmx_pred;
	*(__m64 *)mb_pred[3] = mmx_pred;
	*(__m64 *)mb_pred[4] = mmx_pred;
	*(__m64 *)mb_pred[5] = mmx_pred;
	*(__m64 *)mb_pred[6] = mmx_pred;
	*(__m64 *)mb_pred[7] = mmx_pred;
#else
	int jj;
	for (jj = 0; jj < BLOCK_SIZE_8x8; jj++)
	{
			memcpy(&mb_pred[jj][offset_x], row, 8);
	}
#endif
}

/*!
 *************************************************************************************
 * \brief
 *    Prefiltering for Intra8x8 prediction
 *************************************************************************************
 */
static __forceinline void LowPassForIntra8x8Pred(imgpel *PredPel, int block_up_left, int block_up, int block_left)
{
  imgpel LoopArray[25];

  memcpy(&LoopArray[0], &PredPel[0], 25 * sizeof(imgpel));

  if(block_up_left)
  {
    if(block_up && block_left)
    {
      PredPel[0] = (imgpel) ((LoopArray[17] + (LoopArray[0]<<1) + LoopArray[1] + 2)>>2);
    }
    else
    {
      if(block_up)
        PredPel[0] = (imgpel) ((LoopArray[0] + (LoopArray[0]<<1) + LoopArray[1] + 2)>>2);
      else if (block_left)
        PredPel[0] = (imgpel) ((LoopArray[0] + (LoopArray[0]<<1) + LoopArray[17] + 2)>>2);
    }
  }
  
  if(block_up)
  {    
    if(block_up_left)
    {
      PredPel[1] = (imgpel) ((LoopArray[0] + (LoopArray[1]<<1) + LoopArray[2] + 2)>>2);
    }
    else
      PredPel[1] = (imgpel) ((LoopArray[1] + (LoopArray[1]<<1) + LoopArray[2] + 2)>>2);


      PredPel[2] = (imgpel) ((LoopArray[2-1] + (LoopArray[2]<<1) + LoopArray[2+1] + 2)>>2);
			PredPel[3] = (imgpel) ((LoopArray[3-1] + (LoopArray[3]<<1) + LoopArray[3+1] + 2)>>2);
			PredPel[4] = (imgpel) ((LoopArray[4-1] + (LoopArray[4]<<1) + LoopArray[4+1] + 2)>>2);
			PredPel[5] = (imgpel) ((LoopArray[5-1] + (LoopArray[5]<<1) + LoopArray[5+1] + 2)>>2);
			PredPel[6] = (imgpel) ((LoopArray[6-1] + (LoopArray[6]<<1) + LoopArray[6+1] + 2)>>2);
			PredPel[7] = (imgpel) ((LoopArray[7-1] + (LoopArray[7]<<1) + LoopArray[7+1] + 2)>>2);
			PredPel[8] = (imgpel) ((LoopArray[8-1] + (LoopArray[8]<<1) + LoopArray[8+1] + 2)>>2);
			PredPel[9] = (imgpel) ((LoopArray[9-1] + (LoopArray[9]<<1) + LoopArray[9+1] + 2)>>2);
			PredPel[10] = (imgpel) ((LoopArray[10-1] + (LoopArray[10]<<1) + LoopArray[10+1] + 2)>>2);
			PredPel[11] = (imgpel) ((LoopArray[11-1] + (LoopArray[11]<<1) + LoopArray[11+1] + 2)>>2);
			PredPel[12] = (imgpel) ((LoopArray[12-1] + (LoopArray[12]<<1) + LoopArray[12+1] + 2)>>2);
			PredPel[13] = (imgpel) ((LoopArray[13-1] + (LoopArray[13]<<1) + LoopArray[13+1] + 2)>>2);
			PredPel[14] = (imgpel) ((LoopArray[14-1] + (LoopArray[14]<<1) + LoopArray[14+1] + 2)>>2);
			PredPel[15] = (imgpel) ((LoopArray[15-1] + (LoopArray[15]<<1) + LoopArray[15+1] + 2)>>2);

    PredPel[16] = (imgpel) ((LoopArray[16] + (LoopArray[16]<<1) + LoopArray[15] + 2)>>2);
  }

  if(block_left)
  {
    if(block_up_left)
      PredPel[17] = (imgpel) ((LoopArray[0] + (LoopArray[17]<<1) + LoopArray[18] + 2)>>2);
    else
      PredPel[17] = (imgpel) ((LoopArray[17] + (LoopArray[17]<<1) + LoopArray[18] + 2)>>2);

    PredPel[18] = (imgpel) ((LoopArray[18-1] + (LoopArray[18]<<1) + LoopArray[18+1] + 2)>>2);
		PredPel[19] = (imgpel) ((LoopArray[19-1] + (LoopArray[19]<<1) + LoopArray[19+1] + 2)>>2);
		PredPel[20] = (imgpel) ((LoopArray[20-1] + (LoopArray[20]<<1) + LoopArray[20+1] + 2)>>2);
		PredPel[21] = (imgpel) ((LoopArray[21-1] + (LoopArray[21]<<1) + LoopArray[21+1] + 2)>>2);
		PredPel[22] = (imgpel) ((LoopArray[22-1] + (LoopArray[22]<<1) + LoopArray[22+1] + 2)>>2);
		PredPel[23] = (imgpel) ((LoopArray[23-1] + (LoopArray[23]<<1) + LoopArray[23+1] + 2)>>2);

    PredPel[24] = (imgpel) ((LoopArray[23] + (LoopArray[24]<<1) + LoopArray[24] + 2) >> 2);
  }

  //memcpy(&PredPel[0], &LoopArray[0], 25 * sizeof(imgpel));
}

/*!
 *************************************************************************************
 * \brief
 *    Prefiltering for Intra8x8 prediction (Horizontal)
 *************************************************************************************
 */
static __forceinline void LowPassForIntra8x8PredHor(imgpel *PredPel, int block_up_left, int block_up, int block_left)
{
  imgpel LoopArray[16];

  memcpy(&LoopArray[0], &PredPel[0], 16 * sizeof(imgpel));

  if(block_up_left)
  {
    if(block_up && block_left)
    {
      PredPel[0] = (imgpel) ((PredPel[17] + (LoopArray[0]<<1) + LoopArray[1] + 2)>>2);
    }
    else
    {
      if(block_up)
        PredPel[0] = (imgpel) ((LoopArray[0] + (LoopArray[0]<<1) + LoopArray[1] + 2)>>2);
      else if (block_left)
        PredPel[0] = (imgpel) ((LoopArray[0] + (LoopArray[0]<<1) + PredPel[17] + 2)>>2);
    }
  }
  
  if(block_up)
  {    
    if(block_up_left)
    {
      PredPel[1] = (imgpel) ((LoopArray[0] + (LoopArray[1]<<1) + LoopArray[2] + 2)>>2);
    }
    else
      PredPel[1] = (imgpel) ((LoopArray[1] + (LoopArray[1]<<1) + LoopArray[2] + 2)>>2);


		PredPel[2] = (imgpel) ((LoopArray[2-1] + (LoopArray[2]<<1) + LoopArray[2+1] + 2)>>2);
		PredPel[3] = (imgpel) ((LoopArray[3-1] + (LoopArray[3]<<1) + LoopArray[3+1] + 2)>>2);
		PredPel[4] = (imgpel) ((LoopArray[4-1] + (LoopArray[4]<<1) + LoopArray[4+1] + 2)>>2);
		PredPel[5] = (imgpel) ((LoopArray[5-1] + (LoopArray[5]<<1) + LoopArray[5+1] + 2)>>2);
		PredPel[6] = (imgpel) ((LoopArray[6-1] + (LoopArray[6]<<1) + LoopArray[6+1] + 2)>>2);
		PredPel[7] = (imgpel) ((LoopArray[7-1] + (LoopArray[7]<<1) + LoopArray[7+1] + 2)>>2);
		PredPel[8] = (imgpel) ((LoopArray[8-1] + (LoopArray[8]<<1) + LoopArray[8+1] + 2)>>2);
		PredPel[9] = (imgpel) ((LoopArray[9-1] + (LoopArray[9]<<1) + LoopArray[9+1] + 2)>>2);
		PredPel[10] = (imgpel) ((LoopArray[10-1] + (LoopArray[10]<<1) + LoopArray[10+1] + 2)>>2);
		PredPel[11] = (imgpel) ((LoopArray[11-1] + (LoopArray[11]<<1) + LoopArray[11+1] + 2)>>2);
		PredPel[12] = (imgpel) ((LoopArray[12-1] + (LoopArray[12]<<1) + LoopArray[12+1] + 2)>>2);
		PredPel[13] = (imgpel) ((LoopArray[13-1] + (LoopArray[13]<<1) + LoopArray[13+1] + 2)>>2);
		PredPel[14] = (imgpel) ((LoopArray[14-1] + (LoopArray[14]<<1) + LoopArray[14+1] + 2)>>2);
		PredPel[15] = (imgpel) ((LoopArray[15-1] + (LoopArray[15]<<1) + PredPel[15+1] + 2)>>2);
		PredPel[16] = (imgpel) ((PredPel[16] + (PredPel[16]<<1) + LoopArray[15] + 2)>>2);
  }


  //memcpy(&PredPel[0], &LoopArray[0], 17 * sizeof(imgpel));
}

/*!
 *************************************************************************************
 * \brief
 *    Prefiltering for Intra8x8 prediction (Vertical)
 *************************************************************************************
 */
static __forceinline void LowPassForIntra8x8PredVer(imgpel *PredPel, int block_up_left, int block_up, int block_left)
{
  // These functions need some cleanup and can be further optimized. 
  // For convenience, let us copy all data for now. It is obvious that the filtering makes things a bit more "complex"
  int i;
  imgpel LoopArray[25];

  //memcpy(&LoopArray[0], &PredPel[0], 25 * sizeof(imgpel));
	LoopArray[0] = PredPel[0];
	LoopArray[1] = PredPel[1];
	LoopArray[17] = PredPel[17];
	LoopArray[18] = PredPel[18];
	LoopArray[19] = PredPel[19];
	LoopArray[20] = PredPel[20];
	LoopArray[21] = PredPel[21];
	LoopArray[22] = PredPel[22];
	LoopArray[23] = PredPel[23];
	LoopArray[24] = PredPel[24];

  if(block_up_left)
  {
    if(block_up && block_left)
    {
      PredPel[0] = (imgpel) ((LoopArray[17] + (LoopArray[0]<<1) + LoopArray[1] + 2)>>2);
    }
    else
    {
      if(block_up)
        PredPel[0] = (imgpel) ((LoopArray[0] + (LoopArray[0]<<1) + LoopArray[1] + 2)>>2);
      else if (block_left)
        PredPel[0] = (imgpel) ((LoopArray[0] + (LoopArray[0]<<1) + LoopArray[17] + 2)>>2);
    }
  }
  
  if(block_left)
  {
    if(block_up_left)
      PredPel[17] = (imgpel) ((LoopArray[0] + (LoopArray[17]<<1) + LoopArray[18] + 2)>>2);
    else
      PredPel[17] = (imgpel) ((LoopArray[17] + (LoopArray[17]<<1) + LoopArray[18] + 2)>>2);

    for(i = 18; i <24; i++)
    {
      PredPel[i] = (imgpel) ((LoopArray[i-1] + (LoopArray[i]<<1) + LoopArray[i+1] + 2)>>2);
    }
    PredPel[24] = (imgpel) ((LoopArray[23] + (LoopArray[24]<<1) + LoopArray[24] + 2) >> 2);
  }

  //memcpy(&PredPel[0], &LoopArray[0], 25 * sizeof(imgpel));
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 DC prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_dc_pred(Macroblock *currMB,    //!< current macroblock
                                   ColorPlane pl,         //!< current image plane
                                   int ioff,              //!< pixel offset X within MB
                                   int joff)              //!< pixel offset Y within MB
{
  int s0 = 0;
  imgpel PredPel[25];  // array of predictor pels
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  StorablePicture *dec_picture = p_Vid->dec_picture;
  imgpel **imgY = (pl) ? dec_picture->imgUV[pl - 1]->img : dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;
  
  imgpel *pred_pels;

	if (ioff == 0)
	{
			p_Vid->getNeighbourNPLumaNB(currMB, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourNPLumaNB(currMB, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourNPLumaNB(currMB, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourNPLumaNB(currMB, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourNPLumaNB(currMB, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourNPLumaNB(currMB, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourNPLumaNB(currMB, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourNPLumaNB(currMB,  joff + 7,  &pix_a[7]);

	p_Vid->getNeighbour0XLuma(currMB, joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLumaNB(currMB, 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourNXLuma(currMB, joff - 1,  &pix_d);
	}
	else
	{ // ioff == 8
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourPPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourPXLumaNB(currMB, ioff - 1, joff - 1,  &pix_d);
	}
  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
		int i;
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

	// form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = pred_pels[0];
    PredPel[2] = pred_pels[1];
    PredPel[3] = pred_pels[2];
    PredPel[4] = pred_pels[3];
    PredPel[5] = pred_pels[4];
    PredPel[6] = pred_pels[5];
    PredPel[7] = pred_pels[6];
    PredPel[8] = pred_pels[7];
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = pred_pels[0];
    PredPel[10] = pred_pels[1];
    PredPel[11] = pred_pels[2];
    PredPel[12] = pred_pels[3];
    PredPel[13] = pred_pels[4];
    PredPel[14] = pred_pels[5];
    PredPel[15] = pred_pels[6];
    PredPel[16] = pred_pels[7];

  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[0].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[0].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[0].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[0].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[0].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[0].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[0].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8Pred(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);
  
  if (block_available_up && block_available_left)
  {
    // no edge
    s0 = (PredPel[1] + PredPel[2] + PredPel[3] + PredPel[4] + PredPel[5] + PredPel[6] + PredPel[7] + PredPel[8] + PredPel[17] + PredPel[18] + PredPel[19] + PredPel[20] + PredPel[21] + PredPel[22] + PredPel[23] + PredPel[24] + 8) >> 4;
  }
  else if (!block_available_up && block_available_left)
  {
    // upper edge
    s0 = (PredPel[17] + PredPel[18] + PredPel[19] + PredPel[20] + PredPel[21] + PredPel[22] + PredPel[23] + PredPel[24] + 4) >> 3;
  }
  else if (block_available_up && !block_available_left)
  {
    // left edge
    s0 = (PredPel[1] + PredPel[2] + PredPel[3] + PredPel[4] + PredPel[5] + PredPel[6] + PredPel[7] + PredPel[8] + 4) >> 3;
  }
  else //if (!block_available_up && !block_available_left)
  {
    // top left corner, nothing to predict from
    s0 = p_Vid->dc_pred_value_comp[pl];
  }

	memset_8x8(&currSlice->mb_pred[pl][joff], ioff, s0);

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 vertical prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_vert_pred(Macroblock *currMB,    //!< current macroblock
                                     ColorPlane pl,         //!< current image plane
                                     int ioff,              //!< pixel offset X within MB
                                     int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i;
  imgpel PredPel[25];  // array of predictor pels  
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;

  
  imgpel *pred_pels;

    p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 0,  &pix_a[0]);
		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 1,  &pix_a[1]);
		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 2,  &pix_a[2]);
		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 3,  &pix_a[3]);
		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 4,  &pix_a[4]);
		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 5,  &pix_a[5]);
		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 6,  &pix_a[6]);
		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, ioff - 1, joff + 7,  &pix_a[7]);

  p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1, &pix_b);
  p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1, &pix_c);
  p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1, &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if (!block_available_up)
    printf ("warning: Intra_8x8_Vertical prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = *(pred_pels ++);
    PredPel[2] = *(pred_pels ++);
    PredPel[3] = *(pred_pels ++);
    PredPel[4] = *(pred_pels ++);
    PredPel[5] = *(pred_pels ++);
    PredPel[6] = *(pred_pels ++);
    PredPel[7] = *(pred_pels ++);
    PredPel[8] = *pred_pels;
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = *(pred_pels ++);
    PredPel[10] = *(pred_pels ++);
    PredPel[11] = *(pred_pels ++);
    PredPel[12] = *(pred_pels ++);
    PredPel[13] = *(pred_pels ++);
    PredPel[14] = *(pred_pels ++);
    PredPel[15] = *(pred_pels ++);
    PredPel[16] = *pred_pels;
  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8PredHor(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);
  
	memset_8x8_row(&currSlice->mb_pred[pl][joff], ioff, &PredPel[1]);

	return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 horizontal prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_hor_pred(Macroblock *currMB,    //!< current macroblock
                                    ColorPlane pl,         //!< current image plane
                                    int ioff,              //!< pixel offset X within MB
                                    int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  

  int i,j;
  imgpel PredPel[25];  // array of predictor pels
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;
  int jpos;  

	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1,  &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if (!block_available_left)
    printf ("warning: Intra_8x8_Horizontal prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[4].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[5].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[6].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8PredVer(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);

  for (j=0; j < BLOCK_SIZE_8x8; j++)
  {
    jpos = j + joff;
    currSlice->mb_pred[pl][jpos][ioff]  =
      currSlice->mb_pred[pl][jpos][ioff+1]  =
      currSlice->mb_pred[pl][jpos][ioff+2]  =
      currSlice->mb_pred[pl][jpos][ioff+3]  =
      currSlice->mb_pred[pl][jpos][ioff+4]  =
      currSlice->mb_pred[pl][jpos][ioff+5]  =
      currSlice->mb_pred[pl][jpos][ioff+6]  =
      currSlice->mb_pred[pl][jpos][ioff+7]  = (imgpel) (&PredPel[17])[j];
  }
 
  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 diagonal down right prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_diag_down_right_pred(Macroblock *currMB,    //!< current macroblock
                                                ColorPlane pl,         //!< current image plane
                                                int ioff,              //!< pixel offset X within MB
                                                int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  

  int i;
  imgpel PredPel[25];  // array of predictor pels
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;
h264_imgpel_macroblock_row_t *pred;
  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;

  imgpel *pred_pels;

	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1,  &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
    printf ("warning: Intra_8x8_Diagonal_Down_Right prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = pred_pels[0];
    PredPel[2] = pred_pels[1];
    PredPel[3] = pred_pels[2];
    PredPel[4] = pred_pels[3];
    PredPel[5] = pred_pels[4];
    PredPel[6] = pred_pels[5];
    PredPel[7] = pred_pels[6];
    PredPel[8] = pred_pels[7];
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = pred_pels[0];
    PredPel[10] = pred_pels[1];
    PredPel[11] = pred_pels[2];
    PredPel[12] = pred_pels[3];
    PredPel[13] = pred_pels[4];
    PredPel[14] = pred_pels[5];
    PredPel[15] = pred_pels[6];
    PredPel[16] = pred_pels[7];

  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[4].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[5].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[6].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8Pred(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);

  // Mode DIAG_DOWN_RIGHT_PRED
	pred = &currSlice->mb_pred[pl][joff];
  pred[0+7][ioff+0] = (imgpel) ((PredPel[24] + PredPel[22] + 2*(PredPel[23]) + 2) >> 2);
  pred[0+6][ioff+0] =
    pred[0+7][ioff+1] = (imgpel) ((PredPel[23] + PredPel[21] + 2*(PredPel[22]) + 2) >> 2);
  pred[0+5][ioff+0] =
    pred[0+6][ioff+1] =
    pred[0+7][ioff+2] = (imgpel) ((PredPel[22] + PredPel[20] + 2*(PredPel[21]) + 2) >> 2);
  pred[0+4][ioff+0] =
    pred[0+5][ioff+1] =
    pred[0+6][ioff+2] =
    pred[0+7][ioff+3] = (imgpel) ((PredPel[21] + PredPel[19] + 2*(PredPel[20]) + 2) >> 2);
  pred[0+3][ioff+0] =
    pred[0+4][ioff+1] =
    pred[0+5][ioff+2] =
    pred[0+6][ioff+3] =
    pred[0+7][ioff+4] = (imgpel) ((PredPel[20] + PredPel[18] + 2*(PredPel[19]) + 2) >> 2);
  pred[0+2][ioff+0] =
    pred[0+3][ioff+1] =
    pred[0+4][ioff+2] =
    pred[0+5][ioff+3] =
    pred[0+6][ioff+4] =
    pred[0+7][ioff+5] = (imgpel) ((PredPel[19] + PredPel[17] + 2*(PredPel[18]) + 2) >> 2);
  pred[0+1][ioff+0] =
    pred[0+2][ioff+1] =
    pred[0+3][ioff+2] =
    pred[0+4][ioff+3] =
    pred[0+5][ioff+4] =
    pred[0+6][ioff+5] =
    pred[0+7][ioff+6] = (imgpel) ((PredPel[18] + PredPel[0] + 2*(PredPel[17]) + 2) >> 2);
  pred[0+0][ioff+0] =
    pred[0+1][ioff+1] =
    pred[0+2][ioff+2] =
    pred[0+3][ioff+3] =
    pred[0+4][ioff+4] =
    pred[0+5][ioff+5] =
    pred[0+6][ioff+6] =
    pred[0+7][ioff+7] = (imgpel) ((PredPel[17] + PredPel[1] + 2*(PredPel[0]) + 2) >> 2);
  pred[0+0][ioff+1] =
    pred[0+1][ioff+2] =
    pred[0+2][ioff+3] =
    pred[0+3][ioff+4] =
    pred[0+4][ioff+5] =
    pred[0+5][ioff+6] =
    pred[0+6][ioff+7] = (imgpel) ((PredPel[0] + PredPel[2] + 2*(PredPel[1]) + 2) >> 2);
  pred[0+0][ioff+2] =
    pred[0+1][ioff+3] =
    pred[0+2][ioff+4] =
    pred[0+3][ioff+5] =
    pred[0+4][ioff+6] =
    pred[0+5][ioff+7] = (imgpel) ((PredPel[1] + PredPel[3] + 2*(PredPel[2]) + 2) >> 2);
  pred[0+0][ioff+3] =
    pred[0+1][ioff+4] =
    pred[0+2][ioff+5] =
    pred[0+3][ioff+6] =
    pred[0+4][ioff+7] = (imgpel) ((PredPel[2] + PredPel[4] + 2*(PredPel[3]) + 2) >> 2);
  pred[0+0][ioff+4] =
    pred[0+1][ioff+5] =
    pred[0+2][ioff+6] =
    pred[0+3][ioff+7] = (imgpel) ((PredPel[3] + PredPel[5] + 2*(PredPel[4]) + 2) >> 2);
  pred[0+0][ioff+5] =
    pred[0+1][ioff+6] =
    pred[0+2][ioff+7] = (imgpel) ((PredPel[4] + PredPel[6] + 2*(PredPel[5]) + 2) >> 2);
  pred[0+0][ioff+6] =
    pred[0+1][ioff+7] = (imgpel) ((PredPel[5] + PredPel[7] + 2*(PredPel[6]) + 2) >> 2);
  pred[0+0][ioff+7] = (imgpel) ((PredPel[6] + PredPel[8] + 2*(PredPel[7]) + 2) >> 2);
 
  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 diagonal down left prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_diag_down_left_pred(Macroblock *currMB,    //!< current macroblock
                                               ColorPlane pl,         //!< current image plane
                                               int ioff,              //!< pixel offset X within MB
                                               int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  h264_imgpel_macroblock_row_t *pred;
  int i;
  imgpel PredPel[25];  // array of predictor pels
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;

 
  imgpel *pred_pels;

	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1,  &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if (!block_available_up)
    printf ("warning: Intra_8x8_Diagonal_Down_Left prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = pred_pels[0];
    PredPel[2] = pred_pels[1];
    PredPel[3] = pred_pels[2];
    PredPel[4] = pred_pels[3];
    PredPel[5] = pred_pels[4];
    PredPel[6] = pred_pels[5];
    PredPel[7] = pred_pels[6];
    PredPel[8] = pred_pels[7];
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = pred_pels[0];
    PredPel[10] = pred_pels[1];
    PredPel[11] = pred_pels[2];
    PredPel[12] = pred_pels[3];
    PredPel[13] = pred_pels[4];
    PredPel[14] = pred_pels[5];
    PredPel[15] = pred_pels[6];
    PredPel[16] = pred_pels[7];

  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[4].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[5].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[6].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8Pred(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);

  // Mode DIAG_DOWN_LEFT_PRED
	pred = &currSlice->mb_pred[pl][joff];
  pred[0+0][ioff+0] = (imgpel) ((PredPel[1] + PredPel[3] + 2*(PredPel[2]) + 2) >> 2);
  pred[0+1][ioff+0] =
    pred[0+0][ioff+1] = (imgpel) ((PredPel[2] + PredPel[4] + 2*(PredPel[3]) + 2) >> 2);
  pred[0+2][ioff+0] =
    pred[0+1][ioff+1] =
    pred[0+0][ioff+2] = (imgpel) ((PredPel[3] + PredPel[5] + 2*(PredPel[4]) + 2) >> 2);
  pred[0+3][ioff+0] =
    pred[0+2][ioff+1] =
    pred[0+1][ioff+2] =
    pred[0+0][ioff+3] = (imgpel) ((PredPel[4] + PredPel[6] + 2*(PredPel[5]) + 2) >> 2);
  pred[0+4][ioff+0] =
    pred[0+3][ioff+1] =
    pred[0+2][ioff+2] =
    pred[0+1][ioff+3] =
    pred[0+0][ioff+4] = (imgpel) ((PredPel[5] + PredPel[7] + 2*(PredPel[6]) + 2) >> 2);
  pred[0+5][ioff+0] =
    pred[0+4][ioff+1] =
    pred[0+3][ioff+2] =
    pred[0+2][ioff+3] =
    pred[0+1][ioff+4] =
    pred[0+0][ioff+5] = (imgpel) ((PredPel[6] + PredPel[8] + 2*(PredPel[7]) + 2) >> 2);
  pred[0+6][ioff+0] =
    pred[0+5][ioff+1] =
    pred[0+4][ioff+2] =
    pred[0+3][ioff+3] =
    pred[0+2][ioff+4] =
    pred[0+1][ioff+5] =
    pred[0+0][ioff+6] = (imgpel) ((PredPel[7] + PredPel[9] + 2*(PredPel[8]) + 2) >> 2);
  pred[0+7][ioff+0] =
    pred[0+6][ioff+1] =
    pred[0+5][ioff+2] =
    pred[0+4][ioff+3] =
    pred[0+3][ioff+4] =
    pred[0+2][ioff+5] =
    pred[0+1][ioff+6] =
    pred[0+0][ioff+7] = (imgpel) ((PredPel[8] + PredPel[10] + 2*(PredPel[9]) + 2) >> 2);
  pred[0+7][ioff+1] =
    pred[0+6][ioff+2] =
    pred[0+5][ioff+3] =
    pred[0+4][ioff+4] =
    pred[0+3][ioff+5] =
    pred[0+2][ioff+6] =
    pred[0+1][ioff+7] = (imgpel) ((PredPel[9] + PredPel[11] + 2*(PredPel[10]) + 2) >> 2);
  pred[0+7][ioff+2] =
    pred[0+6][ioff+3] =
    pred[0+5][ioff+4] =
    pred[0+4][ioff+5] =
    pred[0+3][ioff+6] =
    pred[0+2][ioff+7] = (imgpel) ((PredPel[10] + PredPel[12] + 2*(PredPel[11]) + 2) >> 2);
  pred[0+7][ioff+3] =
    pred[0+6][ioff+4] =
    pred[0+5][ioff+5] =
    pred[0+4][ioff+6] =
    pred[0+3][ioff+7] = (imgpel) ((PredPel[11] + PredPel[13] + 2*(PredPel[12]) + 2) >> 2);
  pred[0+7][ioff+4] =
    pred[0+6][ioff+5] =
    pred[0+5][ioff+6] =
    pred[0+4][ioff+7] = (imgpel) ((PredPel[12] + PredPel[14] + 2*(PredPel[13]) + 2) >> 2);
  pred[0+7][ioff+5] =
    pred[0+6][ioff+6] =
    pred[0+5][ioff+7] = (imgpel) ((PredPel[13] + PredPel[15] + 2*(PredPel[14]) + 2) >> 2);
  pred[0+7][ioff+6] =
    pred[0+6][ioff+7] = (imgpel) ((PredPel[14] + PredPel[16] + 2*(PredPel[15]) + 2) >> 2);
  pred[0+7][ioff+7] = (imgpel) ((PredPel[15] + 3*(PredPel[16]) + 2) >> 2);

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 vertical right prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_vert_right_pred(Macroblock *currMB,    //!< current macroblock
                                           ColorPlane pl,         //!< current image plane
                                           int ioff,              //!< pixel offset X within MB
                                           int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  h264_imgpel_macroblock_row_t *pred;
  int i;
  imgpel PredPel[25];  // array of predictor pels
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;
  imgpel *pred_pels;

	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1,  &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
    printf ("warning: Intra_8x8_Vertical_Right prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = pred_pels[0];
    PredPel[2] = pred_pels[1];
    PredPel[3] = pred_pels[2];
    PredPel[4] = pred_pels[3];
    PredPel[5] = pred_pels[4];
    PredPel[6] = pred_pels[5];
    PredPel[7] = pred_pels[6];
    PredPel[8] = pred_pels[7];
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = pred_pels[0];
    PredPel[10] = pred_pels[1];
    PredPel[11] = pred_pels[2];
    PredPel[12] = pred_pels[3];
    PredPel[13] = pred_pels[4];
    PredPel[14] = pred_pels[5];
    PredPel[15] = pred_pels[6];
    PredPel[16] = pred_pels[7];

  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[4].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[5].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[6].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8Pred(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);
pred = &currSlice->mb_pred[pl][joff];
  pred[0+0][ioff+0] =
    pred[0+2][ioff+1] =
    pred[0+4][ioff+2] =
    pred[0+6][ioff+3] = (imgpel) ((PredPel[0] + PredPel[1] + 1) >> 1);
  pred[0+0][ioff+1] =
    pred[0+2][ioff+2] =
    pred[0+4][ioff+3] =
    pred[0+6][ioff+4] = (imgpel) ((PredPel[1] + PredPel[2] + 1) >> 1);
  pred[0+0][ioff+2] =
    pred[0+2][ioff+3] =
    pred[0+4][ioff+4] =
    pred[0+6][ioff+5] = (imgpel) ((PredPel[2] + PredPel[3] + 1) >> 1);
  pred[0+0][ioff+3] =
    pred[0+2][ioff+4] =
    pred[0+4][ioff+5] =
    pred[0+6][ioff+6] = (imgpel) ((PredPel[3] + PredPel[4] + 1) >> 1);
  pred[0+0][ioff+4] =
    pred[0+2][ioff+5] =
    pred[0+4][ioff+6] =
    pred[0+6][ioff+7] = (imgpel) ((PredPel[4] + PredPel[5] + 1) >> 1);
  pred[0+0][ioff+5] =
    pred[0+2][ioff+6] =
    pred[0+4][ioff+7] = (imgpel) ((PredPel[5] + PredPel[6] + 1) >> 1);
  pred[0+0][ioff+6] =
    pred[0+2][ioff+7] = (imgpel) ((PredPel[6] + PredPel[7] + 1) >> 1);
  pred[0+0][ioff+7] = (imgpel) ((PredPel[7] + PredPel[8] + 1) >> 1);
  pred[0+1][ioff+0] =
    pred[0+3][ioff+1] =
    pred[0+5][ioff+2] =
    pred[0+7][ioff+3] = (imgpel) ((PredPel[17] + PredPel[1] + 2*PredPel[0] + 2) >> 2);
  pred[0+1][ioff+1] =
    pred[0+3][ioff+2] =
    pred[0+5][ioff+3] =
    pred[0+7][ioff+4] = (imgpel) ((PredPel[0] + PredPel[2] + 2*PredPel[1] + 2) >> 2);
  pred[0+1][ioff+2] =
    pred[0+3][ioff+3] =
    pred[0+5][ioff+4] =
    pred[0+7][ioff+5] = (imgpel) ((PredPel[1] + PredPel[3] + 2*PredPel[2] + 2) >> 2);
  pred[0+1][ioff+3] =
    pred[0+3][ioff+4] =
    pred[0+5][ioff+5] =
    pred[0+7][ioff+6] = (imgpel) ((PredPel[2] + PredPel[4] + 2*PredPel[3] + 2) >> 2);
  pred[0+1][ioff+4] =
    pred[0+3][ioff+5] =
    pred[0+5][ioff+6] =
    pred[0+7][ioff+7] = (imgpel) ((PredPel[3] + PredPel[5] + 2*PredPel[4] + 2) >> 2);
  pred[0+1][ioff+5] =
    pred[0+3][ioff+6] =
    pred[0+5][ioff+7] = (imgpel) ((PredPel[4] + PredPel[6] + 2*PredPel[5] + 2) >> 2);
  pred[0+1][ioff+6] =
    pred[0+3][ioff+7] = (imgpel) ((PredPel[5] + PredPel[7] + 2*PredPel[6] + 2) >> 2);
  pred[0+1][ioff+7] = (imgpel) ((PredPel[6] + PredPel[8] + 2*PredPel[7] + 2) >> 2);
  pred[0+2][ioff+0] =
    pred[0+4][ioff+1] =
    pred[0+6][ioff+2] = (imgpel) ((PredPel[18] + PredPel[0] + 2*PredPel[17] + 2) >> 2);
  pred[0+3][ioff+0] =
    pred[0+5][ioff+1] =
    pred[0+7][ioff+2] = (imgpel) ((PredPel[19] + PredPel[17] + 2*PredPel[18] + 2) >> 2);
  pred[0+4][ioff+0] =
    pred[0+6][ioff+1] = (imgpel) ((PredPel[20] + PredPel[18] + 2*PredPel[19] + 2) >> 2);
  pred[0+5][ioff+0] =
    pred[0+7][ioff+1] = (imgpel) ((PredPel[21] + PredPel[19] + 2*PredPel[20] + 2) >> 2);
  pred[0+6][ioff+0] = (imgpel) ((PredPel[22] + PredPel[20] + 2*PredPel[21] + 2) >> 2);
  pred[0+7][ioff+0] = (imgpel) ((PredPel[23] + PredPel[21] + 2*PredPel[22] + 2) >> 2);

  return DECODING_OK;
}


/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 vertical left prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_vert_left_pred(Macroblock *currMB,    //!< current macroblock
                                          ColorPlane pl,         //!< current image plane
                                          int ioff,              //!< pixel offset X within MB
                                          int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i;
  imgpel PredPel[25];  // array of predictor pels  
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;
h264_imgpel_macroblock_row_t *pred;
  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;

  imgpel *pred_pels;

	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1,  &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if (!block_available_up)
    printf ("warning: Intra_4x4_Vertical_Left prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = pred_pels[0];
    PredPel[2] = pred_pels[1];
    PredPel[3] = pred_pels[2];
    PredPel[4] = pred_pels[3];
    PredPel[5] = pred_pels[4];
    PredPel[6] = pred_pels[5];
    PredPel[7] = pred_pels[6];
    PredPel[8] = pred_pels[7];
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = pred_pels[0];
    PredPel[10] = pred_pels[1];
    PredPel[11] = pred_pels[2];
    PredPel[12] = pred_pels[3];
    PredPel[13] = pred_pels[4];
    PredPel[14] = pred_pels[5];
    PredPel[15] = pred_pels[6];
    PredPel[16] = pred_pels[7];

  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[4].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[5].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[6].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8Pred(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);
pred = &currSlice->mb_pred[pl][joff];
  pred[0+0][ioff+0] = (imgpel) ((PredPel[1] + PredPel[2] + 1) >> 1);
  pred[0+0][ioff+1] =
    pred[0+2][ioff+0] = (imgpel) ((PredPel[2] + PredPel[3] + 1) >> 1);
  pred[0+0][ioff+2] =
    pred[0+2][ioff+1] =
    pred[0+4][ioff+0] = (imgpel) ((PredPel[3] + PredPel[4] + 1) >> 1);
  pred[0+0][ioff+3] =
    pred[0+2][ioff+2] =
    pred[0+4][ioff+1] =
    pred[0+6][ioff+0] = (imgpel) ((PredPel[4] + PredPel[5] + 1) >> 1);
  pred[0+0][ioff+4] =
    pred[0+2][ioff+3] =
    pred[0+4][ioff+2] =
    pred[0+6][ioff+1] = (imgpel) ((PredPel[5] + PredPel[6] + 1) >> 1);
  pred[0+0][ioff+5] =
    pred[0+2][ioff+4] =
    pred[0+4][ioff+3] =
    pred[0+6][ioff+2] = (imgpel) ((PredPel[6] + PredPel[7] + 1) >> 1);
  pred[0+0][ioff+6] =
    pred[0+2][ioff+5] =
    pred[0+4][ioff+4] =
    pred[0+6][ioff+3] = (imgpel) ((PredPel[7] + PredPel[8] + 1) >> 1);
  pred[0+0][ioff+7] =
    pred[0+2][ioff+6] =
    pred[0+4][ioff+5] =
    pred[0+6][ioff+4] = (imgpel) ((PredPel[8] + PredPel[9] + 1) >> 1);
  pred[0+2][ioff+7] =
    pred[0+4][ioff+6] =
    pred[0+6][ioff+5] = (imgpel) ((PredPel[9] + PredPel[10] + 1) >> 1);
  pred[0+4][ioff+7] =
    pred[0+6][ioff+6] = (imgpel) ((PredPel[10] + PredPel[11] + 1) >> 1);
  pred[0+6][ioff+7] = (imgpel) ((PredPel[11] + PredPel[12] + 1) >> 1);
  pred[0+1][ioff+0] = (imgpel) ((PredPel[1] + PredPel[3] + 2*PredPel[2] + 2) >> 2);
  pred[0+1][ioff+1] =
    pred[0+3][ioff+0] = (imgpel) ((PredPel[2] + PredPel[4] + 2*PredPel[3] + 2) >> 2);
  pred[0+1][ioff+2] =
    pred[0+3][ioff+1] =
    pred[0+5][ioff+0] = (imgpel) ((PredPel[3] + PredPel[5] + 2*PredPel[4] + 2) >> 2);
  pred[0+1][ioff+3] =
    pred[0+3][ioff+2] =
    pred[0+5][ioff+1] =
    pred[0+7][ioff+0] = (imgpel) ((PredPel[4] + PredPel[6] + 2*PredPel[5] + 2) >> 2);
  pred[0+1][ioff+4] =
    pred[0+3][ioff+3] =
    pred[0+5][ioff+2] =
    pred[0+7][ioff+1] = (imgpel) ((PredPel[5] + PredPel[7] + 2*PredPel[6] + 2) >> 2);
  pred[0+1][ioff+5] =
    pred[0+3][ioff+4] =
    pred[0+5][ioff+3] =
    pred[0+7][ioff+2] = (imgpel) ((PredPel[6] + PredPel[8] + 2*PredPel[7] + 2) >> 2);
  pred[0+1][ioff+6] =
    pred[0+3][ioff+5] =
    pred[0+5][ioff+4] =
    pred[0+7][ioff+3] = (imgpel) ((PredPel[7] + PredPel[9] + 2*PredPel[8] + 2) >> 2);
  pred[0+1][ioff+7] =
    pred[0+3][ioff+6] =
    pred[0+5][ioff+5] =
    pred[0+7][ioff+4] = (imgpel) ((PredPel[8] + PredPel[10] + 2*PredPel[9] + 2) >> 2);
  pred[0+3][ioff+7] =
    pred[0+5][ioff+6] =
    pred[0+7][ioff+5] = (imgpel) ((PredPel[9] + PredPel[11] + 2*PredPel[10] + 2) >> 2);
  pred[0+5][ioff+7] =
    pred[0+7][ioff+6] = (imgpel) ((PredPel[10] + PredPel[12] + 2*PredPel[11] + 2) >> 2);
  pred[0+7][ioff+7] = (imgpel) ((PredPel[11] + PredPel[13] + 2*PredPel[12] + 2) >> 2);

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 horizontal up prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_hor_up_pred(Macroblock *currMB,    //!< current macroblock
                                       ColorPlane pl,         //!< current image plane
                                       int ioff,              //!< pixel offset X within MB
                                       int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i;
  imgpel PredPel[25];  // array of predictor pels
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY
h264_imgpel_macroblock_row_t *pred;
  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;
  
  imgpel *pred_pels;
  

	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1,  &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if (!block_available_left)
    printf ("warning: Intra_8x8_Horizontal_Up prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = pred_pels[0];
    PredPel[2] = pred_pels[1];
    PredPel[3] = pred_pels[2];
    PredPel[4] = pred_pels[3];
    PredPel[5] = pred_pels[4];
    PredPel[6] = pred_pels[5];
    PredPel[7] = pred_pels[6];
    PredPel[8] = pred_pels[7];
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = pred_pels[0];
    PredPel[10] = pred_pels[1];
    PredPel[11] = pred_pels[2];
    PredPel[12] = pred_pels[3];
    PredPel[13] = pred_pels[4];
    PredPel[14] = pred_pels[5];
    PredPel[15] = pred_pels[6];
    PredPel[16] = pred_pels[7];

  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[4].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[5].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[6].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8Pred(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);
pred = &currSlice->mb_pred[pl][joff];
  pred[0+0][ioff+0] = (imgpel) ((PredPel[17] + PredPel[18] + 1) >> 1);
  pred[0+1][ioff+0] =
    pred[0+0][ioff+2] = (imgpel) ((PredPel[18] + PredPel[19] + 1) >> 1);
  pred[0+2][ioff+0] =
    pred[0+1][ioff+2] =
    pred[0+0][ioff+4] = (imgpel) ((PredPel[19] + PredPel[20] + 1) >> 1);
  pred[0+3][ioff+0] =
    pred[0+2][ioff+2] =
    pred[0+1][ioff+4] =
    pred[0+0][ioff+6] = (imgpel) ((PredPel[20] + PredPel[21] + 1) >> 1);
  pred[0+4][ioff+0] =
    pred[0+3][ioff+2] =
    pred[0+2][ioff+4] =
    pred[0+1][ioff+6] = (imgpel) ((PredPel[21] + PredPel[22] + 1) >> 1);
  pred[0+5][ioff+0] =
    pred[0+4][ioff+2] =
    pred[0+3][ioff+4] =
    pred[0+2][ioff+6] = (imgpel) ((PredPel[22] + PredPel[23] + 1) >> 1);
  pred[0+6][ioff+0] =
    pred[0+5][ioff+2] =
    pred[0+4][ioff+4] =
    pred[0+3][ioff+6] = (imgpel) ((PredPel[23] + PredPel[24] + 1) >> 1);
  pred[0+4][ioff+6] =
    pred[0+4][ioff+7] =
    pred[0+5][ioff+4] =
    pred[0+5][ioff+5] =
    pred[0+5][ioff+6] =
    pred[0+5][ioff+7] =
    pred[0+6][ioff+2] =
    pred[0+6][ioff+3] =
    pred[0+6][ioff+4] =
    pred[0+6][ioff+5] =
    pred[0+6][ioff+6] =
    pred[0+6][ioff+7] =
    pred[0+7][ioff+0] =
    pred[0+7][ioff+1] =
    pred[0+7][ioff+2] =
    pred[0+7][ioff+3] =
    pred[0+7][ioff+4] =
    pred[0+7][ioff+5] =
    pred[0+7][ioff+6] =
    pred[0+7][ioff+7] = (imgpel) PredPel[24];
  pred[0+6][ioff+1] =
    pred[0+5][ioff+3] =
    pred[0+4][ioff+5] =
    pred[0+3][ioff+7] = (imgpel) ((PredPel[23] + 3*PredPel[24] + 2) >> 2);
  pred[0+5][ioff+1] =
    pred[0+4][ioff+3] =
    pred[0+3][ioff+5] =
    pred[0+2][ioff+7] = (imgpel) ((PredPel[24] + PredPel[22] + 2*PredPel[23] + 2) >> 2);
  pred[0+4][ioff+1] =
    pred[0+3][ioff+3] =
    pred[0+2][ioff+5] =
    pred[0+1][ioff+7] = (imgpel) ((PredPel[23] + PredPel[21] + 2*PredPel[22] + 2) >> 2);
  pred[0+3][ioff+1] =
    pred[0+2][ioff+3] =
    pred[0+1][ioff+5] =
    pred[0+0][ioff+7] = (imgpel) ((PredPel[22] + PredPel[20] + 2*PredPel[21] + 2) >> 2);
  pred[0+2][ioff+1] =
    pred[0+1][ioff+3] =
    pred[0+0][ioff+5] = (imgpel) ((PredPel[21] + PredPel[19] + 2*PredPel[20] + 2) >> 2);
  pred[0+1][ioff+1] =
    pred[0+0][ioff+3] = (imgpel) ((PredPel[20] + PredPel[18] + 2*PredPel[19] + 2) >> 2);
  pred[0+0][ioff+1] = (imgpel) ((PredPel[19] + PredPel[17] + 2*PredPel[18] + 2) >> 2);

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 8x8 horizontal down prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra8x8_hor_down_pred(Macroblock *currMB,    //!< current macroblock
                                         ColorPlane pl,         //!< current image plane
                                         int ioff,              //!< pixel offset X within MB
                                         int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  int i;
  imgpel PredPel[25];  // array of predictor pels
  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img; // For MB level frame/field coding tools -- set default to imgY

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;
	h264_imgpel_macroblock_row_t *pred;
  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;
  
  imgpel *pred_pels;


	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 0,  &pix_a[0]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 1,  &pix_a[1]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 2,  &pix_a[2]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 3,  &pix_a[3]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 4,  &pix_a[4]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 5,  &pix_a[5]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 6,  &pix_a[6]);
	p_Vid->getNeighbourXPLumaNB(currMB, ioff - 1, joff + 7,  &pix_a[7]);

	p_Vid->getNeighbourPXLumaNB(currMB, ioff    , joff - 1,  &pix_b);
	p_Vid->getNeighbourPXLuma(currMB, ioff + 8, joff - 1,  &pix_c);
	p_Vid->getNeighbourLuma(currMB, ioff - 1, joff - 1,  &pix_d);
  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? p_Vid->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? p_Vid->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? p_Vid->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? p_Vid->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
    printf ("warning: Intra_8x8_Horizontal_Down prediction mode not allowed at mb %d\n", (int) p_Vid->current_mb_nr);

  // form predictor pels
  if (block_available_up)
  {
    pred_pels = &imgY[pix_b.pos_y][pix_b.pos_x];
    PredPel[1] = pred_pels[0];
    PredPel[2] = pred_pels[1];
    PredPel[3] = pred_pels[2];
    PredPel[4] = pred_pels[3];
    PredPel[5] = pred_pels[4];
    PredPel[6] = pred_pels[5];
    PredPel[7] = pred_pels[6];
    PredPel[8] = pred_pels[7];
  }
  else
  {
    PredPel[1] = PredPel[2] = PredPel[3] = PredPel[4] = PredPel[5] = PredPel[6] = PredPel[7] = PredPel[8] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    pred_pels = &imgY[pix_c.pos_y][pix_c.pos_x];
    PredPel[9] = pred_pels[0];
    PredPel[10] = pred_pels[1];
    PredPel[11] = pred_pels[2];
    PredPel[12] = pred_pels[3];
    PredPel[13] = pred_pels[4];
    PredPel[14] = pred_pels[5];
    PredPel[15] = pred_pels[6];
    PredPel[16] = pred_pels[7];

  }
  else
  {
    PredPel[9] = PredPel[10] = PredPel[11] = PredPel[12] = PredPel[13] = PredPel[14] = PredPel[15] = PredPel[16] = PredPel[8];
  }

  if (block_available_left)
  {
    PredPel[17] = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    PredPel[18] = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    PredPel[19] = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    PredPel[20] = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
    PredPel[21] = imgY[pix_a[4].pos_y][pix_a[4].pos_x];
    PredPel[22] = imgY[pix_a[5].pos_y][pix_a[5].pos_x];
    PredPel[23] = imgY[pix_a[6].pos_y][pix_a[6].pos_x];
    PredPel[24] = imgY[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    PredPel[17] = PredPel[18] = PredPel[19] = PredPel[20] = PredPel[21] = PredPel[22] = PredPel[23] = PredPel[24] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    PredPel[0] = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = (imgpel) p_Vid->dc_pred_value_comp[pl];
  }

  LowPassForIntra8x8Pred(&(PredPel[0]), block_available_up_left, block_available_up, block_available_left);
pred = &currSlice->mb_pred[pl][joff];
  pred[0][ioff] =
    pred[0+1][ioff+2] =
    pred[0+2][ioff+4] =
    pred[0+3][ioff+6] = (imgpel) ((PredPel[17] + PredPel[0] + 1) >> 1);
  pred[0+1][ioff] =
    pred[0+2][ioff+2] =
    pred[0+3][ioff+4] =
    pred[0+4][ioff+6] = (imgpel) ((PredPel[18] + PredPel[17] + 1) >> 1);
  pred[0+2][ioff] =
    pred[0+3][ioff+2] =
    pred[0+4][ioff+4] =
    pred[0+5][ioff+6] = (imgpel) ((PredPel[19] + PredPel[18] + 1) >> 1);
  pred[0+3][ioff] =
    pred[0+4][ioff+2] =
    pred[0+5][ioff+4] =
    pred[0+6][ioff+6] = (imgpel) ((PredPel[20] + PredPel[19] + 1) >> 1);
  pred[0+4][ioff] =
    pred[0+5][ioff+2] =
    pred[0+6][ioff+4] =
    pred[0+7][ioff+6] = (imgpel) ((PredPel[21] + PredPel[20] + 1) >> 1);
  pred[0+5][ioff] =
    pred[0+6][ioff+2] =
    pred[0+7][ioff+4] = (imgpel) ((PredPel[22] + PredPel[21] + 1) >> 1);
  pred[0+6][ioff] =
    pred[0+7][ioff+2] = (imgpel) ((PredPel[23] + PredPel[22] + 1) >> 1);
  pred[0+7][ioff] = (imgpel) ((PredPel[24] + PredPel[23] + 1) >> 1);
  pred[0][ioff+1] =
    pred[0+1][ioff+3] =
    pred[0+2][ioff+5] =
    pred[0+3][ioff+7] = (imgpel) ((PredPel[17] + PredPel[1] + 2*PredPel[0] + 2) >> 2);
  pred[0+1][ioff+1] =
    pred[0+2][ioff+3] =
    pred[0+3][ioff+5] =
    pred[0+4][ioff+7] = (imgpel) ((PredPel[0] + PredPel[18] + 2*PredPel[17] + 2) >> 2);
  pred[0+2][ioff+1] =
    pred[0+3][ioff+3] =
    pred[0+4][ioff+5] =
    pred[0+5][ioff+7] = (imgpel) ((PredPel[17] + PredPel[19] + 2*PredPel[18] + 2) >> 2);
  pred[0+3][ioff+1] =
    pred[0+4][ioff+3] =
    pred[0+5][ioff+5] =
    pred[0+6][ioff+7] = (imgpel) ((PredPel[18] + PredPel[20] + 2*PredPel[19] + 2) >> 2);
  pred[0+4][ioff+1] =
    pred[0+5][ioff+3] =
    pred[0+6][ioff+5] =
    pred[0+7][ioff+7] = (imgpel) ((PredPel[19] + PredPel[21] + 2*PredPel[20] + 2) >> 2);
  pred[0+5][ioff+1] =
    pred[0+6][ioff+3] =
    pred[0+7][ioff+5] = (imgpel) ((PredPel[20] + PredPel[22] + 2*PredPel[21] + 2) >> 2);
  pred[0+6][ioff+1] =
    pred[0+7][ioff+3] = (imgpel) ((PredPel[21] + PredPel[23] + 2*PredPel[22] + 2) >> 2);
  pred[0+7][ioff+1] = (imgpel) ((PredPel[22] + PredPel[24] + 2*PredPel[23] + 2) >> 2);
  pred[0][ioff+2] =
    pred[0+1][ioff+4] =
    pred[0+2][ioff+6] = (imgpel) ((PredPel[0] + PredPel[2] + 2*PredPel[1] + 2) >> 2);
  pred[0][ioff+3] =
    pred[0+1][ioff+5] =
    pred[0+2][ioff+7] = (imgpel) ((PredPel[1] + PredPel[3] + 2*PredPel[2] + 2) >> 2);
  pred[0][ioff+4] =
    pred[0+1][ioff+6] = (imgpel) ((PredPel[2] + PredPel[4] + 2*PredPel[3] + 2) >> 2);
  pred[0][ioff+5] =
    pred[0+1][ioff+7] = (imgpel) ((PredPel[3] + PredPel[5] + 2*PredPel[4] + 2) >> 2);
  pred[0][ioff+6] = (imgpel) ((PredPel[4] + PredPel[6] + 2*PredPel[5] + 2) >> 2);
  pred[0][ioff+7] = (imgpel) ((PredPel[5] + PredPel[7] + 2*PredPel[6] + 2) >> 2);

  return DECODING_OK;
}

/*!
 ************************************************************************
 * \brief
 *    Make intra 8x8 prediction according to all 9 prediction modes.
 *    The routine uses left and upper neighbouring points from
 *    previous coded blocks to do this (if available). Notice that
 *    inaccessible neighbouring points are signalled with a negative
 *    value in the predmode array .
 *
 *  \par Input:
 *     Starting point of current 8x8 block image position
 *
 ************************************************************************
 */
int intrapred8x8(Macroblock *currMB,    //!< Current Macroblock
                 ColorPlane pl,         //!< Current color plane
                 int ioff,              //!< ioff
                 int joff)              //!< joff

{  
  VideoParameters *p_Vid = currMB->p_Vid;
  int block_x = (currMB->block_x) + (ioff >> 2);
  int block_y = (currMB->block_y) + (joff >> 2);
  byte predmode = p_Vid->ipredmode[block_y][block_x];

  currMB->ipmode_DPCM = predmode;  //For residual DPCM

  switch (predmode)
  {
  case DC_PRED:
    return (intra8x8_dc_pred(currMB, pl, ioff, joff));
    break;
  case VERT_PRED:
    return (intra8x8_vert_pred(currMB, pl, ioff, joff));
    break;
  case HOR_PRED:
    return (intra8x8_hor_pred(currMB, pl, ioff, joff));
    break;
  case DIAG_DOWN_RIGHT_PRED:
    return (intra8x8_diag_down_right_pred(currMB, pl, ioff, joff));
    break;
  case DIAG_DOWN_LEFT_PRED:
    return (intra8x8_diag_down_left_pred(currMB, pl, ioff, joff));
    break;
  case VERT_RIGHT_PRED:
    return (intra8x8_vert_right_pred(currMB, pl, ioff, joff));
    break;
  case VERT_LEFT_PRED:
    return (intra8x8_vert_left_pred(currMB, pl, ioff, joff));
    break;
  case HOR_UP_PRED:
    return (intra8x8_hor_up_pred(currMB, pl, ioff, joff));
    break;
  case HOR_DOWN_PRED:  
    return (intra8x8_hor_down_pred(currMB, pl, ioff, joff));
  default:
    printf("Error: illegal intra_8x8 prediction mode: %d\n", (int) predmode);
    return SEARCH_SYNC;
    break;
  }

  return DECODING_OK;
}


