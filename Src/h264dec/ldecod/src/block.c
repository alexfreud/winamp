
/*!
 ***********************************************************************
 *  \file
 *      block.c
 *
 *  \brief
 *      Block functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Inge Lille-Langoy          <inge.lille-langoy@telenor.com>
 *      - Rickard Sjoberg            <rickard.sjoberg@era.ericsson.se>
 ***********************************************************************
 */

#include "contributors.h"

#include "global.h"
#include "block.h"
#include "image.h"
#include "mb_access.h"
#include "transform.h"
#include "quant.h"
#include "memalloc.h"
#include "optim.h"


/*!
 ****************************************************************************
 * \brief
 *    Inverse 4x4 lossless_qpprime transformation, transforms cof to mb_rres
 ****************************************************************************
 */
void itrans4x4_ls(const h264_short_block_row_t *tblock,
									const h264_imgpel_macroblock_row_t *mb_pred,
									h264_imgpel_macroblock_row_t *mb_rec,
                  int ioff,             //!< index to 4x4 block
                  int joff)             //!< index to 4x4 block
{
  int i,j;
  for (j = 0; j < BLOCK_SIZE; ++j)
  {
    for (i = 0; i < BLOCK_SIZE; ++i)
    {      
      mb_rec[j+joff][i+ioff] = (imgpel) iClip1(255/*max_imgpel_value*/, mb_pred[j+joff][i+ioff] + tblock[j][i]);
    }
  }
}


/*!
************************************************************************
* \brief
*    Inverse residual DPCM for Intra lossless coding
*
************************************************************************
*/
void Inv_Residual_trans_4x4(Macroblock *currMB,   //!< current macroblock
														ColorPlane pl,        //!< used color plane
														int ioff,             //!< index to 4x4 block
														int joff)             //!< index to 4x4 block
{
	int i,j;
	h264_short_block_t temp;
	Slice *currSlice = currMB->p_Slice;
	int subblock = cof4_pos_to_subblock[joff>>2][ioff>>2];

	h264_short_block_row_t *tblock = currSlice->cof4[pl][subblock];

	if(currMB->ipmode_DPCM == VERT_PRED)
	{
		for(i=0; i<4; ++i)
		{
			temp[0][i] = tblock[0][i];
			temp[1][i] = tblock[1][i] + temp[0][i];
			temp[2][i] = tblock[2][i] + temp[1][i];
			temp[3][i] = tblock[3][i] + temp[2][i];
		}
	}
	else if(currMB->ipmode_DPCM == HOR_PRED)
	{
		for(j=0; j<4; ++j)
		{
			temp[j][0] = tblock[j][0];
			temp[j][1] = tblock[j][1] + temp[j][0];
			temp[j][2] = tblock[j][2] + temp[j][1];
			temp[j][3] = tblock[j][3] + temp[j][2];
		}
	}
	else
	{
		for (j = 0; j < BLOCK_SIZE; ++j)
			for (i = 0; i < BLOCK_SIZE; ++i)
				temp[j][i] = tblock[j][i];
	}

	for (j = 0; j < BLOCK_SIZE; ++j)
	{
		for (i = 0; i < BLOCK_SIZE; ++i)
		{
			currSlice->mb_rec[pl][j+joff][i+ioff] = (imgpel) (temp[j][i] + currSlice->mb_pred[pl][j+joff][i+ioff]);
		}
	}
}

/*!
************************************************************************
* \brief
*    Inverse residual DPCM for Intra lossless coding
*
* \par Input:
*    ioff_x,joff_y: Block position inside a macro block (0,8).
************************************************************************
*/
//For residual DPCM
void Inv_Residual_trans_8x8(Macroblock *currMB, ColorPlane pl, int ioff,int joff)
{
  Slice *currSlice = currMB->p_Slice;
  int i, j;
	h264_short_8x8block_t temp;

	int block = (joff>>2) + (ioff>>3);

  if(currMB->ipmode_DPCM == VERT_PRED)
  {
    for(i=0; i<8; ++i)
    {
      temp[0][i] = currSlice->mb_rres8[pl][block][0][i];
      temp[1][i] = currSlice->mb_rres8[pl][block][1][i] + temp[0][i];
      temp[2][i] = currSlice->mb_rres8[pl][block][2][i] + temp[1][i];
      temp[3][i] = currSlice->mb_rres8[pl][block][3][i] + temp[2][i];
      temp[4][i] = currSlice->mb_rres8[pl][block][4][i] + temp[3][i];
      temp[5][i] = currSlice->mb_rres8[pl][block][5][i] + temp[4][i];
      temp[6][i] = currSlice->mb_rres8[pl][block][6][i] + temp[5][i];
      temp[7][i] = currSlice->mb_rres8[pl][block][7][i] + temp[6][i];
    }
    for(i=0; i<8; ++i)
    {
      currSlice->mb_rres8[pl][block][0][i]=temp[0][i];
      currSlice->mb_rres8[pl][block][1][i]=temp[1][i];
      currSlice->mb_rres8[pl][block][2][i]=temp[2][i];
      currSlice->mb_rres8[pl][block][3][i]=temp[3][i];
      currSlice->mb_rres8[pl][block][4][i]=temp[4][i];
      currSlice->mb_rres8[pl][block][5][i]=temp[5][i];
      currSlice->mb_rres8[pl][block][6][i]=temp[6][i];
      currSlice->mb_rres8[pl][block][7][i]=temp[7][i];
    }
  }
  else if(currMB->ipmode_DPCM == HOR_PRED)//HOR_PRED
  {
    for(i=0; i<8; ++i)
    {
      temp[i][0] = currSlice->mb_rres8[pl][block][i][0];
      temp[i][1] = currSlice->mb_rres8[pl][block][i][1] + temp[i][0];
      temp[i][2] = currSlice->mb_rres8[pl][block][i][2] + temp[i][1];
      temp[i][3] = currSlice->mb_rres8[pl][block][i][3] + temp[i][2];
      temp[i][4] = currSlice->mb_rres8[pl][block][i][4] + temp[i][3];
      temp[i][5] = currSlice->mb_rres8[pl][block][i][5] + temp[i][4];
      temp[i][6] = currSlice->mb_rres8[pl][block][i][6] + temp[i][5];
      temp[i][7] = currSlice->mb_rres8[pl][block][i][7] + temp[i][6];
    }
    for(i=0; i<8; ++i)
    {
      currSlice->mb_rres8[pl][block][i][0]=temp[i][0];
      currSlice->mb_rres8[pl][block][i][1]=temp[i][1];
      currSlice->mb_rres8[pl][block][i][2]=temp[i][2];
      currSlice->mb_rres8[pl][block][i][3]=temp[i][3];
      currSlice->mb_rres8[pl][block][i][4]=temp[i][4];
      currSlice->mb_rres8[pl][block][i][5]=temp[i][5];
      currSlice->mb_rres8[pl][block][i][6]=temp[i][6];
      currSlice->mb_rres8[pl][block][i][7]=temp[i][7];
    }
  }

  for (j = 0; j < BLOCK_SIZE_8x8; ++j)
  {
    for (i = 0; i < BLOCK_SIZE_8x8; ++i)
    {
      currSlice->mb_rec[pl][joff+j][ioff+i]  = (imgpel) (currSlice->mb_rres8[pl][block][j][i] + currSlice->mb_pred[pl][joff+j][ioff+i]);
    }
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Luma DC inverse transform
 ***********************************************************************
 */ 
void itrans_2(Macroblock *currMB, ColorPlane pl)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  int transform_pl = IS_INDEPENDENT(p_Vid) ? PLANE_Y /*p_Vid->colour_plane_id*/ : pl;
  h264_short_block_t *blocks = currSlice->cof4[transform_pl];
  int qp_scaled = currMB->qp_scaled[pl];

  int qp_per = p_Vid->qp_per_matrix[ qp_scaled ];
  int qp_rem = p_Vid->qp_rem_matrix[ qp_scaled ];      

  int invLevelScale = currSlice->InvLevelScale4x4_Intra[pl][qp_rem][0][0];
  h264_int_block_t M4;
  
  // horizontal
	M4[0][0]=blocks[0][0][0];
	M4[0][1]=blocks[1][0][0];
	M4[0][2]=blocks[4][0][0];
	M4[0][3]=blocks[5][0][0];
	M4[1][0]=blocks[2][0][0];
	M4[1][1]=blocks[3][0][0];
	M4[1][2]=blocks[6][0][0];
	M4[1][3]=blocks[7][0][0];
	M4[2][0]=blocks[8][0][0];
	M4[2][1]=blocks[9][0][0];
	M4[2][2]=blocks[12][0][0];
	M4[2][3]=blocks[13][0][0];
	M4[3][0]=blocks[10][0][0];
	M4[3][1]=blocks[11][0][0];
	M4[3][2]=blocks[14][0][0];
	M4[3][3]=blocks[15][0][0];

  ihadamard4x4(M4);

  // vertical
	blocks[0][0][0] = rshift_rnd((( M4[0][0] * invLevelScale) << qp_per), 6);
	blocks[1][0][0] = rshift_rnd((( M4[0][1] * invLevelScale) << qp_per), 6);
	blocks[4][0][0] = rshift_rnd((( M4[0][2] * invLevelScale) << qp_per), 6);
	blocks[5][0][0] = rshift_rnd((( M4[0][3] * invLevelScale) << qp_per), 6);
	blocks[2][0][0] = rshift_rnd((( M4[1][0] * invLevelScale) << qp_per), 6);
	blocks[3][0][0] = rshift_rnd((( M4[1][1] * invLevelScale) << qp_per), 6);
	blocks[6][0][0] = rshift_rnd((( M4[1][2] * invLevelScale) << qp_per), 6);
	blocks[7][0][0] = rshift_rnd((( M4[1][3] * invLevelScale) << qp_per), 6);
	blocks[8][0][0] = rshift_rnd((( M4[2][0] * invLevelScale) << qp_per), 6);
	blocks[9][0][0] = rshift_rnd((( M4[2][1] * invLevelScale) << qp_per), 6);
	blocks[12][0][0] = rshift_rnd((( M4[2][2] * invLevelScale) << qp_per), 6);
	blocks[13][0][0] = rshift_rnd((( M4[2][3] * invLevelScale) << qp_per), 6);
	blocks[10][0][0] = rshift_rnd((( M4[3][0] * invLevelScale) << qp_per), 6);
	blocks[11][0][0] = rshift_rnd((( M4[3][1] * invLevelScale) << qp_per), 6);
	blocks[14][0][0] = rshift_rnd((( M4[3][2] * invLevelScale) << qp_per), 6);
	blocks[15][0][0] = rshift_rnd((( M4[3][3] * invLevelScale) << qp_per), 6);
}


void itrans_sp(h264_short_block_row_t *tblock, const h264_imgpel_macroblock_row_t *mb_pred, Macroblock *currMB, ColorPlane pl, int ioff, int joff) 
{
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;
  int i,j;  
  int ilev, icof;

  int qp = (currSlice->slice_type == SI_SLICE) ? currSlice->qs : p_Vid->qp;
  int qp_per = p_Vid->qp_per_matrix[ qp ];
  int qp_rem = p_Vid->qp_rem_matrix[ qp ];

  int qp_per_sp = p_Vid->qp_per_matrix[ currSlice->qs ];
  int qp_rem_sp = p_Vid->qp_rem_matrix[ currSlice->qs ];
  int q_bits_sp = Q_BITS + qp_per_sp;
  int max_imgpel_value = p_Vid->max_pel_value_comp[pl];

  const int (*InvLevelScale4x4)  [4] = dequant_coef[qp_rem];
  const int (*InvLevelScale4x4SP)[4] = dequant_coef[qp_rem_sp];  
  int **PBlock;  

  get_mem2Dint(&PBlock, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

  for (j=0; j< BLOCK_SIZE; ++j)
    for (i=0; i< BLOCK_SIZE; ++i)
      PBlock[j][i] = mb_pred[j+joff][i+ioff];

  forward4x4(PBlock, PBlock, 0, 0);

  if(p_Vid->sp_switch || currSlice->slice_type==SI_SLICE)
  {    
    for (j=0;j<BLOCK_SIZE;++j)
    {
      for (i=0;i<BLOCK_SIZE;++i)
      {
        // recovering coefficient since they are already dequantized earlier
        icof = (tblock[j][i] >> qp_per) / InvLevelScale4x4[j][i];
        ilev  = rshift_rnd_sf(iabs(PBlock[j][i]) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
        ilev  = isignab(ilev, PBlock[j][i]) + icof;
        tblock[j][i] = ilev * InvLevelScale4x4SP[j][i] << qp_per_sp;
      }
    }
  }
  else
  {
    for (j=0;j<BLOCK_SIZE;++j)
    {
      for (i=0;i<BLOCK_SIZE;++i)
      {
        // recovering coefficient since they are already dequantized earlier
        icof = (tblock[j][i] >> qp_per) / InvLevelScale4x4[j][i];
        ilev = PBlock[j][i] + ((icof * InvLevelScale4x4[j][i] * A[j][i] <<  qp_per) >> 6);
        ilev  = isign(ilev) * rshift_rnd_sf(iabs(ilev) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
        tblock[j][i] = ilev * InvLevelScale4x4SP[j][i] << qp_per_sp;
      }
    }
  }

	{
		h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];
		opt_itrans4x4(tblock, mb_pred, mb_rec, ioff, joff);
	}

  free_mem2Dint(PBlock);
}

void itrans_sp_cr(Macroblock *currMB, int uv)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  int i,j,ilev, icof, n2,n1;
  int mp1[BLOCK_SIZE];
  int qp_per,qp_rem;
  int qp_per_sp,qp_rem_sp,q_bits_sp;
  int **PBlock;  

  get_mem2Dint(&PBlock, MB_BLOCK_SIZE, MB_BLOCK_SIZE);


  qp_per    = p_Vid->qp_per_matrix[ ((p_Vid->qp < 0 ? p_Vid->qp : QP_SCALE_CR[p_Vid->qp]))];
  qp_rem    = p_Vid->qp_rem_matrix[ ((p_Vid->qp < 0 ? p_Vid->qp : QP_SCALE_CR[p_Vid->qp]))];

  qp_per_sp = p_Vid->qp_per_matrix[ ((currSlice->qs < 0 ? currSlice->qs : QP_SCALE_CR[currSlice->qs]))];
  qp_rem_sp = p_Vid->qp_rem_matrix[ ((currSlice->qs < 0 ? currSlice->qs : QP_SCALE_CR[currSlice->qs]))];
  q_bits_sp = Q_BITS + qp_per_sp;  

  if (currSlice->slice_type == SI_SLICE)
  {
    qp_per = qp_per_sp;
    qp_rem = qp_rem_sp;
  }

  for (j=0; j < p_Vid->mb_cr_size_y; ++j)
  {
    for (i=0; i < p_Vid->mb_cr_size_x; ++i)
    {
      PBlock[j][i] = currSlice->mb_pred[uv + 1][j][i];
      currSlice->mb_pred[uv + 1][j][i] = 0;
    }
  }

  for (n2=0; n2 < p_Vid->mb_cr_size_y; n2 += BLOCK_SIZE)
  {
    for (n1=0; n1 < p_Vid->mb_cr_size_x; n1 += BLOCK_SIZE)
    {
      forward4x4(PBlock, PBlock, n2, n1);
    }
  }

  //     2X2 transform of DC coeffs.
  mp1[0] = (PBlock[0][0] + PBlock[4][0] + PBlock[0][4] + PBlock[4][4]);
  mp1[1] = (PBlock[0][0] - PBlock[4][0] + PBlock[0][4] - PBlock[4][4]);
  mp1[2] = (PBlock[0][0] + PBlock[4][0] - PBlock[0][4] - PBlock[4][4]);
  mp1[3] = (PBlock[0][0] - PBlock[4][0] - PBlock[0][4] + PBlock[4][4]);

  if (p_Vid->sp_switch || currSlice->slice_type == SI_SLICE)  
  {        
    for (n2=0; n2 < 2; ++n2 )
    {
      for (n1=0; n1 < 2; ++n1 )
      {
        //quantization fo predicted block
        ilev = rshift_rnd_sf(iabs (mp1[n1+n2*2]) * quant_coef[qp_rem_sp][0][0], q_bits_sp + 1);
        //addition
        ilev = isignab(ilev, mp1[n1+n2*2]) + currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2][n1]][0][0];
        //dequantization
        mp1[n1+n2*2] =ilev * dequant_coef[qp_rem_sp][0][0] << qp_per_sp;
      }
    }

    for (n2 = 0; n2 < p_Vid->mb_cr_size_y; n2 += BLOCK_SIZE)
    {
      for (n1 = 0; n1 < p_Vid->mb_cr_size_x; n1 += BLOCK_SIZE)
      {
        for (j = 0; j < BLOCK_SIZE; ++j)
        {
          for (i = 0; i < BLOCK_SIZE; ++i)
          {
            // recovering coefficient since they are already dequantized earlier
            currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2>>2][n1>>2]][j][i] = (currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2>>2][n1>>2]][j][i] >> qp_per) / dequant_coef[qp_rem][j][i];

            //quantization of the predicted block
            ilev = rshift_rnd_sf(iabs(PBlock[n2 + j][n1 + i]) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
            //addition of the residual
            ilev = isignab(ilev,PBlock[n2 + j][n1 + i]) + currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2>>2][n1>>2]][j][i] ;
            // Inverse quantization
            currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2>>2][n1>>2]][j][i]  = ilev * dequant_coef[qp_rem_sp][j][i] << qp_per_sp;
          }
        }
      }
    }
  }
  else
  {
    for (n2=0; n2 < 2; ++n2 )
    {
      for (n1=0; n1 < 2; ++n1 )
      {
        ilev = mp1[n1+n2*2] + (((currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2][n1]][0][0] * dequant_coef[qp_rem][0][0] * A[0][0]) << qp_per) >> 5);
        ilev = isign(ilev) * rshift_rnd_sf(iabs(ilev) * quant_coef[qp_rem_sp][0][0], q_bits_sp + 1);
        //ilev = isignab(rshift_rnd_sf(iabs(ilev)* quant_coef[qp_rem_sp][0][0], q_bits_sp + 1), ilev);
        mp1[n1+n2*2] = ilev * dequant_coef[qp_rem_sp][0][0] << qp_per_sp;
      }
    }

    for (n2 = 0; n2 < p_Vid->mb_cr_size_y; n2 += BLOCK_SIZE)
    {
      for (n1 = 0; n1 < p_Vid->mb_cr_size_x; n1 += BLOCK_SIZE)
      {
        for (j = 0; j< BLOCK_SIZE; ++j)
        {
          for (i = 0; i< BLOCK_SIZE; ++i)
          {
            // recovering coefficient since they are already dequantized earlier
            icof = (currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2>>2][n1>>2]][j][i] >> qp_per) / dequant_coef[qp_rem][j][i];
            //dequantization and addition of the predicted block      
            ilev = PBlock[n2 + j][n1 + i] + ((icof * dequant_coef[qp_rem][j][i] * A[j][i] << qp_per) >> 6);
            //quantization and dequantization
            ilev = isign(ilev) * rshift_rnd_sf(iabs(ilev) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
            currSlice->cof4[uv + 1][cof4_pos_to_subblock[n2>>2][n1>>2]][j][i] = ilev * dequant_coef[qp_rem_sp][j][i] << qp_per_sp;
          }
        }
      }
    }
  }

  currSlice->cof4[uv + 1][0][0][0] = (mp1[0] + mp1[1] + mp1[2] + mp1[3]) >> 1;
  currSlice->cof4[uv + 1][1][0][0] = (mp1[0] + mp1[1] - mp1[2] - mp1[3]) >> 1;
  currSlice->cof4[uv + 1][2][0][0] = (mp1[0] - mp1[1] + mp1[2] - mp1[3]) >> 1;
  currSlice->cof4[uv + 1][3][0][0] = (mp1[0] - mp1[1] - mp1[2] + mp1[3]) >> 1;

  free_mem2Dint(PBlock);
}

#if defined(_DEBUG) || !defined(_M_IX86)
void iMBtrans4x4(Macroblock *currMB, ColorPlane pl, int smb)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  StorablePicture *dec_picture = p_Vid->dec_picture;

  VideoImage *curr_img = pl ? dec_picture->imgUV[pl - 1]: dec_picture->imgY;

  // =============== 4x4 itrans ================
  // -------------------------------------------
	if (smb)
	{
		h264_short_block_t *blocks = currSlice->cof4[pl];
		const h264_imgpel_macroblock_row_t *mb_pred=currSlice->mb_pred[pl];

		itrans_sp(blocks[0], mb_pred, currMB, pl, 0, 0);
		itrans_sp(blocks[1], mb_pred, currMB, pl, 4, 0);
		itrans_sp(blocks[2], mb_pred, currMB, pl, 0, 4);
		itrans_sp(blocks[3], mb_pred, currMB, pl, 4, 4);
		itrans_sp(blocks[4], mb_pred, currMB, pl, 8, 0);
		itrans_sp(blocks[5], mb_pred, currMB, pl, 12, 0);
		itrans_sp(blocks[6], mb_pred, currMB, pl, 8, 4);
		itrans_sp(blocks[7], mb_pred, currMB, pl, 12, 4);
		itrans_sp(blocks[8], mb_pred, currMB, pl, 0, 8);
		itrans_sp(blocks[9], mb_pred, currMB, pl, 4, 8);
		itrans_sp(blocks[10], mb_pred, currMB, pl, 0, 12);
		itrans_sp(blocks[11], mb_pred, currMB, pl, 4, 12);
		itrans_sp(blocks[12], mb_pred, currMB, pl, 8, 8);
		itrans_sp(blocks[13], mb_pred, currMB, pl, 12, 8);
		itrans_sp(blocks[14], mb_pred, currMB, pl, 8, 12);
		itrans_sp(blocks[15], mb_pred, currMB, pl, 12, 12);
	}
	else if (currMB->is_lossless)
	{
		Inv_Residual_trans_4x4(currMB, pl, 0, 0);
		Inv_Residual_trans_4x4(currMB, pl, 4, 0);
		Inv_Residual_trans_4x4(currMB, pl, 0, 4);
		Inv_Residual_trans_4x4(currMB, pl, 4, 4);
		Inv_Residual_trans_4x4(currMB, pl, 8, 0);
		Inv_Residual_trans_4x4(currMB, pl, 12, 0);
		Inv_Residual_trans_4x4(currMB, pl, 8, 4);
		Inv_Residual_trans_4x4(currMB, pl, 12, 4);
		Inv_Residual_trans_4x4(currMB, pl, 0, 8);
		Inv_Residual_trans_4x4(currMB, pl, 4, 8);
		Inv_Residual_trans_4x4(currMB, pl, 0, 12);
		Inv_Residual_trans_4x4(currMB, pl, 4, 12);
		Inv_Residual_trans_4x4(currMB, pl, 8, 8);
		Inv_Residual_trans_4x4(currMB, pl, 12, 8);
		Inv_Residual_trans_4x4(currMB, pl, 8, 12);
		Inv_Residual_trans_4x4(currMB, pl, 12, 12);
	}
	else
	{
			const h264_short_block_t *blocks = currSlice->cof4[pl];
			const h264_imgpel_macroblock_row_t *mb_pred=currSlice->mb_pred[pl];
			h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];

			opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
			opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);
			opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
			opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);
			opt_itrans4x4(blocks[4], mb_pred, mb_rec, 8, 0);
			opt_itrans4x4(blocks[5], mb_pred, mb_rec, 12, 0);
			opt_itrans4x4(blocks[6], mb_pred, mb_rec, 8, 4);
			opt_itrans4x4(blocks[7], mb_pred, mb_rec, 12, 4);
			opt_itrans4x4(blocks[8], mb_pred, mb_rec, 0, 8);
			opt_itrans4x4(blocks[9], mb_pred, mb_rec, 4, 8);
			opt_itrans4x4(blocks[10], mb_pred, mb_rec, 0, 12);
			opt_itrans4x4(blocks[11], mb_pred, mb_rec, 4, 12);
			opt_itrans4x4(blocks[12], mb_pred, mb_rec, 8, 8);
			opt_itrans4x4(blocks[13], mb_pred, mb_rec, 12, 8);
			opt_itrans4x4(blocks[14], mb_pred, mb_rec, 8, 12);
			opt_itrans4x4(blocks[15], mb_pred, mb_rec, 12, 12);
	}

  // construct picture from 4x4 blocks
	opt_copy_image_data_16x16_stride(curr_img, currMB->pix_x, currMB->pix_y, currSlice->mb_rec[pl]);
}
#endif
void iMBtrans8x8(Macroblock *currMB, ColorPlane pl)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	VideoImage *curr_img = pl ? dec_picture->imgUV[pl - 1] : dec_picture->imgY;

	h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];
	h264_imgpel_macroblock_row_t *mb_pred = currSlice->mb_pred[pl];
	h264_short_8x8block_t *mb_rres8 = currSlice->mb_rres8[pl];

	if (currMB->is_lossless == FALSE)
	{
		opt_itrans8x8(mb_rec, mb_pred, mb_rres8[0], 0);
		opt_itrans8x8(mb_rec, mb_pred, mb_rres8[1], 8);
		opt_itrans8x8(mb_rec+8, mb_pred+8, mb_rres8[2], 0);		
		opt_itrans8x8(mb_rec+8, mb_pred+8, mb_rres8[3], 8);
	}
	else
	{
		itrans8x8_lossless(mb_rec, mb_pred, mb_rres8[0], 0);
		itrans8x8_lossless(mb_rec, mb_pred, mb_rres8[1], 8);
		itrans8x8_lossless(mb_rec+8, mb_pred+8, mb_rres8[2], 0);
		itrans8x8_lossless(mb_rec+8, mb_pred+8, mb_rres8[3], 8);
	}

	opt_copy_image_data_16x16_stride(curr_img, currMB->pix_x, currMB->pix_y, mb_rec);
}

void iTransform(Macroblock *currMB, ColorPlane pl, int smb)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  StorablePicture *dec_picture = p_Vid->dec_picture;
  
  int uv = pl-1; 

  if ((currMB->cbp & 15) != 0 || smb)
  {
    if(currMB->luma_transform_size_8x8_flag == 0) // 4x4 inverse transform
    {
      iMBtrans4x4(currMB, pl, smb); 
    }
    else // 8x8 inverse transform
    {  
      iMBtrans8x8(currMB, pl);    
    }
  }
  else
  {
    VideoImage *curr_img = pl ? dec_picture->imgUV[uv] : dec_picture->imgY;
    opt_copy_image_data_16x16_stride(curr_img, currMB->pix_x, currMB->pix_y, currSlice->mb_pred[pl]);
  }
// TODO: fix 4x4 lossless
	if (dec_picture->chroma_format_idc == YUV420)
	{
		VideoImage *curUV;

		for(uv=0;uv<2;++uv)
		{
			int pl = uv + 1;

			const h264_imgpel_macroblock_row_t *mb_pred=currSlice->mb_pred[pl];

			// =============== 4x4 itrans ================
			// -------------------------------------------
			curUV = dec_picture->imgUV[uv];

			if (!smb && (currMB->cbp>>4))
			{
				if (currMB->is_lossless == FALSE)
				{
					const h264_short_block_t *blocks = currSlice->cof4[pl];
					h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];

					opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
					opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);
					opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
					opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);
					copy_image_data_8x8_stride(curUV,currMB->pix_c_x, currMB->pix_c_y,  mb_rec);
				}
				else
				{ // lossless
					const h264_short_block_t *blocks = currSlice->cof4[pl];
					h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];

					itrans4x4_ls(blocks[0], mb_pred, mb_rec, 0, 0);
					itrans4x4_ls(blocks[1], mb_pred, mb_rec, 4, 0);
					itrans4x4_ls(blocks[2], mb_pred, mb_rec, 0, 4);
					itrans4x4_ls(blocks[3], mb_pred, mb_rec, 4, 4);
					copy_image_data_8x8_stride(curUV,currMB->pix_c_x, currMB->pix_c_y,  mb_rec);
				}
			}
			else if (smb)
			{
				const h264_short_block_t *blocks = currSlice->cof4[pl];
				h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];

				itrans_sp_cr(currMB, uv);

				opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
				opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);
				opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
				opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);

				copy_image_data_8x8_stride(curUV,currMB->pix_c_x, currMB->pix_c_y,  mb_rec);
			}
			else 
			{
				copy_image_data_8x8_stride(curUV,currMB->pix_c_x, currMB->pix_c_y,  mb_pred);
			}
		}
	}
	else if (dec_picture->chroma_format_idc == YUV422)
	{
		VideoImage *curUV;

		for(uv=0;uv<2;++uv)
		{
			// =============== 4x4 itrans ================
			// -------------------------------------------
			int pl = uv + 1;
			const h264_imgpel_macroblock_row_t *mb_pred=currSlice->mb_pred[pl];
			curUV = dec_picture->imgUV[uv];

			if (!smb && (currMB->cbp>>4))
			{
				h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];
				const h264_short_block_t *blocks = currSlice->cof4[pl];

				opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
				opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);
				opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
				opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);
				opt_itrans4x4(blocks[8], mb_pred, mb_rec, 0, 8);
				opt_itrans4x4(blocks[9], mb_pred, mb_rec, 4, 8);
				opt_itrans4x4(blocks[10], mb_pred, mb_rec, 0, 12);
				opt_itrans4x4(blocks[11], mb_pred, mb_rec, 4, 12);

				copy_image_data_stride(curUV,currMB->pix_c_x, currMB->pix_c_y,  mb_rec, 8, 16);
			}
			else if (smb)
			{
				const h264_short_block_t *blocks = currSlice->cof4[pl];
				h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];

				itrans_sp_cr(currMB, uv);

				opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
				opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);
				opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
				opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);
				opt_itrans4x4(blocks[8], mb_pred, mb_rec, 0, 8);
				opt_itrans4x4(blocks[9], mb_pred, mb_rec, 4, 8);
				opt_itrans4x4(blocks[10], mb_pred, mb_rec, 0, 12);
				opt_itrans4x4(blocks[11], mb_pred, mb_rec, 4, 12);

				copy_image_data_stride(curUV,currMB->pix_c_x, currMB->pix_c_y,  mb_rec, 8, 16);
			}
			else 
			{
				copy_image_data_stride(curUV,currMB->pix_c_x, currMB->pix_c_y,  mb_pred, 8, 16);
			}
		}
	}
}

/*!
 *************************************************************************************
 * \brief
 *    Copy ImgPel Data from one structure to another (16x16)
 *************************************************************************************
 */
void copy_image_data_16x16(imgpel  **imgBuf1, imgpel  **imgBuf2, int dest_x, int src_x)
{
  int j;
  for(j=0; j<MB_BLOCK_SIZE; ++j)
  {
    memcpy(&imgBuf1[j][dest_x], &imgBuf2[j][src_x], MB_BLOCK_SIZE * sizeof (imgpel));
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Copy ImgPel Data from one structure to another (16x16)
 *************************************************************************************
 */
#ifdef _M_IX86
void copy_image_data_16x16_stride_sse(VideoImage *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source)
{
		ptrdiff_t destination_stride = destination->stride; // in case the compiler doesn't optimize this
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;
		_asm
		{
			mov eax, dest
			mov ecx, destination_stride
			mov edx, source;
			movaps xmm0, 0[edx]
			movaps xmm1, 16[edx]
			movaps xmm2, 32[edx]
			movaps xmm3, 64[edx]
			movups [eax], xmm0 // dest[0]
			movups [eax+ecx], xmm1 // dest[1]
			movups [eax+2*ecx], xmm2 // dest[2]
			movups [eax+4*ecx], xmm3 // dest[4]

			movaps xmm0, 48[edx] 
			movaps xmm1, 96[edx] 
			lea eax, [eax+2*ecx] // dest = &dest[2]
			movups [eax+ecx], xmm0 // dest[3]
			movups [eax+4*ecx], xmm1 // dest[6]

			movaps xmm0, 80[edx] 
			movaps xmm1, 128[edx] 
			lea eax, [eax+2*ecx] // dest = &dest[2] (dest[4] from start)
			movups [eax+ecx], xmm0 // dest[5]
			movups [eax+4*ecx], xmm1 // dest[8]

			movaps xmm0, 112[edx] 
			movaps xmm1, 160[edx] 
			lea eax, [eax+2*ecx] // dest = &dest[2] (dest[6] from start)
			movups [eax+ecx], xmm0 // dest[7]
			movups [eax+4*ecx], xmm1 // dest[10]

			movaps xmm0, 144[edx] 
			movaps xmm1, 192[edx] 
			lea eax, [eax+2*ecx] // dest = &dest[2] (dest[8] from start)
			movups [eax+ecx], xmm0 // dest[9]
			movups [eax+4*ecx], xmm1 // dest[12]

			movaps xmm0, 176[edx] 
			movaps xmm1, 224[edx] 
			lea eax, [eax+2*ecx] // dest = &dest[2] (dest[10] from start)
			movups [eax+ecx], xmm0 // dest[11]
			movups [eax+4*ecx], xmm1 // dest[14]

			movaps xmm0, 208[edx] 
			movaps xmm1, 240[edx] 
			lea eax, [eax+ecx] // dest = &dest[1] (dest[11] from start)
			movups [eax+2*ecx], xmm0 // dest[13]
			movups [eax+4*ecx], xmm1 // dest[15]
		}
}
#endif

void copy_image_data_16x16_stride_c(VideoImage *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source)
{
	ptrdiff_t destination_stride = destination->stride; // in case the compiler doesn't optimize this
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;

  int j;
  for(j=0; j<MB_BLOCK_SIZE; j++)
  {
    memcpy(dest, source[j], MB_BLOCK_SIZE * sizeof (imgpel));
		dest+=destination_stride;
	}
}

/*!
 *************************************************************************************
 * \brief
 *    Copy ImgPel Data from one structure to another (8x8)
 *************************************************************************************
 */
void copy_image_data_8x8_stride2(VideoImage *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t imgBuf2, int src_x, int src_y)
{
#ifdef _M_IX86
	ptrdiff_t destination_stride = destination->stride;
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;
	_asm
	{
		mov eax, src_y
		shl eax, 4
		add eax, src_x
		add eax, imgBuf2

		mov edx, dest
		mov ecx, destination_stride

		movq mm0, MMWORD PTR 0[eax]
		movq mm1, MMWORD PTR 16[eax]
		movq mm2, MMWORD PTR 32[eax]
		movq mm3, MMWORD PTR 48[eax]
		movq mm4, MMWORD PTR 64[eax]
		movq mm5, MMWORD PTR 80[eax]
		movq mm6, MMWORD PTR 96[eax]
		movq mm7, MMWORD PTR 112[eax]

		movntq [edx], mm0
		movntq [edx+ecx], mm1
		movntq [edx+2*ecx], mm2
		movntq [edx+4*ecx], mm4
		add edx, ecx
		movntq 0[edx+2*ecx], mm3
		movntq 0[edx+4*ecx], mm5
		add edx, ecx
		movntq 0[edx+4*ecx], mm6
		add edx, ecx
		movntq 0[edx+4*ecx], mm7
	}
#else
 	ptrdiff_t destination_stride = destination->stride; // in case the compiler doesn't optimize this
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;
	int j;
	for(j = 0; j < BLOCK_SIZE_8x8; ++j)
	{
		memcpy(dest, &imgBuf2[src_y+j][src_x], BLOCK_SIZE_8x8 * sizeof (imgpel));
		dest+=destination_stride;
	}
#endif

}

void copy_image_data_8x8_stride(VideoImage *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t imgBuf2)
{
#ifdef _M_IX86
	ptrdiff_t destination_stride = destination->stride;
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;
	_asm
	{
		mov eax, imgBuf2
		mov edx, dest
		mov ecx, destination_stride

		movq mm0, MMWORD PTR 0[eax]
		movq mm1, MMWORD PTR 16[eax]
		movq mm2, MMWORD PTR 32[eax]
		movq mm3, MMWORD PTR 48[eax]
		movq mm4, MMWORD PTR 64[eax]
		movq mm5, MMWORD PTR 80[eax]
		movq mm6, MMWORD PTR 96[eax]
		movq mm7, MMWORD PTR 112[eax]

		movntq [edx], mm0
		movntq [edx+ecx], mm1
		movntq [edx+2*ecx], mm2
		movntq [edx+4*ecx], mm4
		add edx, ecx
		movntq 0[edx+2*ecx], mm3
		movntq 0[edx+4*ecx], mm5
		add edx, ecx
		movntq 0[edx+4*ecx], mm6
		add edx, ecx
		movntq 0[edx+4*ecx], mm7
	}
#else
 	ptrdiff_t destination_stride = destination->stride; // in case the compiler doesn't optimize this
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;
	int j;
	for(j = 0; j < BLOCK_SIZE_8x8; ++j)
	{
		memcpy(dest, &imgBuf2[j][0], BLOCK_SIZE_8x8 * sizeof (imgpel));
		dest+=destination_stride;
	}
#endif
}

/*!
 *************************************************************************************
 * \brief
 *    Copy ImgPel Data from one structure to another (4x4)
 *************************************************************************************
 */

void copy_image_data_4x4_stride(VideoImage *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t source, int src_x, int src_y)
{
	ptrdiff_t destination_stride = destination->stride; // in case the compiler doesn't optimize this
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;
	h264_imgpel_macroblock_row_t *src = (h264_imgpel_macroblock_row_t *)source[src_y]; /* cast is for const */

	int j;
	for(j = 0; j < BLOCK_SIZE; ++j)
	{
		memcpy(dest, &src[j][src_x], BLOCK_SIZE * sizeof (imgpel));
		dest+=destination_stride;
	}
}

/*!
 *************************************************************************************
 * \brief
 *    Copy ImgPel Data from one structure to another (8x8)
 *************************************************************************************
 */
void copy_image_data(imgpel  **imgBuf1, imgpel  **imgBuf2, int dest_x, int src_x, int width, int height)
{
  int j;
  for(j = 0; j < height; ++j)
  {
    memcpy(&imgBuf1[j][dest_x], &imgBuf2[j][src_x], width * sizeof (imgpel));
  }
}

void copy_image_data_stride(VideoImage *destination, int dest_x, int dest_y, const h264_imgpel_macroblock_t imgBuf2, int width, int height)
{
	ptrdiff_t destination_stride = destination->stride; // in case the compiler doesn't optimize this
	imgpel *dest = destination->base_address + destination_stride * dest_y + dest_x;
	#ifdef H264_IPP
	IppiSize roi = {width,height};
	ippiCopy_8u_C1R(imgBuf2[0], sizeof(imgBuf2[0]), dest, destination_stride, roi);
#else
  int j;
  for(j = 0; j < height; ++j)
  {
		memcpy(dest, imgBuf2[j], width * sizeof (imgpel));
		dest+=destination_stride;
  }
#endif
}
