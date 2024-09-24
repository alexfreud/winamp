
/*!
***********************************************************************
* \file macroblock.c
*
* \brief
*     Decode a Macroblock
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Inge Lille-Langøy               <inge.lille-langoy@telenor.com>
*    - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
*    - Jani Lainema                    <jani.lainema@nokia.com>
*    - Sebastian Purreiter             <sebastian.purreiter@mch.siemens.de>
*    - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
*    - Detlev Marpe                    <marpe@hhi.de>
*    - Gabi Blaettermann
*    - Ye-Kui Wang                     <wyk@ieee.org>
*    - Lowell Winger                   <lwinger@lsil.com>
*    - Alexis Michael Tourapis         <alexismt@ieee.org>
***********************************************************************
*/

#include "contributors.h"

#include <math.h>

#include "block.h"
#include "global.h"
#include "mbuffer.h"
#include "elements.h"
#include "errorconcealment.h"
#include "macroblock.h"
#include "fmo.h"
#include "cabac.h"
#include "vlc.h"
#include "image.h"
#include "mb_access.h"
#include "biaridecod.h"
#include "transform8x8.h"
#include "transform.h"
#include "mc_prediction.h"
#include "quant.h"
#include "intra4x4_pred.h"
#include "intra8x8_pred.h"
#include "intra16x16_pred.h"
#include "mv_prediction.h"
#include "optim.h"
#include "mb_prediction.h"
#include <emmintrin.h>
#include <smmintrin.h>

#if TRACE
#define TRACE_STRING(s) strncpy(currSE.tracestring, s, TRACESTRING_SIZE)
#define TRACE_DECBITS(i) dectracebitcnt(1)
#define TRACE_PRINTF(s) sprintf(type, "%s", s);
#define TRACE_STRING_P(s) strncpy(currSE->tracestring, s, TRACESTRING_SIZE)
#else
#define TRACE_STRING(s)
#define TRACE_DECBITS(i)
#define TRACE_PRINTF(s) 
#define TRACE_STRING_P(s)
#endif

//! look up tables for FRExt_chroma support
void dectracebitcnt(int count);

static void read_motion_info_from_NAL_p_slice  (Macroblock *currMB);
static void read_motion_info_from_NAL_b_slice  (Macroblock *currMB);
static void read_ipred_modes                   (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CABAC (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CAVLC (Macroblock *currMB);
static void read_IPCM_coeffs_from_NAL          (Slice *currSlice, struct datapartition *dP);
static void read_one_macroblock_i_slice        (Macroblock *currMB);
static void read_one_macroblock_p_slice        (Macroblock *currMB);
static void read_one_macroblock_b_slice        (Macroblock *currMB);
static int  decode_one_component_i_slice       (Macroblock *currMB, ColorPlane curr_plane, struct video_image *image, StorablePicture *dec_picture);
static int  decode_one_component_p_slice       (Macroblock *currMB, ColorPlane curr_plane, struct video_image *image, StorablePicture *dec_picture);
static int  decode_one_component_b_slice       (Macroblock *currMB, ColorPlane curr_plane, struct video_image *image, StorablePicture *dec_picture);
static int  decode_one_component_sp_slice      (Macroblock *currMB, ColorPlane curr_plane, struct video_image *image, StorablePicture *dec_picture);

static inline void or_bits(int64 *x, int mask, int position)
{
#ifdef _M_IX86
	__m64 mmx_x = *(__m64 *)x;
	__m64 mmx_mask = _mm_cvtsi32_si64(mask);
	mmx_mask=_mm_slli_si64(mmx_mask, position);
	mmx_x = _mm_or_si64(mmx_x, mmx_mask);
	*(__m64 *)x = mmx_x;
#else
	*x   |= ((int64) mask << position);
#endif
}

/*!
************************************************************************
* \brief
*    Set context for reference frames
************************************************************************
*/
static inline int BType2CtxRef (int btype)
{
	return (btype >= 4);
}

/*!
************************************************************************
* \brief
*    Function for reading the reference picture indices using VLC
************************************************************************
*/
static char readRefPictureIdx_VLC(SyntaxElement *currSE, DataPartition *dP, int list)
{
#if TRACE
	char tstring[20];   
	sprintf( tstring, "ref_idx_l%d", list); 
	strncpy(currSE->tracestring, tstring, TRACESTRING_SIZE);
#endif
	currSE->value2 = list;
	readSyntaxElement_UVLC(currSE, dP);
	return (char) currSE->value1;
}

/*!
************************************************************************
* \brief
*    Function for reading the reference picture indices using FLC
************************************************************************
*/
static char readRefPictureIdx_FLC(SyntaxElement *currSE, DataPartition *dP, int list)
{
#if TRACE
	char tstring[20];   
	sprintf( tstring, "ref_idx_l%d", list); 
	strncpy(currSE->tracestring, tstring, TRACESTRING_SIZE);
#endif
	//currSE->len = 1;
	currSE->value1 = 1 - readSyntaxElement_FLC(dP->bitstream, 1);

	return (char) currSE->value1;
}

/*!
************************************************************************
* \brief
*    Dummy Function for reading the reference picture indices
************************************************************************
*/
static char readRefPictureIdx_Null(SyntaxElement *currSE, DataPartition *dP, int list)
{
	return 0;
}

/*!
************************************************************************
* \brief
*    Function to prepare reference picture indice function pointer
************************************************************************
*/
static void prepareListforRefIdx ( Macroblock *currMB, SyntaxElement *currSE, int num_ref_idx_active, int refidx_present)
{
	currMB->readRefPictureIdx = readRefPictureIdx_Null; // Initialize readRefPictureIdx
	if(num_ref_idx_active > 1)
	{
		currSE->mapping = linfo_ue;
		if (refidx_present)
		{
			if (num_ref_idx_active == 2)
				currMB->readRefPictureIdx = readRefPictureIdx_FLC;        
			else
				currMB->readRefPictureIdx = readRefPictureIdx_VLC;
		}
	}    
}

#if defined(_DEBUG) || defined(_M_X64)
void set_chroma_qp(Macroblock* currMB)
{
	// TODO: benski> we could use MMX for this if we could find a formula for QP_SCALE_CR
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	int i;
	for (i=0; i<2; ++i)
	{
		currMB->qpc[i] = iClip3 ( -p_Vid->bitdepth_chroma_qp_scale, 51, currMB->qp + dec_picture->chroma_qp_offset[i] );
		currMB->qpc[i] = currMB->qpc[i] < 0 ? currMB->qpc[i] : QP_SCALE_CR[currMB->qpc[i]];
		currMB->qp_scaled[i + 1] = currMB->qpc[i] + p_Vid->bitdepth_chroma_qp_scale;
	}
}
#else
void set_chroma_qp(Macroblock* currMB);
#endif

/*!
************************************************************************
* \brief
*    updates chroma QP according to luma QP and bit depth
************************************************************************
*/
static inline void update_qp(Macroblock *currMB, int qp)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	currMB->qp = qp;
	currMB->qp_scaled[0] = qp + p_Vid->bitdepth_luma_qp_scale;
	set_chroma_qp(currMB);
	currMB->is_lossless = (Boolean) ((currMB->qp_scaled[0] == 0) && (p_Vid->lossless_qpprime_flag == 1));
}

static void read_delta_quant_CAVLC(SyntaxElement *currSE, DataPartition *dP, Macroblock *currMB, const byte *partMap, int type)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;

	dP = &(currSlice->partArr[partMap[type]]);


	currSE->mapping = linfo_se;
	readSyntaxElement_UVLC(currSE, dP);
	currMB->delta_quant = (short) currSE->value1;


	if ((currMB->delta_quant < -(26 + p_Vid->bitdepth_luma_qp_scale/2)) || (currMB->delta_quant > (25 + p_Vid->bitdepth_luma_qp_scale/2)))
		error ("mb_qp_delta is out of range", 500);

	p_Vid->qp = ((p_Vid->qp + currMB->delta_quant + 52 + 2*p_Vid->bitdepth_luma_qp_scale)%(52+p_Vid->bitdepth_luma_qp_scale)) -
		p_Vid->bitdepth_luma_qp_scale;
	update_qp(currMB, p_Vid->qp);
}

static void inline read_delta_quant_CABAC(SyntaxElement *currSE, DataPartition *dP, Macroblock *currMB, const byte *partMap, int type)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;

	dP = &(currSlice->partArr[partMap[type]]);

	currMB->delta_quant = readDquant_CABAC(currSlice, &dP->de_cabac);

	if ((currMB->delta_quant < -(26 + p_Vid->bitdepth_luma_qp_scale/2)) || (currMB->delta_quant > (25 + p_Vid->bitdepth_luma_qp_scale/2)))
		error ("mb_qp_delta is out of range", 500);

	p_Vid->qp = ((p_Vid->qp + currMB->delta_quant + 52 + 2*p_Vid->bitdepth_luma_qp_scale)%(52+p_Vid->bitdepth_luma_qp_scale)) - p_Vid->bitdepth_luma_qp_scale;
	update_qp(currMB, p_Vid->qp);
}

/*!
************************************************************************
* \brief
*    Function to read reference picture indice values
************************************************************************
*/
static void readMBRefPictureIdx(SyntaxElement *currSE, DataPartition *dP, Macroblock *currMB, PicMotion **motion, int list, int step_v0, int step_h0)
{
	int k, j, j0, i0, i;
	char refframe;

	for (j0 = 0; j0 < 4; j0 += step_v0)
	{
		currMB->subblock_y = j0 << 2;
		for (i0 = 0; i0 < 4; i0 += step_h0)
		{
			currMB->subblock_x = i0 << 2;
			k = 2 * (j0 >> 1) + (i0 >> 1);

			if ((currMB->b8pdir[k] == list || currMB->b8pdir[k] == BI_PRED) && currMB->b8mode[k] != 0)
			{
				refframe = currMB->readRefPictureIdx(currSE, dP, list);

				for (j = j0; j < j0 + step_v0; ++j)
				{
					for (i=0;i<step_h0;i++)
					{
						motion[j][currMB->block_x + i0 + i].ref_idx = refframe;
					}
				}
			}
		}
	}
}

static void readMBRefPictureIdx_CABAC1(DataPartition *dP, Macroblock *currMB, PicMotion **motion, int list, int step_v0)
{
	int k, j, j0, i0;
	char refframe;

	for (j0 = 0; j0 < 4; j0 += step_v0)
	{
		currMB->subblock_y = j0 << 2;
		for (i0 = 0; i0 < 4; i0 += 1)
		{
			currMB->subblock_x = i0 << 2;
			k = 2 * (j0 >> 1) + (i0 >> 1);

			if ((currMB->b8pdir[k] == list || currMB->b8pdir[k] == BI_PRED) && currMB->b8mode[k] != 0)
			{
				refframe = readRefFrame_CABAC(currMB, &dP->de_cabac, list, i0<<2, j0<<2);

				for (j = j0; j < j0 + step_v0; ++j)
					motion[j][currMB->block_x + i0].ref_idx=refframe;
			}
		}
	}
}

static void readMBRefPictureIdx_CABAC2(DataPartition *dP, Macroblock *currMB, PicMotion **motion, int list, int step_v0)
{
	int k, j, j0;
	char refframe;

	for (j0 = 0; j0 < 4; j0 += step_v0)
	{
		currMB->subblock_y = j0 << 2;

		currMB->subblock_x = 0 << 2;
		k = 2 * (j0 >> 1) + (0 >> 1);

		if ((currMB->b8pdir[k] == list || currMB->b8pdir[k] == BI_PRED) && currMB->b8mode[k] != 0)
		{
			refframe = readRefFrame_CABAC0(currMB, &dP->de_cabac, list, j0<<2);

			for (j = j0; j < j0 + step_v0; ++j)
			{
				motion[j][currMB->block_x + 0].ref_idx=refframe;
				motion[j][currMB->block_x + 1].ref_idx=refframe;
			}
		}

		//

		currMB->subblock_x = 2 << 2;
		k = 2 * (j0 >> 1) + (2 >> 1);

		if ((currMB->b8pdir[k] == list || currMB->b8pdir[k] == BI_PRED) && currMB->b8mode[k] != 0)
		{
			refframe = readRefFrame_CABAC(currMB, &dP->de_cabac, list, 8, j0<<2);

			for (j = j0; j < j0 + step_v0; ++j)
			{
				motion[j][currMB->block_x + 2].ref_idx=refframe;
				motion[j][currMB->block_x + 3].ref_idx=refframe;
			}
		}

	}
}


static void readMBRefPictureIdx_CABAC4(DataPartition *dP, Macroblock *currMB, PicMotion **motion, int list, int step_v0)
{
	int k, j, j0;
	char refframe;

	for (j0 = 0; j0 < 4; j0 += step_v0)
	{
		currMB->subblock_y = j0 << 2;
		currMB->subblock_x = 0;
		k = j0 & ~1;

		if ((currMB->b8pdir[k] == list || currMB->b8pdir[k] == BI_PRED) && currMB->b8mode[k] != 0)
		{
			refframe =  readRefFrame_CABAC0(currMB, &dP->de_cabac, list, j0<<2);
			for (j = j0; j < j0 + step_v0; ++j)
			{
				motion[j][currMB->block_x + 0].ref_idx=refframe;
				motion[j][currMB->block_x + 1].ref_idx=refframe;
				motion[j][currMB->block_x + 2].ref_idx=refframe;
				motion[j][currMB->block_x + 3].ref_idx=refframe;
			}
		}
	}
}

static void readMBRefPictureIdx_CABAC(DataPartition *dP, Macroblock *currMB, PicMotion **motion, int list, int step_v0, int step_h0)
{
	switch(step_h0)
	{
	case 1:
		readMBRefPictureIdx_CABAC1(dP, currMB, motion, list, step_v0);
		break;
	case 2:
		readMBRefPictureIdx_CABAC2(dP, currMB, motion, list, step_v0);
		break;
	case 4:
		readMBRefPictureIdx_CABAC4(dP, currMB, motion, list, step_v0);
		break;
	}
}

static void readMBRefPictureIdx_CABAC_NoReference(Macroblock *currMB, PicMotion **motion, int list, int step_v0, int step_h0)
{
	int k, j, j0, i0, i;

	for (j0 = 0; j0 < 4; j0 += step_v0)
	{
		for (i0 = 0; i0 < 4; i0 += step_h0)
		{
			k = 2 * (j0 >> 1) + (i0 >> 1);

			if ((currMB->b8pdir[k] == list || currMB->b8pdir[k] == BI_PRED) && currMB->b8mode[k] != 0)
			{
				for (j = j0; j < j0 + step_v0; ++j)
				{
					for (i=0;i<step_h0;i++)
					{
						motion[j][currMB->block_x + i0 + i].ref_idx=0;
					}
				}
			}
		}
	}
}

/*!
************************************************************************
* \brief
*    Function to read reference picture indice values
************************************************************************
*/
static void readMBMotionVectors(SyntaxElement *currSE, DataPartition *dP, Macroblock *currMB, int list, int step_h0, int step_v0)
{
	int i, j, k, i4, j4, ii, jj, kk, i0, j0;
	short curr_mvd[2], curr_mv[2], pred_mv[2];
	MotionVector  (*mvd)[4];
	//MotionVector **mv;
	int mv_mode, step_h, step_v;
	char cur_ref_idx;
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;
	PixelPos block[4]; // neighbor blocks


	for (j0=0; j0<4; j0+=step_v0)
	{
		for (i0=0; i0<4; i0+=step_h0)
		{       
			kk = 2 * (j0 >> 1) + (i0 >> 1);
			if ((currMB->b8pdir[kk]== list || currMB->b8pdir[kk]== BI_PRED) && (currMB->b8mode[kk] !=0))//has forward vector
			{
				PicMotion **list_motion = motion->motion[list];
				cur_ref_idx = list_motion[currMB->block_y+j0][currMB->block_x+i0].ref_idx;
				mv_mode  = currMB->b8mode[kk];
				step_h = BLOCK_STEP [mv_mode][0];
				step_v = BLOCK_STEP [mv_mode][1];

				for (j = j0; j < j0 + step_v0; j += step_v)
				{
					PicMotion **mv;
					currMB->subblock_y = j << 2; // position used for context determination
					j4 = currMB->block_y + j;
					mv = &list_motion[j4];
					mvd = &currMB->mvd [list][j];
					for (i = i0; i < i0 + step_h0; i += step_h)
					{
						currMB->subblock_x = i << 2; // position used for context determination
						i4 = currMB->block_x + i;

						get_neighbors(currMB, block, BLOCK_SIZE * i, BLOCK_SIZE * j, 4 * step_h);

						// first make mv-prediction
						currMB->GetMVPredictor (currMB, block, pred_mv, cur_ref_idx, list_motion, BLOCK_SIZE * i, BLOCK_SIZE * j, 4 * step_h, 4 * step_v);

						for (k=0; k < 2; ++k)
						{
							currSE->value2   = (k << 1) + list; // identifies the component; only used for context determination
							readSyntaxElement_UVLC(currSE, dP);
							curr_mvd[k] = (short) currSE->value1;
							curr_mv [k] = (short)(curr_mvd[k] + pred_mv[k]);  // compute motion vector 
						}

						// Init motion vectors
						for(jj = 0; jj < step_v; ++jj)
						{
							for(ii = i4; ii < i4 + step_h; ++ii)
							{
								memcpy(&mv[jj][ii].mv, curr_mv,  sizeof(MotionVector));
							}
						}

						// Init first line (mvd)
						for(ii = i; ii < i + step_h; ++ii)
						{
							memcpy(mvd[0][ii], curr_mvd,  sizeof(MotionVector));
						}              

						// now copy all other lines
						for(jj = 1; jj < step_v; ++jj)
						{
							memcpy(mvd[jj][i], mvd[0][i],  step_h * sizeof(MotionVector));
						}
					}
				}
			}
		}
	}
}

static void readMBMotionVectors_CABAC(DataPartition *dP, Macroblock *currMB, int list, int step_h0, int step_v0)
{
	int i, j, k, i4, j4, ii, jj, kk, i0, j0;
	short curr_mvd[2], curr_mv[2], pred_mv[2];
	MotionVector  (*mvd)[4];
	//MotionVector **mv;
	int mv_mode, step_h, step_v;
	char cur_ref_idx;
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;
	PixelPos block[4]; // neighbor blocks

	for (j0=0; j0<4; j0+=step_v0)
	{
		for (i0=0; i0<4; i0+=step_h0)
		{       
			kk = (j0 & ~1) + (i0 >> 1);
			if ((currMB->b8pdir[kk]== list || currMB->b8pdir[kk]== BI_PRED) && (currMB->b8mode[kk] !=0))//has forward vector
			{
				PicMotion **list_motion = motion->motion[list];
				cur_ref_idx = list_motion[currMB->block_y+j0][currMB->block_x+i0].ref_idx;
				mv_mode  = currMB->b8mode[kk];
				step_h = BLOCK_STEP [mv_mode][0];
				step_v = BLOCK_STEP [mv_mode][1];

				for (j = j0; j < j0 + step_v0; j += step_v)
				{
					PicMotion **mv;
					int block_j = j << 2;
					currMB->subblock_y = block_j; // position used for context determination
					j4 = currMB->block_y + j;
					mv = &list_motion[j4];
					mvd = &currMB->mvd [list][j];
					for (i = i0; i < i0 + step_h0; i += step_h)
					{
						int block_i=i << 2;
						currMB->subblock_x = block_i; // position used for context determination
						i4 = currMB->block_x + i;

						get_neighbors(currMB, block, block_i, block_j, 4 * step_h);

						// first make mv-prediction
						currMB->GetMVPredictor (currMB, block, pred_mv, cur_ref_idx, list_motion, block_i, block_j, 4 * step_h, 4 * step_v);

						for (k=0; k < 2; ++k)
						{
							//currSE.value2   = (k << 1) + list; // identifies the component; only used for context determination
							curr_mvd[k] = (short)readMVD_CABAC(currMB, &dP->de_cabac, k, list, block_i, block_j);
							curr_mv [k] = (short)(curr_mvd[k] + pred_mv[k]);  // compute motion vector 
						}

						// Init motion vectors
						for(jj = 0; jj < step_v; ++jj)
						{
							for(ii = i4; ii < i4 + step_h; ++ii)
							{
								*(int32_t *)(&mv[jj][ii].mv) = *(int32_t *)curr_mv;
							}
						}

						// Init first line (mvd)
						for(ii = i; ii < i + step_h; ++ii)
						{
							*(int32_t *)(mvd[0][ii]) = *(int32_t *)curr_mvd;
						}              

						// now copy all other lines
						for(jj = 1; jj < step_v; ++jj)
						{
							memcpy_amd(mvd[jj][i], mvd[0][i],  step_h * sizeof(MotionVector));
						}
					}
				}
			}
		}
	}
}

/*!
************************************************************************
* \brief
*    initializes the current macroblock
************************************************************************
*/
void start_macroblock(Slice *currSlice, Macroblock **currMB)
{
	VideoParameters *p_Vid = currSlice->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	int mb_nr = p_Vid->current_mb_nr;
	Macroblock *mb = &p_Vid->mb_data[mb_nr];   // intialization code deleted, see below, StW  
	*currMB = mb;

	mb->p_Vid   = p_Vid;
	mb->p_Slice = currSlice;
	mb->mbAddrX = mb_nr;

	//assert (mb_nr < (int) p_Vid->PicSizeInMbs);

	/* Update coordinates of the current macroblock */
	if (currSlice->mb_aff_frame_flag)
	{
		mb->mb_x =    (mb_nr) % ((2*p_Vid->width) / MB_BLOCK_SIZE);
		mb->mb_y = 2*((mb_nr) / ((2*p_Vid->width) / MB_BLOCK_SIZE));

		mb->mb_y += (mb->mb_x & 0x01);
		mb->mb_x >>= 1;
	}
	else
	{
		mb->mb_x = p_Vid->PicPos[mb_nr][0];
		mb->mb_y = p_Vid->PicPos[mb_nr][1];
	}

	/* Define vertical positions */
	mb->block_y = mb->mb_y * BLOCK_SIZE;      /* luma block position */
	mb->block_y_aff = mb->block_y;
	mb->pix_y   = mb->mb_y * MB_BLOCK_SIZE;   /* luma macroblock position */
	mb->pix_c_y = mb->mb_y * p_Vid->mb_cr_size_y; /* chroma macroblock position */

	/* Define horizontal positions */
	mb->block_x = mb->mb_x * BLOCK_SIZE;      /* luma block position */
	mb->pix_x   = mb->mb_x * MB_BLOCK_SIZE;   /* luma pixel position */
	mb->pix_c_x = mb->mb_x * p_Vid->mb_cr_size_x; /* chroma pixel position */

	// Save the slice number of this macroblock. When the macroblock below
	// is coded it will use this to decide if prediction for above is possible
	mb->slice_nr = (short) p_Vid->current_slice_nr;

	if (p_Vid->current_slice_nr >= MAX_NUM_SLICES)
	{
		error ("Maximum number of supported slices exceeded. \nPlease recompile with increased value for MAX_NUM_SLICES", 200);
	}

	dec_picture->slice_id[mb->mb_y][mb->mb_x] = (short) p_Vid->current_slice_nr;
	dec_picture->max_slice_id = (short) imax(p_Vid->current_slice_nr, dec_picture->max_slice_id);

	CheckAvailabilityOfNeighbors(mb);

	// Select appropriate MV predictor function
	init_motion_vector_prediction(*currMB, currSlice->mb_aff_frame_flag);

	set_read_and_store_CBP(currMB, currSlice->active_sps->chroma_format_idc);

	// Reset syntax element entries in MB struct
	update_qp(*currMB, p_Vid->qp);
	mb->mb_type         = 0;
	mb->delta_quant     = 0;
	mb->cbp             = 0;    
	mb->c_ipred_mode    = DC_PRED_8; //GB

	if (currSlice->slice_type != I_SLICE)
	{
		if (currSlice->slice_type != B_SLICE)
			memzero64(mb->mvd);//, BLOCK_MULTIPLE * BLOCK_MULTIPLE * 2 * sizeof(short));
		else
			memzero128(mb->mvd);//, 2 * BLOCK_MULTIPLE * BLOCK_MULTIPLE * 2 * sizeof(short));
	}

	memzero24(mb->cbp_blk);  
	memzero24(mb->cbp_bits);
	memzero24(mb->cbp_bits_8x8);

	// initialize currSlice->mb_rres
	memset(currSlice->mb_rres8, 0, sizeof(currSlice->mb_rres8));

	// store filtering parameters for this MB
	mb->DFDisableIdc    = currSlice->DFDisableIdc;
	mb->DFAlphaC0Offset = currSlice->DFAlphaC0Offset;
	mb->DFBetaOffset    = currSlice->DFBetaOffset;

}

/*!
************************************************************************
* \brief
*    set coordinates of the next macroblock
*    check end_of_slice condition
************************************************************************
*/
Boolean exit_macroblock(Slice *currSlice, int eos_bit)
{
	VideoParameters *p_Vid = currSlice->p_Vid;

	//! The if() statement below resembles the original code, which tested
	//! p_Vid->current_mb_nr == p_Vid->PicSizeInMbs.  Both is, of course, nonsense
	//! In an error prone environment, one can only be sure to have a new
	//! picture by checking the tr of the next slice header!

	// printf ("exit_macroblock: FmoGetLastMBOfPicture %d, p_Vid->current_mb_nr %d\n", FmoGetLastMBOfPicture(), p_Vid->current_mb_nr);
	++(p_Vid->num_dec_mb);

	if (p_Vid->num_dec_mb == p_Vid->PicSizeInMbs)
	{
		return TRUE;
	}
	// ask for last mb in the slice  CAVLC
	else
	{

		p_Vid->current_mb_nr = FmoGetNextMBNr (p_Vid, p_Vid->current_mb_nr);

		if (p_Vid->current_mb_nr == -1)     // End of Slice group, MUST be end of slice
		{
			assert (currSlice->nal_startcode_follows (currSlice, eos_bit) == TRUE);
			return TRUE;
		}

		if(currSlice->nal_startcode_follows(currSlice, eos_bit) == FALSE)
			return FALSE;

		if(currSlice->slice_type == I_SLICE  || currSlice->slice_type == SI_SLICE || p_Vid->active_pps->entropy_coding_mode_flag == CABAC)
			return TRUE;
		if(p_Vid->cod_counter <= 0)
			return TRUE;
		return FALSE;
	}
}

/*!
************************************************************************
* \brief
*    Interpret the mb mode for P-Frames
************************************************************************
*/
static void interpret_mb_mode_P(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;

	static const int ICBPTAB[6] = {0,16,32,15,31,47};
	int         mbmode = currMB->mb_type;

#define ZERO_P8x8     (mbmode==5)
#define MODE_IS_P8x8  (mbmode==4 || mbmode==5)
#define MODE_IS_I4x4  (mbmode==6)
#define I16OFFSET     (mbmode-7)
#define MODE_IS_IPCM  (mbmode==31)

	if(mbmode <4)
	{
		currMB->mb_type = mbmode;
		memset(&currMB->b8mode[0],mbmode,4 * sizeof(char));
		memset(&currMB->b8pdir[0], 0, 4 * sizeof(char));
	}
	else if(MODE_IS_P8x8)
	{
		currMB->mb_type = P8x8;
		p_Vid->allrefzero = ZERO_P8x8;
	}
	else if(MODE_IS_I4x4)
	{
		currMB->mb_type = I4MB;
		memset(&currMB->b8mode[0],IBLOCK, 4 * sizeof(char));
		memset(&currMB->b8pdir[0],    -1, 4 * sizeof(char));
	}
	else if(MODE_IS_IPCM)
	{
		currMB->mb_type = IPCM;
		currMB->cbp = -1;
		currMB->i16mode = 0;

		memset(&currMB->b8mode[0], 0, 4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1, 4 * sizeof(char));
	}
	else
	{
		currMB->mb_type = I16MB;
		currMB->cbp = ICBPTAB[(I16OFFSET)>>2];
		currMB->i16mode = (I16OFFSET) & 0x03;
		memset(&currMB->b8mode[0], 0, 4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1, 4 * sizeof(char));
	}
}

/*!
************************************************************************
* \brief
*    Interpret the mb mode for I-Frames
************************************************************************
*/
static void interpret_mb_mode_I(Macroblock *currMB)
{
	static const int ICBPTAB[6] = {0,16,32,15,31,47};
	int         mbmode   = currMB->mb_type;

	if (mbmode==0)
	{
		currMB->mb_type = I4MB;
		memset(&currMB->b8mode[0],IBLOCK,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));
	}
	else if(mbmode==25)
	{
		currMB->mb_type=IPCM;
		currMB->cbp= -1;
		currMB->i16mode = 0;

		memset(&currMB->b8mode[0],0,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));
	}
	else
	{
		currMB->mb_type = I16MB;
		currMB->cbp= ICBPTAB[(mbmode-1)>>2];
		currMB->i16mode = (mbmode-1) & 0x03;
		memset(&currMB->b8mode[0], 0, 4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1, 4 * sizeof(char));
	}
}

/*!
************************************************************************
* \brief
*    Interpret the mb mode for B-Frames
************************************************************************
*/
static void interpret_mb_mode_B(Macroblock *currMB)
{
	static const int offset2pdir16x16[12]   = {0, 0, 1, 2, 0,0,0,0,0,0,0,0};
	static const int offset2pdir16x8[22][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,1},{0,0},{0,1},{0,0},{1,0},
	{0,0},{0,2},{0,0},{1,2},{0,0},{2,0},{0,0},{2,1},{0,0},{2,2},{0,0}};
	static const int offset2pdir8x16[22][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,1},{0,0},{0,1},{0,0},
	{1,0},{0,0},{0,2},{0,0},{1,2},{0,0},{2,0},{0,0},{2,1},{0,0},{2,2}};

	static const int ICBPTAB[6] = {0,16,32,15,31,47};

	int i, mbmode;
	int mbtype  = currMB->mb_type;

	//--- set mbtype, b8type, and b8pdir ---
	if (mbtype==0)       // direct
	{
		mbmode=0;
		memset(&currMB->b8mode[0],0,4 * sizeof(char));
		memset(&currMB->b8pdir[0],2,4 * sizeof(char));
	}
	else if (mbtype==23) // intra4x4
	{
		mbmode=I4MB;
		memset(&currMB->b8mode[0],IBLOCK,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));
	}
	else if ((mbtype>23) && (mbtype<48) ) // intra16x16
	{
		mbmode=I16MB;
		memset(&currMB->b8mode[0],0,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));

		currMB->cbp     = ICBPTAB[(mbtype-24)>>2];
		currMB->i16mode = (mbtype-24) & 0x03;
	}
	else if (mbtype==22) // 8x8(+split)
	{
		mbmode=P8x8;       // b8mode and pdir is transmitted in additional codewords
	}
	else if (mbtype<4)   // 16x16
	{
		mbmode=1;
		memset(&currMB->b8mode[0], 1,4 * sizeof(char));
		memset(&currMB->b8pdir[0],offset2pdir16x16[mbtype],4 * sizeof(char));
	}
	else if(mbtype==48)
	{
		mbmode=IPCM;
		memset(&currMB->b8mode[0], 0,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));

		currMB->cbp= -1;
		currMB->i16mode = 0;
	}

	else if ((mbtype&0x01)==0) // 16x8
	{
		mbmode=2;
		memset(&currMB->b8mode[0], 2,4 * sizeof(char));
		for(i=0;i<4;++i)
		{
			currMB->b8pdir[i] = (char) offset2pdir16x8 [mbtype][i>>1];
		}
	}
	else
	{
		mbmode=3;
		memset(&currMB->b8mode[0], 3,4 * sizeof(char));
		for(i=0;i<4; ++i)
		{
			currMB->b8pdir[i] = (char) offset2pdir8x16 [mbtype][i&0x01];
		}
	}
	currMB->mb_type = mbmode;
}
/*!
************************************************************************
* \brief
*    Interpret the mb mode for SI-Frames
************************************************************************
*/
static void interpret_mb_mode_SI(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	const int ICBPTAB[6] = {0,16,32,15,31,47};
	int         mbmode   = currMB->mb_type;

	if (mbmode==0)
	{
		currMB->mb_type = SI4MB;
		memset(&currMB->b8mode[0],IBLOCK,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));
		p_Vid->siblock[currMB->mb_y][currMB->mb_x]=1;
	}
	else if (mbmode==1)
	{
		currMB->mb_type = I4MB;
		memset(&currMB->b8mode[0],IBLOCK,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));
	}
	else if(mbmode==26)
	{
		currMB->mb_type=IPCM;
		currMB->cbp= -1;
		currMB->i16mode = 0;
		memset(&currMB->b8mode[0],0,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));
	}

	else
	{
		currMB->mb_type = I16MB;
		currMB->cbp= ICBPTAB[(mbmode-2)>>2];
		currMB->i16mode = (mbmode-2) & 0x03;
		memset(&currMB->b8mode[0],0,4 * sizeof(char));
		memset(&currMB->b8pdir[0],-1,4 * sizeof(char));
	}
}

/*!
************************************************************************
* \brief
*    Set mode interpretation based on slice type
************************************************************************
*/
void setup_slice_methods(Slice *currSlice)
{
	switch (currSlice->slice_type)
	{
	case P_SLICE: 
		currSlice->interpret_mb_mode         = interpret_mb_mode_P;
		currSlice->read_motion_info_from_NAL = read_motion_info_from_NAL_p_slice;
		currSlice->read_one_macroblock       = read_one_macroblock_p_slice;
		currSlice->decode_one_component      = decode_one_component_p_slice;
		break;
	case SP_SLICE:
		currSlice->interpret_mb_mode         = interpret_mb_mode_P;
		currSlice->read_motion_info_from_NAL = read_motion_info_from_NAL_p_slice;
		currSlice->read_one_macroblock       = read_one_macroblock_p_slice;
		currSlice->decode_one_component      = decode_one_component_sp_slice;
		break;
	case B_SLICE:
		currSlice->interpret_mb_mode         = interpret_mb_mode_B;
		currSlice->read_motion_info_from_NAL = read_motion_info_from_NAL_b_slice;
		currSlice->read_one_macroblock       = read_one_macroblock_b_slice;
		currSlice->decode_one_component      = decode_one_component_b_slice;
		break;
	case I_SLICE: 
		currSlice->interpret_mb_mode         = interpret_mb_mode_I;
		currSlice->read_motion_info_from_NAL = NULL;
		currSlice->read_one_macroblock       = read_one_macroblock_i_slice;
		currSlice->decode_one_component      = decode_one_component_i_slice;
		break;
	case SI_SLICE: 
		currSlice->interpret_mb_mode         = interpret_mb_mode_SI;
		currSlice->read_motion_info_from_NAL = NULL;
		currSlice->read_one_macroblock       = read_one_macroblock_i_slice;
		currSlice->decode_one_component      = decode_one_component_i_slice;
		break;
	default:
		printf("Unsupported slice type\n");
		break;
	}

	if( IS_INDEPENDENT(currSlice->p_Vid) )
		currSlice->compute_colocated  = compute_colocated_JV;
	else
	{
		if (currSlice->active_sps->frame_mbs_only_flag)
			currSlice->compute_colocated  = compute_colocated;
		else
			currSlice->compute_colocated  = compute_colocated_frames_mbs;     
	}

	switch(currSlice->p_Vid->active_pps->entropy_coding_mode_flag)
	{
	case CABAC:
		currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CABAC;
		break;
	case CAVLC:
		currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CAVLC;
		break;
	default:
		printf("Unsupported entropy coding mode\n");
		break;
	}

}

void macroblock_set_dc_pred(VideoParameters *p_Vid, int block_x, int block_y)
{
	int32_t dc_pred = 2 + (2 << 8) + (2 << 16) + (2 << 24);
	int32_t *pred = (int32_t *)&p_Vid->ipredmode[block_y][block_x];
	int stride = p_Vid->PicWidthInMbs;
	int i;
	for (i=0;i<BLOCK_SIZE;i++)
	{
		*pred = dc_pred;
		pred += stride;
	}
}
/*!
************************************************************************
* \brief
*    init macroblock I and P frames
************************************************************************
*/
#ifdef _M_IX86
static void init_macroblock(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int j;
	int block_x = currMB->block_x, block_y = currMB->block_y;
	PicMotionParams *motion = &p_Vid->dec_picture->motion;
	PicMotion **list_motion0, **list_motion1;
	__m64 const_0_minus_1 = _mm_setr_pi32(0, -1);
	macroblock_set_dc_pred(p_Vid, block_x, block_y);

	// reset vectors and pred. modes
	list_motion0 = motion->motion[LIST_0];
	for(j = 0; j < BLOCK_SIZE; j++)
	{                           
		PicMotion *block = &list_motion0[block_y+j][block_x];
		block[0].ref_pic_id = UNDEFINED_REFERENCE;	
		*(__m64 *)&block[0].mv = const_0_minus_1;

		block[1].ref_pic_id = UNDEFINED_REFERENCE;			
		*(__m64 *)&block[1].mv = const_0_minus_1;

		block[2].ref_pic_id = UNDEFINED_REFERENCE;			
		*(__m64 *)&block[2].mv = const_0_minus_1;


		block[3].ref_pic_id = UNDEFINED_REFERENCE;			
		*(__m64 *)&block[3].mv = const_0_minus_1;
	}

	list_motion1 = motion->motion[LIST_1];
	for(j = 0; j < BLOCK_SIZE; j++)
	{                           
		PicMotion *block = &list_motion1[block_y+j][block_x];

		block[0].ref_pic_id = UNDEFINED_REFERENCE;	
		*(__m64 *)&block[0].mv = const_0_minus_1;

		block[1].ref_pic_id = UNDEFINED_REFERENCE;			
		*(__m64 *)&block[1].mv = const_0_minus_1;

		block[2].ref_pic_id = UNDEFINED_REFERENCE;			
		*(__m64 *)&block[2].mv = const_0_minus_1;


		block[3].ref_pic_id = UNDEFINED_REFERENCE;			
		*(__m64 *)&block[3].mv = const_0_minus_1;
	}

}


#else
static void init_macroblock(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int i, j;
	int block_x = currMB->block_x, block_y = currMB->block_y;
	PicMotionParams *motion = &p_Vid->dec_picture->motion;
	PicMotion **list_motion0, **list_motion1;
	macroblock_set_dc_pred(p_Vid, block_x, block_y);

	// reset vectors and pred. modes
	list_motion0 = motion->motion[LIST_0];
	for(j = 0; j < BLOCK_SIZE; j++)
	{                           
		PicMotion *block0 = &list_motion0[block_y+j][block_x];
		block0[0].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block0[0].mv, 0, sizeof(MotionVector));
		block0[0].ref_idx = -1;

		block0[1].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block0[1].mv, 0, sizeof(MotionVector));
		block0[1].ref_idx = -1;			

		block0[2].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block0[2].mv, 0, sizeof(MotionVector));
		block0[2].ref_idx = -1;		

		block0[3].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block0[3].mv, 0, sizeof(MotionVector));
		block0[3].ref_idx = -1;			


	}

	list_motion1 = motion->motion[LIST_1];
	for(j = 0; j < BLOCK_SIZE; j++)
	{                           
		PicMotion *block1 = &list_motion1[block_y+j][block_x];

		block1[0].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block1[0].mv, 0, sizeof(MotionVector));
		block1[0].ref_idx = -1;

		block1[1].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block1[1].mv, 0, sizeof(MotionVector));
		block1[1].ref_idx = -1;			

		block1[2].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block1[2].mv, 0, sizeof(MotionVector));
		block1[2].ref_idx = -1;			

		block1[3].ref_pic_id = UNDEFINED_REFERENCE;			
		memset(block1[3].mv, 0, sizeof(MotionVector));
		block1[3].ref_idx = -1;			
	}

}


#endif
/*!
************************************************************************
* \brief
*    Sets mode for 8x8 block
************************************************************************
*/
void SetB8Mode (Macroblock* currMB, int value, int i)
{
	Slice* currSlice = currMB->p_Slice;
	static const char p_v2b8 [ 5] = {4, 5, 6, 7, IBLOCK};
	static const char p_v2pd [ 5] = {0, 0, 0, 0, -1};
	static const char b_v2b8 [14] = {0, 4, 4, 4, 5, 6, 5, 6, 5, 6, 7, 7, 7, IBLOCK};
	static const char b_v2pd [14] = {2, 0, 1, 2, 0, 0, 1, 1, 2, 2, 0, 1, 2, -1};

	if (currSlice->slice_type==B_SLICE)
	{
		currMB->b8mode[i] = b_v2b8[value];
		currMB->b8pdir[i] = b_v2pd[value];
	}
	else
	{
		currMB->b8mode[i] = p_v2b8[value];
		currMB->b8pdir[i] = p_v2pd[value];
	}
}


void reset_coeffs(Slice *currSlice)
{

	VideoParameters *p_Vid = currSlice->p_Vid;

	// reset all coeffs
#ifdef _DEBUG
	{
		int m;
		for (m=0;m<3;m++)
		{
			int z;
			short *b = &currSlice->cof[m][0][0];
			for (z=0;z<256;z++)
			{
				if (b[z] != 0)
				{
					DebugBreak();
				}
			}
		}
	}
#endif

	// benski> don't think this is necessary... enable check above to be sure
	// 	memset(currSlice->cof, 0, sizeof(currSlice->cof));

	// CAVLC
	if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
		memzero48(p_Vid->nz_coeff[p_Vid->current_mb_nr]);
}

void field_flag_inference(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	if (currMB->mb_avail_left)
	{
		currMB->mb_field = p_Vid->mb_data[currMB->mb_addr_left].mb_field;
	}
	else
	{
		// check top macroblock pair
		currMB->mb_field = currMB->mb_avail_up ? p_Vid->mb_data[currMB->mb_addr_up].mb_field : FALSE;
	}
}


static void skip_macroblock(Macroblock *currMB)
{
	short pred_mv[2];
	int zeroMotionAbove;
	int zeroMotionLeft;
	PixelPos mb[4];    // neighbor blocks
	int   i, j;
	int   a_mv_y = 0;
	int   a_ref_idx = 0;
	int   b_mv_y = 0;
	int   b_ref_idx = 0;
	int   img_block_y   = currMB->block_y;
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;
	int   list_offset = ((currSlice->mb_aff_frame_flag) && (currMB->mb_field)) ? (currMB->mbAddrX & 0x01) ? 4 : 2 : 0;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;
	short *a_mv = NULL;
	short *b_mv = NULL;

	get_neighbors0016(currMB, mb);

	if (mb[0].available)
	{
		a_mv      = motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].mv;
		a_mv_y    = a_mv[1];    
		a_ref_idx = motion->motion[LIST_0][mb[0].pos_y][mb[0].pos_x].ref_idx;

		if (currMB->mb_field && !p_Vid->mb_data[mb[0].mb_addr].mb_field)
		{
			a_mv_y    /=2;
			a_ref_idx *=2;
		}
		if (!currMB->mb_field && p_Vid->mb_data[mb[0].mb_addr].mb_field)
		{
			a_mv_y    *=2;
			a_ref_idx >>=1;
		}
	}

	if (mb[1].available)
	{
		b_mv      = motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].mv;
		b_mv_y    = b_mv[1];
		b_ref_idx = motion->motion[LIST_0][mb[1].pos_y][mb[1].pos_x].ref_idx;

		if (currMB->mb_field && !p_Vid->mb_data[mb[1].mb_addr].mb_field)
		{
			b_mv_y    /=2;
			b_ref_idx *=2;
		}
		if (!currMB->mb_field && p_Vid->mb_data[mb[1].mb_addr].mb_field)
		{
			b_mv_y    *=2;
			b_ref_idx >>=1;
		}
	}

	zeroMotionLeft  = !mb[0].available ? 1 : a_ref_idx==0 && a_mv[0]==0 && a_mv_y==0 ? 1 : 0;
	zeroMotionAbove = !mb[1].available ? 1 : b_ref_idx==0 && b_mv[0]==0 && b_mv_y==0 ? 1 : 0;

	currMB->cbp = 0;
	reset_coeffs(currSlice);

	if (zeroMotionAbove || zeroMotionLeft)
	{
		for(j = img_block_y; j < img_block_y + BLOCK_SIZE; ++j)
		{
			for(i=currMB->block_x;i<currMB->block_x + BLOCK_SIZE; ++i)
			{
				memset(&motion->motion[LIST_0][j][i].mv, 0, sizeof(MotionVector));
				motion->motion[LIST_0][j][i].ref_idx=0;
				motion->motion[LIST_0][j][i].ref_pic_id = dec_picture->ref_pic_num[p_Vid->current_slice_nr][LIST_0 + list_offset][0];
			}
		}
	}
	else
	{
		currMB->GetMVPredictor (currMB, mb, pred_mv, 0, motion->motion[LIST_0], 0, 0, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

		// Set first block line (position img_block_y)
		for(j=img_block_y; j < img_block_y + BLOCK_SIZE; ++j)
		{
			for(i=currMB->block_x;i<currMB->block_x + BLOCK_SIZE; ++i)
			{
				memcpy(&motion->motion[LIST_0][j][i].mv, pred_mv, sizeof(MotionVector));
				motion->motion[LIST_0][j][i].ref_idx=0;
				motion->motion[LIST_0][j][i].ref_pic_id = dec_picture->ref_pic_num[p_Vid->current_slice_nr][LIST_0 + list_offset][0];
			}
		}
	}
}

static void concealIPCMcoeffs(Macroblock *currMB)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	int i, j, k;

	for(i=0;i<MB_BLOCK_SIZE;++i)
	{
		for(j=0;j<MB_BLOCK_SIZE;++j)
		{
			currSlice->ipcm[0][i][j] = p_Vid->dc_pred_value_comp[0];
		}
	}

	if ((dec_picture->chroma_format_idc != YUV400) && !IS_INDEPENDENT(p_Vid))
	{
		for (k = 0; k < 2; ++k)
		{
			for(i=0;i<p_Vid->mb_cr_size_y;++i)
			{
				for(j=0;j<p_Vid->mb_cr_size_x;++j)
				{
					currSlice->ipcm[k][i][j] = p_Vid->dc_pred_value_comp[k];
				}
			}
		}
	}
}

/*!
************************************************************************
* \brief
*    Get the syntax elements from the NAL
************************************************************************
*/
static void read_one_macroblock_i_slice(Macroblock *currMB)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;

	SyntaxElement currSE;
	int mb_nr = currMB->mbAddrX; 

	DataPartition *dP;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;

	currMB->mb_field = ((mb_nr&0x01) == 0)? FALSE : p_Vid->mb_data[mb_nr-1].mb_field;

	update_qp(currMB, p_Vid->qp);

	//  read MB mode *****************************************************************
	dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

	if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
		currSE.mapping = linfo_ue;

	// read MB aff
	if (currSlice->mb_aff_frame_flag && (mb_nr&0x01)==0)
	{
		TRACE_STRING("mb_field_decoding_flag");
		if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
		{
			currMB->mb_field = readSyntaxElement_FLC(dP->bitstream, 1);
		}
		else
		{
			currMB->mb_field = readFieldModeInfo_CABAC(currMB, &dP->de_cabac);
		}
	}

	if(p_Vid->active_pps->entropy_coding_mode_flag  == CABAC)
	{
		CheckAvailabilityOfNeighborsCABAC(currMB);

		//  read MB type
		currMB->mb_type = readMB_typeInfo_CABAC(currMB, &dP->de_cabac);
	}
	else 
	{ // CAVLC
		//  read MB type
		readSyntaxElement_UVLC(&currSE, dP);
		currMB->mb_type = currSE.value1;
	}



	currMB->ei_flag = 0;

	motion->mb_field[mb_nr] = (byte) currMB->mb_field;

	currMB->block_y_aff = ((currSlice->mb_aff_frame_flag) && (currMB->mb_field)) ? (mb_nr&0x01) ? (currMB->block_y - 4)>>1 : currMB->block_y >> 1 : currMB->block_y;

	p_Vid->siblock[currMB->mb_y][currMB->mb_x] = 0;

	currSlice->interpret_mb_mode(currMB);

	//init NoMbPartLessThan8x8Flag
	currMB->NoMbPartLessThan8x8Flag = TRUE;

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB->mb_type == I4MB && p_Vid->Transform8x8Mode)
	{
		dP = &(currSlice->partArr[partMap[SE_HEADER]]);
		TRACE_STRING("transform_size_8x8_flag");

		// read CAVLC transform_size_8x8_flag
		if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
		{
			currMB->luma_transform_size_8x8_flag = readSyntaxElement_FLC(dP->bitstream, 1);
		}
		else
		{
			currMB->luma_transform_size_8x8_flag = readMB_transform_size_flag_CABAC(currMB, &dP->de_cabac);
		}

		if (currMB->luma_transform_size_8x8_flag)
		{      
			currMB->mb_type = I8MB;
			memset(&currMB->b8mode, I8MB, 4 * sizeof(char));
			memset(&currMB->b8pdir, -1, 4 * sizeof(char));
		}
	}
	else
	{
		currMB->luma_transform_size_8x8_flag = FALSE;
	}

	//--- init macroblock data ---
	init_macroblock(currMB);

	if(currMB->mb_type != IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		read_ipred_modes(currMB);

		// read CBP and Coeffs  ***************************************************************
		currSlice->read_CBP_and_coeffs_from_NAL (currMB);
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dP is assigned with the same dP as SE_MBTYPE, because IPCM syntax is in the
		// same category as MBTYPE
		if ( currSlice->dp_mode && currSlice->dpB_NotPresent )
		{
			concealIPCMcoeffs(currMB);
		}
		else
		{
			dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);
			read_IPCM_coeffs_from_NAL(currSlice, dP);
		}
	}

	return;
}

/*!
************************************************************************
* \brief
*    Get the syntax elements from the NAL
************************************************************************
*/
static void read_one_macroblock_p_slice(Macroblock *currMB)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;

	int i;

	SyntaxElement currSE;
	int mb_nr = currMB->mbAddrX; 

	DataPartition *dP;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	Macroblock *topMB = NULL;
	int  prevMbSkipped = 0;
	int  check_bottom, read_bottom, read_top;  
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;

	if (currSlice->mb_aff_frame_flag)
	{
		if (mb_nr&0x01)
		{
			topMB= &p_Vid->mb_data[mb_nr-1];
			prevMbSkipped = (topMB->mb_type == 0);
		}
		else
			prevMbSkipped = 0;
	}

	currMB->mb_field = ((mb_nr&0x01) == 0)? FALSE : p_Vid->mb_data[mb_nr-1].mb_field;

	update_qp(currMB, p_Vid->qp);

	//  read MB mode *****************************************************************
	dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

	if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)   
		currSE.mapping = linfo_ue;

	if (p_Vid->active_pps->entropy_coding_mode_flag == CABAC)
	{
		int skip;
		// read MB skip_flag
		if (currSlice->mb_aff_frame_flag && ((mb_nr&0x01) == 0||prevMbSkipped))
			field_flag_inference(currMB);

		CheckAvailabilityOfNeighborsCABAC(currMB);
		TRACE_STRING("mb_skip_flag");
		skip = readMB_skip_flagInfo_CABAC(currMB, &dP->de_cabac);

		currMB->mb_type   = !skip;
		currMB->skip_flag = skip;

		currMB->ei_flag = 0;

		// read MB AFF
		if (currSlice->mb_aff_frame_flag)
		{
			check_bottom=read_bottom=read_top=0;
			if ((mb_nr&0x01)==0)
			{
				check_bottom =  currMB->skip_flag;
				read_top = !check_bottom;
			}
			else
			{
				read_bottom = (topMB->skip_flag && (!currMB->skip_flag));
			}

			if (read_bottom || read_top)
			{
				TRACE_STRING("mb_field_decoding_flag");
				currMB->mb_field = readFieldModeInfo_CABAC(currMB, &dP->de_cabac);
			}
			if (check_bottom)
				check_next_mb_and_get_field_mode_CABAC(currSlice, dP);

			CheckAvailabilityOfNeighborsCABAC(currMB);
		}

		// read MB type
		if (currMB->mb_type != 0 )
		{
			TRACE_STRING("mb_type");
			currMB->mb_type = readMB_typeInfo_CABAC(currMB, &dP->de_cabac);
			currMB->ei_flag = 0;
		}
	}
	// VLC Non-Intra
	else
	{
		if(p_Vid->cod_counter == -1)
		{
			TRACE_STRING("mb_skip_run");
			readSyntaxElement_UVLC(&currSE, dP);
			p_Vid->cod_counter = currSE.value1;
		}
		if (p_Vid->cod_counter==0)
		{
			// read MB aff
			if ((currSlice->mb_aff_frame_flag) && (((mb_nr&0x01)==0) || ((mb_nr&0x01) && prevMbSkipped)))
			{
				TRACE_STRING("mb_field_decoding_flag");
				currMB->mb_field = (Boolean) readSyntaxElement_FLC(dP->bitstream, 1);
			}

			// read MB type
			TRACE_STRING("mb_type");
			readSyntaxElement_UVLC(&currSE, dP);
			if(currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE)
				++(currSE.value1);
			currMB->mb_type = currSE.value1;
			currMB->ei_flag = 0;
			p_Vid->cod_counter--;
			currMB->skip_flag = 0;
		}
		else
		{
			p_Vid->cod_counter--;
			currMB->mb_type = 0;
			currMB->ei_flag = 0;
			currMB->skip_flag = 1;

			// read field flag of bottom block
			if(currSlice->mb_aff_frame_flag)
			{
				if(p_Vid->cod_counter == 0 && ((mb_nr&0x01) == 0))
				{
					TRACE_STRING("mb_field_decoding_flag (of coded bottom mb)");
					currMB->mb_field = (Boolean) readSyntaxElement_FLC(dP->bitstream, 1);
					dP->bitstream->frame_bitoffset--;
					TRACE_DECBITS(1);
				}
				else if (p_Vid->cod_counter > 0 && ((mb_nr & 0x01) == 0))
				{
					// check left macroblock pair first
					if (mb_is_available(mb_nr - 2, currMB) && ((mb_nr % (p_Vid->PicWidthInMbs * 2))!=0))
					{
						currMB->mb_field = p_Vid->mb_data[mb_nr-2].mb_field;
					}
					else
					{
						// check top macroblock pair
						if (mb_is_available(mb_nr - 2*p_Vid->PicWidthInMbs, currMB))
						{
							currMB->mb_field = p_Vid->mb_data[mb_nr-2*p_Vid->PicWidthInMbs].mb_field;
						}
						else
							currMB->mb_field = FALSE;
					}
				}
			}
		}
	}

	motion->mb_field[mb_nr] = (byte) currMB->mb_field;

	currMB->block_y_aff = ((currSlice->mb_aff_frame_flag) && (currMB->mb_field)) ? (mb_nr&0x01) ? (currMB->block_y - 4)>>1 : currMB->block_y >> 1 : currMB->block_y;

	p_Vid->siblock[currMB->mb_y][currMB->mb_x] = 0;

	currSlice->interpret_mb_mode(currMB);

	if(currSlice->mb_aff_frame_flag)
	{
		if(currMB->mb_field)
		{
			currSlice->num_ref_idx_l0_active <<=1;
			currSlice->num_ref_idx_l1_active <<=1;
		}
	}

	//init NoMbPartLessThan8x8Flag
	currMB->NoMbPartLessThan8x8Flag = (IS_DIRECT(currMB) && !(p_Vid->active_sps->direct_8x8_inference_flag))? FALSE: TRUE;

	//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
	if (currMB->mb_type == P8x8)
	{
		dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

		if (p_Vid->active_pps->entropy_coding_mode_flag ==CAVLC)
		{
			currSE.mapping = linfo_ue;
			for (i = 0; i < 4; ++i)
			{
				TRACE_STRING("sub_mb_type");
				readSyntaxElement_UVLC(&currSE, dP);
				SetB8Mode (currMB, currSE.value1, i);

				//set NoMbPartLessThan8x8Flag for P8x8 mode
				currMB->NoMbPartLessThan8x8Flag &= (currMB->b8mode[i]==0 && p_Vid->active_sps->direct_8x8_inference_flag) ||
					(currMB->b8mode[i]==4);
			}
		}
		else
		{
			for (i = 0; i < 4; ++i)
			{
				int value = readB8_typeInfo_CABAC(currSlice, &dP->de_cabac);
				SetB8Mode (currMB, value, i);

				//set NoMbPartLessThan8x8Flag for P8x8 mode
				currMB->NoMbPartLessThan8x8Flag &= (currMB->b8mode[i]==0 && p_Vid->active_sps->direct_8x8_inference_flag) ||
					(currMB->b8mode[i]==4);
			}
		}

		//--- init macroblock data ---
		init_macroblock       (currMB);
		currSlice->read_motion_info_from_NAL (currMB);
	}

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB->mb_type == I4MB && p_Vid->Transform8x8Mode)
	{
		dP = &(currSlice->partArr[partMap[SE_HEADER]]);
		TRACE_STRING("transform_size_8x8_flag");

		// read CAVLC transform_size_8x8_flag
		if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
		{
			currMB->luma_transform_size_8x8_flag = (Boolean) readSyntaxElement_FLC(dP->bitstream, 1);
		}
		else
		{
			currMB->luma_transform_size_8x8_flag = readMB_transform_size_flag_CABAC(currMB,  &dP->de_cabac);
		}

		if (currMB->luma_transform_size_8x8_flag)
		{      
			currMB->mb_type = I8MB;
			memset(&currMB->b8mode, I8MB, 4 * sizeof(char));
			memset(&currMB->b8pdir, -1, 4 * sizeof(char));
		}
	}
	else
	{
		currMB->luma_transform_size_8x8_flag = FALSE;
	}

	if(p_Vid->active_pps->constrained_intra_pred_flag)
	{
		if( !IS_INTRA(currMB) )
		{
			p_Vid->intra_block[mb_nr] = 0;
		}
	}

	//--- init macroblock data ---
	if (currMB->mb_type != P8x8)
		init_macroblock(currMB);

	if (IS_SKIP (currMB)) //keep last macroblock
	{
		skip_macroblock(currMB);
	}
	else if(currMB->mb_type != IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		if (IS_INTRA(currMB))
			read_ipred_modes(currMB);

		// read inter frame vector data *********************************************************
		if (IS_INTERMV (currMB) && (currMB->mb_type != P8x8))
		{
			currSlice->read_motion_info_from_NAL (currMB);
		}
		// read CBP and Coeffs  ***************************************************************
		currSlice->read_CBP_and_coeffs_from_NAL (currMB);
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dP is assigned with the same dP as SE_MBTYPE, because IPCM syntax is in the
		// same category as MBTYPE
		if ( currSlice->dp_mode && currSlice->dpB_NotPresent )
		{
			concealIPCMcoeffs(currMB);
		}
		else
		{
			dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);
			read_IPCM_coeffs_from_NAL(currSlice, dP);
		}
	}

	return;
}

/*!
************************************************************************
* \brief
*    Get the syntax elements from the NAL
************************************************************************
*/
static void read_one_macroblock_b_slice(Macroblock *currMB)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	int i;

	SyntaxElement currSE;
	int mb_nr = currMB->mbAddrX; 

	DataPartition *dP;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	Macroblock *topMB = NULL;
	int  prevMbSkipped = 0;
	int  check_bottom, read_bottom, read_top;  
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;

	if (currSlice->mb_aff_frame_flag)
	{
		if (mb_nr&0x01)
		{
			topMB= &p_Vid->mb_data[mb_nr-1];
			prevMbSkipped = topMB->skip_flag;
		}
		else
			prevMbSkipped = 0;
	}

	currMB->mb_field = ((mb_nr&0x01) == 0)? FALSE : p_Vid->mb_data[mb_nr-1].mb_field;

	update_qp(currMB, p_Vid->qp);

	//  read MB mode *****************************************************************
	dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

	if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)   
		currSE.mapping = linfo_ue;

	if (p_Vid->active_pps->entropy_coding_mode_flag == CABAC)
	{
		// read MB skip_flag
		int skip;
		if (currSlice->mb_aff_frame_flag && ((mb_nr&0x01) == 0||prevMbSkipped))
			field_flag_inference(currMB);

		CheckAvailabilityOfNeighborsCABAC(currMB);
		TRACE_STRING("mb_skip_flag");
		skip = readMB_skip_flagInfo_CABAC(currMB, &dP->de_cabac);

		currMB->mb_type   = !skip;
		currMB->skip_flag = skip;

		currMB->cbp = !skip;

		currMB->ei_flag = 0;

		if (skip)
			p_Vid->cod_counter=0;

		// read MB AFF
		if (currSlice->mb_aff_frame_flag)
		{
			check_bottom=read_bottom=read_top=0;
			if ((mb_nr&0x01)==0)
			{
				check_bottom =  currMB->skip_flag;
				read_top = !check_bottom;
			}
			else
			{
				read_bottom = (topMB->skip_flag && (!currMB->skip_flag));
			}

			if (read_bottom || read_top)
			{
				TRACE_STRING("mb_field_decoding_flag");
				currMB->mb_field = readFieldModeInfo_CABAC(currMB, &dP->de_cabac);
			}
			if (check_bottom)
				check_next_mb_and_get_field_mode_CABAC(currSlice,dP);

			CheckAvailabilityOfNeighborsCABAC(currMB);
		}

		// read MB type
		if (currMB->mb_type != 0 )
		{
			TRACE_STRING("mb_type");
			currMB->mb_type = readMB_typeInfo_CABAC(currMB, &dP->de_cabac);
			currMB->ei_flag = 0;
		}
	}
	// VLC Non-Intra
	else
	{
		if(p_Vid->cod_counter == -1)
		{
			TRACE_STRING("mb_skip_run");
			readSyntaxElement_UVLC(&currSE, dP);
			p_Vid->cod_counter = currSE.value1;
		}
		if (p_Vid->cod_counter==0)
		{
			// read MB aff
			if ((currSlice->mb_aff_frame_flag) && (((mb_nr&0x01)==0) || ((mb_nr&0x01) && prevMbSkipped)))
			{
				TRACE_STRING("mb_field_decoding_flag");
				currMB->mb_field = (Boolean) readSyntaxElement_FLC(dP->bitstream, 1);
			}

			// read MB type
			TRACE_STRING("mb_type");
			readSyntaxElement_UVLC(&currSE, dP);
			if(currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE)
				++(currSE.value1);
			currMB->mb_type = currSE.value1;
			currMB->ei_flag = 0;
			p_Vid->cod_counter--;
			currMB->skip_flag = 0;
		}
		else
		{
			p_Vid->cod_counter--;
			currMB->mb_type = 0;
			currMB->ei_flag = 0;
			currMB->skip_flag = 1;

			// read field flag of bottom block
			if(currSlice->mb_aff_frame_flag)
			{
				if(p_Vid->cod_counter == 0 && ((mb_nr&0x01) == 0))
				{
					TRACE_STRING("mb_field_decoding_flag (of coded bottom mb)");
					currMB->mb_field = (Boolean) readSyntaxElement_FLC(dP->bitstream, 1);
					dP->bitstream->frame_bitoffset--;
					TRACE_DECBITS(1);
				}
				else if (p_Vid->cod_counter > 0 && ((mb_nr & 0x01) == 0))
				{
					// check left macroblock pair first
					if (mb_is_available(mb_nr - 2, currMB) && ((mb_nr % (p_Vid->PicWidthInMbs * 2))!=0))
					{
						currMB->mb_field = p_Vid->mb_data[mb_nr-2].mb_field;
					}
					else
					{
						// check top macroblock pair
						if (mb_is_available(mb_nr - 2*p_Vid->PicWidthInMbs, currMB))
						{
							currMB->mb_field = p_Vid->mb_data[mb_nr-2*p_Vid->PicWidthInMbs].mb_field;
						}
						else
							currMB->mb_field = FALSE;
					}
				}
			}
		}
	}

	motion->mb_field[mb_nr] = (byte) currMB->mb_field;

	currMB->block_y_aff = ((currSlice->mb_aff_frame_flag) && (currMB->mb_field)) ? (mb_nr&0x01) ? (currMB->block_y - 4)>>1 : currMB->block_y >> 1 : currMB->block_y;

	p_Vid->siblock[currMB->mb_y][currMB->mb_x] = 0;

	currSlice->interpret_mb_mode(currMB);

	if(currSlice->mb_aff_frame_flag)
	{
		if(currMB->mb_field)
		{
			currSlice->num_ref_idx_l0_active <<=1;
			currSlice->num_ref_idx_l1_active <<=1;
		}
	}

	//init NoMbPartLessThan8x8Flag
	currMB->NoMbPartLessThan8x8Flag = (IS_DIRECT(currMB) && !(p_Vid->active_sps->direct_8x8_inference_flag))? FALSE: TRUE;

	//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
	if (currMB->mb_type == P8x8)
	{
		dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

		if (p_Vid->active_pps->entropy_coding_mode_flag ==CAVLC)
		{
			currSE.mapping = linfo_ue;
			for (i = 0; i < 4; ++i)
			{
				TRACE_STRING("sub_mb_type");
				readSyntaxElement_UVLC(&currSE, dP);
				SetB8Mode (currMB, currSE.value1, i);

				//set NoMbPartLessThan8x8Flag for P8x8 mode
				currMB->NoMbPartLessThan8x8Flag &= (currMB->b8mode[i]==0 && p_Vid->active_sps->direct_8x8_inference_flag) ||
					(currMB->b8mode[i]==4);
			}
		}
		else
		{
			for (i = 0; i < 4; ++i)
			{
				int value = readB8_typeInfo_CABAC(currSlice,  &dP->de_cabac);
				SetB8Mode (currMB, value, i);

				//set NoMbPartLessThan8x8Flag for P8x8 mode
				currMB->NoMbPartLessThan8x8Flag &= (currMB->b8mode[i]==0 && p_Vid->active_sps->direct_8x8_inference_flag) ||
					(currMB->b8mode[i]==4);
			}
		}

		//--- init macroblock data ---
		init_macroblock       (currMB);
		currSlice->read_motion_info_from_NAL (currMB);
	}

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB->mb_type == I4MB && p_Vid->Transform8x8Mode)
	{
		dP = &(currSlice->partArr[partMap[SE_HEADER]]);
		TRACE_STRING("transform_size_8x8_flag");

		// read CAVLC transform_size_8x8_flag
		if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
		{
			currMB->luma_transform_size_8x8_flag = (Boolean) readSyntaxElement_FLC(dP->bitstream, 1);
		}
		else
		{
			currMB->luma_transform_size_8x8_flag = readMB_transform_size_flag_CABAC(currMB, &dP->de_cabac);
		}


		if (currMB->luma_transform_size_8x8_flag)
		{      
			currMB->mb_type = I8MB;
			memset(&currMB->b8mode, I8MB, 4 * sizeof(char));
			memset(&currMB->b8pdir, -1, 4 * sizeof(char));
		}
	}
	else
	{
		currMB->luma_transform_size_8x8_flag = FALSE;
	}

	if(p_Vid->active_pps->constrained_intra_pred_flag) // inter frame
	{
		if( !IS_INTRA(currMB) )
		{
			p_Vid->intra_block[mb_nr] = 0;
		}
	}

	//--- init macroblock data ---
	if (currMB->mb_type != P8x8)
		init_macroblock(currMB);

	if (IS_DIRECT (currMB) && p_Vid->cod_counter >= 0)
	{
		currMB->cbp = 0;
		reset_coeffs(currSlice);

		if (p_Vid->active_pps->entropy_coding_mode_flag ==CABAC)
			p_Vid->cod_counter=-1;
	}
	else if (IS_SKIP (currMB)) //keep last macroblock
	{
		skip_macroblock(currMB);
	}
	else if(currMB->mb_type != IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		if (IS_INTRA(currMB))
			read_ipred_modes(currMB);

		// read inter frame vector data *********************************************************
		if (IS_INTERMV (currMB) && (currMB->mb_type != P8x8))
		{
			currSlice->read_motion_info_from_NAL (currMB);
		}
		// read CBP and Coeffs  ***************************************************************
		currSlice->read_CBP_and_coeffs_from_NAL (currMB);
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dP is assigned with the same dP as SE_MBTYPE, because IPCM syntax is in the
		// same category as MBTYPE
		if ( currSlice->dp_mode && currSlice->dpB_NotPresent )
		{
			concealIPCMcoeffs(currMB);
		}
		else
		{
			dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);
			read_IPCM_coeffs_from_NAL(currSlice, dP);
		}
	}

	return;
}


/*!
************************************************************************
* \brief
*    Initialize decoding engine after decoding an IPCM macroblock
*    (for IPCM CABAC  28/11/2003)
*
* \author
*    Dong Wang <Dong.Wang@bristol.ac.uk>
************************************************************************
*/
static void init_decoding_engine_IPCM(Slice *currSlice)
{   
	Bitstream *currStream;
	int ByteStartPosition;
	int PartitionNumber;
	int i;

	if(currSlice->dp_mode==PAR_DP_1)
		PartitionNumber=1;
	else if(currSlice->dp_mode==PAR_DP_3)
		PartitionNumber=3;
	else
	{
		printf("Partition Mode is not supported\n");
		exit(1);
	}

	for(i=0;i<PartitionNumber;++i)
	{
		currStream = currSlice->partArr[i].bitstream;
		ByteStartPosition = currStream->read_len;

		arideco_start_decoding (&currSlice->partArr[i].de_cabac, currStream->streamBuffer, ByteStartPosition, &currStream->read_len);
	}
}




/*!
************************************************************************
* \brief
*    Read IPCM pcm_alignment_zero_bit and pcm_byte[i] from stream to currSlice->ipcm
*    (for IPCM CABAC and IPCM CAVLC)
*
* \author
*    Dong Wang <Dong.Wang@bristol.ac.uk>
************************************************************************
*/

static void read_IPCM_coeffs_from_NAL(Slice *currSlice, struct datapartition *dP)
{
	VideoParameters *p_Vid = currSlice->p_Vid;

	StorablePicture *dec_picture = p_Vid->dec_picture;
	int i,j;

	//For CABAC, we don't need to read bits to let stream byte aligned
	//  because we have variable for integer bytes position
	if(p_Vid->active_pps->entropy_coding_mode_flag  == CABAC)
	{
		readIPCM_CABAC(currSlice, dP);
		init_decoding_engine_IPCM(currSlice);
	}
	else
	{
		//read bits to let stream byte aligned

		if(((dP->bitstream->frame_bitoffset) & 0x07) != 0)
		{
			TRACE_STRING("pcm_alignment_zero_bit");
			readSyntaxElement_FLC(dP->bitstream, (8 - ((dP->bitstream->frame_bitoffset) & 0x07)));
		}

		//read luma and chroma IPCM coefficients
		TRACE_STRING("pcm_sample_luma");

		for(i=0;i<MB_BLOCK_SIZE;++i)
		{
			for(j=0;j<MB_BLOCK_SIZE;++j)
			{
				currSlice->ipcm[0][i][j] = readSyntaxElement_FLC(dP->bitstream, p_Vid->bitdepth_luma);
			}
		}
		if ((dec_picture->chroma_format_idc != YUV400) && !IS_INDEPENDENT(p_Vid))
		{
			TRACE_STRING("pcm_sample_chroma (u)");
			for(i=0;i<p_Vid->mb_cr_size_y;++i)
			{
				for(j=0;j<p_Vid->mb_cr_size_x;++j)
				{
					currSlice->ipcm[1][i][j] = readSyntaxElement_FLC(dP->bitstream, p_Vid->bitdepth_chroma);
				}
			}
			TRACE_STRING("pcm_sample_chroma (v)");
			for(i=0;i<p_Vid->mb_cr_size_y;++i)
			{
				for(j=0;j<p_Vid->mb_cr_size_x;++j)
				{
					currSlice->ipcm[2][i][j] = readSyntaxElement_FLC(dP->bitstream, p_Vid->bitdepth_chroma);
				}
			}
		}
	}
}


/*!
************************************************************************
* \brief
*    If data partition B is lost, conceal PCM sample values with DC.
*
************************************************************************
*/


static void __forceinline read_ipred_iblock(VideoParameters *p_Vid, Macroblock *currMB, Slice *currSlice, DataPartition *dP, int b8)
{
	int i, j;
	int mostProbableIntraPredMode;
	int upIntraPredMode;
	int leftIntraPredMode;
	int bx, by, bi, bj;
	SyntaxElement currSE;
	int ts, ls;
	PixelPos left_block, top_block;
	int dec;

	for(j=0;j<2;j++)  //loop subblocks
	{
		by = (b8&2) + j;
		bj = currMB->block_y + by;
		for(i=0;i<2;i++)
		{
			int pred_mode;
			bx = ((b8&1)<<1) + i;
			bi = currMB->block_x + bx;

			//get from stream
			if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
			{
				readSyntaxElement_Intra4x4PredictionMode(&currSE, dP->bitstream);
				pred_mode = currSE.value1;
			}
			else
			{
				pred_mode = readIntraPredMode_CABAC(currSlice, &dP->de_cabac);
			}

			p_Vid->getNeighbourXPLumaNB(currMB, (bx<<2) - 1, (by<<2),      &left_block);
			p_Vid->getNeighbourPXLumaNB(currMB, (bx<<2),     (by<<2) - 1,  &top_block );

			//get from array and decode

			if (p_Vid->active_pps->constrained_intra_pred_flag)
			{
				left_block.available = left_block.available ? p_Vid->intra_block[left_block.mb_addr] : 0;
				top_block.available  = top_block.available  ? p_Vid->intra_block[top_block.mb_addr]  : 0;
			}

			// !! KS: not sure if the following is still correct...
			ts = ls = 0;   // Check to see if the neighboring block is SI
			if (currMB->mb_type == I4MB && currSlice->slice_type == SI_SLICE)           // need support for MBINTLC1
			{
				if (left_block.available)
					if (p_Vid->siblock [left_block.mb_addr / p_Vid->PicWidthInMbs][left_block.mb_addr % p_Vid->PicWidthInMbs])
						ls=1;

				if (top_block.available)
					if (p_Vid->siblock [top_block.mb_addr / p_Vid->PicWidthInMbs][top_block.mb_addr % p_Vid->PicWidthInMbs])
						ts=1;
			}

			upIntraPredMode            = (top_block.available  &&(ts == 0)) ? p_Vid->ipredmode[top_block.pos_y>>2 ][top_block.pos_x>>2 ] : -1;
			leftIntraPredMode          = (left_block.available &&(ls == 0)) ? p_Vid->ipredmode[left_block.pos_y>>2][left_block.pos_x>>2] : -1;

			mostProbableIntraPredMode  = (upIntraPredMode < 0 || leftIntraPredMode < 0) ? DC_PRED : upIntraPredMode < leftIntraPredMode ? upIntraPredMode : leftIntraPredMode;

			dec = (pred_mode == -1) ? mostProbableIntraPredMode : pred_mode + (pred_mode >= mostProbableIntraPredMode);


			p_Vid->ipredmode[bj][bi] = dec;
		}
	}
}

static void __forceinline read_ipred_i8mb(VideoParameters *p_Vid, Macroblock *currMB, Slice *currSlice, DataPartition *dP, int b8)
{
	int mostProbableIntraPredMode;
	int upIntraPredMode;
	int leftIntraPredMode;
	int bx, by, bi, bj;
	int pred_mode;
	SyntaxElement currSE;
	int ts, ls;
	PixelPos left_block, top_block;
	int dec;

	by = (b8&2);
	bj = currMB->block_y + by;

	bx = ((b8&1)<<1);
	bi = currMB->block_x + bx;

	//get from stream
	if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
	{
		readSyntaxElement_Intra4x4PredictionMode(&currSE, dP->bitstream);
		pred_mode = currSE.value1;
	}
	else
	{
		pred_mode = readIntraPredMode_CABAC(currSlice, &dP->de_cabac);
	}

	p_Vid->getNeighbourXPLumaNB(currMB, (bx<<2) - 1, (by<<2),      &left_block);
	p_Vid->getNeighbourPXLumaNB(currMB, (bx<<2),     (by<<2) - 1,  &top_block );

	//get from array and decode

	if (p_Vid->active_pps->constrained_intra_pred_flag)
	{
		left_block.available = left_block.available ? p_Vid->intra_block[left_block.mb_addr] : 0;
		top_block.available  = top_block.available  ? p_Vid->intra_block[top_block.mb_addr]  : 0;
	}

	// !! KS: not sure if the following is still correct...
	ts = ls = 0;   // Check to see if the neighboring block is SI
	if (currMB->mb_type == I4MB && currSlice->slice_type == SI_SLICE)           // need support for MBINTLC1
	{
		if (left_block.available)
			if (p_Vid->siblock [left_block.mb_addr / p_Vid->PicWidthInMbs][left_block.mb_addr % p_Vid->PicWidthInMbs])
				ls=1;

		if (top_block.available)
			if (p_Vid->siblock [top_block.mb_addr / p_Vid->PicWidthInMbs][top_block.mb_addr % p_Vid->PicWidthInMbs])
				ts=1;
	}

	upIntraPredMode            = (top_block.available  &&(ts == 0)) ? p_Vid->ipredmode[top_block.pos_y>>2 ][top_block.pos_x>>2 ] : -1;
	leftIntraPredMode          = (left_block.available &&(ls == 0)) ? p_Vid->ipredmode[left_block.pos_y>>2][left_block.pos_x>>2] : -1;

	mostProbableIntraPredMode  = (upIntraPredMode < 0 || leftIntraPredMode < 0) ? DC_PRED : upIntraPredMode < leftIntraPredMode ? upIntraPredMode : leftIntraPredMode;

	dec = (pred_mode == -1) ? mostProbableIntraPredMode : pred_mode + (pred_mode >= mostProbableIntraPredMode);

	//set
	p_Vid->ipredmode[bj][bi] = dec;
	p_Vid->ipredmode[bj][bi+1] = dec;
	p_Vid->ipredmode[bj+1][bi] = dec;
	p_Vid->ipredmode[bj+1][bi+1] = dec;				
}

static void read_ipred_modes(Macroblock *currMB)
{
	int b8;
	SyntaxElement currSE;
	DataPartition *dP;
	Slice *currSlice = currMB->p_Slice;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	VideoParameters *p_Vid = currMB->p_Vid;

	StorablePicture *dec_picture = p_Vid->dec_picture;
	char IntraChromaPredModeFlag = IS_INTRA(currMB);

	dP = &(currSlice->partArr[partMap[SE_INTRAPREDMODE]]);

	for(b8 = 0; b8 < 4; ++b8)  //loop 8x8 blocks
	{
		if (currMB->b8mode[b8]==IBLOCK)
		{
			IntraChromaPredModeFlag = 1;
			read_ipred_iblock(p_Vid, currMB, currSlice, dP, b8);
		} 
		else if (currMB->b8mode[b8]==I8MB)
		{
			IntraChromaPredModeFlag = 1;
			read_ipred_i8mb(p_Vid, currMB, currSlice, dP, b8);
		}
	}

	if (IntraChromaPredModeFlag && (dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444))
	{
		TRACE_STRING("intra_chroma_pred_mode");
		dP = &(currSlice->partArr[partMap[SE_INTRAPREDMODE]]);

		if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC) 
		{
			currSE.mapping = linfo_ue;
			readSyntaxElement_UVLC(&currSE, dP);
			currMB->c_ipred_mode = (char) currSE.value1;
		}
		else
		{
			currMB->c_ipred_mode = readCIPredMode_CABAC(currMB, &dP->de_cabac);
		}



		if (currMB->c_ipred_mode < DC_PRED_8 || currMB->c_ipred_mode > PLANE_8)
		{
			error("illegal chroma intra pred mode!\n", 600);
		}
	}
}


/*!
************************************************************************
* \brief
*    Get current block spatial neighbors
************************************************************************
*/
void get_neighbors(Macroblock *currMB,       // <--  current Macroblock
									 PixelPos   *block,     // <--> neighbor blocks
									 int         mb_x,         // <--  block x position
									 int         mb_y,         // <--  block y position
									 int         blockshape_x  // <--  block width
									 )
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int i;
	p_Vid->getNeighbourXPLumaNB(currMB, mb_x - 1,            mb_y    ,  &block[0]); // left
	p_Vid->getNeighbourPXLumaNB(currMB, mb_x,                mb_y - 1,  &block[1]); // up
	p_Vid->getNeighbourPXLuma(currMB, mb_x + blockshape_x, mb_y - 1,  &block[2]); // upper right
	p_Vid->getNeighbourLuma(currMB, mb_x - 1,            mb_y - 1,  &block[3]); // upper left
	for (i = 0; i < 4; i++)
	{
		block[i].pos_x >>= 2;
		block[i].pos_y >>= 2;
	}

	if (mb_y > 0)
	{
		if (mb_x < 8)  // first column of 8x8 blocks
		{
			if (mb_y == 8 )
			{
				if (blockshape_x == MB_BLOCK_SIZE)      
					block[2].available  = 0;
			}
			else if (mb_x+blockshape_x == 8)
			{
				block[2].available = 0;
			}
		}
		else if (mb_x + blockshape_x == MB_BLOCK_SIZE)
		{
			block[2].available = 0;
		}
	}

	if (!block[2].available)
	{
		block[2] = block[3];
	}
}

/* this version is for mb_x == 0, mb_y == 0 and blockshape_x == 16 */
void get_neighbors0016(Macroblock *currMB,       // <--  current Macroblock
											 PixelPos   *block   // <--> neighbor blocks
											 )
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int i;

	p_Vid->getNeighbourLeftLuma(currMB, &block[0]); // left
	p_Vid->getNeighbourPXLumaNB(currMB, 0,  -1,  &block[1]); // up
	p_Vid->getNeighbourPXLuma(currMB, 16, -1,  &block[2]); // upper right
	p_Vid->getNeighbourLuma(currMB, -1, -1,  &block[3]); // upper left
	for (i = 0; i < 4; i++)
	{
		if (block[i].available)
		{
			block[i].pos_x >>= 2;
			block[i].pos_y >>= 2;
		}
	}	

	if (!block[2].available)
	{
		block[2] = block[3];
	}
}

/*!
************************************************************************
* \brief
*    Read motion info
************************************************************************
*/
static void read_motion_info_from_NAL_p_slice(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;

	int mb_nr = currMB->mbAddrX;

	DataPartition *dP = NULL;
	const byte *partMap       = assignSE2partition[currSlice->dp_mode];
	int partmode        = ((currMB->mb_type == P8x8) ? 4 : currMB->mb_type);
	int step_h0         = BLOCK_STEP [partmode][0];
	int step_v0         = BLOCK_STEP [partmode][1];
	h264_ref_t *pic_num;

	int j4;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;

	int list_offset = ((currSlice->mb_aff_frame_flag)&&(currMB->mb_field))? (mb_nr&0x01) ? 4 : 2 : 0;

	if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC) 
	{
		SyntaxElement currSE;
		//=====  READ REFERENCE PICTURE INDICES =====
		dP = &(currSlice->partArr[partMap[SE_REFFRAME]]);
		//  For LIST_0, if multiple ref. pictures, read LIST_0 reference picture indices for the MB ***********
		prepareListforRefIdx (currMB, &currSE, currSlice->num_ref_idx_l0_active, (currMB->mb_type != P8x8) || (!p_Vid->allrefzero));
		readMBRefPictureIdx  (&currSE, dP, currMB, &motion->motion[LIST_0][currMB->block_y], LIST_0, step_v0, step_h0);

		//  For LIST_1, if multiple ref. pictures, read LIST_1 reference picture indices for the MB ***********
		prepareListforRefIdx (currMB, &currSE, currSlice->num_ref_idx_l1_active, (currMB->mb_type != P8x8) || (!p_Vid->allrefzero));
		readMBRefPictureIdx  (&currSE, dP, currMB, &motion->motion[LIST_1][currMB->block_y], LIST_1, step_v0, step_h0);

		//=====  READ MOTION VECTORS =====
		dP = &(currSlice->partArr[partMap[SE_MVD]]);

		currSE.mapping = linfo_se;
		readMBMotionVectors (&currSE, dP, currMB, LIST_0, step_h0, step_v0);
	}
	else                                                  
	{
		if (currMB->mb_type != P8x8 || !p_Vid->allrefzero)
		{
			//=====  READ REFERENCE PICTURE INDICES =====
			dP = &(currSlice->partArr[partMap[SE_REFFRAME]]);
			if (currSlice->num_ref_idx_l0_active > 1)
			{
				//  For LIST_0, if multiple ref. pictures, read LIST_0 reference picture indices for the MB ***********
				readMBRefPictureIdx_CABAC(dP, currMB, &motion->motion[LIST_0][currMB->block_y], LIST_0, step_v0, step_h0);
			}
			else
			{
				readMBRefPictureIdx_CABAC_NoReference(currMB, &motion->motion[LIST_0][currMB->block_y], LIST_0, step_v0, step_h0);
			}

			if (currSlice->num_ref_idx_l1_active > 1)
			{
				//  For LIST_1, if multiple ref. pictures, read LIST_1 reference picture indices for the MB ***********
				readMBRefPictureIdx_CABAC(dP, currMB, &motion->motion[LIST_1][currMB->block_y], LIST_1, step_v0, step_h0);
			}
			else
			{
				readMBRefPictureIdx_CABAC_NoReference(currMB, &motion->motion[LIST_1][currMB->block_y], LIST_1, step_v0, step_h0);
			}
		}
		else
		{
			readMBRefPictureIdx_CABAC_NoReference(currMB, &motion->motion[LIST_0][currMB->block_y], LIST_0, step_v0, step_h0);
			readMBRefPictureIdx_CABAC_NoReference(currMB, &motion->motion[LIST_1][currMB->block_y], LIST_1, step_v0, step_h0);
		}
		//=====  READ MOTION VECTORS =====
		dP = &(currSlice->partArr[partMap[SE_MVD]]);

		readMBMotionVectors_CABAC(dP, currMB, LIST_0, step_h0, step_v0);

	}

	// LIST_0 Motion vectors


	// record reference picture Ids for deblocking decisions
	pic_num = dec_picture->ref_pic_num[p_Vid->current_slice_nr][LIST_0 + list_offset];
	for(j4 = currMB->block_y; j4 < (currMB->block_y +4);++j4)
	{
		PicMotion *ref = &motion->motion[LIST_0][j4][currMB->block_x];
		ref[0].ref_pic_id = (ref[0].ref_idx >= 0)?pic_num[(short)ref[0].ref_idx]:UNDEFINED_REFERENCE;
		ref[1].ref_pic_id = (ref[1].ref_idx >= 0)?pic_num[(short)ref[1].ref_idx]:UNDEFINED_REFERENCE;
		ref[2].ref_pic_id = (ref[2].ref_idx >= 0)?pic_num[(short)ref[2].ref_idx]:UNDEFINED_REFERENCE;
		ref[3].ref_pic_id = (ref[3].ref_idx >= 0)?pic_num[(short)ref[3].ref_idx]:UNDEFINED_REFERENCE;
	}
}

/*!
************************************************************************
* \brief
*    Read motion info
************************************************************************
*/
static void read_motion_info_from_NAL_b_slice (Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;
	int i,j,k;
	int mb_nr = currMB->mbAddrX;
	DataPartition *dP = NULL;
	const byte *partMap      = assignSE2partition[currSlice->dp_mode];
	int partmode       = ((currMB->mb_type == P8x8) ? 4 : currMB->mb_type);
	int step_h0        = BLOCK_STEP [partmode][0];
	int step_v0        = BLOCK_STEP [partmode][1];

	int i0, j0, j6;

	int j4, i4, ii;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PicMotionParams *motion = &dec_picture->motion;
	MotionParams *colocated;

	int mv_scale = 0;

	int list_offset = ((currSlice->mb_aff_frame_flag)&&(currMB->mb_field))? (mb_nr&0x01) ? 4 : 2 : 0;


	if ((currSlice->mb_aff_frame_flag) && (currMB->mb_field))
	{
		if(mb_nr&0x01)
		{
			colocated = &currSlice->p_colocated->bottom;
		}
		else
		{
			colocated = &currSlice->p_colocated->top;
		}
	}
	else
	{
		colocated = &currSlice->p_colocated->frame;
	}

	if (currMB->mb_type == P8x8)
	{
		if (currSlice->direct_spatial_mv_pred_flag)
		{
			char  l0_rFrame, l1_rFrame;
			short pmvl0[2]={0,0}, pmvl1[2]={0,0};

			prepare_direct_params(currMB, dec_picture, pmvl0, pmvl1, &l0_rFrame, &l1_rFrame);

			for (k = 0; k < 4; ++k)
			{        
				if (currMB->b8mode[k] == 0)
				{
					i = currMB->block_x + 2 * (k & 0x01);
					for(j = 2 * (k >> 1); j < 2 * (k >> 1)+2;++j)
					{
						j6 = currMB->block_y_aff + j;
						j4 = currMB->block_y     + j;
						for(i4 = i; i4 < i + 2; ++i4)
						{
							if (l0_rFrame >= 0)
							{
								if  (!l0_rFrame  && ((!colocated->moving_block[j6][i4]) && (!p_Vid->listX[LIST_1 + list_offset][0]->is_long_term)))
								{
									motion->motion[LIST_0][j4][i4].mv[0] = 0;
									motion->motion[LIST_0][j4][i4].mv[1] = 0;
									motion->motion[LIST_0][j4][i4].ref_idx = 0;
								}
								else
								{
									motion->motion[LIST_0][j4][i4].mv[0] = pmvl0[0];
									motion->motion[LIST_0][j4][i4].mv[1] = pmvl0[1];
									motion->motion[LIST_0][j4][i4].ref_idx = l0_rFrame;
								}
							}
							else
							{
								motion->motion[LIST_0][j4][i4].mv[0] = 0;
								motion->motion[LIST_0][j4][i4].mv[1] = 0;
								motion->motion[LIST_0][j4][i4].ref_idx = -1;
							}

							if (l1_rFrame >= 0)
							{
								if  (l1_rFrame==0 && ((!colocated->moving_block[j6][i4])&& (!p_Vid->listX[LIST_1 + list_offset][0]->is_long_term)))
								{
									motion->motion[LIST_1][j4][i4].mv[0] = 0;
									motion->motion[LIST_1][j4][i4].mv[1] = 0;
									motion->motion[LIST_1][j4][i4].ref_idx = 0;
								}
								else
								{
									motion->motion[LIST_1][j4][i4].mv[0] = pmvl1[0];
									motion->motion[LIST_1][j4][i4].mv[1] = pmvl1[1];
									motion->motion[LIST_1][j4][i4].ref_idx = l1_rFrame;
								}
							}
							else
							{
								motion->motion[LIST_1][j4][i4].mv[0] = 0;
								motion->motion[LIST_1][j4][i4].mv[1] = 0;
								motion->motion[LIST_1][j4][i4].ref_idx = -1;
							}

							if (l0_rFrame <0 && l1_rFrame <0)
							{
								motion->motion[LIST_0][j4][i4].ref_idx = 0;
								motion->motion[LIST_1][j4][i4].ref_idx = 0;
							}
						}
					}
				}
			}
		}
		else
		{
			for (k = 0; k < 4; ++k) // Scan all blocks
			{
				if (currMB->b8mode[k] == 0)
				{
					for(j0 = 2 * (k >> 1); j0 < 2 * (k >> 1) + 2; j0 += step_v0)
					{
						for(i0 = currMB->block_x + 2*(k & 0x01); i0 < currMB->block_x + 2 * (k & 0x01)+2; i0 += step_h0)
						{
							int refList = colocated->motion[LIST_0 ][currMB->block_y_aff + j0][i0].ref_idx== -1 ? LIST_1 : LIST_0;
							int ref_idx = colocated->motion[refList][currMB->block_y_aff + j0][i0].ref_idx;
							int mapped_idx = -1, iref;

							if (ref_idx == -1)
							{
								for (j4 = currMB->block_y + j0; j4 < currMB->block_y + j0 + step_v0; ++j4)
								{
									int h;
									for (h=0;h<step_h0;h++)
									{
										PicMotion *m0 = &motion->motion[LIST_0][j4][i0+h];
										PicMotion *m1 = &motion->motion[LIST_1][j4][i0+h];
										m0->ref_idx = 0;
										m1->ref_idx = 0;
										memset(&m0->mv, 0, sizeof(MotionVector));
										memset(&m1->mv, 0, sizeof(MotionVector));
									}
								}
							}
							else
							{
								for (iref = 0; iref < imin(currSlice->num_ref_idx_l0_active, p_Vid->listXsize[LIST_0 + list_offset]); ++iref)
								{
									int curr_mb_field = ((currSlice->mb_aff_frame_flag)&&(currMB->mb_field));

									if(p_Vid->structure==0 && curr_mb_field==0)
									{
										// If the current MB is a frame MB and the colocated is from a field picture,
										// then the colocated->ref_pic_id may have been generated from the wrong value of
										// frame_poc if it references it's complementary field, so test both POC values
										if(p_Vid->listX[0][iref]->top_poc * 2    == colocated->motion[refList][currMB->block_y_aff + j0][i0].ref_pic_id
											|| p_Vid->listX[0][iref]->bottom_poc * 2 == colocated->motion[refList][currMB->block_y_aff + j0][i0].ref_pic_id)
										{
											mapped_idx=iref;
											break;
										}
										else //! invalid index. Default to zero even though this case should not happen
											mapped_idx=INVALIDINDEX;
										continue;
									}
									if (dec_picture->ref_pic_num[p_Vid->current_slice_nr][LIST_0 + list_offset][iref]==colocated->motion[refList][currMB->block_y_aff + j0][i0].ref_pic_id)
									{
										mapped_idx=iref;
										break;
									}
									else //! invalid index. Default to zero even though this case should not happen
										mapped_idx=INVALIDINDEX;
								}

								if (INVALIDINDEX == mapped_idx)
								{
									error("temporal direct error: colocated block has ref that is unavailable",-1111);
								}

								for (j = j0; j < j0 + step_v0; ++j)
								{
									j4 = currMB->block_y + j;
									j6 = currMB->block_y_aff + j;

									for (i4 = i0; i4 < i0 + step_h0; ++i4)
									{
										mv_scale = currSlice->mvscale[LIST_0 + list_offset][mapped_idx];

										motion->motion[LIST_0][j4][i4].ref_idx = (char) mapped_idx;
										motion->motion[LIST_1][j4][i4].ref_idx = 0;

										if (mv_scale == 9999 || p_Vid->listX[LIST_0+list_offset][mapped_idx]->is_long_term)
										{
											for (ii=0; ii < 2; ++ii)
											{
												motion->motion[LIST_0][j4][i4].mv[ii] = colocated->motion[refList][j6][i4].mv[ii];
												motion->motion[LIST_1][j4][i4].mv[ii] = 0;
											}
										}
										else
										{
											for (ii=0; ii < 2; ++ii)
											{
												motion->motion[LIST_0][j4][i4].mv[ii] = (short) ((mv_scale * colocated->motion[refList][j6][i4].mv[ii] + 128 ) >> 8);
												motion->motion[LIST_1][j4][i4].mv[ii] = (short) (motion->motion[LIST_0][j4][i4].mv[ii] - colocated->motion[refList][j6][i4].mv[ii]);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}



	if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC) 
	{
		SyntaxElement currSE;
		//=====  READ REFERENCE PICTURE INDICES =====
		dP = &(currSlice->partArr[partMap[SE_REFFRAME]]);
		//  For LIST_0, if multiple ref. pictures, read LIST_0 reference picture indices for the MB ***********
		prepareListforRefIdx (currMB, &currSE,  currSlice->num_ref_idx_l0_active, TRUE);
		readMBRefPictureIdx  (&currSE, dP, currMB, &motion->motion[LIST_0][currMB->block_y], LIST_0, step_v0, step_h0);

		//  For LIST_1, if multiple ref. pictures, read LIST_1 reference picture indices for the MB ***********
		prepareListforRefIdx (currMB, &currSE, currSlice->num_ref_idx_l1_active, TRUE);
		readMBRefPictureIdx  (&currSE, dP, currMB, &motion->motion[LIST_1][currMB->block_y], LIST_1, step_v0, step_h0);

		//=====  READ MOTION VECTORS =====
		dP = &(currSlice->partArr[partMap[SE_MVD]]);

		currSE.mapping = linfo_se;
		// LIST_0 Motion vectors
		readMBMotionVectors (&currSE, dP, currMB, LIST_0, step_h0, step_v0);
		// LIST_1 Motion vectors
		readMBMotionVectors (&currSE, dP, currMB, LIST_1, step_h0, step_v0);
	}
	else                                                  
	{
		//=====  READ REFERENCE PICTURE INDICES =====
		dP = &(currSlice->partArr[partMap[SE_REFFRAME]]);
		if (currSlice->num_ref_idx_l0_active>1)
		{
			//  For LIST_0, if multiple ref. pictures, read LIST_0 reference picture indices for the MB ***********
			readMBRefPictureIdx_CABAC(dP, currMB, &motion->motion[LIST_0][currMB->block_y], LIST_0, step_v0, step_h0);
		}
		else
		{
			readMBRefPictureIdx_CABAC_NoReference(currMB, &motion->motion[LIST_0][currMB->block_y], LIST_0, step_v0, step_h0);
		}

		if (currSlice->num_ref_idx_l1_active > 1)
		{
			//  For LIST_1, if multiple ref. pictures, read LIST_1 reference picture indices for the MB ***********
			readMBRefPictureIdx_CABAC(dP, currMB, &motion->motion[LIST_1][currMB->block_y], LIST_1, step_v0, step_h0);
		}
		else
		{
			readMBRefPictureIdx_CABAC_NoReference(currMB, &motion->motion[LIST_1][currMB->block_y], LIST_1, step_v0, step_h0);
		}

		//=====  READ MOTION VECTORS =====
		dP = &(currSlice->partArr[partMap[SE_MVD]]);

		// LIST_0 Motion vectors
		readMBMotionVectors_CABAC(dP, currMB, LIST_0, step_h0, step_v0);
		// LIST_1 Motion vectors
		readMBMotionVectors_CABAC(dP, currMB, LIST_1, step_h0, step_v0);
	}



	// record reference picture Ids for deblocking decisions

	for (k = LIST_0; k <= LIST_1; ++k)
	{
		const h264_ref_t *rec_pic_num = dec_picture->ref_pic_num[p_Vid->current_slice_nr][k+list_offset];
		PicMotion **list_motion = &motion->motion[k][currMB->block_y];
		for(j4 = 0; j4 < 4 ;++j4)
		{
			PicMotion *m = &list_motion[j4][currMB->block_x];
			m[0].ref_pic_id = (m[0].ref_idx>=0)?rec_pic_num[(short)m[0].ref_idx]:UNDEFINED_REFERENCE;
			m[1].ref_pic_id = (m[1].ref_idx>=0)?rec_pic_num[(short)m[1].ref_idx]:UNDEFINED_REFERENCE;
			m[2].ref_pic_id = (m[2].ref_idx>=0)?rec_pic_num[(short)m[2].ref_idx]:UNDEFINED_REFERENCE;
			m[3].ref_pic_id = (m[3].ref_idx>=0)?rec_pic_num[(short)m[3].ref_idx]:UNDEFINED_REFERENCE;
		}
	}
}

/*!
************************************************************************
* \brief
*    Get the Prediction from the Neighboring Blocks for Number of 
*    Nonzero Coefficients
*
*    Luma Blocks
************************************************************************
*/
static int predict_nnz_cb(Macroblock *currMB, int i,int j)
{
	VideoParameters *p_Vid = currMB->p_Vid;

	PixelPos pix;

	int pred_nnz = 0;
	int cnt      = 0;

	// left block
	p_Vid->getNeighbourLuma(currMB, i - 1, j, &pix);

	if (IS_INTRA(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
	{
		pix.available &= p_Vid->intra_block[pix.mb_addr];
		if (!pix.available)
			++cnt;
	}

	if (pix.available)
	{ 
		pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][1][pix.y>>2][pix.x>>2];
		++cnt;
	}

	// top block
	p_Vid->getNeighbourLuma(currMB, i, j - 1, &pix);

	if (IS_INTRA(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
	{
		pix.available &= p_Vid->intra_block[pix.mb_addr];
		if (!pix.available)
			++cnt;
	}

	if (pix.available)
	{
		pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][1][pix.y>>2][pix.x>>2];
		++cnt;
	}

	if (cnt==2)
	{
		++pred_nnz;
		pred_nnz>>=1;
	}

	return pred_nnz;
}


static int predict_nnz_cr(Macroblock *currMB, int i,int j)
{
	VideoParameters *p_Vid = currMB->p_Vid;

	PixelPos pix;

	int pred_nnz = 0;
	int cnt      = 0;

	// left block
	p_Vid->getNeighbourLuma(currMB, i - 1, j, &pix);

	if (IS_INTRA(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
	{
		pix.available &= p_Vid->intra_block[pix.mb_addr];
		if (!pix.available)
			++cnt;
	}

	if (pix.available)
	{ 
		pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][2][pix.y>>2][pix.x>>2];
		++cnt;
	}

	// top block
	p_Vid->getNeighbourLuma(currMB, i, j - 1, &pix);

	if (IS_INTRA(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
	{
		pix.available &= p_Vid->intra_block[pix.mb_addr];
		if (!pix.available)
			++cnt;
	}

	if (pix.available)
	{
		pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][2][pix.y>>2][pix.x>>2];
		++cnt;
	}

	if (cnt==2)
	{
		++pred_nnz;
		pred_nnz>>=1;
	}

	return pred_nnz;
}


static int predict_nnz_luma(Macroblock *currMB, int i,int j)
{
	VideoParameters *p_Vid = currMB->p_Vid;

	PixelPos pix;

	int pred_nnz = 0;
	int cnt      = 0;

	// left block
	p_Vid->getNeighbourXPLuma(currMB, i - 1, j, &pix);

	if (pix.available)
	{ 
		pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][0][pix.y>>2][pix.x>>2];
		++cnt;
	}

	// top block
	p_Vid->getNeighbourPXLuma(currMB, i, j - 1, &pix);

	if (pix.available)
	{
		pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][0][pix.y>>2][pix.x>>2];
		++cnt;
	}

	if (cnt==2)
	{
		++pred_nnz;
		pred_nnz>>=1;
	}

	return pred_nnz;
}


static int predict_nnz_luma_intra(Macroblock *currMB, int i,int j)
{
	VideoParameters *p_Vid = currMB->p_Vid;

	PixelPos pix;

	int pred_nnz = 0;
	int cnt      = 0;

	// left block
	p_Vid->getNeighbourXPLuma(currMB, i - 1, j, &pix);

	if (pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
	{
		pix.available &= p_Vid->intra_block[pix.mb_addr];
		if (!pix.available)
			++cnt;
	}

	if (pix.available)
	{ 
		pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][0][pix.y>>2][pix.x>>2];
		++cnt;
	}

	// top block
	p_Vid->getNeighbourPXLuma(currMB, i, j - 1, &pix);

	if (pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
	{
		pix.available &= p_Vid->intra_block[pix.mb_addr];
		if (!pix.available)
			++cnt;
	}

	if (pix.available)
	{
		pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][0][pix.y>>2][pix.x>>2];
		++cnt;
	}

	if (cnt==2)
	{
		++pred_nnz;
		pred_nnz>>=1;
	}

	return pred_nnz;
}


/*!
************************************************************************
* \brief
*    Get the Prediction from the Neighboring Blocks for Number of 
*    Nonzero Coefficients
*
*    Chroma Blocks
************************************************************************
*/
static int predict_nnz_chroma_inter(Macroblock *currMB, int i,int j)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PixelPos pix;

	int pred_nnz = 0;
	int cnt      = 0;

	if (dec_picture->chroma_format_idc != YUV444)
	{
		//YUV420 and YUV422
		// left block
		p_Vid->getNeighbour(currMB, ((i&0x01)<<2) - 1, j, p_Vid->mb_size[IS_CHROMA], &pix);
		if (pix.available)
		{
			pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][1][pix.y>>2][2 * (i>>1) + (pix.x>>2)];
			++cnt;
		}

		// top block
		p_Vid->getNeighbour(currMB, ((i&0x01)<<2), j - 1, p_Vid->mb_size[IS_CHROMA], &pix);
		if (pix.available)
		{
			pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][1][pix.y>>2][2 * (i>>1) + (pix.x>>2)];
			++cnt;
		}

		if (cnt==2)
		{
			++pred_nnz;
			pred_nnz >>= 1;
		}
	}

	return pred_nnz;
}


static int predict_nnz_chroma_intra(Macroblock *currMB, int i,int j)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	PixelPos pix;

	int pred_nnz = 0;
	int cnt      = 0;

	if (dec_picture->chroma_format_idc != YUV444)
	{
		//YUV420 and YUV422
		// left block
		p_Vid->getNeighbour(currMB, ((i&0x01)<<2) - 1, j, p_Vid->mb_size[IS_CHROMA], &pix);

		if (pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
		{
			pix.available &= p_Vid->intra_block[pix.mb_addr];
			if (!pix.available)
				++cnt;
		}

		if (pix.available)
		{
			pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][1][pix.y>>2][2 * (i>>1) + (pix.x>>2)];
			++cnt;
		}

		// top block
		p_Vid->getNeighbour(currMB, ((i&0x01)<<2), j - 1, p_Vid->mb_size[IS_CHROMA], &pix);

		if (pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (p_Vid->currentSlice->dp_mode==PAR_DP_3))
		{
			pix.available &= p_Vid->intra_block[pix.mb_addr];
			if (!pix.available)
				++cnt;
		}

		if (pix.available)
		{
			pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][1][pix.y>>2][2 * (i>>1) + (pix.x>>2)];
			++cnt;
		}

		if (cnt==2)
		{
			++pred_nnz;
			pred_nnz >>= 1;
		}
	}

	return pred_nnz;
}


/*!
************************************************************************
* \brief
*    Reads coeff of an 4x4 block (CAVLC)
*
* \author
*    Karl Lillevold <karll@real.com>
*    contributions by James Au <james@ubvideo.com>
************************************************************************
*/
static void readCoeff4x4_CAVLC_Luma (Macroblock *currMB, 
																		 int i, int j, int levarr[16], int runarr[16],
																		 int *number_coefficients)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	int mb_nr = currMB->mbAddrX;
	SyntaxElement currSE;
	DataPartition *dP;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	Bitstream *currStream;

	int k, code, vlcnum;
	int numcoeff = 0, numtrailingones, numcoeff_vlc;
	int level_two_or_higher;
	int numones, totzeros, abslevel;
	int zerosleft;
	int nnz;
	static const int incVlc[] = {0,3,6,12,24,48,32768};    // maximum vlc = 6

	p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 

	if (IS_INTRA (currMB))
	{
		dP = &(currSlice->partArr[partMap[SE_LUM_AC_INTRA]]);
		nnz = predict_nnz_luma_intra(currMB, i<<2, j<<2);
	}
	else
	{
		dP = &(currSlice->partArr[partMap[SE_LUM_AC_INTER]]);
		nnz = predict_nnz_luma(currMB, i<<2, j<<2);
	}

	if (nnz < 2)
	{
		numcoeff_vlc = 0;
	}
	else if (nnz < 4)
	{
		numcoeff_vlc = 1;
	}
	else if (nnz < 8)
	{
		numcoeff_vlc = 2;
	}
	else //
	{
		numcoeff_vlc = 3;
	}

	currStream = dP->bitstream;
	readSyntaxElement_NumCoeffTrailingOnes(&currSE, currStream, numcoeff_vlc);

	numcoeff        =  currSE.value1;
	numtrailingones =  currSE.value2;

	p_Vid->nz_coeff[mb_nr][0][j][i] = (byte) numcoeff;

	memzero64(levarr);
	memzero64(runarr);

	numones = numtrailingones;
	*number_coefficients = numcoeff;

	if (numcoeff)
	{
		if (numtrailingones)
		{ 
			code = readSyntaxElement_FLC(currStream, numtrailingones);

			for (k=0;k<numtrailingones;k++)
			{
#ifdef _M_IX86
				levarr[k+numcoeff-numtrailingones] = ((_bittest((const long *)&code, k)<<1) ^ 0xFFFFFFFF) + 2;
#else
				levarr[k+numcoeff-numtrailingones] = (code>>k)&1 ? -1:1;
#endif
			}
		}

		// decode levels
		level_two_or_higher = (numcoeff > 3 && numtrailingones == 3)? 0 : 1;
		vlcnum = (numcoeff > 10 && numtrailingones < 3) ? 1 : 0;

		for (k = numcoeff - 1 - numtrailingones; k >= 0; k--)
		{
			int level;
			if (vlcnum == 0)
				level=readSyntaxElement_Level_VLC0(currStream);
			else
				level=readSyntaxElement_Level_VLCN(vlcnum, currStream);

			if (level_two_or_higher)
			{
				level += (level > 0) ? 1 : -1;
				level_two_or_higher = 0;
			}

			levarr[k] = level;
			abslevel = iabs(levarr[k]);
			if (abslevel  == 1)
				++numones;

			// update VLC table
			if (abslevel  > incVlc[vlcnum])
				++vlcnum;

			if (k == numcoeff - 1 - numtrailingones && abslevel >3)
				vlcnum = 2;      
		}

		if (numcoeff < 16)
		{
			// decode total run
			vlcnum = numcoeff - 1;
			totzeros = readSyntaxElement_TotalZeros(currStream, vlcnum);
		}
		else
		{
			totzeros = 0;
		}

		// decode run before each coefficient
		zerosleft = totzeros;
		i = numcoeff - 1;

		if (zerosleft > 0 && i > 0)
		{
			do
			{
				// select VLC for runbefore
				vlcnum = imin(zerosleft - 1, RUNBEFORE_NUM_M1);

				runarr[i] = readSyntaxElement_Run(currStream, vlcnum);

				zerosleft -= runarr[i];
				i --;
			} while (zerosleft != 0 && i != 0);
		}
		runarr[i] = zerosleft;    
	} // if numcoeff
}


static void readCoeff4x4_CAVLC_ChromaAC(Macroblock *currMB, 
																				int i, int j, int levarr[16], int runarr[16],
																				int *number_coefficients)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	int mb_nr = currMB->mbAddrX;
	SyntaxElement currSE;
	DataPartition *dP;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	Bitstream *currStream;

	int k, code, vlcnum;
	int numcoeff = 0, numtrailingones, numcoeff_vlc;
	int level_two_or_higher;
	int numones, totzeros, abslevel;
	int zerosleft, ntr;
	int nnz;
	static const int incVlc[] = {0,3,6,12,24,48,32768};    // maximum vlc = 6

	TRACE_PRINTF("ChrDC");
	p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 

	if (IS_INTRA (currMB))
	{
		dP = &(currSlice->partArr[partMap[SE_CHR_AC_INTRA]]);
		nnz = predict_nnz_chroma_intra(currMB, i, ((j-4)<<2));
	}
	else
	{
		dP = &(currSlice->partArr[partMap[SE_CHR_AC_INTER]]);
		nnz = predict_nnz_chroma_inter(currMB, i, ((j-4)<<2));
	}
	currStream = dP->bitstream;  


	// luma or chroma AC    

	if (nnz < 2)
	{
		numcoeff_vlc = 0;
	}
	else if (nnz < 4)
	{
		numcoeff_vlc = 1;
	}
	else if (nnz < 8)
	{
		numcoeff_vlc = 2;
	}
	else //
	{
		numcoeff_vlc = 3;
	}

	readSyntaxElement_NumCoeffTrailingOnes(&currSE, currStream, numcoeff_vlc);

	numcoeff        =  currSE.value1;
	numtrailingones =  currSE.value2;


	p_Vid->nz_coeff[mb_nr][0][j][i] = (byte) numcoeff;

	memzero64(levarr);
	memzero64(runarr);

	numones = numtrailingones;
	*number_coefficients = numcoeff;

	if (numcoeff)
	{
		if (numtrailingones)
		{      
			code = readSyntaxElement_FLC (currStream, numtrailingones);

			ntr = numtrailingones;
			for (k = numcoeff - 1; k > numcoeff - 1 - numtrailingones; k--)
			{
				ntr --;
				levarr[k] = (code>>ntr)&1 ? -1 : 1;
			}
		}

		// decode levels
		level_two_or_higher = (numcoeff > 3 && numtrailingones == 3)? 0 : 1;
		vlcnum = (numcoeff > 10 && numtrailingones < 3) ? 1 : 0;

		for (k = numcoeff - 1 - numtrailingones; k >= 0; k--)
		{

#if TRACE
			snprintf(currSE.tracestring,
				TRACESTRING_SIZE, "%s lev (%d,%d) k=%d vlc=%d ", type, i, j, k, vlcnum);
#endif

			int level;
			if (vlcnum == 0)
				level=readSyntaxElement_Level_VLC0(currStream);
			else
				level=readSyntaxElement_Level_VLCN(vlcnum, currStream);

			if (level_two_or_higher)
			{
				level += (level > 0) ? 1 : -1;
				level_two_or_higher = 0;
			}

			levarr[k] = level;
			abslevel = iabs(levarr[k]);
			if (abslevel  == 1)
				++numones;

			// update VLC table
			if (abslevel  > incVlc[vlcnum])
				++vlcnum;

			if (k == numcoeff - 1 - numtrailingones && abslevel >3)
				vlcnum = 2;      
		}

		if (numcoeff < 15)
		{
			// decode total run
			vlcnum = numcoeff - 1;
			totzeros = readSyntaxElement_TotalZeros(currStream, vlcnum);
		}
		else
		{
			totzeros = 0;
		}

		// decode run before each coefficient
		zerosleft = totzeros;
		i = numcoeff - 1;

		if (zerosleft > 0 && i > 0)
		{
			do
			{
				// select VLC for runbefore
				vlcnum = imin(zerosleft - 1, RUNBEFORE_NUM_M1);

				runarr[i] = readSyntaxElement_Run(currStream, vlcnum);

				zerosleft -= runarr[i];
				i --;
			} while (zerosleft != 0 && i != 0);
		}
		runarr[i] = zerosleft;    
	} // if numcoeff
}

static void readCoeff4x4_CAVLC_ChromaDC(Macroblock *currMB, int i, int j, int levarr[16], int runarr[16], int *number_coefficients)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	int mb_nr = currMB->mbAddrX;
	SyntaxElement currSE;
	DataPartition *dP;
	Bitstream *currStream;

	int k, code, vlcnum;
	int numcoeff = 0, numtrailingones;
	int level_two_or_higher;
	int numones, totzeros, abslevel;
	int zerosleft, ntr;
	int max_coeff_num;
	static const int incVlc[] = {0,3,6,12,24,48,32768};    // maximum vlc = 6

	max_coeff_num = p_Vid->num_cdc_coeff;
	TRACE_PRINTF("ChrDC");
	p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
	if (IS_INTRA (currMB))
		dP = &(currSlice->partArr[assignSE2partition[currSlice->dp_mode][SE_CHR_DC_INTRA]]);
	else
		dP = &(currSlice->partArr[assignSE2partition[currSlice->dp_mode][SE_CHR_DC_INTER]]);
	currStream = dP->bitstream;  

	readSyntaxElement_NumCoeffTrailingOnesChromaDC(p_Vid, &currSE, currStream);

	numcoeff        =  currSE.value1;
	numtrailingones =  currSE.value2;

	memzero64(levarr);
	memzero64(runarr);

	numones = numtrailingones;
	*number_coefficients = numcoeff;

	if (numcoeff)
	{
		if (numtrailingones)
		{      
			code = readSyntaxElement_FLC (currStream, numtrailingones);

			ntr = numtrailingones;
			for (k = numcoeff - 1; k > numcoeff - 1 - numtrailingones; k--)
			{
				ntr --;
				levarr[k] = (code>>ntr)&1 ? -1 : 1;
			}
		}

		// decode levels
		level_two_or_higher = (numcoeff > 3 && numtrailingones == 3)? 0 : 1;
		vlcnum = (numcoeff > 10 && numtrailingones < 3) ? 1 : 0;

		for (k = numcoeff - 1 - numtrailingones; k >= 0; k--)
		{
			int level;
			if (vlcnum == 0)
				level=readSyntaxElement_Level_VLC0(currStream);
			else
				level=readSyntaxElement_Level_VLCN(vlcnum, currStream);

			if (level_two_or_higher)
			{
				level += (level > 0) ? 1 : -1;
				level_two_or_higher = 0;
			}

			levarr[k] = level;
			abslevel = iabs(levarr[k]);
			if (abslevel  == 1)
				++numones;

			// update VLC table
			if (abslevel  > incVlc[vlcnum])
				++vlcnum;

			if (k == numcoeff - 1 - numtrailingones && abslevel >3)
				vlcnum = 2;      
		}

		if (numcoeff < max_coeff_num)
		{
			// decode total run
			vlcnum = numcoeff - 1;
			totzeros = readSyntaxElement_TotalZerosChromaDC(p_Vid, currStream, vlcnum);
		}
		else
		{
			totzeros = 0;
		}

		// decode run before each coefficient
		zerosleft = totzeros;
		i = numcoeff - 1;

		if (zerosleft > 0 && i > 0)
		{
			do
			{
				// select VLC for runbefore
				vlcnum = imin(zerosleft - 1, RUNBEFORE_NUM_M1);

				runarr[i] = readSyntaxElement_Run(currStream, vlcnum);

				zerosleft -= runarr[i];
				i --;
			} while (zerosleft != 0 && i != 0);
		}
		runarr[i] = zerosleft;    
	} // if numcoeff
}

static void readCoeff4x4_CAVLC(Macroblock *currMB, int block_type, int i, int j, int levarr[16], int runarr[16], int *number_coefficients)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	int mb_nr = currMB->mbAddrX;
	SyntaxElement currSE;
	DataPartition *dP;
	Bitstream *currStream;

	int k, code, vlcnum;
	int numcoeff = 0, numtrailingones, numcoeff_vlc;
	int level_two_or_higher;
	int numones, totzeros, abslevel;
	int zerosleft, ntr, dptype = 0;
	int max_coeff_num, nnz;
	static const int incVlc[] = {0,3,6,12,24,48,32768};    // maximum vlc = 6

	switch (block_type)
	{
	case LUMA:
		readCoeff4x4_CAVLC_Luma(currMB, i, j, levarr, runarr, number_coefficients);
		return;
	case LUMA_INTRA16x16DC:
		max_coeff_num = 16;
		TRACE_PRINTF("Lum16DC");
		dptype = SE_LUM_DC_INTRA;
		p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
		break;
	case LUMA_INTRA16x16AC:
		max_coeff_num = 15;
		TRACE_PRINTF("Lum16AC");
		dptype = SE_LUM_AC_INTRA;
		p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
		break;
	case CB:
		max_coeff_num = 16;
		TRACE_PRINTF("Luma_add1");
		dptype = (IS_INTRA (currMB)) ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER;
		p_Vid->nz_coeff[mb_nr][1][j][i] = 0; 
		break;
	case CB_INTRA16x16DC:
		max_coeff_num = 16;
		TRACE_PRINTF("Luma_add1_16DC");
		dptype = SE_LUM_DC_INTRA;
		p_Vid->nz_coeff[mb_nr][1][j][i] = 0; 
		break;
	case CB_INTRA16x16AC:
		max_coeff_num = 15;
		TRACE_PRINTF("Luma_add1_16AC");
		dptype = SE_LUM_AC_INTRA;
		p_Vid->nz_coeff[mb_nr][1][j][i] = 0; 
		break;
	case CR:
		max_coeff_num = 16;
		TRACE_PRINTF("Luma_add2");
		dptype = (IS_INTRA (currMB)) ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER;
		p_Vid->nz_coeff[mb_nr][2][j][i] = 0; 
		break;
	case CR_INTRA16x16DC:
		max_coeff_num = 16;
		TRACE_PRINTF("Luma_add2_16DC");
		dptype = SE_LUM_DC_INTRA;
		p_Vid->nz_coeff[mb_nr][2][j][i] = 0; 
		break;
	case CR_INTRA16x16AC:
		max_coeff_num = 15;
		TRACE_PRINTF("Luma_add1_16AC");
		dptype = SE_LUM_AC_INTRA;
		p_Vid->nz_coeff[mb_nr][2][j][i] = 0; 
		break;        
	case CHROMA_DC:
		readCoeff4x4_CAVLC_ChromaDC(currMB, i, j, levarr, runarr, number_coefficients);
		return;
	case CHROMA_AC:
		readCoeff4x4_CAVLC_ChromaAC(currMB, i, j, levarr, runarr, number_coefficients);
		return;
	default:
		error ("readCoeff4x4_CAVLC: invalid block type", 600);
		p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
		break;
	}

	dP = &(currSlice->partArr[assignSE2partition[currSlice->dp_mode][dptype]]);
	currStream = dP->bitstream;  

	// luma or chroma AC    
	if(block_type==LUMA_INTRA16x16DC || block_type==LUMA_INTRA16x16AC)
	{
		nnz = predict_nnz_luma_intra(currMB, i<<2, j<<2);
	}
	else if (block_type==CB || block_type==CB_INTRA16x16DC || block_type==CB_INTRA16x16AC)
	{   
		nnz = predict_nnz_cb(currMB, i<<2, j<<2);
	}
	else
	{ 
		nnz = predict_nnz_cr(currMB, i<<2, j<<2);
	}

	if (nnz < 2)
	{
		numcoeff_vlc = 0;
	}
	else if (nnz < 4)
	{
		numcoeff_vlc = 1;
	}
	else if (nnz < 8)
	{
		numcoeff_vlc = 2;
	}
	else //
	{
		numcoeff_vlc = 3;
	}

	readSyntaxElement_NumCoeffTrailingOnes(&currSE, currStream, numcoeff_vlc);

	numcoeff        =  currSE.value1;
	numtrailingones =  currSE.value2;

	if(block_type==LUMA_INTRA16x16DC || block_type==LUMA_INTRA16x16AC)
		p_Vid->nz_coeff[mb_nr][0][j][i] = (byte) numcoeff;
	else if (block_type==CB || block_type==CB_INTRA16x16DC || block_type==CB_INTRA16x16AC)
		p_Vid->nz_coeff[mb_nr][1][j][i] = (byte) numcoeff;
	else
		p_Vid->nz_coeff[mb_nr][2][j][i] = (byte) numcoeff;        


	memzero64(levarr);
	memzero64(runarr);

	numones = numtrailingones;
	*number_coefficients = numcoeff;

	if (numcoeff)
	{
		if (numtrailingones)
		{      
			code = readSyntaxElement_FLC(currStream, numtrailingones);

			ntr = numtrailingones;
			for (k = numcoeff - 1; k > numcoeff - 1 - numtrailingones; k--)
			{
				ntr --;
				levarr[k] = (code>>ntr)&1 ? -1 : 1;
			}
		}

		// decode levels
		level_two_or_higher = (numcoeff > 3 && numtrailingones == 3)? 0 : 1;
		vlcnum = (numcoeff > 10 && numtrailingones < 3) ? 1 : 0;

		for (k = numcoeff - 1 - numtrailingones; k >= 0; k--)
		{
			int level;
			if (vlcnum == 0)
				level=readSyntaxElement_Level_VLC0(currStream);
			else
				level=readSyntaxElement_Level_VLCN(vlcnum, currStream);

			if (level_two_or_higher)
			{
				level += (level > 0) ? 1 : -1;
				level_two_or_higher = 0;
			}

			levarr[k] = level;
			abslevel = iabs(levarr[k]);
			if (abslevel  == 1)
				++numones;

			// update VLC table
			if (abslevel  > incVlc[vlcnum])
				++vlcnum;

			if (k == numcoeff - 1 - numtrailingones && abslevel >3)
				vlcnum = 2;      
		}

		if (numcoeff < max_coeff_num)
		{
			// decode total run
			vlcnum = numcoeff - 1;
			totzeros = readSyntaxElement_TotalZeros(currStream, vlcnum);
		}
		else
		{
			totzeros = 0;
		}

		// decode run before each coefficient
		zerosleft = totzeros;
		i = numcoeff - 1;

		if (zerosleft > 0 && i > 0)
		{
			do
			{
				// select VLC for runbefore
				vlcnum = imin(zerosleft - 1, RUNBEFORE_NUM_M1);

				runarr[i] = readSyntaxElement_Run(currStream, vlcnum);

				zerosleft -= runarr[i];
				i --;
			} while (zerosleft != 0 && i != 0);
		}
		runarr[i] = zerosleft;    
	} // if numcoeff
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 4x4 blocks in a SMB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff4x4SMB_I16MB_CABAC(Macroblock *currMB, int context, h264_short_block_t *blocks, int block_y, int block_x, int64 *cbp_blk)
{
	// start_scan == 1
	int i,j,k;
	RunLevel rl;
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];

	const byte *pos_scan4x4 = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN_1D : FIELD_SCAN_1D;
	const byte *pos_scan_4x4;
	// make distinction between INTRA and INTER coded luminance coefficients
	int type = (currMB->is_intra_block ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER);
	DecodingEnvironment *de_cabac = &currSlice->partArr[partMap[type]].de_cabac;

	for (j = 0; j < BLOCK_SIZE_8x8; j += BLOCK_SIZE)
	{
		currMB->subblock_y = block_y + j; // position for coeff_count ctx

		for (i = 0; i < BLOCK_SIZE_8x8; i += BLOCK_SIZE)
		{
			int16_t *block = (int16_t *)(*blocks++);
			currMB->subblock_x = block_x + i; // position for coeff_count ctx
			pos_scan_4x4 = &pos_scan4x4[1];
			for(k = 0; k < 16; k++)
			{
				rl = readRunLevel_CABAC(currMB, de_cabac, context);

				if (rl.level != 0)    /* leave if level == 0 */
				{
					pos_scan_4x4 += rl.run;
					block[*pos_scan_4x4++] = rl.level;
				}
				else
					break;
			}
		}
	}
}

#ifdef _M_IX86
static void readCompCoeff4x4SMB_CABAC(Macroblock *currMB, int context, h264_short_block_t *blocks, int block_y, int block_x, int64_t *cbp_blk64)
#else
static void readCompCoeff4x4SMB_CABAC(Macroblock *currMB, int context, h264_short_block_t *blocks, int block_y, int block_x, int64_t *cbp_blk)
#endif
{
	int k;
	RunLevel rl;
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	const byte *pos_scan4x4 = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN_1D : FIELD_SCAN_1D;
	const byte *pos_scan_4x4;
	int16_t *block;
#ifdef _M_IX86
	int32_t *cbp_blk = (int32_t *)cbp_blk64; 
#endif
	//h264_short_block_t *blocks = &currSlice->cof4[pl][cof4_pos_to_subblock[block_y>>2][block_x>>2]];
	DecodingEnvironment *de_cabac_dc, *de_cabac_ac;
	/*
	* make distinction between INTRA and INTER coded
	* luminance coefficients
	*/
	if (currMB->is_intra_block)
	{
		de_cabac_dc = &currSlice->partArr[partMap[SE_LUM_DC_INTRA]].de_cabac;
		de_cabac_ac = &currSlice->partArr[partMap[SE_LUM_AC_INTRA]].de_cabac;
	}
	else
	{
		de_cabac_dc = &currSlice->partArr[partMap[SE_LUM_DC_INTER]].de_cabac;
		de_cabac_ac = &currSlice->partArr[partMap[SE_LUM_AC_INTER]].de_cabac;
	}
//	for (j = block_y; j < (block_y+BLOCK_SIZE_8x8); j += 4)
	

		block = (int16_t *)(*blocks++);
		currMB->subblock_y = block_y; // position for coeff_count ctx		
		currMB->subblock_x = block_x; // position for coeff_count ctx
		pos_scan_4x4 = pos_scan4x4;
		rl = readRunLevel_CABAC(currMB, de_cabac_dc, context);
		if (rl.level != 0)    /* leave if level == 0 */
		{
			pos_scan_4x4 += rl.run;
			*cbp_blk |= 1 << (block_y + (block_x >> 2)) ;
			block[*pos_scan_4x4++] = rl.level;
			for(k = 0; k < 16; ++k)
			{
				rl = readRunLevel_CABAC(currMB, de_cabac_ac, context);
				if (rl.level != 0)    /* leave if level == 0 */
				{
					pos_scan_4x4 += rl.run;
					block[*pos_scan_4x4++] = rl.level;
				}
				else
					break;
			}
		}

		block = (int16_t *)(*blocks++);
		currMB->subblock_x += 4; // position for coeff_count ctx
		pos_scan_4x4 = pos_scan4x4;
		rl = readRunLevel_CABAC(currMB, de_cabac_dc, context);
		if (rl.level != 0)    /* leave if level == 0 */
		{
			pos_scan_4x4 += rl.run;
			*cbp_blk |= 2 << (block_y + (block_x >> 2)) ;
			block[*pos_scan_4x4++] = rl.level;
			for(k = 0; k < 16; ++k)
			{
				rl = readRunLevel_CABAC(currMB, de_cabac_ac, context);
				if (rl.level != 0)    /* leave if level == 0 */
				{
					pos_scan_4x4 += rl.run;
					block[*pos_scan_4x4++] = rl.level;
				}
				else
					break;
			}
		}
		/* ---- */
		block = (int16_t *)(*blocks++);
		currMB->subblock_y += 4; // position for coeff_count ctx		
		currMB->subblock_x = block_x; // position for coeff_count ctx
		pos_scan_4x4 = pos_scan4x4;
		rl = readRunLevel_CABAC(currMB, de_cabac_dc, context);
		if (rl.level != 0)    /* leave if level == 0 */
		{
			pos_scan_4x4 += rl.run;
			*cbp_blk |= 16 << (block_y + (block_x >> 2)) ;
			block[*pos_scan_4x4++] = rl.level;
			for(k = 0; k < 16; ++k)
			{
				rl = readRunLevel_CABAC(currMB, de_cabac_ac, context);
				if (rl.level != 0)    /* leave if level == 0 */
				{
					pos_scan_4x4 += rl.run;
					block[*pos_scan_4x4++] = rl.level;
				}
				else
					break;
			}
		}

		block = (int16_t *)(*blocks++);
		currMB->subblock_x += 4; // position for coeff_count ctx
		pos_scan_4x4 = pos_scan4x4;
		rl = readRunLevel_CABAC(currMB, de_cabac_dc, context);
		if (rl.level != 0)    /* leave if level == 0 */
		{
			pos_scan_4x4 += rl.run;
			*cbp_blk |= 32 << (block_y + (block_x >> 2)) ;
			block[*pos_scan_4x4++] = rl.level;
			for(k = 0; k < 16; ++k)
			{
				rl = readRunLevel_CABAC(currMB, de_cabac_ac, context);
				if (rl.level != 0)    /* leave if level == 0 */
				{
					pos_scan_4x4 += rl.run;
					block[*pos_scan_4x4++] = rl.level;
				}
				else
					break;
			}
		}
	
}

#if defined(_DEBUG) || defined(_M_IX64)
static void inv_level_coefficients(h264_short_block_t *blocks, const int (*InvLevelScale)[4], int qp_per)
{
	int j, b;

	for (b = 0;b<4;b++)
	{
		h264_short_block_row_t *block = blocks[b];
		for (j = 0; j < 4; ++j)
		{
			if (block[j][0]) block[j][0]= rshift_rnd_sf((block[j][0] * InvLevelScale[j][0]) << qp_per, 4);
			if (block[j][1]) block[j][1]= rshift_rnd_sf((block[j][1] * InvLevelScale[j][1]) << qp_per, 4);
			if (block[j][2]) block[j][2]= rshift_rnd_sf((block[j][2] * InvLevelScale[j][2]) << qp_per, 4);
			if (block[j][3]) block[j][3]= rshift_rnd_sf((block[j][3] * InvLevelScale[j][3]) << qp_per, 4);
		}
	}
}
#else
void inv_level_coefficients(h264_short_block_t *blocks, const int (*InvLevelScale)[4], int qp_per);
#endif

static void inv_level_coefficients_AC(h264_short_block_t *blocks, const int (*InvLevelScale)[4], int qp_per)
{
	int b;

	for (b = 0;b<4;b++)
	{
		h264_short_block_row_t *block = blocks[b];
		if (block[0][1]) block[0][1]= rshift_rnd_sf((block[0][1] * InvLevelScale[0][1]) << qp_per, 4);
		if (block[0][2]) block[0][2]= rshift_rnd_sf((block[0][2] * InvLevelScale[0][2]) << qp_per, 4);
		if (block[0][3]) block[0][3]= rshift_rnd_sf((block[0][3] * InvLevelScale[0][3]) << qp_per, 4);

		if (block[1][0]) block[1][0]= rshift_rnd_sf((block[1][0] * InvLevelScale[1][0]) << qp_per, 4);
		if (block[1][1]) block[1][1]= rshift_rnd_sf((block[1][1] * InvLevelScale[1][1]) << qp_per, 4);
		if (block[1][2]) block[1][2]= rshift_rnd_sf((block[1][2] * InvLevelScale[1][2]) << qp_per, 4);
		if (block[1][3]) block[1][3]= rshift_rnd_sf((block[1][3] * InvLevelScale[1][3]) << qp_per, 4);

		if (block[2][0]) block[2][0]= rshift_rnd_sf((block[2][0] * InvLevelScale[2][0]) << qp_per, 4);
		if (block[2][1]) block[2][1]= rshift_rnd_sf((block[2][1] * InvLevelScale[2][1]) << qp_per, 4);
		if (block[2][2]) block[2][2]= rshift_rnd_sf((block[2][2] * InvLevelScale[2][2]) << qp_per, 4);
		if (block[2][3]) block[2][3]= rshift_rnd_sf((block[2][3] * InvLevelScale[2][3]) << qp_per, 4);

		if (block[3][0]) block[3][0]= rshift_rnd_sf((block[3][0] * InvLevelScale[3][0]) << qp_per, 4);
		if (block[3][1]) block[3][1]= rshift_rnd_sf((block[3][1] * InvLevelScale[3][1]) << qp_per, 4);
		if (block[3][2]) block[3][2]= rshift_rnd_sf((block[3][2] * InvLevelScale[3][2]) << qp_per, 4);
		if (block[3][3]) block[3][3]= rshift_rnd_sf((block[3][3] * InvLevelScale[3][3]) << qp_per, 4);
	}
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of all 4x4 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff4x4MB_CABAC(Macroblock *currMB, ColorPlane pl, int intra, int (*InvLevelScale4x4)[4], int qp_per, int cbp)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	int start_scan = IS_I16MB (currMB)? 1 : 0; 
	int64 *cbp_blk = &currMB->cbp_blk[pl];
	int context;
	h264_short_block_t *blocks = currSlice->cof4[pl];

	currMB->is_intra_block = intra;  	

	if( pl == PLANE_Y || IS_INDEPENDENT(p_Vid) )
		context = (IS_I16MB(currMB) ? LUMA_16AC: LUMA_4x4);
	else if (pl == PLANE_U)
		context = (IS_I16MB(currMB) ? CB_16AC: CB_4x4);
	else
		context = (IS_I16MB(currMB) ? CR_16AC: CR_4x4);  
	if (start_scan == 0)
	{
		if (currMB->is_lossless == FALSE)
		{
			if (cbp & 1)
			{
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[0], 0, 0, cbp_blk);
				inv_level_coefficients(&blocks[0], InvLevelScale4x4, qp_per);
			}
			if (cbp & 2)
			{
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[4], 0, 8, cbp_blk);
				inv_level_coefficients(&blocks[4], InvLevelScale4x4, qp_per);
			}
			if (cbp & 4)
			{
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[8], 8, 0, cbp_blk);
				inv_level_coefficients(&blocks[8], InvLevelScale4x4, qp_per);
			}
			if (cbp & 8)
			{
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[12], 8, 8, cbp_blk);
				inv_level_coefficients(&blocks[12], InvLevelScale4x4, qp_per);
			}
		}
		else
		{
			if (cbp & 1)
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[0], 0, 0, cbp_blk);
			if (cbp & 2)
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[4], 0, 8, cbp_blk);
			if (cbp & 4)
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[8], 8, 0, cbp_blk);
			if (cbp & 8)
				readCompCoeff4x4SMB_CABAC(currMB, context, &blocks[12], 8, 8, cbp_blk);
		}
	}
	else
	{
		if (currMB->is_lossless == FALSE)
		{
			if (cbp & 1)  // are there any coeff in current block at all
			{
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[0], 0, 0, cbp_blk);
				inv_level_coefficients_AC(&blocks[0], InvLevelScale4x4, qp_per);
			}
			if (cbp & 2)  // are there any coeff in current block at all
			{
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[4], 0, 8, cbp_blk);
				inv_level_coefficients_AC(&blocks[4], InvLevelScale4x4, qp_per);
			}
			if (cbp & 4)  // are there any coeff in current block at all
			{
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[8], 8, 0, cbp_blk);
				inv_level_coefficients_AC(&blocks[8], InvLevelScale4x4, qp_per);
			}
			if (cbp & 8)  // are there any coeff in current block at all
			{
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[12], 8, 8, cbp_blk);
				inv_level_coefficients_AC(&blocks[12], InvLevelScale4x4, qp_per);
			}
		}
		else
		{
			if (cbp & 1)
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[0], 0, 0, cbp_blk);
			if (cbp & 2)
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[4], 0, 8, cbp_blk);
			if (cbp & 4)
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[8], 8, 0, cbp_blk);
			if (cbp & 8)
				readCompCoeff4x4SMB_I16MB_CABAC(currMB, context, &blocks[12], 8, 8, cbp_blk);
		}
	}
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of one 8x8 block
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff8x8_CABAC_Lossless(Macroblock *currMB, ColorPlane pl, int b8)
{
	if (currMB->cbp & (1<<b8))  // are there any coefficients in the current block
	{
		VideoParameters *p_Vid = currMB->p_Vid;
		int transform_pl = IS_INDEPENDENT(p_Vid) ? p_Vid->colour_plane_id : pl;
		int scan;
		short *tcoeffs;
		int k;
		RunLevel rl;
		int context;
		DataPartition *dP;
		Slice *currSlice = currMB->p_Slice;
		const byte *partMap = assignSE2partition[currSlice->dp_mode];

		int cbp_mask = (int64) 51 << (4 * b8 - 2 * (b8 & 0x01)); // corresponds to 110011, as if all four 4x4 blocks contain coeff, shifted to block position            
		int64 *cur_cbp = &currMB->cbp_blk[pl];

		// select scan type
		const byte *pos_scan8x8 = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN8x8_1D : FIELD_SCAN8x8_1D;

		int qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[pl] ];
		int qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[pl] ];

		const int *InvLevelScale8x8 = IS_INTRA(currMB)? currSlice->InvLevelScale8x8_Intra[transform_pl][qp_rem] : currSlice->InvLevelScale8x8_Inter[transform_pl][qp_rem];

		currMB->is_intra_block = IS_INTRA(currMB);

		// === set offset in current macroblock ===
		tcoeffs = (short *)(currSlice->mb_rres8[pl][b8]);

		currMB->subblock_x = (b8&0x01) << 3; // position for coeff_count ctx
		currMB->subblock_y = (b8 >> 1) << 3; // position for coeff_count ctx

		if (pl==PLANE_Y || IS_INDEPENDENT(p_Vid))  
			context = LUMA_8x8;
		else if (pl==PLANE_U)
			context = CB_8x8;
		else
			context = CR_8x8;  

		for(k=0; (k < 65);++k)
		{
			//============ read =============
			/*
			* make distinction between INTRA and INTER coded
			* luminance coefficients
			*/

			int type = ((currMB->is_intra_block == 1)
				? (k==0 ? SE_LUM_DC_INTRA : SE_LUM_AC_INTRA) 
				: (k==0 ? SE_LUM_DC_INTER : SE_LUM_AC_INTER));

			dP = &(currSlice->partArr[partMap[type]]);
			rl = readRunLevel_CABAC(currMB, &(dP->de_cabac), context);

			//============ decode =============
			if (rl.level != 0)    /* leave if level == 0 */
			{
				pos_scan8x8 += rl.run;

				scan = *pos_scan8x8++;

				*cur_cbp |= cbp_mask;

				tcoeffs[scan] = rl.level;
			}
			else
				break;
		}
	}
}


static void readCompCoeff8x8_CABAC_Intra(Macroblock *currMB, ColorPlane pl, int b8)
{
	if (currMB->cbp & (1<<b8))  // are there any coefficients in the current block
	{
		VideoParameters *p_Vid = currMB->p_Vid;
		int transform_pl = IS_INDEPENDENT(p_Vid) ? p_Vid->colour_plane_id : pl;
		int scan;
		short *tcoeffs;
		RunLevel rl;
		int k;
		int context;
		DecodingEnvironment *cabac;
		Slice *currSlice = currMB->p_Slice;
		const byte *partMap = assignSE2partition[currSlice->dp_mode];

		int cbp_mask = (int64) 51 << (4 * b8 - 2 * (b8 & 0x01)); // corresponds to 110011, as if all four 4x4 blocks contain coeff, shifted to block position            
		int64 *cur_cbp = &currMB->cbp_blk[pl];

		// select scan type
		const byte *pos_scan8x8 = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN8x8_1D : FIELD_SCAN8x8_1D;

		int qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[pl] ];
		int qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[pl] ];

		const int *InvLevelScale8x8 = currSlice->InvLevelScale8x8_Intra[transform_pl][qp_rem];

		currMB->is_intra_block = 1;

		// === set offset in current macroblock ===
		tcoeffs = (short *)(currSlice->mb_rres8[pl][b8]);

		currMB->subblock_x = (b8&0x01) << 3; // position for coeff_count ctx
		currMB->subblock_y = (b8 >> 1) << 3; // position for coeff_count ctx

		if (pl==PLANE_Y || IS_INDEPENDENT(p_Vid))  
			context = LUMA_8x8;
		else if (pl==PLANE_U)
			context = CB_8x8;
		else
			context = CR_8x8;  

		// Read DC
		cabac = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]].de_cabac);
		rl = readRunLevel_CABAC(currMB, cabac, context);

		//============ decode =============
		if (rl.level != 0)    /* leave if level == 0 */
		{
			*cur_cbp |= cbp_mask; 

			pos_scan8x8 += rl.run;

			scan = *pos_scan8x8++;

			tcoeffs[scan] = rshift_rnd_sf((rl.level * InvLevelScale8x8[scan]) << qp_per, 6); // dequantization

			// AC coefficients
			cabac = &(currSlice->partArr[partMap[SE_LUM_AC_INTRA]].de_cabac);

			k = 64;
			do
			{
				rl = readRunLevel_CABAC(currMB, cabac, context);

				//============ decode =============
				if (rl.level != 0)    /* leave if level == 0 */
				{
					pos_scan8x8 += rl.run;

					scan = *pos_scan8x8++;

					tcoeffs[scan] = rshift_rnd_sf((rl.level * InvLevelScale8x8[scan]) << qp_per, 6); // dequantization
				}
				else
					break;
			} while (--k);
		}
	}

}



static void readCompCoeff8x8_CABAC_Inter(Macroblock *currMB, ColorPlane pl, int b8)
{
	if (currMB->cbp & (1<<b8))  // are there any coefficients in the current block
	{
		VideoParameters *p_Vid = currMB->p_Vid;
		int transform_pl = IS_INDEPENDENT(p_Vid) ? p_Vid->colour_plane_id : pl;
		int scan;
		short *tcoeffs;
		int k;
		RunLevel rl;
		int context;
		DecodingEnvironment *cabac;
		Slice *currSlice = currMB->p_Slice;
		const byte *partMap = assignSE2partition[currSlice->dp_mode];

		int cbp_mask = (int64) 51 << (4 * b8 - 2 * (b8 & 0x01)); // corresponds to 110011, as if all four 4x4 blocks contain coeff, shifted to block position            
		int64 *cur_cbp = &currMB->cbp_blk[pl];

		// select scan type
		const byte *pos_scan8x8 = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN8x8_1D : FIELD_SCAN8x8_1D;

		int qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[pl] ];
		int qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[pl] ];

		const int *InvLevelScale8x8 = currSlice->InvLevelScale8x8_Inter[transform_pl][qp_rem];

		currMB->is_intra_block = 0;

		// === set offset in current macroblock ===
		tcoeffs = (short *)(currSlice->mb_rres8[pl][b8]);

		currMB->subblock_x = (b8&0x01) << 3; // position for coeff_count ctx
		currMB->subblock_y = (b8 >> 1) << 3; // position for coeff_count ctx

		if (pl==PLANE_Y || IS_INDEPENDENT(p_Vid))  
			context = LUMA_8x8;
		else if (pl==PLANE_U)
			context = CB_8x8;
		else
			context = CR_8x8;  

		// Read DC
		cabac = &(currSlice->partArr[partMap[SE_LUM_DC_INTER]].de_cabac);
		rl = readRunLevel_CABAC(currMB, cabac, context);

		//============ decode =============
		if (rl.level != 0)    /* leave if level == 0 */
		{
			*cur_cbp |= cbp_mask; 

			pos_scan8x8 += rl.run;

			scan = *pos_scan8x8++;

			tcoeffs[scan] = rshift_rnd_sf((rl.level * InvLevelScale8x8[scan]) << qp_per, 6); // dequantization

			// AC coefficients
			cabac = &(currSlice->partArr[partMap[SE_LUM_AC_INTER]].de_cabac);

			k=64;
			do
			{
				rl = readRunLevel_CABAC(currMB, cabac, context);

				//============ decode =============
				if (rl.level != 0)    /* leave if level == 0 */
				{
					pos_scan8x8 += rl.run;

					scan = *pos_scan8x8++;

					tcoeffs[scan] = rshift_rnd_sf((rl.level * InvLevelScale8x8[scan]) << qp_per, 6); // dequantization
				}
				else
					break;
			} while (--k);
		}
	}

}


/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 8x8 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff8x8MB_CABAC(Macroblock *currMB, ColorPlane pl)
{
	//======= 8x8 transform size & CABAC ========
	if(currMB->is_lossless == FALSE)
	{
		if (IS_INTRA(currMB))
		{
			readCompCoeff8x8_CABAC_Intra(currMB, pl, 0); 
			readCompCoeff8x8_CABAC_Intra(currMB, pl, 1); 
			readCompCoeff8x8_CABAC_Intra(currMB, pl, 2); 
			readCompCoeff8x8_CABAC_Intra(currMB, pl, 3); 
		}
		else
		{
			readCompCoeff8x8_CABAC_Inter(currMB, pl, 0); 
			readCompCoeff8x8_CABAC_Inter(currMB, pl, 1); 
			readCompCoeff8x8_CABAC_Inter(currMB, pl, 2); 
			readCompCoeff8x8_CABAC_Inter(currMB, pl, 3); 
		}
	}
	else
	{
		readCompCoeff8x8_CABAC_Lossless(currMB, pl, 0); 
		readCompCoeff8x8_CABAC_Lossless(currMB, pl, 1); 
		readCompCoeff8x8_CABAC_Lossless(currMB, pl, 2); 
		readCompCoeff8x8_CABAC_Lossless(currMB, pl, 3); 
	}
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 4x4 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff4x4MB_CAVLC (Macroblock *currMB, ColorPlane pl, int (*InvLevelScale4x4)[4], int qp_per, int cbp, h264_4x4_byte nzcoeff)
{
	int block_y, block_x, b8;
	int i, j, k;
	int i0, j0;
	__declspec(align(32)) int levarr[16], runarr[16];
	int numcoeff;
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
	const byte *pos_scan_4x4 = pos_scan4x4[0];
	int start_scan = IS_I16MB(currMB) ? 1 : 0;
	int64 *cur_cbp = &currMB->cbp_blk[pl];
	int coef_ctr, cur_context; 

	memzero64(levarr);
	memzero64(runarr);

	if (IS_I16MB(currMB))
	{
		if (pl == PLANE_Y)
			cur_context = LUMA_INTRA16x16AC;
		else if (pl == PLANE_U)
			cur_context = CB_INTRA16x16AC;
		else
			cur_context = CR_INTRA16x16AC;
	}
	else
	{
		if (pl == PLANE_Y)
			cur_context = LUMA;
		else if (pl == PLANE_U)
			cur_context = CB;
		else
			cur_context = CR;
	}

	if (currMB->is_lossless == FALSE)
	{
		for (block_y = 0; block_y < 4; block_y += 2) /* all modes */
		{
			for (block_x = 0; block_x < 4; block_x += 2)
			{
				b8 = (block_y + (block_x >> 1));

				if (cbp & (1 << b8))  // test if the block contains any coefficients
				{
					for (j=block_y << 2; j < (block_y + 2) << 2; j += BLOCK_SIZE)
					{
						for (i=block_x << 2; i < (block_x + 2) << 2; i += BLOCK_SIZE)
						{
							readCoeff4x4_CAVLC(currMB, cur_context, i >> 2, j >> 2, levarr, runarr, &numcoeff);
							pos_scan_4x4 = pos_scan4x4[start_scan];

							for (k = 0; k < numcoeff; ++k)
							{
								if (levarr[k] != 0)
								{
									pos_scan_4x4 += (runarr[k] << 1);

									i0 = *pos_scan_4x4++;
									j0 = *pos_scan_4x4++;

									// inverse quant for 4x4 transform only
									*cur_cbp |= (int64) 1 << (j + (i >> 2));

									currSlice->cof4[pl][cof4_pos_to_subblock[j>>2][i>>2]][j0][i0]= rshift_rnd_sf((levarr[k] * InvLevelScale4x4[j0][i0])<<qp_per, 4);
								}
							}
						}
					}
				}
				else
				{
					for (j=0; j < 2; j++)
					{
						for (i=0;i<2;i++)
						{
							nzcoeff[block_y+j][block_x+i]=0;
						}
					}
				}
			}
		}
	}
	else
	{   
		for (block_y=0; block_y < 4; block_y += 2) /* all modes */
		{
			for (block_x=0; block_x < 4; block_x += 2)
			{
				b8 = 2*(block_y>>1) + (block_x>>1);

				if (cbp & (1<<b8))  /* are there any coeff in current block at all */
				{
					for (j=block_y; j < block_y+2; ++j)
					{
						for (i=block_x; i < block_x+2; ++i)
						{
							readCoeff4x4_CAVLC(currMB, cur_context, i, j, levarr, runarr, &numcoeff);

							coef_ctr = start_scan - 1;

							for (k = 0; k < numcoeff; ++k)
							{
								if (levarr[k] != 0)
								{
									coef_ctr += runarr[k]+1;

									i0=pos_scan4x4[coef_ctr][0];
									j0=pos_scan4x4[coef_ctr][1];

									*cur_cbp |= (int64) 1 << ((j<<2) + i);
									currSlice->cof4[pl][cof4_pos_to_subblock[j>>2][i>>2]][j0][i0]= levarr[k];
								}
							}
						}
					}
				}
				else
				{
					for (j=0; j < 2; j++)
					{
						for (i=0;i<2;i++)
						{
							nzcoeff[block_y+j][block_x+i]=0;
						}
					}
				}
			}
		}
	}  
}


/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 4x4 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff8x8MB_CAVLC (Macroblock *currMB, ColorPlane pl, const int *InvLevelScale8x8, int qp_per, int cbp, h264_4x4_byte nzcoeff)
{
	int block_y, block_x, b4, b8;
	int i,j,k;
	int scan;
	__declspec(align(32)) int levarr[16] = {0}, runarr[16] = {0};
	int numcoeff;
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	const byte *pos_scan8x8 = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN8x8_1D : FIELD_SCAN8x8_1D;
	int start_scan = IS_I16MB(currMB) ? 1 : 0;
	int64 *cur_cbp = &currMB->cbp_blk[pl];
	int coef_ctr, cur_context; 
	short *coefficients;	

	if (IS_I16MB(currMB))
	{
		if (pl == PLANE_Y)
			cur_context = LUMA_INTRA16x16AC;
		else if (pl == PLANE_U)
			cur_context = CB_INTRA16x16AC;
		else
			cur_context = CR_INTRA16x16AC;
	}
	else
	{
		if (pl == PLANE_Y)
			cur_context = LUMA;
		else if (pl == PLANE_U)
			cur_context = CB;
		else
			cur_context = CR;
	}

	if (currMB->is_lossless == FALSE)
	{    
		for (block_y=0; block_y < 4; block_y += 2) /* all modes */
		{
			for (block_x=0; block_x < 4; block_x += 2)
			{
				b8 = block_y + (block_x>>1);
				coefficients =(short *)(currSlice->mb_rres8[pl][b8]);
				if (cbp & (1<<b8))  /* are there any coeff in current block at all */
				{
					for (j=block_y; j < block_y+2; ++j)
					{
						for (i=block_x; i < block_x+2; ++i)
						{
							readCoeff4x4_CAVLC(currMB, cur_context, i, j, levarr, runarr, &numcoeff);

							coef_ctr = start_scan - 1;

							for (k = 0; k < numcoeff; ++k)
							{
								if (levarr[k] != 0)
								{
									coef_ctr += runarr[k]+1;

									// do same as CABAC for deblocking: any coeff in the 8x8 marks all the 4x4s
									//as containing coefficients
									*cur_cbp |= 51 << ((block_y<<2) + block_x);

									b4 = (coef_ctr << 2) + 2*(j - block_y)+(i - block_x);

									scan = pos_scan8x8[b4];

									coefficients[scan] = rshift_rnd_sf((levarr[k] * InvLevelScale8x8[scan])<<qp_per, 6); // dequantization
								}
							}//else (!currMB->luma_transform_size_8x8_flag)
						}
					}
				}
				else
				{
					for (j=block_y; j < block_y+2; ++j)
					{
						memset(&nzcoeff[j][block_x], 0, 2 * sizeof(byte));
					}
				}
			}
		}
	}
	else // inverse quant for 8x8 transform
	{
		for (block_y=0; block_y < 4; block_y += 2) /* all modes */
		{
			for (block_x=0; block_x < 4; block_x += 2)
			{
				b8 = 2*(block_y>>1) + (block_x>>1);
				coefficients =(short *)(currSlice->mb_rres8[pl][b8]);
				if (cbp & (1<<b8))  /* are there any coeff in current block at all */
				{
					for (j=block_y; j < block_y+2; ++j)
					{
						for (i=block_x; i < block_x+2; ++i)
						{

							readCoeff4x4_CAVLC(currMB, cur_context, i, j, levarr, runarr, &numcoeff);

							coef_ctr = start_scan - 1;

							for (k = 0; k < numcoeff; ++k)
							{
								if (levarr[k] != 0)
								{
									coef_ctr += runarr[k]+1;

									// do same as CABAC for deblocking: any coeff in the 8x8 marks all the 4x4s
									//as containing coefficients
									*cur_cbp  |= 51 << ((block_y<<2) + block_x);

									b4 = 2*(j-block_y)+(i-block_x);

									scan=pos_scan8x8[coef_ctr*4+b4];

									coefficients[scan] = levarr[k];
								}
							}
						}
					}
				}
				else
				{
					for (j=block_y; j < block_y+2; ++j)
					{
						memset(&nzcoeff[j][block_x], 0, 2 * sizeof(byte));
					}
				}
			}
		}
	}
}

/*!
************************************************************************
* \brief
*    Data partitioning: Check if neighboring macroblock is needed for 
*    CAVLC context decoding, and disable current MB if data partition
*    is missing.
************************************************************************
*/
static void check_dp_neighbors (Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;

	if (IS_INTER (currMB) || (IS_INTRA (currMB) && !(p_Vid->active_pps->constrained_intra_pred_flag)) )
	{
		PixelPos up, left;

		p_Vid->getNeighbourLeft(currMB, p_Vid->mb_size[1], &left);
		p_Vid->getNeighbourUp(currMB, p_Vid->mb_size[1], &up);

		if (left.available)
		{
			currMB->dpl_flag |= p_Vid->mb_data[left.mb_addr].dpl_flag;
		}
		if (up.available)
		{
			currMB->dpl_flag |= p_Vid->mb_data[up.mb_addr].dpl_flag;
		}
	}
}


/*!
************************************************************************
* \brief
*    Get coded block pattern and coefficients (run/level)
*    from the NAL
************************************************************************
*/
static void read_CBP_and_coeffs_from_NAL_CABAC(Macroblock *currMB)
{
	int i,j,k;
	int cbp;
	SyntaxElement currSE;
	DataPartition *dP = NULL;
	Slice *currSlice = currMB->p_Slice;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	int coef_ctr, i0, j0, b8;
	int ll;
	RunLevel rl;

	int qp_per, qp_rem;
	VideoParameters *p_Vid = currMB->p_Vid;
	int intra = IS_INTRA (currMB);
	int smb = ((p_Vid->type==SP_SLICE) && !intra) || (p_Vid->type == SI_SLICE && currMB->mb_type == SI4MB);

	int uv; 
	int qp_per_uv[2];
	int qp_rem_uv[2];


	int temp[4];

	int b4;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	int yuv = dec_picture->chroma_format_idc - 1;
	int m6[4];

	int need_transform_size_flag;

	int (*InvLevelScale4x4)[4] = NULL;

	// select scan type
	const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
	const byte *pos_scan4x4_1d = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN_1D : FIELD_SCAN_1D;
	const byte *pos_scan4x4_dc = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN_DC : FIELD_SCAN_DC;
	const byte *pos_scan_4x4;

	// QPI
	//init constants for every chroma qp offset
	if (dec_picture->chroma_format_idc != YUV400)
	{
		for (i=0; i<2; ++i)
		{
			qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
			qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
		}
	}

	// read CBP if not new intra mode
	if (!IS_I16MB (currMB))
	{
		//=====   C B P   =====
		//---------------------
		int type =  (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
			? SE_CBP_INTRA
			: SE_CBP_INTER;

		dP = &(currSlice->partArr[partMap[type]]);

		currMB->cbp = cbp = readCBP_CABAC(currMB, &(dP->de_cabac));

		TRACE_STRING("coded_block_pattern");


		//============= Transform size flag for INTER MBs =============
		//-------------------------------------------------------------
		need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
			(IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
			(currMB->NoMbPartLessThan8x8Flag))
			&& currMB->mb_type != I8MB && currMB->mb_type != I4MB
			&& (currMB->cbp&15)
			&& p_Vid->Transform8x8Mode);

		if (need_transform_size_flag)
		{
			dP = &(currSlice->partArr[partMap[SE_HEADER]]);
			TRACE_STRING("transform_size_8x8_flag");

			// read CAVLC transform_size_8x8_flag
			currMB->luma_transform_size_8x8_flag = readMB_transform_size_flag_CABAC(currMB, &(dP->de_cabac));
		}

		//=====   DQUANT   =====
		//----------------------
		// Delta quant only if nonzero coeffs
		if (cbp !=0)
		{
			read_delta_quant_CABAC(&currSE, dP, currMB, partMap, (!intra) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

			if (currSlice->dp_mode)
			{
				if (!intra && currSlice->dpC_NotPresent ) 
					currMB->dpl_flag = 1;

				if( intra && currSlice->dpB_NotPresent )
				{
					currMB->ei_flag = 1;
					currMB->dpl_flag = 1;
				}

				// check for prediction from neighbours
				check_dp_neighbors (currMB);
				if (currMB->dpl_flag)
				{
					cbp = 0; 
					currMB->cbp = cbp;
				}
			}
		}
	}
	else
	{
		cbp = currMB->cbp;
	}

	if (IS_I16MB (currMB)) // read DC coeffs for new intra modes
	{
		read_delta_quant_CABAC(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

		macroblock_set_dc_pred(p_Vid, currMB->block_x, currMB->block_y);

		if (currSlice->dp_mode)
		{  
			if (currSlice->dpB_NotPresent)
			{
				currMB->ei_flag  = 1;
				currMB->dpl_flag = 1;
			}
			check_dp_neighbors (currMB);
			if (currMB->dpl_flag)
			{
				currMB->cbp = cbp = 0; 
			}
		}

		if (!currMB->dpl_flag)
		{
			pos_scan_4x4 = pos_scan4x4_dc;

			{
				dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);

				currMB->is_intra_block = 1;

				for(k = 0; k < 17 ; k++)
				{
					rl = readRunLevel_CABAC(currMB, &(dP->de_cabac), LUMA_16DC);

					if (rl.level != 0)    /* leave if level == 0 */
					{
						pos_scan_4x4 += rl.run;
						currSlice->cof4[0][*pos_scan_4x4++][0][0] = rl.level;// add new intra DC coeff
					}
					else
						break;
				}

			}

			if(currMB->is_lossless == FALSE)
				itrans_2(currMB, (ColorPlane) p_Vid->colour_plane_id);// transform new intra DC
		}
	}

	update_qp(currMB, p_Vid->qp);

	qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[p_Vid->colour_plane_id] ];
	qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[p_Vid->colour_plane_id] ];

	//init quant parameters for chroma 
	if (dec_picture->chroma_format_idc != YUV400)
	{
		for(i=0; i < 2; ++i)
		{
			qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
			qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
		}
	}

	InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[p_Vid->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[p_Vid->colour_plane_id][qp_rem];

	// luma coefficients
	{
		//======= Other Modes & CABAC ========
		//------------------------------------          
		if (cbp)
		{
			if(currMB->luma_transform_size_8x8_flag) 
			{
				//======= 8x8 transform size & CABAC ========
				readCompCoeff8x8MB_CABAC (currMB, PLANE_Y); 
			}
			else
			{
				readCompCoeff4x4MB_CABAC (currMB, PLANE_Y, intra, InvLevelScale4x4, qp_per, cbp);        
			}
		}
	}

	if ( p_Vid->active_sps->chroma_format_idc==YUV444 && !IS_INDEPENDENT(p_Vid) ) 
	{
		for (uv = 0; uv < 2; ++uv )
		{
			/*----------------------16x16DC Luma_Add----------------------*/
			if (IS_I16MB (currMB)) // read DC coeffs for new intra modes       
			{
				macroblock_set_dc_pred(p_Vid, currMB->block_x, currMB->block_y);

				{              
					int context;
					dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);

					if( IS_INDEPENDENT(p_Vid) )
						context = LUMA_16DC; 
					else
						context = (uv==0) ? CB_16DC : CR_16DC;

					currMB->is_intra_block = 1;

					coef_ctr = -1;

					for(k=0;k<17;++k)
					{
						rl = readRunLevel_CABAC(currMB, &dP->de_cabac, context);

						if (rl.level != 0)                     // leave if level == 0
						{
							coef_ctr += rl.run + 1;
							currSlice->cof4[uv + 1][pos_scan4x4_1d[coef_ctr]][0][0] = rl.level;
						} 
						else
							break;
					} //k loop
				} // else CAVLC

				if(currMB->is_lossless == FALSE)
				{
					itrans_2(currMB, (ColorPlane) (uv + 1)); // transform new intra DC
				}
			} //IS_I16MB

			update_qp(currMB, p_Vid->qp);

			qp_per = p_Vid->qp_per_matrix[ (p_Vid->qp + p_Vid->bitdepth_luma_qp_scale) ];
			qp_rem = p_Vid->qp_rem_matrix[ (p_Vid->qp + p_Vid->bitdepth_luma_qp_scale) ];

			//init constants for every chroma qp offset
			qp_per_uv[uv] = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];
			qp_rem_uv[uv] = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];

			InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

			{  
				if (cbp)
				{
					if(currMB->luma_transform_size_8x8_flag) 
					{
						//======= 8x8 transform size & CABAC ========
						readCompCoeff8x8MB_CABAC(currMB, (ColorPlane) (PLANE_U + uv)); 
					}
					else //4x4
					{        
						readCompCoeff4x4MB_CABAC(currMB, (ColorPlane) (PLANE_U + uv), intra, InvLevelScale4x4,  qp_per_uv[uv], cbp);
					}
				}
			}
		} 
	} //444
	else  if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444))
	{
		//========================== CHROMA DC ============================
		//-----------------------------------------------------------------
		// chroma DC coeff
		if(cbp>15)
		{
			if (dec_picture->chroma_format_idc == YUV420)
			{    
				for (ll=0;ll<3;ll+=2)
				{
					uv = ll>>1;          

					InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];
					//===================== CHROMA DC YUV420 ======================
					memzero16(&currSlice->cofu[0]);
					coef_ctr=-1;

					{
						int type = (intra ? SE_CHR_DC_INTRA : SE_CHR_DC_INTER);

						currMB->is_intra_block =  intra;
						currMB->is_v_block     = ll;

						dP = &(currSlice->partArr[partMap[type]]);

						for(k = 0; k < (p_Vid->num_cdc_coeff + 1);++k)
						{
							rl = readRunLevel_CABAC(currMB, &(dP->de_cabac), CHROMA_DC);

							if (rl.level != 0)
							{
								currMB->cbp_blk[0] |= 0xf0000 << (ll<<1) ;
								coef_ctr += rl.run + 1;

								// Bug: currSlice->cofu has only 4 entries, hence coef_ctr MUST be <4 (which is
								// caught by the assert().  If it is bigger than 4, it starts patching the
								// p_Vid->predmode pointer, which leads to bugs later on.
								//
								// This assert() should be left in the code, because it captures a very likely
								// bug early when testing in error prone environments (or when testing NAL
								// functionality).
								assert (coef_ctr < p_Vid->num_cdc_coeff);
								currSlice->cofu[coef_ctr&3]=rl.level;
							}
							else
								break;
						}
					}

					if (smb || (currMB->is_lossless == TRUE)) // check to see if MB type is SPred or SIntra4x4
					{
						currSlice->cof4[uv + 1][0][0][0] = currSlice->cofu[0];
						currSlice->cof4[uv + 1][1][0][0] = currSlice->cofu[1];
						currSlice->cof4[uv + 1][2][0][0] = currSlice->cofu[2];
						currSlice->cof4[uv + 1][3][0][0] = currSlice->cofu[3];
					}
					else
					{
						ihadamard2x2(currSlice->cofu, temp);

						currSlice->cof4[uv + 1][0][0][0] = (((temp[0] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
						currSlice->cof4[uv + 1][1][0][0] = (((temp[1] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
						currSlice->cof4[uv + 1][2][0][0] = (((temp[2] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
						currSlice->cof4[uv + 1][3][0][0] = (((temp[3] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
					}          
				}
			}
			else if (dec_picture->chroma_format_idc == YUV422)
			{
				for (ll=0;ll<3;ll+=2)
				{
					int (*InvLevelScale4x4)[4] = NULL;
					uv = ll>>1;
					{
						h264_short_block_t *imgcof = currSlice->cof4[uv + 1];
						int m3[2][4] = {{0,0,0,0},{0,0,0,0}};
						int m4[2][4] = {{0,0,0,0},{0,0,0,0}};
						int qp_per_uv_dc = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
						int qp_rem_uv_dc = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
						if (intra)
							InvLevelScale4x4 = currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv_dc];
						else 
							InvLevelScale4x4 = currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv_dc];


						//===================== CHROMA DC YUV422 ======================
						{
							coef_ctr=-1;
							for(k=0;k<9;++k)
							{
								int type = (intra ? SE_CHR_DC_INTRA : SE_CHR_DC_INTER);
								currMB->is_intra_block =  intra;
								currMB->is_v_block     = ll;

								dP = &(currSlice->partArr[partMap[type]]);

								rl = readRunLevel_CABAC(currMB, &dP->de_cabac, CHROMA_DC_2x4);

								if (rl.level != 0)
								{
									currMB->cbp_blk[0] |= ((int64)0xff0000) << (ll<<2) ;
									coef_ctr += rl.run + 1;
									assert (coef_ctr < p_Vid->num_cdc_coeff);
									i0=SCAN_YUV422[coef_ctr][0];
									j0=SCAN_YUV422[coef_ctr][1];

									m3[i0][j0]=rl.level;
								}
								else
									break;
							}
						}
						// inverse CHROMA DC YUV422 transform
						// horizontal
						if(currMB->is_lossless == FALSE)
						{
							m4[0][0] = m3[0][0] + m3[1][0];
							m4[0][1] = m3[0][1] + m3[1][1];
							m4[0][2] = m3[0][2] + m3[1][2];
							m4[0][3] = m3[0][3] + m3[1][3];

							m4[1][0] = m3[0][0] - m3[1][0];
							m4[1][1] = m3[0][1] - m3[1][1];
							m4[1][2] = m3[0][2] - m3[1][2];
							m4[1][3] = m3[0][3] - m3[1][3];

							for (i = 0; i < 2; ++i)
							{
								m6[0] = m4[i][0] + m4[i][2];
								m6[1] = m4[i][0] - m4[i][2];
								m6[2] = m4[i][1] - m4[i][3];
								m6[3] = m4[i][1] + m4[i][3];

								imgcof[cof4_pos_to_subblock[0][i]][0][0] = m6[0] + m6[3];
								imgcof[cof4_pos_to_subblock[1][i]][0][0] = m6[1] + m6[2];
								imgcof[cof4_pos_to_subblock[2][i]][0][0] = m6[1] - m6[2];
								imgcof[cof4_pos_to_subblock[3][i]][0][0]= m6[0] - m6[3];
							}//for (i=0;i<2;++i)
						}
						else
						{
							for(j=0;j<4;++j)
							{
								for(i=0;i<2;++i)                
								{
									currSlice->cof4[uv + 1][cof4_pos_to_subblock[j][i]][0][0] = m3[i][j];
								}
							}
						}

						for(j = 0;j < p_Vid->mb_cr_size_y; j += BLOCK_SIZE)
						{
							for(i=0;i < p_Vid->mb_cr_size_x;i+=BLOCK_SIZE)
							{
								imgcof[cof4_pos_to_subblock[j>>2][i>>2]][0][0] = rshift_rnd_sf((imgcof[cof4_pos_to_subblock[j>>2][i>>2]][0][0] * InvLevelScale4x4[0][0]) << qp_per_uv_dc, 6);
							}
						}
					}
				}//for (ll=0;ll<3;ll+=2)
			}//else if (dec_picture->chroma_format_idc == YUV422)
		}

		//========================== CHROMA AC ============================
		//-----------------------------------------------------------------
		// chroma AC coeff, all zero fram start_scan
		if (cbp<=31)
		{
		}
		else
		{
			{
				int type;
				currMB->is_intra_block =  intra;
				type = (intra ? SE_CHR_AC_INTRA : SE_CHR_AC_INTER);

				dP = &(currSlice->partArr[partMap[type]]);


				if(currMB->is_lossless == FALSE)
				{ 
					for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
					{
						currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));
						InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

						for (b4 = 0; b4 < 4; ++b4)
						{
							int *scale = &InvLevelScale4x4[0][0];
							i = cofuv_blk_x[yuv][b8][b4];
							j = cofuv_blk_y[yuv][b8][b4];

							currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
							currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

							pos_scan_4x4 = &pos_scan4x4_1d[1];
							for(k = 0; k < 16;++k)
							{
								rl = readRunLevel_CABAC(currMB, &(dP->de_cabac), CHROMA_AC);

								if (rl.level != 0)
								{
									byte position;
									currMB->cbp_blk[0] |= ((int64)1) << cbp_blk_chroma[b8][b4];
									pos_scan_4x4 += rl.run;
									position = *pos_scan_4x4++;

									((int16_t *)currSlice->cof4[uv + 1][cof4_pos_to_subblock[j][i]])[position] = rshift_rnd_sf((rl.level * scale[position])<<qp_per_uv[uv], 4);
								}
								else
									break;
							} //for(k=0;(k<16)&&(level!=0);++k)
						}
					}
				}
				else
				{
					for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
					{
						currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));

						for (b4=0; b4 < 4; ++b4)
						{
							i = cofuv_blk_x[yuv][b8][b4];
							j = cofuv_blk_y[yuv][b8][b4];

							pos_scan_4x4 = &pos_scan4x4_1d[1];

							currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
							currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

							for(k=0;k<16;++k)
							{
								rl = readRunLevel_CABAC(currMB, &dP->de_cabac, CHROMA_AC);

								if (rl.level != 0)
								{
									currMB->cbp_blk[0] |= ((int64)1) << cbp_blk_chroma[b8][b4];
									pos_scan_4x4 += rl.run;

									((int16_t *)currSlice->cof4[uv + 1][cof4_pos_to_subblock[j][i]])[*pos_scan_4x4++] = rl.level;
								}
								else
									break;
							} 
						}
					} 
				} //for (b4=0; b4 < 4; b4++)
			} //for (b8=0; b8 < p_Vid->num_blk8x8_uv; b8++)
		} //if (dec_picture->chroma_format_idc != YUV400)
	}
}

/*!
************************************************************************
* \brief
*    Get coded block pattern and coefficients (run/level)
*    from the NAL
************************************************************************
*/
static void read_CBP_and_coeffs_from_NAL_CAVLC(Macroblock *currMB)
{
	int i,j,k;
	int level;
	int mb_nr = currMB->mbAddrX;
	int cbp;
	SyntaxElement currSE;
	DataPartition *dP = NULL;
	Slice *currSlice = currMB->p_Slice;
	const byte *partMap = assignSE2partition[currSlice->dp_mode];
	int coef_ctr, i0, j0, b8;
	int ll;
	__declspec(align(32)) int levarr[16], runarr[16];
	int numcoeff;

	int qp_per, qp_rem;
	VideoParameters *p_Vid = currMB->p_Vid;
	int smb = ((p_Vid->type==SP_SLICE) && IS_INTER (currMB)) || (p_Vid->type == SI_SLICE && currMB->mb_type == SI4MB);

	int uv; 
	int qp_per_uv[2];
	int qp_rem_uv[2];

	int intra = IS_INTRA (currMB);
	int temp[4];

	int b4;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	int yuv = dec_picture->chroma_format_idc - 1;
	int m6[4];

	int need_transform_size_flag;

	int (*InvLevelScale4x4)[4] = NULL;
	const int *InvLevelScale8x8 = NULL;
	// select scan type
	const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
	const byte *pos_scan_4x4 = pos_scan4x4[0];

	// QPI
	//init constants for every chroma qp offset
	if (dec_picture->chroma_format_idc != YUV400)
	{
		for (i=0; i<2; ++i)
		{
			qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
			qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
		}
	}

	// read CBP if not new intra mode
	if (!IS_I16MB (currMB))
	{
		//=====   C B P   =====
		//---------------------
		int type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
			? SE_CBP_INTRA
			: SE_CBP_INTER;

		dP = &(currSlice->partArr[partMap[type]]);

		currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
			? currSlice->linfo_cbp_intra
			: currSlice->linfo_cbp_inter;

		TRACE_STRING("coded_block_pattern");
		readSyntaxElement_UVLC(&currSE, dP);
		currMB->cbp = cbp = currSE.value1;


		//============= Transform size flag for INTER MBs =============
		//-------------------------------------------------------------
		need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
			(IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
			(currMB->NoMbPartLessThan8x8Flag))
			&& currMB->mb_type != I8MB && currMB->mb_type != I4MB
			&& (currMB->cbp&15)
			&& p_Vid->Transform8x8Mode);

		if (need_transform_size_flag)
		{
			dP = &(currSlice->partArr[partMap[SE_HEADER]]);
			TRACE_STRING("transform_size_8x8_flag");

			// read CAVLC transform_size_8x8_flag
			currMB->luma_transform_size_8x8_flag = (Boolean) readSyntaxElement_FLC(dP->bitstream, 1);
		}

		//=====   DQUANT   =====
		//----------------------
		// Delta quant only if nonzero coeffs
		if (cbp !=0)
		{
			read_delta_quant_CAVLC(&currSE, dP, currMB, partMap, (IS_INTER (currMB)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

			if (currSlice->dp_mode)
			{
				if (IS_INTER (currMB) && currSlice->dpC_NotPresent ) 
					currMB->dpl_flag = 1;

				if( intra && currSlice->dpB_NotPresent )
				{
					currMB->ei_flag = 1;
					currMB->dpl_flag = 1;
				}

				// check for prediction from neighbours
				check_dp_neighbors (currMB);
				if (currMB->dpl_flag)
				{
					cbp = 0; 
					currMB->cbp = cbp;
				}
			}
		}
	}
	else
	{
		cbp = currMB->cbp;
	}

	if (IS_I16MB (currMB)) // read DC coeffs for new intra modes
	{
		read_delta_quant_CAVLC(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

		macroblock_set_dc_pred(p_Vid, currMB->block_x, currMB->block_y);

		if (currSlice->dp_mode)
		{  
			if (currSlice->dpB_NotPresent)
			{
				currMB->ei_flag  = 1;
				currMB->dpl_flag = 1;
			}
			check_dp_neighbors (currMB);
			if (currMB->dpl_flag)
			{
				currMB->cbp = cbp = 0; 
			}
		}

		if (!currMB->dpl_flag)
		{
			pos_scan_4x4 = pos_scan4x4[0];

			readCoeff4x4_CAVLC(currMB, LUMA_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);

			for(k = 0; k < numcoeff; ++k)
			{
				if (levarr[k] != 0)                     // leave if level == 0
				{
					pos_scan_4x4 += 2 * runarr[k];

					i0 = (*pos_scan_4x4++);
					j0 = (*pos_scan_4x4++);

					currSlice->cof4[0][cof4_pos_to_subblock[j0][i0]][0][0] = levarr[k];// add new intra DC coeff
				}
			}


			if(currMB->is_lossless == FALSE)
				itrans_2(currMB, (ColorPlane) p_Vid->colour_plane_id);// transform new intra DC
		}
	}

	update_qp(currMB, p_Vid->qp);

	qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[p_Vid->colour_plane_id] ];
	qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[p_Vid->colour_plane_id] ];

	//init quant parameters for chroma 
	if (dec_picture->chroma_format_idc != YUV400)
	{
		for(i=0; i < 2; ++i)
		{
			qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
			qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
		}
	}

	InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[p_Vid->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[p_Vid->colour_plane_id][qp_rem];
	InvLevelScale8x8 = intra? currSlice->InvLevelScale8x8_Intra[p_Vid->colour_plane_id][qp_rem] : currSlice->InvLevelScale8x8_Inter[p_Vid->colour_plane_id][qp_rem];

	// luma coefficients
	if (cbp)
	{
		if (!currMB->luma_transform_size_8x8_flag) // 4x4 transform
		{
			readCompCoeff4x4MB_CAVLC(currMB, PLANE_Y, InvLevelScale4x4, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
		}
		else // 8x8 transform
		{
			readCompCoeff8x8MB_CAVLC(currMB, PLANE_Y, InvLevelScale8x8, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
		}
	}
	else
	{
		memset(&p_Vid->nz_coeff[mb_nr][0][0][0], 0, BLOCK_SIZE * BLOCK_SIZE * sizeof(byte));
	}

	if ( p_Vid->active_sps->chroma_format_idc==YUV444 && !IS_INDEPENDENT(p_Vid) ) 
	{
		for (uv = 0; uv < 2; ++uv )
		{
			/*----------------------16x16DC Luma_Add----------------------*/
			if (IS_I16MB (currMB)) // read DC coeffs for new intra modes       
			{
				macroblock_set_dc_pred(p_Vid, currMB->block_x, currMB->block_y);

				if (uv == 0)
					readCoeff4x4_CAVLC(currMB, CB_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);
				else
					readCoeff4x4_CAVLC(currMB, CR_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);

				coef_ctr=-1;
				level = 1;                            // just to get inside the loop

				for(k = 0; k < numcoeff; ++k)
				{
					if (levarr[k] != 0)                     // leave if level == 0
					{
						coef_ctr += runarr[k] + 1;

						i0 = pos_scan4x4[coef_ctr][0];
						j0 = pos_scan4x4[coef_ctr][1];
						currSlice->cof4[uv + 1][cof4_pos_to_subblock[j0][i0]][0][0] = levarr[k];// add new intra DC coeff
					} //if leavarr[k]
				} //k loop

				if(currMB->is_lossless == FALSE)
				{
					itrans_2(currMB, (ColorPlane) (uv + 1)); // transform new intra DC
				}
			} //IS_I16MB

			update_qp(currMB, p_Vid->qp);

			qp_per = p_Vid->qp_per_matrix[ (p_Vid->qp + p_Vid->bitdepth_luma_qp_scale) ];
			qp_rem = p_Vid->qp_rem_matrix[ (p_Vid->qp + p_Vid->bitdepth_luma_qp_scale) ];

			//init constants for every chroma qp offset
			qp_per_uv[uv] = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];
			qp_rem_uv[uv] = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];

			InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];
			InvLevelScale8x8 = intra? currSlice->InvLevelScale8x8_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale8x8_Inter[uv + 1][qp_rem_uv[uv]];

			if (!currMB->luma_transform_size_8x8_flag) // 4x4 transform
			{
				readCompCoeff4x4MB_CAVLC(currMB, (ColorPlane) (PLANE_U + uv), InvLevelScale4x4, qp_per_uv[uv], cbp, p_Vid->nz_coeff[mb_nr][PLANE_U + uv]);
			}
			else // 8x8 transform
			{
				readCompCoeff8x8MB_CAVLC(currMB, (ColorPlane) (PLANE_U + uv), InvLevelScale8x8, qp_per_uv[uv], cbp, p_Vid->nz_coeff[mb_nr][PLANE_U + uv]);
			}   
		} 
	} //444
	else  if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444))
	{
		//========================== CHROMA DC ============================
		//-----------------------------------------------------------------
		// chroma DC coeff
		if(cbp>15)
		{
			if (dec_picture->chroma_format_idc == YUV420)
			{    
				for (ll=0;ll<3;ll+=2)
				{
					uv = ll>>1;          

					InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];
					//===================== CHROMA DC YUV420 ======================
					memset(&currSlice->cofu[0], 0, 4 *sizeof(int));
					coef_ctr=-1;

					readCoeff4x4_CAVLC(currMB, CHROMA_DC, 0, 0, levarr, runarr, &numcoeff);

					for(k = 0; k < numcoeff; ++k)
					{
						if (levarr[k] != 0)
						{
							currMB->cbp_blk[0] |= 0xf0000 << (ll<<1) ;
							coef_ctr += runarr[k] + 1;
							currSlice->cofu[coef_ctr]=levarr[k];
						}
					}

					if (smb || (currMB->is_lossless == TRUE)) // check to see if MB type is SPred or SIntra4x4
					{
						currSlice->cof4[uv + 1][0][0][0] = currSlice->cofu[0];
						currSlice->cof4[uv + 1][1][0][0] = currSlice->cofu[1];
						currSlice->cof4[uv + 1][2][0][0] = currSlice->cofu[2];
						currSlice->cof4[uv + 1][3][0][0] = currSlice->cofu[3];
					}
					else
					{
						ihadamard2x2(currSlice->cofu, temp);

						currSlice->cof4[uv + 1][0][0][0] = (((temp[0] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
						currSlice->cof4[uv + 1][1][0][0] = (((temp[1] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
						currSlice->cof4[uv + 1][2][0][0] = (((temp[2] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
						currSlice->cof4[uv + 1][3][0][0] = (((temp[3] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
					}          
				}
			}
			else if (dec_picture->chroma_format_idc == YUV422)
			{
				for (ll=0;ll<3;ll+=2)
				{
					int (*InvLevelScale4x4)[4] = NULL;
					uv = ll>>1;
					{
						h264_short_block_t *imgcof = currSlice->cof4[uv + 1];
						int m3[2][4] = {{0,0,0,0},{0,0,0,0}};
						int m4[2][4] = {{0,0,0,0},{0,0,0,0}};
						int qp_per_uv_dc = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
						int qp_rem_uv_dc = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
						if (intra)
							InvLevelScale4x4 = currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv_dc];
						else 
							InvLevelScale4x4 = currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv_dc];


						//===================== CHROMA DC YUV422 ======================
						readCoeff4x4_CAVLC(currMB, CHROMA_DC, 0, 0, levarr, runarr, &numcoeff);
						coef_ctr=-1;
						level=1;
						for(k = 0; k < numcoeff; ++k)
						{
							if (levarr[k] != 0)
							{
								currMB->cbp_blk[0] |= ((int64)0xff0000) << (ll<<2);
								coef_ctr += runarr[k]+1;
								i0 = SCAN_YUV422[coef_ctr][0];
								j0 = SCAN_YUV422[coef_ctr][1];

								m3[i0][j0]=levarr[k];
							}
						}

						// inverse CHROMA DC YUV422 transform
						// horizontal
						if(currMB->is_lossless == FALSE)
						{
							m4[0][0] = m3[0][0] + m3[1][0];
							m4[0][1] = m3[0][1] + m3[1][1];
							m4[0][2] = m3[0][2] + m3[1][2];
							m4[0][3] = m3[0][3] + m3[1][3];

							m4[1][0] = m3[0][0] - m3[1][0];
							m4[1][1] = m3[0][1] - m3[1][1];
							m4[1][2] = m3[0][2] - m3[1][2];
							m4[1][3] = m3[0][3] - m3[1][3];

							for (i = 0; i < 2; ++i)
							{
								m6[0] = m4[i][0] + m4[i][2];
								m6[1] = m4[i][0] - m4[i][2];
								m6[2] = m4[i][1] - m4[i][3];
								m6[3] = m4[i][1] + m4[i][3];

								imgcof[cof4_pos_to_subblock[0][i]][0][0] = m6[0] + m6[3];
								imgcof[cof4_pos_to_subblock[1][i]][0][0] = m6[1] + m6[2];
								imgcof[cof4_pos_to_subblock[2][i]][0][0] = m6[1] - m6[2];
								imgcof[cof4_pos_to_subblock[3][i]][0][0] = m6[0] - m6[3];
							}//for (i=0;i<2;++i)
						}
						else
						{
							currSlice->cof4[uv + 1][0][0][0] = m3[0][0];
							currSlice->cof4[uv + 1][1][0][0] = m3[1][0];
							currSlice->cof4[uv + 1][2][0][0] = m3[0][1];
							currSlice->cof4[uv + 1][3][0][0] = m3[1][1];
							currSlice->cof4[uv + 1][8][0][0] = m3[0][2];
							currSlice->cof4[uv + 1][9][0][0] = m3[1][2];
							currSlice->cof4[uv + 1][10][0][0] = m3[0][3];
							currSlice->cof4[uv + 1][11][0][0] = m3[1][3];
						}

						for(j = 0;j < 16; j += BLOCK_SIZE)
						{
							for(i=0;i < 8;i+=BLOCK_SIZE)
							{
								imgcof[cof4_pos_to_subblock[j>>2][i>>2]][0][0] = rshift_rnd_sf((imgcof[cof4_pos_to_subblock[j>>2][i>>2]][0][0] * InvLevelScale4x4[0][0]) << qp_per_uv_dc, 6);
							}
						}
					}
				}//for (ll=0;ll<3;ll+=2)
			}//else if (dec_picture->chroma_format_idc == YUV422)
		}

		//========================== CHROMA AC ============================
		//-----------------------------------------------------------------
		// chroma AC coeff, all zero fram start_scan
		if (cbp<=31)
		{
			memset(&p_Vid->nz_coeff [mb_nr ][1][0][0], 0, 2 * BLOCK_SIZE * BLOCK_SIZE * sizeof(byte));
		}
		else
		{
			if(currMB->is_lossless == FALSE)
			{
				for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
				{
					currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));
					InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

					for (b4=0; b4 < 4; ++b4)
					{
						i = cofuv_blk_x[yuv][b8][b4];
						j = cofuv_blk_y[yuv][b8][b4];

						readCoeff4x4_CAVLC(currMB, CHROMA_AC, i + 2*uv, j + 4, levarr, runarr, &numcoeff);
						coef_ctr = 0;

						for(k = 0; k < numcoeff;++k)
						{
							if (levarr[k] != 0)
							{
								currMB->cbp_blk[0] |= ((int64)1) << cbp_blk_chroma[b8][b4];
								coef_ctr += runarr[k] + 1;

								i0=pos_scan4x4[coef_ctr][0];
								j0=pos_scan4x4[coef_ctr][1];

								currSlice->cof4[uv + 1][cof4_pos_to_subblock[j][i]][j0][i0] = rshift_rnd_sf((levarr[k] * InvLevelScale4x4[j0][i0])<<qp_per_uv[uv], 4);
							}
						}
					}
				}        
			}
			else
			{
				int type;
				currMB->is_intra_block =  IS_INTRA(currMB);
				type =  (currMB->is_intra_block ? SE_CHR_AC_INTRA : SE_CHR_AC_INTER);

				dP = &(currSlice->partArr[partMap[type]]);
				currSE.mapping = linfo_levrun_inter;

				if(currMB->is_lossless == FALSE)
				{          
					for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
					{
						currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));
						InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

						for (b4 = 0; b4 < 4; ++b4)
						{
							i = cofuv_blk_x[yuv][b8][b4];
							j = cofuv_blk_y[yuv][b8][b4];

							currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
							currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

							pos_scan_4x4 = pos_scan4x4[1];

							for(k = 0; k < 16;k++)
							{
								readSyntaxElement_UVLC(&currSE, dP);
								level = currSE.value1;

								if (level != 0)
								{
									currMB->cbp_blk[0] |= ((int64)1) << cbp_blk_chroma[b8][b4];
									pos_scan_4x4 += (currSE.value2 << 1);

									i0 = *pos_scan_4x4++;
									j0 = *pos_scan_4x4++;

									currSlice->cof4[uv + 1][cof4_pos_to_subblock[j][i]][j0][i0] = rshift_rnd_sf((level * InvLevelScale4x4[j0][i0])<<qp_per_uv[uv], 4);
								}
								else
									break;
							} //for(k=0;(k<16)&&(level!=0);++k)
						}
					}
				}
				else
				{
					for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
					{
						currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));

						for (b4=0; b4 < 4; ++b4)
						{
							i = cofuv_blk_x[yuv][b8][b4];
							j = cofuv_blk_y[yuv][b8][b4];

							pos_scan_4x4 = pos_scan4x4[1];

							currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
							currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

							for(k=0;k<16;++k)
							{
								readSyntaxElement_UVLC(&currSE, dP);
								level = currSE.value1;

								if (level != 0)
								{
									currMB->cbp_blk[0] |= ((int64)1) << cbp_blk_chroma[b8][b4];
									pos_scan_4x4 += (currSE.value2 << 1);

									i0 = *pos_scan_4x4++;
									j0 = *pos_scan_4x4++;

									currSlice->cof4[uv + 1][cof4_pos_to_subblock[j][i]][j0][i0] = level;
								}
								else
									break;
							} 
						}
					} 
				} //for (b4=0; b4 < 4; b4++)
			} //for (b8=0; b8 < p_Vid->num_blk8x8_uv; b8++)
		} //if (dec_picture->chroma_format_idc != YUV400)
	}
}


/*!
************************************************************************
* \brief
*    decode one color component in an I slice
************************************************************************
*/

static int decode_one_component_i_slice(Macroblock *currMB, ColorPlane curr_plane, VideoImage *image, StorablePicture *dec_picture)
{
	//For residual DPCM
	currMB->ipmode_DPCM = NO_INTRA_PMODE; 
	if(currMB->mb_type == IPCM)
		mb_pred_ipcm(currMB);
	else if (IS_I16MB (currMB)) // get prediction for INTRA_MB_16x16
		mb_pred_intra16x16(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I4MB)
		mb_pred_intra4x4(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I8MB) 
		mb_pred_intra8x8(currMB, curr_plane, image, dec_picture);

	return 1;
}

/*!
************************************************************************
* \brief
*    decode one color component for a p slice
************************************************************************
*/
static int decode_one_component_p_slice(Macroblock *currMB, ColorPlane curr_plane, VideoImage *image, StorablePicture *dec_picture)
{
	//For residual DPCM
	currMB->ipmode_DPCM = NO_INTRA_PMODE; 
	if(currMB->mb_type == IPCM)
		mb_pred_ipcm(currMB);
	else if (IS_I16MB (currMB)) // get prediction for INTRA_MB_16x16
		mb_pred_intra16x16(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I4MB)
		mb_pred_intra4x4(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I8MB) 
		mb_pred_intra8x8(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == PSKIP)
		mb_pred_skip(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == P16x16)
		mb_pred_p_inter16x16(currMB, curr_plane, image, dec_picture);  
	else if (currMB->mb_type == P16x8)
		mb_pred_p_inter16x8(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == P8x16)
		mb_pred_p_inter8x16(currMB, curr_plane, image, dec_picture);
	else
		mb_pred_p_inter8x8(currMB, curr_plane, image, dec_picture);

	return 1;
}


/*!
************************************************************************
* \brief
*    decode one color component for a sp slice
************************************************************************
*/
static int decode_one_component_sp_slice(Macroblock *currMB, ColorPlane curr_plane, VideoImage *image, StorablePicture *dec_picture)
{
	//For residual DPCM
	currMB->ipmode_DPCM = NO_INTRA_PMODE; 

	if(currMB->mb_type == IPCM)
		mb_pred_ipcm(currMB);
	else if (IS_I16MB (currMB)) // get prediction for INTRA_MB_16x16
		mb_pred_intra16x16(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I4MB)
		mb_pred_intra4x4(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I8MB) 
		mb_pred_intra8x8(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == PSKIP)
		mb_pred_sp_skip(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == P16x16)
		mb_pred_p_inter16x16(currMB, curr_plane, image, dec_picture);  
	else if (currMB->mb_type == P16x8)
		mb_pred_p_inter16x8(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == P8x16)
		mb_pred_p_inter8x16(currMB, curr_plane, image, dec_picture);
	else
		mb_pred_p_inter8x8(currMB, curr_plane, image, dec_picture);

	return 1;
}

static void set_chroma_vector(Macroblock *currMB, int *list_offset)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;

	if (!currSlice->mb_aff_frame_flag)
	{
		if(p_Vid->structure == TOP_FIELD)
		{
			int k,l;  
			for (l = LIST_0; l <= (LIST_1); l++)
			{
				for(k = 0; k < p_Vid->listXsize[l]; k++)
				{
					if(p_Vid->structure != p_Vid->listX[l][k]->structure)
						p_Vid->listX[l][k]->chroma_vector_adjustment = -2;
					else
						p_Vid->listX[l][k]->chroma_vector_adjustment= 0;
				}
			}
		}
		else if(p_Vid->structure == BOTTOM_FIELD)
		{
			int k,l;  
			for (l = LIST_0; l <= (LIST_1); l++)
			{
				for(k = 0; k < p_Vid->listXsize[l]; k++)
				{
					if (p_Vid->structure != p_Vid->listX[l][k]->structure)
						p_Vid->listX[l][k]->chroma_vector_adjustment = 2;
					else
						p_Vid->listX[l][k]->chroma_vector_adjustment= 0;
				}
			}
		}
		else
		{
			int k,l;  
			for (l = LIST_0; l <= (LIST_1); l++)
			{
				for(k = 0; k < p_Vid->listXsize[l]; k++)
				{
					p_Vid->listX[l][k]->chroma_vector_adjustment= 0;
				}
			}
		}
	}
	else
	{
		int mb_nr = (currMB->mbAddrX & 0x01);
		int k,l;  

		//////////////////////////
		// find out the correct list offsets
		if (currMB->mb_field)
		{
			*list_offset = mb_nr ? 4 : 2;

			for (l = LIST_0 + *list_offset; l <= (LIST_1 + *list_offset); l++)
			{
				for(k = 0; k < p_Vid->listXsize[l]; k++)
				{          
					if(mb_nr == 0 && p_Vid->listX[l][k]->structure == BOTTOM_FIELD)
						p_Vid->listX[l][k]->chroma_vector_adjustment = -2;
					else if(mb_nr == 1 && p_Vid->listX[l][k]->structure == TOP_FIELD)
						p_Vid->listX[l][k]->chroma_vector_adjustment = 2;
					else
						p_Vid->listX[l][k]->chroma_vector_adjustment= 0;
				}
			}
		}
		else
		{
			for (l = LIST_0; l <= (LIST_1); l++)
			{
				for(k = 0; k < p_Vid->listXsize[l]; k++)
				{
					p_Vid->listX[l][k]->chroma_vector_adjustment= 0;
				}
			}
		}
	}

	p_Vid->max_mb_vmv_r = (p_Vid->structure != FRAME || (currSlice->mb_aff_frame_flag && currMB->mb_field)) ? p_Vid->max_vmv_r >> 1 : p_Vid->max_vmv_r;
}


static void mb_pred_b_dspatial(Macroblock *currMB, ColorPlane curr_plane, VideoImage *image, StorablePicture *dec_picture)
{
	char l0_rFrame = -1, l1_rFrame = -1;
	PicMotionParams *motion = &dec_picture->motion;
	MotionVector pmvl0={0,0}, pmvl1={0,0};
	int k;
	int block8x8;
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	int curr_mb_field = ((currSlice->mb_aff_frame_flag)&&(currMB->mb_field));

	MotionParams *colocated = &currSlice->p_colocated->frame;
	int list_offset = 0;
	int pred_dir = 0;

	Boolean has_zero_partitions = FALSE;
	h264_ref_t *ref_pic_num_l0, *ref_pic_num_l1;

	set_chroma_vector(currMB, &list_offset);

	if (currMB->mb_field)
	{
		if(currMB->mbAddrX & 0x01)
		{
			colocated = &currSlice->p_colocated->bottom;
		}
		else
		{
			colocated = &currSlice->p_colocated->top;
		}
	}

	prepare_direct_params(currMB, dec_picture, pmvl0, pmvl1, &l0_rFrame, &l1_rFrame);

	ref_pic_num_l0 = dec_picture->ref_pic_num[p_Vid->current_slice_nr][LIST_0 + list_offset];
	ref_pic_num_l1 = dec_picture->ref_pic_num[p_Vid->current_slice_nr][LIST_1 + list_offset];

	if (p_Vid->active_sps->direct_8x8_inference_flag)
	{
		if (l0_rFrame >=0 && l1_rFrame >=0)
		{
			PicMotion **motion0 = &motion->motion[LIST_0][currMB->block_y];
			PicMotion **motion1 = &motion->motion[LIST_1][currMB->block_y];
			int block_x = currMB->block_x;
			has_zero_partitions = TRUE;
			pred_dir = 2;
			if (p_Vid->listX[LIST_1 + list_offset][0]->is_long_term)
			{ // long term
				//---
				memcpy(motion0[0][block_x + 0].mv, pmvl0, sizeof(MotionVector));
				motion0[0][block_x + 0].ref_idx    = l0_rFrame;
				memcpy(motion1[0][block_x + 0].mv, pmvl1, sizeof(MotionVector));
				motion1[0][block_x + 0].ref_idx    = l1_rFrame;
				motion0[0][block_x + 0].ref_pic_id = ref_pic_num_l0[(short)motion0[0][block_x + 0].ref_idx];
				motion1[0][block_x + 0].ref_pic_id = ref_pic_num_l1[(short)motion1[0][block_x + 0].ref_idx];
				memcpy(motion0[0][block_x + 1].mv, pmvl0, sizeof(MotionVector));
				motion0[0][block_x + 1].ref_idx    = l0_rFrame;
				memcpy(motion1[0][block_x + 1].mv, pmvl1, sizeof(MotionVector));
				motion1[0][block_x + 1].ref_idx    = l1_rFrame;
				motion0[0][block_x + 1].ref_pic_id = ref_pic_num_l0[(short)motion0[0][block_x + 1].ref_idx];
				motion1[0][block_x + 1].ref_pic_id = ref_pic_num_l1[(short)motion1[0][block_x + 1].ref_idx];
				memcpy(motion0[1][block_x + 0].mv, pmvl0, sizeof(MotionVector));
				motion0[1][block_x + 0].ref_idx    = l0_rFrame;
				memcpy(motion1[1][block_x + 0].mv, pmvl1, sizeof(MotionVector));
				motion1[1][block_x + 0].ref_idx    = l1_rFrame;
				motion0[1][block_x + 0].ref_pic_id = ref_pic_num_l0[(short)motion0[1][block_x + 0].ref_idx];
				motion1[1][block_x + 0].ref_pic_id = ref_pic_num_l1[(short)motion1[1][block_x + 0].ref_idx];
				memcpy(motion0[1][block_x + 1].mv, pmvl0, sizeof(MotionVector));
				motion0[1][block_x + 1].ref_idx    = l0_rFrame;
				memcpy(motion1[1][block_x + 1].mv, pmvl1, sizeof(MotionVector));
				motion1[1][block_x + 1].ref_idx    = l1_rFrame;
				motion0[1][block_x + 1].ref_pic_id = ref_pic_num_l0[(short)motion0[1][block_x + 1].ref_idx];
				motion1[1][block_x + 1].ref_pic_id = ref_pic_num_l1[(short)motion1[1][block_x + 1].ref_idx];
				perform_mc8x8(currMB, curr_plane, dec_picture, pred_dir, 0, 0, list_offset, curr_mb_field);
				//---
				memcpy(motion0[0][block_x + 2].mv, pmvl0, sizeof(MotionVector));
				motion0[0][block_x + 2].ref_idx    = l0_rFrame;
				memcpy(motion1[0][block_x + 2].mv, pmvl1, sizeof(MotionVector));
				motion1[0][block_x + 2].ref_idx    = l1_rFrame;
				motion0[0][block_x + 2].ref_pic_id = ref_pic_num_l0[(short)motion0[0][block_x + 2].ref_idx];
				motion1[0][block_x + 2].ref_pic_id = ref_pic_num_l1[(short)motion1[0][block_x + 2].ref_idx];
				memcpy(motion0[0][block_x + 3].mv, pmvl0, sizeof(MotionVector));
				motion0[0][block_x + 3].ref_idx    = l0_rFrame;
				memcpy(motion1[0][block_x + 3].mv, pmvl1, sizeof(MotionVector));
				motion1[0][block_x + 3].ref_idx    = l1_rFrame;
				motion0[0][block_x + 3].ref_pic_id = ref_pic_num_l0[(short)motion0[0][block_x + 3].ref_idx];
				motion1[0][block_x + 3].ref_pic_id = ref_pic_num_l1[(short)motion1[0][block_x + 3].ref_idx];
				memcpy(motion0[1][block_x + 2].mv, pmvl0, sizeof(MotionVector));
				motion0[1][block_x + 2].ref_idx    = l0_rFrame;
				memcpy(motion1[1][block_x + 2].mv, pmvl1, sizeof(MotionVector));
				motion1[1][block_x + 2].ref_idx    = l1_rFrame;
				motion0[1][block_x + 2].ref_pic_id = ref_pic_num_l0[(short)motion0[1][block_x + 2].ref_idx];
				motion1[1][block_x + 2].ref_pic_id = ref_pic_num_l1[(short)motion1[1][block_x + 2].ref_idx];
				memcpy(motion0[1][block_x + 3].mv, pmvl0, sizeof(MotionVector));
				motion0[1][block_x + 3].ref_idx    = l0_rFrame;
				memcpy(motion1[1][block_x + 3].mv, pmvl1, sizeof(MotionVector));
				motion1[1][block_x + 3].ref_idx    = l1_rFrame;
				motion0[1][block_x + 3].ref_pic_id = ref_pic_num_l0[(short)motion0[1][block_x + 3].ref_idx];
				motion1[1][block_x + 3].ref_pic_id = ref_pic_num_l1[(short)motion1[1][block_x + 3].ref_idx];
				perform_mc8x8(currMB, curr_plane, dec_picture, pred_dir, 2, 0, list_offset, curr_mb_field);
				//---
				memcpy(motion0[2][block_x + 0].mv, pmvl0, sizeof(MotionVector));
				motion0[2][block_x + 0].ref_idx    = l0_rFrame;
				memcpy(motion1[2][block_x + 0].mv, pmvl1, sizeof(MotionVector));
				motion1[2][block_x + 0].ref_idx    = l1_rFrame;
				motion0[2][block_x + 0].ref_pic_id = ref_pic_num_l0[(short)motion0[2][block_x + 0].ref_idx];
				motion1[2][block_x + 0].ref_pic_id = ref_pic_num_l1[(short)motion1[2][block_x + 0].ref_idx];
				memcpy(motion0[2][block_x + 1].mv, pmvl0, sizeof(MotionVector));
				motion0[2][block_x + 1].ref_idx    = l0_rFrame;
				memcpy(motion1[2][block_x + 1].mv, pmvl1, sizeof(MotionVector));
				motion1[2][block_x + 1].ref_idx    = l1_rFrame;
				motion0[2][block_x + 1].ref_pic_id = ref_pic_num_l0[(short)motion0[2][block_x + 1].ref_idx];
				motion1[2][block_x + 1].ref_pic_id = ref_pic_num_l1[(short)motion1[2][block_x + 1].ref_idx];
				memcpy(motion0[3][block_x + 0].mv, pmvl0, sizeof(MotionVector));
				motion0[3][block_x + 0].ref_idx    = l0_rFrame;
				memcpy(motion1[3][block_x + 0].mv, pmvl1, sizeof(MotionVector));
				motion1[3][block_x + 0].ref_idx    = l1_rFrame;
				motion0[3][block_x + 0].ref_pic_id = ref_pic_num_l0[(short)motion0[3][block_x + 0].ref_idx];
				motion1[3][block_x + 0].ref_pic_id = ref_pic_num_l1[(short)motion1[3][block_x + 0].ref_idx];
				memcpy(motion0[3][block_x + 1].mv, pmvl0, sizeof(MotionVector));
				motion0[3][block_x + 1].ref_idx    = l0_rFrame;
				memcpy(motion1[3][block_x + 1].mv, pmvl1, sizeof(MotionVector));
				motion1[3][block_x + 1].ref_idx    = l1_rFrame;
				motion0[3][block_x + 1].ref_pic_id = ref_pic_num_l0[(short)motion0[3][block_x + 1].ref_idx];
				motion1[3][block_x + 1].ref_pic_id = ref_pic_num_l1[(short)motion1[3][block_x + 1].ref_idx];
				perform_mc8x8(currMB, curr_plane, dec_picture, pred_dir, 0, 2, list_offset, curr_mb_field);
				//---
				memcpy(motion0[2][block_x + 2].mv, pmvl0, sizeof(MotionVector));
				motion0[2][block_x + 2].ref_idx    = l0_rFrame;
				memcpy(motion1[2][block_x + 2].mv, pmvl1, sizeof(MotionVector));
				motion1[2][block_x + 2].ref_idx    = l1_rFrame;
				motion0[2][block_x + 2].ref_pic_id = ref_pic_num_l0[(short)motion0[2][block_x + 2].ref_idx];
				motion1[2][block_x + 2].ref_pic_id = ref_pic_num_l1[(short)motion1[2][block_x + 2].ref_idx];
				memcpy(motion0[2][block_x + 3].mv, pmvl0, sizeof(MotionVector));
				motion0[2][block_x + 3].ref_idx    = l0_rFrame;
				memcpy(motion1[2][block_x + 3].mv, pmvl1, sizeof(MotionVector));
				motion1[2][block_x + 3].ref_idx    = l1_rFrame;
				motion0[2][block_x + 3].ref_pic_id = ref_pic_num_l0[(short)motion0[2][block_x + 3].ref_idx];
				motion1[2][block_x + 3].ref_pic_id = ref_pic_num_l1[(short)motion1[2][block_x + 3].ref_idx];
				memcpy(motion0[3][block_x + 2].mv, pmvl0, sizeof(MotionVector));
				motion0[3][block_x + 2].ref_idx    = l0_rFrame;
				memcpy(motion1[3][block_x + 2].mv, pmvl1, sizeof(MotionVector));
				motion1[3][block_x + 2].ref_idx    = l1_rFrame;
				motion0[3][block_x + 2].ref_pic_id = ref_pic_num_l0[(short)motion0[3][block_x + 2].ref_idx];
				motion1[3][block_x + 2].ref_pic_id = ref_pic_num_l1[(short)motion1[3][block_x + 2].ref_idx];
				memcpy(motion0[3][block_x + 3].mv, pmvl0, sizeof(MotionVector));
				motion0[3][block_x + 3].ref_idx    = l0_rFrame;
				memcpy(motion1[3][block_x + 3].mv, pmvl1, sizeof(MotionVector));
				motion1[3][block_x + 3].ref_idx    = l1_rFrame;
				motion0[3][block_x + 3].ref_pic_id = ref_pic_num_l0[(short)motion0[3][block_x + 3].ref_idx];
				motion1[3][block_x + 3].ref_pic_id = ref_pic_num_l1[(short)motion1[3][block_x + 3].ref_idx];
				perform_mc8x8(currMB, curr_plane, dec_picture, pred_dir, 2, 2, list_offset, curr_mb_field);
			}
			else
			{ // not long term
				const byte **colocated_moving_block = &colocated->moving_block[currMB->block_y_aff];
				for (block8x8 = 0; block8x8 < 4; block8x8++)
				{
					int k_start = (block8x8 << 2);
					for (k = k_start; k < k_start + BLOCK_MULTIPLE; k ++)
					{
						int i  =  (decode_block_scan[k] & 3);
						int j  = ((decode_block_scan[k] >> 2) & 3);
						int i4  = currMB->block_x + i;

						//===== DIRECT PREDICTION =====
						if (!l0_rFrame  && !colocated_moving_block[j][i4])
						{
							motion0[j][i4].mv[0] = 0;
							motion0[j][i4].mv[1] = 0;
							motion0[j][i4].ref_idx    = 0;
						}
						else
						{
							motion0[j][i4].mv[0] = pmvl0[0];
							motion0[j][i4].mv[1] = pmvl0[1];
							motion0[j][i4].ref_idx    = l0_rFrame;
						}

						if  (l1_rFrame == 0 && !colocated_moving_block[j][i4])
						{
							motion1[j][i4].mv[0] = 0;
							motion1[j][i4].mv[1] = 0;
							motion1[j][i4].ref_idx    = 0;
						}
						else
						{
							motion1[j][i4].mv[0] = pmvl1[0];
							motion1[j][i4].mv[1] = pmvl1[1];
							motion1[j][i4].ref_idx    = l1_rFrame;
						}

						motion0[j][i4].ref_pic_id = ref_pic_num_l0[(short)motion0[j][i4].ref_idx];
						motion1[j][i4].ref_pic_id = ref_pic_num_l1[(short)motion1[j][i4].ref_idx];
					}

					perform_mc8x8(currMB, curr_plane, dec_picture, pred_dir, (decode_block_scan[k_start] & 3), ((decode_block_scan[k_start] >> 2) & 3), list_offset, curr_mb_field);
				}
			}
		}
		else
		{
			for (block8x8 = 0; block8x8 < 4; block8x8++)
			{
				int k_start = (block8x8 << 2);
				for (k = k_start; k < k_start + BLOCK_MULTIPLE; k ++)
				{
					int i  =  (decode_block_scan[k] & 3);
					int j  = ((decode_block_scan[k] >> 2) & 3);
					int i4  = currMB->block_x + i;
					int j4  = currMB->block_y + j;
					int j6  = currMB->block_y_aff + j;

					//printf("%d %d\n", i, j);

					//===== DIRECT PREDICTION =====

					if (l0_rFrame >=0)
					{
						if (!l0_rFrame  && ((!colocated->moving_block[j6][i4]) && (!p_Vid->listX[LIST_1 + list_offset][0]->is_long_term)))
						{
							has_zero_partitions = TRUE;
							motion->motion[LIST_0][j4][i4].mv[0] = 0;
							motion->motion[LIST_0][j4][i4].mv[1] = 0;
							motion->motion[LIST_0][j4][i4].ref_idx    = 0;
						}
						else
						{
							has_zero_partitions = TRUE;
							motion->motion[LIST_0][j4][i4].mv[0] = pmvl0[0];
							motion->motion[LIST_0][j4][i4].mv[1] = pmvl0[1];
							motion->motion[LIST_0][j4][i4].ref_idx    = l0_rFrame;
						}
					}
					else
					{        
						motion->motion[LIST_0][j4][i4].mv[0] = 0;
						motion->motion[LIST_0][j4][i4].mv[1] = 0;
						motion->motion[LIST_0][j4][i4].ref_idx    = -1;
					}

					if (l1_rFrame >=0)
					{
						if  (l1_rFrame == 0 && ((!colocated->moving_block[j6][i4]) && (!p_Vid->listX[LIST_1 + list_offset][0]->is_long_term)))
						{
							has_zero_partitions = TRUE;
							motion->motion[LIST_1][j4][i4].mv[0] = 0;
							motion->motion[LIST_1][j4][i4].mv[1] = 0;
							motion->motion[LIST_1][j4][i4].ref_idx    = 0;
						}
						else
						{
							has_zero_partitions = TRUE;
							motion->motion[LIST_1][j4][i4].mv[0] = pmvl1[0];
							motion->motion[LIST_1][j4][i4].mv[1] = pmvl1[1];
							motion->motion[LIST_1][j4][i4].ref_idx    = l1_rFrame;
						}
					}
					else
					{
						motion->motion[LIST_1][j4][i4].mv[0] = 0;
						motion->motion[LIST_1][j4][i4].mv[1] = 0;
						motion->motion[LIST_1][j4][i4].ref_idx    = -1;
					}

					if (l1_rFrame == -1) 
						pred_dir = 0;
					else if (l0_rFrame == -1) 
						pred_dir = 1;
					else
						pred_dir = 2;

					if (l0_rFrame < 0 && l1_rFrame < 0)
					{
						motion->motion[LIST_0][j4][i4].ref_idx = 0;
						motion->motion[LIST_1][j4][i4].ref_idx = 0;
						pred_dir = 2;
					}

					motion->motion[LIST_0][j4][i4].ref_pic_id = ref_pic_num_l0[(short)motion->motion[LIST_0][j4][i4].ref_idx];
					motion->motion[LIST_1][j4][i4].ref_pic_id = ref_pic_num_l1[(short)motion->motion[LIST_1][j4][i4].ref_idx];
				}

				if (has_zero_partitions == TRUE)
				{
					int i =  (decode_block_scan[k_start] & 3);
					int j = ((decode_block_scan[k_start] >> 2) & 3);

					perform_mc8x8(currMB, curr_plane, dec_picture, pred_dir, i, j, list_offset, curr_mb_field);
				}
			}
		}
	}
	else
	{
		for (block8x8 = 0; block8x8 < 4; block8x8++)
		{
			int k_start = (block8x8 << 2);
			int k_end = k_start + BLOCK_MULTIPLE;

			for (k = k_start; k < k_end; k ++)
			{
				int i  =  (decode_block_scan[k] & 3);
				int j  = ((decode_block_scan[k] >> 2) & 3);
				int i4  = currMB->block_x + i;
				int j4  = currMB->block_y + j;
				int j6  = currMB->block_y_aff + j;

				//===== DIRECT PREDICTION =====

				if (l0_rFrame >=0)
				{
					if (!l0_rFrame  && ((!colocated->moving_block[j6][i4]) && (!p_Vid->listX[LIST_1 + list_offset][0]->is_long_term)))
					{
						has_zero_partitions = TRUE;
						motion->motion[LIST_0][j4][i4].mv[0] = 0;
						motion->motion[LIST_0][j4][i4].mv[1] = 0;
						motion->motion[LIST_0][j4][i4].ref_idx    = 0;
					}
					else
					{
						has_zero_partitions = TRUE;
						motion->motion[LIST_0][j4][i4].mv[0] = pmvl0[0];
						motion->motion[LIST_0][j4][i4].mv[1] = pmvl0[1];
						motion->motion[LIST_0][j4][i4].ref_idx    = l0_rFrame;
					}
				}
				else
				{        
					motion->motion[LIST_0][j4][i4].mv[0] = 0;
					motion->motion[LIST_0][j4][i4].mv[1] = 0;
					motion->motion[LIST_0][j4][i4].ref_idx    = -1;
				}

				if (l1_rFrame >=0)
				{
					if  (l1_rFrame == 0 && ((!colocated->moving_block[j6][i4]) && (!p_Vid->listX[LIST_1 + list_offset][0]->is_long_term)))
					{
						has_zero_partitions = TRUE;
						motion->motion[LIST_1][j4][i4].mv[0] = 0;
						motion->motion[LIST_1][j4][i4].mv[1] = 0;
						motion->motion[LIST_1][j4][i4].ref_idx    = 0;
					}
					else
					{
						has_zero_partitions = TRUE;
						motion->motion[LIST_1][j4][i4].mv[0] = pmvl1[0];
						motion->motion[LIST_1][j4][i4].mv[1] = pmvl1[1];
						motion->motion[LIST_1][j4][i4].ref_idx    = l1_rFrame;
					}
				}
				else
				{
					motion->motion[LIST_1][j4][i4].mv[0] = 0;
					motion->motion[LIST_1][j4][i4].mv[1] = 0;
					motion->motion[LIST_1][j4][i4].ref_idx    = -1;
				}

				if (l0_rFrame < 0 && l1_rFrame < 0)
				{
					motion->motion[LIST_0][j4][i4].ref_idx = 0;
					motion->motion[LIST_1][j4][i4].ref_idx = 0;
				}

				if (l1_rFrame == -1) 
				{
					if (l0_rFrame == -1) 
						pred_dir = 2;
					else
						pred_dir = 0;
				}
				else if (l0_rFrame == -1) 
				{
					pred_dir = 1;
				}
				else                                               
					pred_dir = 2;

				motion->motion[LIST_0][j4][i4].ref_pic_id = ref_pic_num_l0[(short)motion->motion[LIST_0][j4][i4].ref_idx];
				motion->motion[LIST_1][j4][i4].ref_pic_id = ref_pic_num_l1[(short)motion->motion[LIST_1][j4][i4].ref_idx];
			}

			if (has_zero_partitions == TRUE)
			{
				for (k = k_start; k < k_end; k ++)
				{        
					int i =  (decode_block_scan[k] & 3);
					int j = ((decode_block_scan[k] >> 2) & 3);

					perform_mc(currMB, curr_plane, dec_picture, pred_dir, i, j, list_offset, BLOCK_SIZE, BLOCK_SIZE, curr_mb_field);           
				}
			}
		}
	}

	if (has_zero_partitions == FALSE)
	{
		perform_mc16x16(currMB, curr_plane, dec_picture, pred_dir, list_offset, curr_mb_field);           
	}  

	if (currMB->cbp == 0)
	{
		opt_copy_image_data_16x16_stride(image, currMB->pix_x, currMB->pix_y, currSlice->mb_pred[curr_plane]);

		if (dec_picture->chroma_format_idc == YUV420)
		{
			copy_image_data_8x8_stride(dec_picture->imgUV[0], currMB->pix_c_x, currMB->pix_c_y, currSlice->mb_pred[1]);
			copy_image_data_8x8_stride(dec_picture->imgUV[1], currMB->pix_c_x, currMB->pix_c_y, currSlice->mb_pred[2]);
		}
		else if (dec_picture->chroma_format_idc == YUV422)
		{
			copy_image_data_stride(dec_picture->imgUV[0], currMB->pix_c_x, currMB->pix_c_y, currSlice->mb_pred[1], 8, 16);
			copy_image_data_stride(dec_picture->imgUV[1], currMB->pix_c_x, currMB->pix_c_y, currSlice->mb_pred[2], 8, 16);
		}
	}
	else
		iTransform(currMB, curr_plane, 0); 
}



/*!
************************************************************************
* \brief
*    decode one color component for a b slice
************************************************************************
*/

static int decode_one_component_b_slice(Macroblock *currMB, ColorPlane curr_plane, VideoImage *image, StorablePicture *dec_picture)
{  
	//For residual DPCM
	currMB->ipmode_DPCM = NO_INTRA_PMODE; 

	if(currMB->mb_type == IPCM)
		mb_pred_ipcm(currMB);
	else if (IS_I16MB (currMB)) // get prediction for INTRA_MB_16x16
		mb_pred_intra16x16(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I4MB)
		mb_pred_intra4x4(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == I8MB) 
		mb_pred_intra8x8(currMB, curr_plane, image, dec_picture);  
	else if (currMB->mb_type == P16x16)
		mb_pred_p_inter16x16(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == P16x8)
		mb_pred_p_inter16x8(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == P8x16)
		mb_pred_p_inter8x16(currMB, curr_plane, image, dec_picture);
	else if (currMB->mb_type == BSKIP_DIRECT)
	{
		if (currMB->p_Slice->direct_spatial_mv_pred_flag == 0)
			mb_pred_b_dtemporal (currMB, curr_plane, image, dec_picture);
		else
			mb_pred_b_dspatial (currMB, curr_plane, image, dec_picture);
	}
	else
		mb_pred_b_inter8x8 (currMB, curr_plane, image, dec_picture);

	return 1;
}

/*!
************************************************************************
* \brief
*    decode one macroblock
************************************************************************
*/

int decode_one_macroblock(Macroblock *currMB, StorablePicture *dec_picture)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;  

	// luma decoding **************************************************
	currSlice->decode_one_component(currMB, PLANE_Y, dec_picture->imgY, dec_picture);

	if ((p_Vid->active_sps->chroma_format_idc==YUV444)&&(!IS_INDEPENDENT(p_Vid)))  
	{
		currSlice->decode_one_component(currMB, PLANE_U, dec_picture->imgUV[0], dec_picture);
		currSlice->decode_one_component(currMB, PLANE_V, dec_picture->imgUV[1], dec_picture);
	}
	return 0;
}


/*!
************************************************************************
* \brief
*    change target plane
*    for 4:4:4 Independent mode
************************************************************************
*/
void change_plane_JV( VideoParameters *p_Vid, int nplane )
{
	Slice *currSlice = p_Vid->currentSlice;
	p_Vid->colour_plane_id = nplane;
	p_Vid->mb_data = p_Vid->mb_data_JV[nplane];
	p_Vid->dec_picture  = p_Vid->dec_picture_JV[nplane];
	currSlice->p_colocated   = currSlice->Co_located_JV[nplane];
}

/*!
************************************************************************
* \brief
*    make frame picture from each plane data
*    for 4:4:4 Independent mode
************************************************************************
*/
void make_frame_picture_JV(VideoParameters *p_Vid)
{
	int uv, line;
	int nsize;
	int nplane;
	p_Vid->dec_picture = p_Vid->dec_picture_JV[0];

	// Copy Storable Params
	for( nplane=0; nplane<MAX_PLANE; nplane++ )
	{
		copy_storable_param_JV( p_Vid, &p_Vid->dec_picture->JVmotion[nplane], &p_Vid->dec_picture_JV[nplane]->motion );
	}

	// This could be done with pointers and seems not necessary
	for( uv=0; uv<2; uv++ )
	{
		for( line=0; line<p_Vid->height; line++ )
		{
			nsize = sizeof(imgpel) * p_Vid->width;
			memcpy( p_Vid->dec_picture->imgUV[uv]->img[line], p_Vid->dec_picture_JV[uv+1]->imgY->img[line], nsize );
		}
		free_storable_picture(p_Vid, p_Vid->dec_picture_JV[uv+1]);
	}
}


