/*!
 *************************************************************************************
 * \file intra16x16_pred.c
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
#include "intra16x16_pred.h"
#include "mb_access.h"
#include "image.h"
#include <emmintrin.h>
static void memset_16x16(h264_imgpel_macroblock_row_t *mb_pred, int pred)
{
	if (sse2_flag)
	{
		__m128i xmm_pred = _mm_set1_epi8(pred);
		int i;
		__m128i *xmm_macroblock = (__m128i *)mb_pred;
		for (i=0;i<16;i++)
		{
			_mm_store_si128(xmm_macroblock++, xmm_pred);
		}
	}
#ifdef _M_IX86
	else
	{
		__m64 mmx_pred = _mm_set1_pi8(pred);
		int i;
		__m64 *mmx_macroblock = (__m64 *)mb_pred;
		for (i=0;i<16;i++)
		{
			*mmx_macroblock++ = mmx_pred;
			*mmx_macroblock++ = mmx_pred;
		}
	}
#else
	else
	{
	int ii, jj;
	for (jj = 0; jj < MB_BLOCK_SIZE; jj++)
	{
		for (ii = 0; ii < MB_BLOCK_SIZE; ii++)
		{
			mb_pred[jj][ii]=(imgpel) pred;
		}
	}
	}
	#endif
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 16x16 DC prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra16x16_dc_pred(Macroblock *currMB, 
                                     ColorPlane pl)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  int s0 = 0, s1 = 0, s2 = 0;

  int i;

  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img;

  PixelPos up;          //!< pixel position p(0,-1)
  PixelPos left[17];    //!< pixel positions p(-1, -1..15)

  int up_avail, left_avail, left_up_avail;

  s1=s2=0;

	p_Vid->getNeighbourNXLuma(currMB,  -1, &left[0]);
	p_Vid->getNeighbourLeftLuma(currMB,  &left[1]);
     p_Vid->getNeighbourNPLumaNB(currMB,  2-1, &left[2]);
		p_Vid->getNeighbourNPLumaNB(currMB,  3-1, &left[3]);
		p_Vid->getNeighbourNPLumaNB(currMB,  4-1, &left[4]);
		p_Vid->getNeighbourNPLumaNB(currMB,  5-1, &left[5]);
		p_Vid->getNeighbourNPLumaNB(currMB,  6-1, &left[6]);
		p_Vid->getNeighbourNPLumaNB(currMB,  7-1, &left[7]);
		p_Vid->getNeighbourNPLumaNB(currMB,  8-1, &left[8]);
		p_Vid->getNeighbourNPLumaNB(currMB,  9-1, &left[9]);
		p_Vid->getNeighbourNPLumaNB(currMB,  10-1, &left[10]);
		p_Vid->getNeighbourNPLumaNB(currMB,  11-1, &left[11]);
		p_Vid->getNeighbourNPLumaNB(currMB,  12-1, &left[12]);
		p_Vid->getNeighbourNPLumaNB(currMB,  13-1, &left[13]);
		p_Vid->getNeighbourNPLumaNB(currMB,  14-1, &left[14]);
	p_Vid->getNeighbourNPLumaNB(currMB,  15-1, &left[15]);
	p_Vid->getNeighbourNPLumaNB(currMB,  16-1, &left[16]);

  p_Vid->getNeighbourUpLuma(currMB, &up);

  if (!p_Vid->active_pps->constrained_intra_pred_flag)
  {
    up_avail      = up.available;
    left_avail    = left[1].available;
    left_up_avail = left[0].available;
  }
  else
  {
    up_avail      = up.available ? p_Vid->intra_block[up.mb_addr] : 0;
    for (i = 1, left_avail = 1; i < 17; ++i)
      left_avail  &= left[i].available ? p_Vid->intra_block[left[i].mb_addr]: 0;
    left_up_avail = left[0].available ? p_Vid->intra_block[left[0].mb_addr]: 0;
  }

	if (up_avail)
	{
		s1 += imgY[up.pos_y][up.pos_x+0];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+1];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+2];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+3];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+4];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+5];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+6];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+7];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+8];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+9];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+10];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+11];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+12];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+13];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+14];    // sum hor pix
		s1 += imgY[up.pos_y][up.pos_x+15];    // sum hor pix
	}

	if (left_avail)
	{
		s2 += imgY[left[0 + 1].pos_y][left[0 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[1 + 1].pos_y][left[1 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[2 + 1].pos_y][left[2 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[3 + 1].pos_y][left[3 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[4 + 1].pos_y][left[4 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[5 + 1].pos_y][left[5 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[6 + 1].pos_y][left[6 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[7 + 1].pos_y][left[7 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[8 + 1].pos_y][left[8 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[9 + 1].pos_y][left[9 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[10 + 1].pos_y][left[10 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[11 + 1].pos_y][left[11 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[12 + 1].pos_y][left[12 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[13 + 1].pos_y][left[13 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[14 + 1].pos_y][left[14 + 1].pos_x];    // sum vert pix
		s2 += imgY[left[15 + 1].pos_y][left[15 + 1].pos_x];    // sum vert pix
	}
  
  if (up_avail && left_avail)
    s0 = (s1 + s2 + 16)>>5;       // no edge
  else if (!up_avail && left_avail)
    s0 = (s2 + 8)>>4;              // upper edge
  else if (up_avail && !left_avail)
    s0 = (s1 + 8)>>4;              // left edge
  else
    s0 = p_Vid->dc_pred_value_comp[pl];                            // top left corner, nothing to predict from

	memset_16x16(currSlice->mb_pred[pl], s0);

  return DECODING_OK;
}


/*!
 ***********************************************************************
 * \brief
 *    makes and returns 16x16 vertical prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra16x16_vert_pred(Macroblock *currMB, 
                                       ColorPlane pl)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int j;

  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img;

  PixelPos up;          //!< pixel position p(0,-1)

  int up_avail;

  p_Vid->getNeighbourUpLuma(currMB, &up);

  if (!p_Vid->active_pps->constrained_intra_pred_flag)
  {
    up_avail = up.available;
  }
  else
  {
    up_avail = up.available ? p_Vid->intra_block[up.mb_addr] : 0;
  }

  if (!up_avail)
    error ("invalid 16x16 intra pred Mode VERT_PRED_16",500);

  for(j=0;j<MB_BLOCK_SIZE;++j)
	{
		// TODO; take advantage of imgY's stride
    memcpy(&currSlice->mb_pred[pl][j][0], &(imgY[up.pos_y][up.pos_x]), MB_BLOCK_SIZE * sizeof(imgpel));
	}

  return DECODING_OK;
}


/*!
 ***********************************************************************
 * \brief
 *    makes and returns 16x16 horizontal prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static inline int intra16x16_hor_pred(Macroblock *currMB, 
                                      ColorPlane pl)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  int i,j;

  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img;
  imgpel prediction;

  PixelPos left[17];    //!< pixel positions p(-1, -1..15)

  int left_avail, left_up_avail;

  for (i=0;i<17;++i)
  {
    p_Vid->getNeighbourNXLuma(currMB, i-1, &left[i]);
  }

  if (!p_Vid->active_pps->constrained_intra_pred_flag)
  {
    left_avail    = left[1].available;
    left_up_avail = left[0].available;
  }
  else
  {
    for (i = 1, left_avail = 1; i < 17; ++i)
      left_avail  &= left[i].available ? p_Vid->intra_block[left[i].mb_addr]: 0;
    left_up_avail = left[0].available ? p_Vid->intra_block[left[0].mb_addr]: 0;
  }

  if (!left_avail)
    error ("invalid 16x16 intra pred Mode HOR_PRED_16",500);

  for(j = 0; j < MB_BLOCK_SIZE; ++j)
  {
    prediction = imgY[left[j+1].pos_y][left[j+1].pos_x];
    for(i = 0; i < MB_BLOCK_SIZE; ++i)
      currSlice->mb_pred[pl][j][i]= prediction; // store predicted 16x16 block
  }

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 16x16 horizontal prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *
 ***********************************************************************
 */
static void planeset(h264_imgpel_macroblock_row_t *dest, int iaa, int ib, int ic)
{
	int j;
	__m128i i0_7 = _mm_setr_epi16(-7,-6,-5,-4,-3,-2,-1, 0);
	__m128i i8_15 = _mm_setr_epi16(1,2,3,4,5,6,7,8);
	__m128i xmm_ib = _mm_set1_epi16(ib);
	int j7ic = iaa + -7 * ic + 16;
	i0_7 = _mm_mullo_epi16(i0_7, xmm_ib);
	i8_15 = _mm_mullo_epi16(i8_15, xmm_ib);
  for (j = 0;j < MB_BLOCK_SIZE; ++j)
  {
		__m128i xmm_j7ic = _mm_set1_epi16(j7ic);
		__m128i xmm_lo = _mm_add_epi16(i0_7, xmm_j7ic);
		__m128i xmm_hi = _mm_add_epi16(i8_15, xmm_j7ic);
		__m128i xmm_store;
		xmm_lo = _mm_srai_epi16(xmm_lo, 5);
		xmm_hi = _mm_srai_epi16(xmm_hi, 5);
		xmm_store = _mm_packus_epi16(xmm_lo, xmm_hi);
		_mm_store_si128((__m128i *)dest[j], xmm_store);
		j7ic += ic;
  }// store plane prediction
}

static inline int intra16x16_plane_pred(Macroblock *currMB, 
                                        ColorPlane pl)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i,j;

  int ih = 0, iv = 0;
  int ib,ic,iaa;

  imgpel **imgY = (pl) ? p_Vid->dec_picture->imgUV[pl - 1]->img : p_Vid->dec_picture->imgY->img;
  imgpel *mpr_line;
  int max_imgpel_value = p_Vid->max_pel_value_comp[pl];

  PixelPos up;          //!< pixel position p(0,-1)
  PixelPos left[17];    //!< pixel positions p(-1, -1..15)

  int up_avail, left_avail, left_up_avail;

	p_Vid->getNeighbourNXLuma(currMB,  -1, &left[0]);
	p_Vid->getNeighbourLeftLuma(currMB, &left[1]);
  for (i=2;i<17; ++i)
  {
    p_Vid->getNeighbourNPLumaNB(currMB,  i-1, &left[i]);
  }
  p_Vid->getNeighbourUpLuma(currMB, &up);

  if (!p_Vid->active_pps->constrained_intra_pred_flag)
  {
    up_avail      = up.available;
    left_avail    = left[1].available;
    left_up_avail = left[0].available;
  }
  else
  {
    up_avail      = up.available ? p_Vid->intra_block[up.mb_addr] : 0;
    for (i = 1, left_avail = 1; i < 17; ++i)
      left_avail  &= left[i].available ? p_Vid->intra_block[left[i].mb_addr]: 0;
    left_up_avail = left[0].available ? p_Vid->intra_block[left[0].mb_addr]: 0;
  }

  if (!up_avail || !left_up_avail  || !left_avail)
    error ("invalid 16x16 intra pred Mode PLANE_16",500);

  mpr_line = &imgY[up.pos_y][up.pos_x+7];
  for (i = 1; i < 8; ++i)
  {
    ih += i*(mpr_line[i] - mpr_line[-i]);
    iv += i*(imgY[left[8+i].pos_y][left[8+i].pos_x] - imgY[left[8-i].pos_y][left[8-i].pos_x]);
  }

  ih += 8*(mpr_line[8] - imgY[left[0].pos_y][left[0].pos_x]);
  iv += 8*(imgY[left[16].pos_y][left[16].pos_x] - imgY[left[0].pos_y][left[0].pos_x]);

  ib=(5 * ih + 32)>>6;
  ic=(5 * iv + 32)>>6;

  iaa=16 * (mpr_line[8] + imgY[left[16].pos_y][left[16].pos_x]);
	if (sse2_flag)
	{
		planeset(currSlice->mb_pred[pl], iaa, ib, ic);
	}
	else
	{
	// TODO: MMX
  for (j = 0;j < MB_BLOCK_SIZE; ++j)
  {
		int j7ic = iaa + (j - 7) * ic + 16;
    for (i = 0;i < MB_BLOCK_SIZE; ++i)
    {
      currSlice->mb_pred[pl][j][i] = (imgpel) iClip1(max_imgpel_value, (((i - 7) * ib + j7ic) >> 5));
    }
  }// store plane prediction
	}
  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 16x16 intra prediction blocks 
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *    SEARCH_SYNC   search next sync element as errors while decoding occured
 ***********************************************************************
 */
// TODO: replace with ippiPredictIntra_16x16_H264_8u_C1IR ?
int intrapred16x16(Macroblock *currMB,  //!< Current Macroblock
                   ColorPlane pl,       //!< Current colorplane (for 4:4:4)                         
                   int predmode)        //!< prediction mode
{
  switch (predmode)
  {
  case VERT_PRED_16:                       // vertical prediction from block above
    return (intra16x16_vert_pred(currMB, pl));
    break;
  case HOR_PRED_16:                        // horizontal prediction from left block
    return (intra16x16_hor_pred(currMB, pl));
    break;
  case DC_PRED_16:                         // DC prediction
    return (intra16x16_dc_pred(currMB, pl));
    break;
  case PLANE_16:// 16 bit integer plan pred
    return (intra16x16_plane_pred(currMB, pl));
    break;
  default:
    {                                    // indication of fault in bitstream,exit
      printf("illegal 16x16 intra prediction mode input: %d\n",predmode);
      return SEARCH_SYNC;
    }
  }
}

