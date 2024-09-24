
/*!
***********************************************************************
*  \file
*      quant.c
*
*  \brief
*      Quantization functions
*
*  \author
*      Main contributors (see contributors.h for copyright, address and affiliation details)
*
***********************************************************************
*/

#include "contributors.h"

#include "global.h"
#include "memalloc.h"
#include "block.h"
#include "image.h"
#include "mb_access.h"
#include "transform.h"
#include "quant.h"

int quant_intra_default[16] = {
	6,13,20,28,
	13,20,28,32,
	20,28,32,37,
	28,32,37,42
};

int quant_inter_default[16] = {
	10,14,20,24,
	14,20,24,27,
	20,24,27,30,
	24,27,30,34
};

int quant8_intra_default[64] = {
	6,10,13,16,18,23,25,27,
	10,11,16,18,23,25,27,29,
	13,16,18,23,25,27,29,31,
	16,18,23,25,27,29,31,33,
	18,23,25,27,29,31,33,36,
	23,25,27,29,31,33,36,38,
	25,27,29,31,33,36,38,40,
	27,29,31,33,36,38,40,42
};

int quant8_inter_default[64] = {
	9,13,15,17,19,21,22,24,
	13,13,17,19,21,22,24,25,
	15,17,19,21,22,24,25,27,
	17,19,21,22,24,25,27,28,
	19,21,22,24,25,27,28,30,
	21,22,24,25,27,28,30,32,
	22,24,25,27,28,30,32,33,
	24,25,27,28,30,32,33,35
};

int quant_org[16] = { //to be use if no q matrix is chosen
	16,16,16,16,
	16,16,16,16,
	16,16,16,16,
	16,16,16,16
};

int quant8_org[64] = { //to be use if no q matrix is chosen
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16
};

static void CalculateQuant8x8Param(Slice *currslice);

/*!
***********************************************************************
* \brief
*    Initiate quantization process arrays
***********************************************************************
*/
void init_qp_process(VideoParameters *p_Vid)
{
	int bitdepth_qp_scale = imax(p_Vid->bitdepth_luma_qp_scale,p_Vid->bitdepth_chroma_qp_scale);
	int i;

	// We should allocate memory outside of this process since maybe we will have a change of SPS 
	// and we may need to recreate these. Currently should only support same bitdepth
	if (p_Vid->qp_per_matrix == NULL)
		if ((p_Vid->qp_per_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
			no_mem_exit("init_qp_process: p_Vid->qp_per_matrix");

	if (p_Vid->qp_rem_matrix == NULL)
		if ((p_Vid->qp_rem_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
			no_mem_exit("init_qp_process: p_Vid->qp_rem_matrix");

	for (i = 0; i < MAX_QP + bitdepth_qp_scale + 1; i++)
	{
		p_Vid->qp_per_matrix[i] = i / 6;
		p_Vid->qp_rem_matrix[i] = i % 6;
	}
}

void free_qp_matrices(VideoParameters *p_Vid)
{
	if (p_Vid->qp_per_matrix != NULL)
	{
		free (p_Vid->qp_per_matrix);
		p_Vid->qp_per_matrix = NULL;
	}

	if (p_Vid->qp_rem_matrix != NULL)
	{
		free (p_Vid->qp_rem_matrix);
		p_Vid->qp_rem_matrix = NULL;
	}
}

/*!
************************************************************************
* \brief
*    For mapping the q-matrix to the active id and calculate quantisation values
*
* \param currSlice
*    Slice pointer
* \param pps
*    Picture parameter set
* \param sps
*    Sequence parameter set
*
************************************************************************
*/
void assign_quant_params(Slice *currSlice)
{
	seq_parameter_set_rbsp_t* sps = currSlice->active_sps;
	pic_parameter_set_rbsp_t* pps = currSlice->active_pps;
	int i;
	int n_ScalingList;

	if(!pps->pic_scaling_matrix_present_flag && !sps->seq_scaling_matrix_present_flag)
	{
		for(i=0; i<12; i++)
			currSlice->qmatrix[i] = (i < 6) ? quant_org : quant8_org;
	}
	else
	{
		n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
		if(sps->seq_scaling_matrix_present_flag) // check sps first
		{
			for(i=0; i<n_ScalingList; i++)
			{
				if(i<6)
				{
					if(!sps->seq_scaling_list_present_flag[i]) // fall-back rule A
					{
						if(i==0)
							currSlice->qmatrix[i] = quant_intra_default;
						else if(i==3)
							currSlice->qmatrix[i] = quant_inter_default;
						else
							currSlice->qmatrix[i] = currSlice->qmatrix[i-1];
					}
					else
					{
						if(sps->UseDefaultScalingMatrix4x4Flag[i])
							currSlice->qmatrix[i] = (i<3) ? quant_intra_default : quant_inter_default;
						else
							currSlice->qmatrix[i] = sps->ScalingList4x4[i];
					}
				}
				else
				{
					if(!sps->seq_scaling_list_present_flag[i]) // fall-back rule A
					{
						if(i==6)
							currSlice->qmatrix[i] = quant8_intra_default;
						else if(i==7)
							currSlice->qmatrix[i] = quant8_inter_default;
						else
							currSlice->qmatrix[i] = currSlice->qmatrix[i-2];
					}
					else
					{
						if(sps->UseDefaultScalingMatrix8x8Flag[i-6])
							currSlice->qmatrix[i] = (i==6 || i==8 || i==10) ? quant8_intra_default:quant8_inter_default;
						else
							currSlice->qmatrix[i] = sps->ScalingList8x8[i-6];
					}
				}
			}
		}

		if(pps->pic_scaling_matrix_present_flag) // then check pps
		{
			for(i=0; i<n_ScalingList; i++)
			{
				if(i<6)
				{
					if(!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
					{
						if (i==0)
						{
							if(!sps->seq_scaling_matrix_present_flag)
								currSlice->qmatrix[i] = quant_intra_default;
						}
						else if (i==3)
						{
							if(!sps->seq_scaling_matrix_present_flag)
								currSlice->qmatrix[i] = quant_inter_default;
						}
						else
							currSlice->qmatrix[i] = currSlice->qmatrix[i-1];
					}
					else
					{
						if(pps->UseDefaultScalingMatrix4x4Flag[i])
							currSlice->qmatrix[i] = (i<3) ? quant_intra_default:quant_inter_default;
						else
							currSlice->qmatrix[i] = pps->ScalingList4x4[i];
					}
				}
				else
				{
					if(!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
					{
						if (i==6)
						{
							if(!sps->seq_scaling_matrix_present_flag)
								currSlice->qmatrix[i] = quant8_intra_default;
						}
						else if(i==7)
						{
							if(!sps->seq_scaling_matrix_present_flag)
								currSlice->qmatrix[i] = quant8_inter_default;
						}
						else  
							currSlice->qmatrix[i] = currSlice->qmatrix[i-2];
					}
					else
					{
						if(pps->UseDefaultScalingMatrix8x8Flag[i-6])
							currSlice->qmatrix[i] = (i==6 || i==8 || i==10) ? quant8_intra_default:quant8_inter_default;
						else
							currSlice->qmatrix[i] = pps->ScalingList8x8[i-6];
					}
				}
			}
		}
	}

	CalculateQuant4x4Param(currSlice);
	if(pps->transform_8x8_mode_flag)
		CalculateQuant8x8Param(currSlice);
}

/*!
************************************************************************
* \brief
*    For calculating the quantisation values at frame level
*
************************************************************************
*/
void CalculateQuant4x4Param(Slice *currSlice)
{
	int i, j, k, temp;

	for(k=0; k<6; k++)
	{
		for(i=0; i<4; i++)
		{
			for(j=0; j<4; j++)
			{
				temp = (i<<2)+j;
				currSlice->InvLevelScale4x4_Intra[0][k][i][j] = dequant_coef[k][i][j] * currSlice->qmatrix[0][temp];
				currSlice->InvLevelScale4x4_Intra[1][k][i][j] = dequant_coef[k][i][j] * currSlice->qmatrix[1][temp];
				currSlice->InvLevelScale4x4_Intra[2][k][i][j] = dequant_coef[k][i][j] * currSlice->qmatrix[2][temp];

				currSlice->InvLevelScale4x4_Inter[0][k][i][j] = dequant_coef[k][i][j] * currSlice->qmatrix[3][temp];
				currSlice->InvLevelScale4x4_Inter[1][k][i][j] = dequant_coef[k][i][j] * currSlice->qmatrix[4][temp];
				currSlice->InvLevelScale4x4_Inter[2][k][i][j] = dequant_coef[k][i][j] * currSlice->qmatrix[5][temp];
			}
		}
	}
}

/*!
************************************************************************
* \brief
*    Calculate the quantisation and inverse quantisation parameters
*
************************************************************************
*/
static void CalculateQuant8x8Param(Slice *currSlice)
{
	VideoParameters *p_Vid = currSlice->p_Vid;
	int i, j, k, temp;

	for(k=0; k<6; k++)
	{
		int x = 0;
		for(i=0; i<8; i++)
		{
			for(j=0; j<8; j++)
			{
				temp = (i<<3)+j;
				currSlice->InvLevelScale8x8_Intra[0][k][x] = dequant_coef8[k][x] * currSlice->qmatrix[6][temp];
				currSlice->InvLevelScale8x8_Inter[0][k][x] = dequant_coef8[k][x] * currSlice->qmatrix[7][temp];
				x++;
			}
		}
	}

	if( p_Vid->active_sps->chroma_format_idc == YUV444 )  // 4:4:4
	{
		for(k=0; k<6; k++)
		{
			int x=0;
			for(i=0; i<8; i++)
			{
				for(j=0; j<8; j++)
				{
					temp = (i<<3)+j;
					currSlice->InvLevelScale8x8_Intra[1][k][x] = dequant_coef8[k][x] * currSlice->qmatrix[8][temp];
					currSlice->InvLevelScale8x8_Inter[1][k][x] = dequant_coef8[k][x] * currSlice->qmatrix[9][temp];
					currSlice->InvLevelScale8x8_Intra[2][k][x] = dequant_coef8[k][x] * currSlice->qmatrix[10][temp];
					currSlice->InvLevelScale8x8_Inter[2][k][x] = dequant_coef8[k][x] * currSlice->qmatrix[11][temp];
					x++;
				}
			}
		}
	}
}
