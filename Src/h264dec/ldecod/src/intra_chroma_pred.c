/*!
*************************************************************************************
* \file intra_chroma_pred.c
*
* \brief
*    Functions for intra chroma prediction
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
#include "mb_access.h"
#include "image.h"

static void intra_chroma_DC_single(imgpel **curr_img, int up_avail, int left_avail, PixelPos up, PixelPos left[17], int blk_x, int blk_y, int *pred, int direction )
{
	int s0;

	if ((direction && up_avail) || (!left_avail && up_avail))
	{
		imgpel *img = &curr_img[up.pos_y][up.pos_x + blk_x];
		s0 = img[0] + img[1] + img[2] + img[3];
		*pred = (s0+2) >> 2;
	}
	else if (left_avail)  
	{
		s0 = curr_img[left[blk_y].pos_y][left[blk_y].pos_x];
		s0 += curr_img[left[blk_y+1].pos_y][left[blk_y+1].pos_x];
		s0 += curr_img[left[blk_y+2].pos_y][left[blk_y+2].pos_x];
		s0 += curr_img[left[blk_y+3].pos_y][left[blk_y+3].pos_x];

		*pred = (s0+2) >> 2;
	}
}


static void intra_chroma_DC_all(imgpel **curr_img, int up_avail, int left_avail, PixelPos up, PixelPos left[17], int blk_x, int blk_y, int *pred )
{
	int s0 = 0, s1 = 0;

	if (up_avail)
	{
		imgpel *img = &curr_img[up.pos_y][up.pos_x + blk_x];
		s0 = img[0] + img[1] + img[2] + img[3];
	}

	if (left_avail)
	{
		s1 += curr_img[left[blk_y].pos_y][left[blk_y].pos_x];
		s1 += curr_img[left[blk_y+1].pos_y][left[blk_y+1].pos_x];
		s1 += curr_img[left[blk_y+2].pos_y][left[blk_y+2].pos_x];
		s1 += curr_img[left[blk_y+3].pos_y][left[blk_y+3].pos_x];
	}

	if (up_avail && left_avail)
		*pred = (s0 + s1 + 4) >> 3;
	else if (up_avail)
		*pred = (s0 + 2) >> 2;
	else if (left_avail)
		*pred = (s1 + 2) >> 2;
}

/*!
************************************************************************
* \brief
*    Chroma Intra prediction. Note that many operations can be moved
*    outside since they are repeated for both components for no reason.
************************************************************************
*/

static void memset_4x4(h264_imgpel_macroblock_row_t *mb_pred, int offset_x, int pred)
{
#ifdef _M_IX86
	// benski> can't believe the shitty code that the compiler generated...  this code is better
	int dword_pred = pred * 0x01010101;
	mb_pred = (h264_imgpel_macroblock_row_t *)&mb_pred[0][offset_x];
	*(int *)mb_pred[0] = dword_pred;
	*(int *)mb_pred[1] = dword_pred;
	*(int *)mb_pred[2] = dword_pred;
	*(int *)mb_pred[3] = dword_pred;
#else
	int ii, jj;
	for (jj = 0; jj < BLOCK_SIZE; jj++)
	{
		for (ii = 0; ii < BLOCK_SIZE; ii++)
		{
			mb_pred[jj][offset_x+ii]=(imgpel) pred;
		}
	}
#endif
}

static void chroma_dc_pred8(VideoParameters *p_Vid, int yuv, imgpel **imgUV, int up_avail, int left_avail[2], PixelPos up, PixelPos left[17], h264_imgpel_macroblock_row_t *mb_pred)
{
	static const byte block_pos[3][4][4]= //[yuv][b8][b4]
	{
		{ {0, 1, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
		{ {0, 1, 2, 3},{2, 3, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0}},
		{ {0, 1, 2, 3},{1, 1, 3, 3},{2, 3, 2, 3},{3, 3, 3, 3}}
	};

	int b8, b4;
	int pred;

	// DC prediction
	// Note that unlike what is stated in many presentations and papers, this mode does not operate
	// the same way as I_16x16 DC prediction.

	for(b8 = 0; b8 < (p_Vid->num_uv_blocks) ;b8++)
	{
		for (b4 = 0; b4 < 4; b4++)
		{
			int blk_y = subblk_offset_y[yuv][b8][b4];
			int blk_x = subblk_offset_x[yuv][b8][b4];

			pred = p_Vid->dc_pred_value_comp[1];

			//===== get prediction value =====
			switch (block_pos[yuv][b8][b4])
			{
			case 0:  //===== TOP LEFT =====
				intra_chroma_DC_all   (imgUV, up_avail, left_avail[0], up, left, blk_x, blk_y + 1, &pred);
				break;
			case 1: //===== TOP RIGHT =====
				intra_chroma_DC_single(imgUV, up_avail, left_avail[0], up, left, blk_x, blk_y + 1, &pred, 1);
				break;
			case 2: //===== BOTTOM LEFT =====
				intra_chroma_DC_single(imgUV, up_avail, left_avail[1], up, left, blk_x, blk_y + 1, &pred, 0);
				break;
			case 3: //===== BOTTOM RIGHT =====
				intra_chroma_DC_all   (imgUV, up_avail, left_avail[1], up, left, blk_x, blk_y + 1, &pred);          
				break;
			}

			memset_4x4(mb_pred+blk_y, blk_x, pred);
		}
	}

}

static void chroma_pred_horiz8(int cr_MB_x, int cr_MB_y, PixelPos left[17], imgpel **imgUV, h264_imgpel_macroblock_row_t *mb_pred)
{
	// Horizontal Prediction
	int i,j;

	if (cr_MB_x == 8)
	{
		for (j = 0; j < cr_MB_y; ++j)
	{

		int pred = imgUV[left[1 + j].pos_y][left[1 + j].pos_x];
		for (i = 0; i < 8; ++i)
			mb_pred[j][i]=(imgpel) pred;
	}
	}
	else
	{
		assert(cr_MB_x == 16);
	for (j = 0; j < cr_MB_y; ++j)
	{

		int pred = imgUV[left[1 + j].pos_y][left[1 + j].pos_x];
		for (i = 0; i < 16; ++i)
			mb_pred[j][i]=(imgpel) pred;
	}
	}
}

static void chroma_pred_vert8(int cr_MB_x, int cr_MB_y, PixelPos up, imgpel **imgUV,  h264_imgpel_macroblock_row_t *mb_pred)
{
	// Vertical Prediction
	const imgpel *source = &(imgUV[up.pos_y][up.pos_x]);
	if (cr_MB_x == 8)
	{
		int j;
		for (j = 0; j < cr_MB_y; ++j)
		{
			memcpy(mb_pred[j], source, 8 * sizeof(imgpel));
		}
	}
	else
	{
		int j;
		assert(cr_MB_x == 16);

		
		for (j = 0; j < cr_MB_y; ++j)
		{
			memcpy(mb_pred[j], source, 16 * sizeof(imgpel));
		}
	}
}

static void chroma_pred_plane8(int cr_MB_x, int cr_MB_y, int cr_MB_x2, int cr_MB_y2, PixelPos up, PixelPos left[17], int max_imgpel_value, imgpel **imgUV,  h264_imgpel_macroblock_row_t *mb_pred)
{
	int ih, iv, ib, ic, i, j, iaa;
	imgpel *upPred = &imgUV[up.pos_y][up.pos_x];

	ih = cr_MB_x2 * (upPred[cr_MB_x - 1] - imgUV[left[0].pos_y][left[0].pos_x]);
	for (i = 0; i < cr_MB_x2 - 1; ++i)
		ih += (i + 1) * (upPred[cr_MB_x2 + i] - upPred[cr_MB_x2 - 2 - i]);

	iv = cr_MB_y2 * (imgUV[left[cr_MB_y].pos_y][left[cr_MB_y].pos_x] - imgUV[left[0].pos_y][left[0].pos_x]);
	for (i = 0; i < cr_MB_y2 - 1; ++i)
		iv += (i + 1)*(imgUV[left[cr_MB_y2 + 1 + i].pos_y][left[cr_MB_y2 + 1 + i].pos_x] -
		imgUV[left[cr_MB_y2 - 1 - i].pos_y][left[cr_MB_y2 - 1 - i].pos_x]);

	ib= ((cr_MB_x == 8 ? 17 : 5) * ih + 2 * cr_MB_x)>>(cr_MB_x == 8 ? 5 : 6);
	ic= ((cr_MB_y == 8 ? 17 : 5) * iv + 2 * cr_MB_y)>>(cr_MB_y == 8 ? 5 : 6);

	iaa=16*(imgUV[left[cr_MB_y].pos_y][left[cr_MB_y].pos_x] + upPred[cr_MB_x-1]);

	for (j = 0; j < cr_MB_y; ++j)
		for (i = 0; i < cr_MB_x; ++i)
			mb_pred[j][i]=(imgpel) iClip1(max_imgpel_value, ((iaa + (i - cr_MB_x2 + 1) * ib + (j - cr_MB_y2 + 1) * ic + 16) >> 5));  
}

// TODO: benski> replace with PredictIntraChroma8x8_H264 ?
void intrapred_chroma(Macroblock *currMB, int uv)
{
	if (currMB->c_ipred_mode == VERT_PRED_8)
	{
		Slice *currSlice = currMB->p_Slice;
		VideoParameters *p_Vid = currMB->p_Vid;
		StorablePicture *dec_picture = p_Vid->dec_picture;
		imgpel **imgUV = dec_picture->imgUV[uv]->img;

		h264_imgpel_macroblock_row_t *mb_pred = currSlice->mb_pred[uv + 1];

		PixelPos up;        //!< pixel position  p(0,-1)

		int up_avail;

		int cr_MB_x = p_Vid->mb_cr_size_x;
		int cr_MB_y = p_Vid->mb_cr_size_y;

		p_Vid->getNeighbourUp(currMB, p_Vid->mb_size[IS_CHROMA], &up);

		if (!p_Vid->active_pps->constrained_intra_pred_flag)
		{
			up_avail      = up.available;
		}
		else
		{
			up_avail = up.available ? p_Vid->intra_block[up.mb_addr] : 0;
		}

		// Vertical Prediction
		if (!up_avail)
			error("unexpected VERT_PRED_8 chroma intra prediction mode",-1);

		chroma_pred_vert8(cr_MB_x, cr_MB_y, up, imgUV, mb_pred);
	}
	else
	{
		Slice *currSlice = currMB->p_Slice;
		VideoParameters *p_Vid = currMB->p_Vid;
		int i;
		StorablePicture *dec_picture = p_Vid->dec_picture;
		imgpel **imgUV = dec_picture->imgUV[uv]->img;
		int     max_imgpel_value = p_Vid->max_pel_value_comp[uv + 1];

		int        yuv = dec_picture->chroma_format_idc - 1;
		h264_imgpel_macroblock_row_t *mb_pred = currSlice->mb_pred[uv + 1];


		PixelPos up;        //!< pixel position  p(0,-1)
		PixelPos left[17];  //!< pixel positions p(-1, -1..16)

		int up_avail, left_avail[2], left_up_avail;

		int cr_MB_x = p_Vid->mb_cr_size_x;
		int cr_MB_y = p_Vid->mb_cr_size_y;
		int cr_MB_y2 = (cr_MB_y >> 1);
		int cr_MB_x2 = (cr_MB_x >> 1);

		p_Vid->getNeighbourNX(currMB, -1, p_Vid->mb_size[IS_CHROMA], &left[0]);
		p_Vid->getNeighbourLeft(currMB, p_Vid->mb_size[IS_CHROMA], &left[1]); 

		p_Vid->getNeighbourNPChromaNB(currMB, 2-1, p_Vid->mb_size[IS_CHROMA], &left[2]);
		p_Vid->getNeighbourNPChromaNB(currMB, 3-1, p_Vid->mb_size[IS_CHROMA], &left[3]);
		p_Vid->getNeighbourNPChromaNB(currMB, 4-1, p_Vid->mb_size[IS_CHROMA], &left[4]);
		p_Vid->getNeighbourNPChromaNB(currMB, 5-1, p_Vid->mb_size[IS_CHROMA], &left[5]);
		p_Vid->getNeighbourNPChromaNB(currMB, 6-1, p_Vid->mb_size[IS_CHROMA], &left[6]);
		p_Vid->getNeighbourNPChromaNB(currMB, 7-1, p_Vid->mb_size[IS_CHROMA], &left[7]);
		p_Vid->getNeighbourNPChromaNB(currMB, 8-1, p_Vid->mb_size[IS_CHROMA], &left[8]);

		if (cr_MB_y == 16)
		{
			p_Vid->getNeighbourNPChromaNB(currMB, 9-1, p_Vid->mb_size[IS_CHROMA], &left[9]);
			p_Vid->getNeighbourNPChromaNB(currMB, 10-1, p_Vid->mb_size[IS_CHROMA], &left[10]);
			p_Vid->getNeighbourNPChromaNB(currMB, 11-1, p_Vid->mb_size[IS_CHROMA], &left[11]);
			p_Vid->getNeighbourNPChromaNB(currMB, 12-1, p_Vid->mb_size[IS_CHROMA], &left[12]);
			p_Vid->getNeighbourNPChromaNB(currMB, 13-1, p_Vid->mb_size[IS_CHROMA], &left[13]);
			p_Vid->getNeighbourNPChromaNB(currMB, 14-1, p_Vid->mb_size[IS_CHROMA], &left[14]);
			p_Vid->getNeighbourNPChromaNB(currMB, 15-1, p_Vid->mb_size[IS_CHROMA], &left[15]);
			p_Vid->getNeighbourNPChromaNB(currMB, 16-1, p_Vid->mb_size[IS_CHROMA], &left[16]);
		}

		p_Vid->getNeighbourUp(currMB, p_Vid->mb_size[IS_CHROMA], &up);

		if (!p_Vid->active_pps->constrained_intra_pred_flag)
		{
			up_avail      = up.available;
			left_avail[0] = left_avail[1] = left[1].available;
			left_up_avail = left[0].available;
		}
		else
		{
			up_avail = up.available ? p_Vid->intra_block[up.mb_addr] : 0;
			for (i=0, left_avail[0] = 1; i < cr_MB_y2;++i)
				left_avail[0]  &= left[i + 1].available ? p_Vid->intra_block[left[i + 1].mb_addr]: 0;

			for (i = cr_MB_y2, left_avail[1] = 1; i<cr_MB_y;++i)
				left_avail[1]  &= left[i + 1].available ? p_Vid->intra_block[left[i + 1].mb_addr]: 0;

			left_up_avail = left[0].available ? p_Vid->intra_block[left[0].mb_addr]: 0;
		}

		switch (currMB->c_ipred_mode)
		{
		case DC_PRED_8:
			chroma_dc_pred8(p_Vid, yuv, imgUV, up_avail, left_avail, up, left, mb_pred);
			break;
		case HOR_PRED_8:
			{
				// Horizontal Prediction
				if (!left_avail[0] || !left_avail[1])
					error("unexpected HOR_PRED_8 chroma intra prediction mode",-1);

				chroma_pred_horiz8(cr_MB_x, cr_MB_y, left, imgUV, mb_pred);
			}
			break;
		case PLANE_8:
			// plane prediction
			if (!left_up_avail || !left_avail[0] || !left_avail[1] || !up_avail)
				error("unexpected PLANE_8 chroma intra prediction mode",-1);
			else
			{
				chroma_pred_plane8(cr_MB_x, cr_MB_y, cr_MB_x2, cr_MB_y2, up, left, max_imgpel_value, imgUV, mb_pred);
			}
			break;
		default:
			error("illegal chroma intra prediction mode", 600);
			break;
		}
	}
}




