/*!
*************************************************************************************
* \file cabac.c
*
* \brief
*    CABAC entropy coding routines
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Detlev Marpe                    <marpe@hhi.de>
**************************************************************************************
*/

#include "global.h"
#include "cabac.h"
#include "memalloc.h"
#include "elements.h"
#include "image.h"
#include "biaridecod.h"
#include "mb_access.h"
#include "vlc.h"
#include <mmintrin.h>
#define get_bit(x, n) (_mm_cvtsi64_si32(_mm_srli_si64(*(__m64 *)&(x), n)) & 1)
/*static inline int get_bit(int64 x,int n)
{
return (int)(((x >> n) & 1));
}*/

static __forceinline void or_bits_low(int64 *x, int mask, int position)
{
	*(int32_t *)x |= (mask << position);
}

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
#if TRACE
int symbolCount = 0;
#endif

/***********************************************************************
* L O C A L L Y   D E F I N E D   F U N C T I O N   P R O T O T Y P E S
***********************************************************************
*/
static unsigned int unary_bin_decode(DecodingEnvironmentPtr dep_dp,
																		 BiContextTypePtr ctx,
																		 int ctx_offset);
static unsigned int unary_bin_max_decode(DecodingEnvironmentPtr dep_dp,
																				 BiContextTypePtr ctx,
																				 int ctx_offset,
																				 unsigned int max_symbol);

unsigned int unary_exp_golomb_mv_decode(DecodingEnvironmentPtr dep_dp, BiContextTypePtr ctx, unsigned int max_bin);
unsigned int unary_exp_golomb_mv_decode3(DecodingEnvironmentPtr dep_dp, BiContextTypePtr ctx);

void CheckAvailabilityOfNeighborsCABAC(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	PixelPos up, left;

	p_Vid->getNeighbourLeftLuma(currMB, &left);
	p_Vid->getNeighbourUpLuma(currMB, &up);

	if (up.available)
		currMB->mb_up = &p_Vid->mb_data[up.mb_addr];
	else
		currMB->mb_up = NULL;

	if (left.available)
		currMB->mb_left = &p_Vid->mb_data[left.mb_addr];
	else
		currMB->mb_left = NULL;
}

void cabac_new_slice(Slice *currSlice)
{
	currSlice->last_dquant=0;
}

/*!
************************************************************************
* \brief
*    Allocation of contexts models for the motion info
*    used for arithmetic decoding
*
************************************************************************
*/
MotionInfoContexts* create_contexts_MotionInfo(void)
{
	MotionInfoContexts *deco_ctx;

	deco_ctx = (MotionInfoContexts*) calloc(1, sizeof(MotionInfoContexts) );
	if( deco_ctx == NULL )
		no_mem_exit("create_contexts_MotionInfo: deco_ctx");

	return deco_ctx;
}


/*!
************************************************************************
* \brief
*    Allocates of contexts models for the texture info
*    used for arithmetic decoding
************************************************************************
*/
TextureInfoContexts* create_contexts_TextureInfo(void)
{
	TextureInfoContexts *deco_ctx;

	deco_ctx = (TextureInfoContexts*) calloc(1, sizeof(TextureInfoContexts) );
	if( deco_ctx == NULL )
		no_mem_exit("create_contexts_TextureInfo: deco_ctx");

	return deco_ctx;
}


/*!
************************************************************************
* \brief
*    Frees the memory of the contexts models
*    used for arithmetic decoding of the motion info.
************************************************************************
*/
void delete_contexts_MotionInfo(MotionInfoContexts *deco_ctx)
{
	if( deco_ctx == NULL )
		return;

	free( deco_ctx );
}


/*!
************************************************************************
* \brief
*    Frees the memory of the contexts models
*    used for arithmetic decoding of the texture info.
************************************************************************
*/
void delete_contexts_TextureInfo(TextureInfoContexts *deco_ctx)
{
	if( deco_ctx == NULL )
		return;

	free( deco_ctx );
}

Boolean readFieldModeInfo_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp)
{  
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	MotionInfoContexts *ctx  = currSlice->mot_ctx;
	int a = currMB->mb_avail_left ? p_Vid->mb_data[currMB->mb_addr_left].mb_field : 0;
	int b = currMB->mb_avail_up ? p_Vid->mb_data[currMB->mb_addr_up].mb_field : 0;
	int act_ctx = a + b;

	return biari_decode_symbol (dep_dp, &ctx->mb_aff_contexts[act_ctx]);
}


int check_next_mb_and_get_field_mode_CABAC(Slice *currSlice, DataPartition  *act_dp)
{
	VideoParameters *p_Vid = currSlice->p_Vid;
	BiContextTypePtr          mb_type_ctx_copy[3];
	BiContextTypePtr          mb_aff_ctx_copy;
	DecodingEnvironmentPtr    dep_dp_copy;

	int length;
	DecodingEnvironmentPtr    dep_dp = &(act_dp->de_cabac);

	int bframe = (currSlice->slice_type == B_SLICE);
	int skip   = 0;
	int field  = 0;
	int i;

	Macroblock *currMB;

	//get next MB
	++p_Vid->current_mb_nr;

	currMB = &p_Vid->mb_data[p_Vid->current_mb_nr];
	currMB->p_Vid    = p_Vid;
	currMB->p_Slice  = currSlice; 
	currMB->slice_nr = p_Vid->current_slice_nr;
	currMB->mb_field = p_Vid->mb_data[p_Vid->current_mb_nr-1].mb_field;
	currMB->mbAddrX  = p_Vid->current_mb_nr;

	CheckAvailabilityOfNeighbors(currMB);
	CheckAvailabilityOfNeighborsCABAC(currMB);

	//create
	dep_dp_copy = (DecodingEnvironmentPtr) calloc(1, sizeof(DecodingEnvironment) );
	for (i=0;i<3;++i)
		mb_type_ctx_copy[i] = (BiContextTypePtr) calloc(NUM_MB_TYPE_CTX, sizeof(BiContextType) );
	mb_aff_ctx_copy = (BiContextTypePtr) calloc(NUM_MB_AFF_CTX, sizeof(BiContextType) );

	//copy
	memcpy(dep_dp_copy,dep_dp,sizeof(DecodingEnvironment));
	length = *(dep_dp_copy->Dcodestrm_len) = *(dep_dp->Dcodestrm_len);
	for (i=0;i<3;++i)
		memcpy(mb_type_ctx_copy[i], currSlice->mot_ctx->mb_type_contexts[i],NUM_MB_TYPE_CTX*sizeof(BiContextType) );
	memcpy(mb_aff_ctx_copy, currSlice->mot_ctx->mb_aff_contexts,NUM_MB_AFF_CTX*sizeof(BiContextType) );

	//check_next_mb
	currSlice->last_dquant = 0;
	skip = readMB_skip_flagInfo_CABAC(currMB, dep_dp);

	if (!skip)
	{
		field = readFieldModeInfo_CABAC(currMB, dep_dp);
		p_Vid->mb_data[p_Vid->current_mb_nr-1].mb_field = field;
	}

	//reset
	p_Vid->current_mb_nr--;

	memcpy(dep_dp,dep_dp_copy,sizeof(DecodingEnvironment));
	*(dep_dp->Dcodestrm_len) = length;
	for (i=0;i<3;++i)
		memcpy(currSlice->mot_ctx->mb_type_contexts[i],mb_type_ctx_copy[i], NUM_MB_TYPE_CTX*sizeof(BiContextType) );
	memcpy( currSlice->mot_ctx->mb_aff_contexts,mb_aff_ctx_copy,NUM_MB_AFF_CTX*sizeof(BiContextType) );

	CheckAvailabilityOfNeighborsCABAC(currMB);

	//delete
	free(dep_dp_copy);
	for (i=0;i<3;++i)
		free(mb_type_ctx_copy[i]);
	free(mb_aff_ctx_copy);

	return skip;
}




/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the motion
*    vector data of a B-frame MB.
************************************************************************
*/
#if defined(_DEBUG) || !defined(_M_IX86)
int decodeMVD_CABAC(DecodingEnvironmentPtr dep_dp, BiContextType mv_ctx[2][NUM_MV_RES_CTX], int act_ctx, int err)
{
	int act_sym = biari_decode_symbol(dep_dp,&mv_ctx[0][act_ctx+err] );

	if (act_sym != 0)
	{
		int mv_sign;
		act_sym = unary_exp_golomb_mv_decode3(dep_dp,mv_ctx[1]+act_ctx);
		++act_sym;
		mv_sign = biari_decode_symbol_eq_prob(dep_dp);

		if(mv_sign)
			act_sym = -act_sym;
	}
	return act_sym;
}
#else
int decodeMVD_CABAC(DecodingEnvironmentPtr dep_dp, BiContextType mv_ctx[2][NUM_MV_RES_CTX], int act_ctx, int err);
#endif

int readMVD_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int k, int list_idx, int x, int y)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	Slice *currSlice = currMB->p_Slice;
	MotionInfoContexts *ctx = currSlice->mot_ctx;
	int a = 0, b = 0;
//	int act_ctx;
//	int act_sym;
	int mv_local_err;
	int err;

	PixelPos block_a, block_b;

	p_Vid->getNeighbourPXLumaNB_NoPos(currMB, y - 1, &block_b);
	if (block_b.available)
	{
		b = abs(p_Vid->mb_data[block_b.mb_addr].mvd[list_idx][block_b.y>>2][x>>2][k]);
		if (currSlice->mb_aff_frame_flag && (k==1))
		{
			if ((currMB->mb_field==0) && (p_Vid->mb_data[block_b.mb_addr].mb_field==1))
				b *= 2;
			else if ((currMB->mb_field==1) && (p_Vid->mb_data[block_b.mb_addr].mb_field==0))
				b /= 2;
		}
	}

	p_Vid->getNeighbourXPLumaNB_NoPos(currMB, x - 1, y    , &block_a);
	if (block_a.available)
	{
		a = abs(p_Vid->mb_data[block_a.mb_addr].mvd[list_idx][block_a.y>>2][block_a.x>>2][k]);
		if (currSlice->mb_aff_frame_flag && (k==1))
		{
			if ((currMB->mb_field==0) && (p_Vid->mb_data[block_a.mb_addr].mb_field==1))
				a *= 2;
			else if ((currMB->mb_field==1) && (p_Vid->mb_data[block_a.mb_addr].mb_field==0))
				a /= 2;
		}
	}

	if ((mv_local_err = a + b)<3)
		err = 0;
	else
	{
		if (mv_local_err > 32)
			err = 3;
		else
			err = 2;
	}

	return decodeMVD_CABAC(dep_dp, ctx->mv_res_contexts, 5*k, err);
	/*
	act_sym = biari_decode_symbol(dep_dp,&ctx->mv_res_contexts[0][act_ctx] );

	if (act_sym != 0)
	{
		int mv_sign;
		act_ctx = 5 * k;
		act_sym = unary_exp_golomb_mv_decode3(dep_dp,ctx->mv_res_contexts[1]+act_ctx);
		++act_sym;
		mv_sign = biari_decode_symbol_eq_prob(dep_dp);

		if(mv_sign)
			act_sym = -act_sym;
	}
	return act_sym;
	*/
}


/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the 8x8 block type.
************************************************************************
*/
int readB8_typeInfo_CABAC(Slice *currSlice, DecodingEnvironmentPtr dep_dp)
{
	int act_sym = 0;
	int bframe  = (currSlice->slice_type == B_SLICE);

	MotionInfoContexts *ctx = currSlice->mot_ctx;


	if (!bframe)
	{
		if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[0][1]))
		{
			act_sym = 0;
		}
		else
		{
			if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[0][3]))
			{
				if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[0][4])) act_sym = 2;
				else                                                            act_sym = 3;
			}
			else
			{
				act_sym = 1;
			}
		}
	}
	else
	{
		if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][0]))
		{
			if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][1]))
			{
				if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][2]))
				{
					if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][3]))
					{
						act_sym = 10;
						if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][3])) act_sym++;
					}
					else
					{
						act_sym = 6;
						if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][3])) act_sym+=2;
						if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][3])) act_sym++;
					}
				}
				else
				{
					act_sym=2;
					if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][3])) act_sym+=2;
					if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][3])) act_sym+=1;
				}
			}
			else
			{
				if (biari_decode_symbol (dep_dp, &ctx->b8_type_contexts[1][3])) act_sym = 1;
				else                                                            act_sym = 0;
			}
			++act_sym;
		}
		else
		{
			act_sym= 0;
		}
	}
	return act_sym;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the macroblock
*    type info of a given MB.
************************************************************************
*/
#if defined(_DEBUG) || !defined(_M_IX86)
int readMB_skip_flagInfo_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp)
{  
	Slice *currSlice = currMB->p_Slice;
	int bframe=(currSlice->slice_type == B_SLICE);
	MotionInfoContexts *ctx = currSlice->mot_ctx;  
	int a = (currMB->mb_left != NULL) ? (currMB->mb_left->skip_flag == 0) : 0;
	int b = (currMB->mb_up   != NULL) ? (currMB->mb_up  ->skip_flag == 0) : 0;
	int act_ctx;
	int skip;

	if (bframe)
	{
		act_ctx = 7 + a + b;

		skip = biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][act_ctx]);
	}
	else
	{
		act_ctx = a + b;

		skip = biari_decode_symbol(dep_dp, &ctx->mb_type_contexts[1][act_ctx]);
	}

	if (skip)
	{
		currSlice->last_dquant = 0;
	}
	return skip;
}
#endif

/*!
***************************************************************************
* \brief
*    This function is used to arithmetically decode the macroblock
*    intra_pred_size flag info of a given MB.
***************************************************************************
*/

Boolean readMB_transform_size_flag_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp)
{
	Slice *currSlice = currMB->p_Slice;
	TextureInfoContexts*ctx = currSlice->tex_ctx;

	int b = (currMB->mb_up   == NULL) ? 0 : currMB->mb_up->luma_transform_size_8x8_flag;
	int a = (currMB->mb_left == NULL) ? 0 : currMB->mb_left->luma_transform_size_8x8_flag;

	int act_ctx = a + b;
	int act_sym = biari_decode_symbol(dep_dp, ctx->transform_size_contexts + act_ctx);

	return act_sym;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the macroblock
*    type info of a given MB.
************************************************************************
*/
int readMB_typeInfo_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp)
{
	Slice *currSlice = currMB->p_Slice;
	MotionInfoContexts *ctx = currSlice->mot_ctx;

	int a = 0, b = 0;
	int act_ctx;
	int act_sym;
	int bframe=(currSlice->slice_type == B_SLICE);
	int mode_sym;
	int curr_mb_type;

	if(currSlice->slice_type == I_SLICE)  // INTRA-frame
	{
		if (currMB->mb_up != NULL)
			b = (((currMB->mb_up)->mb_type != I4MB && currMB->mb_up->mb_type != I8MB) ? 1 : 0 );

		if (currMB->mb_left != NULL)
			a = (((currMB->mb_left)->mb_type != I4MB && currMB->mb_left->mb_type != I8MB) ? 1 : 0 );

		act_ctx = a + b;
		act_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx);

		if (act_sym==0) // 4x4 Intra
		{
			curr_mb_type = act_sym;
		}
		else // 16x16 Intra
		{
			mode_sym = biari_decode_final(dep_dp);
			if(mode_sym == 1)
			{
				curr_mb_type = 25;
			}
			else
			{
				act_sym = 1;
				act_ctx = 4;
				mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx ); // decoding of AC/no AC
				act_sym += mode_sym*12;
				act_ctx = 5;
				// decoding of cbp: 0,1,2
				mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
				if (mode_sym!=0)
				{
					act_ctx=6;
					mode_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
					act_sym+=4;
					if (mode_sym!=0)
						act_sym+=4;
				}
				// decoding of I pred-mode: 0,1,2,3
				act_ctx = 7;
				mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
				act_sym += mode_sym*2;
				act_ctx = 8;
				mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
				act_sym += mode_sym;
				curr_mb_type = act_sym;
			}
		}
	}
	else if(currSlice->slice_type == SI_SLICE)  // SI-frame
	{
		// special ctx's for SI4MB
		if (currMB->mb_up != NULL)
			b = (( (currMB->mb_up)->mb_type != SI4MB) ? 1 : 0 );

		if (currMB->mb_left != NULL)
			a = (( (currMB->mb_left)->mb_type != SI4MB) ? 1 : 0 );

		act_ctx = a + b;
		act_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[1] + act_ctx);

		if (act_sym==0) //  SI 4x4 Intra
		{
			curr_mb_type = 0;
		}
		else // analog INTRA_IMG
		{
			if (currMB->mb_up != NULL)
				b = (( (currMB->mb_up)->mb_type != I4MB) ? 1 : 0 );

			if (currMB->mb_left != NULL)
				a = (( (currMB->mb_left)->mb_type != I4MB) ? 1 : 0 );

			act_ctx = a + b;
			act_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx);

			if (act_sym==0) // 4x4 Intra
			{
				curr_mb_type = 1;
			}
			else // 16x16 Intra
			{
				mode_sym = biari_decode_final(dep_dp);
				if( mode_sym==1 )
				{
					curr_mb_type = 26;
				}
				else
				{
					act_sym = 2;
					act_ctx = 4;
					mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx ); // decoding of AC/no AC
					act_sym += mode_sym*12;
					act_ctx = 5;
					// decoding of cbp: 0,1,2
					mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
					if (mode_sym!=0)
					{
						act_ctx=6;
						mode_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
						act_sym+=4;
						if (mode_sym!=0)
							act_sym+=4;
					}
					// decoding of I pred-mode: 0,1,2,3
					act_ctx = 7;
					mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
					act_sym += mode_sym*2;
					act_ctx = 8;
					mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[0] + act_ctx );
					act_sym += mode_sym;
					curr_mb_type = act_sym;
				}
			}
		}
	}
	else
	{
		if (bframe)
		{
			if (currMB->mb_up != NULL)
				b = (( (currMB->mb_up)->mb_type != 0) ? 1 : 0 );

			if (currMB->mb_left != NULL)
				a = (( (currMB->mb_left)->mb_type != 0) ? 1 : 0 );

			act_ctx = a + b;

			if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][act_ctx]))
			{
				if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][4]))
				{
					if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][5]))
					{
						act_sym=12;
						if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym+=8;
						if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym+=4;
						if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym+=2;

						if      (act_sym==24)  act_sym=11;
						else if (act_sym==26)  act_sym=22;
						else
						{
							if (act_sym==22)     act_sym=23;
							if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym+=1;
						}
					}
					else
					{
						act_sym=3;
						if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym+=4;
						if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym+=2;
						if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym+=1;
					}
				}
				else
				{
					if (biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][6])) act_sym=2;
					else                                                            act_sym=1;
				}
			}
			else
			{
				act_sym = 0;
			}
		}
		else // P-frame
		{
			{
				if (biari_decode_symbol(dep_dp, &ctx->mb_type_contexts[1][4] ))
				{
					if (biari_decode_symbol(dep_dp, &ctx->mb_type_contexts[1][7] ))   act_sym = 7;
					else                                                              act_sym = 6;
				}
				else
				{
					if (biari_decode_symbol(dep_dp, &ctx->mb_type_contexts[1][5] ))
					{
						if (biari_decode_symbol(dep_dp, &ctx->mb_type_contexts[1][7] )) act_sym = 2;
						else                                                            act_sym = 3;
					}
					else
					{
						if (biari_decode_symbol(dep_dp, &ctx->mb_type_contexts[1][6] )) act_sym = 4;
						else                                                            act_sym = 1;
					}
				}
			}
		}

		if (act_sym<=6 || (((currSlice->slice_type == B_SLICE) ? 1 : 0) && act_sym<=23))
		{
			curr_mb_type = act_sym;
		}
		else  // additional info for 16x16 Intra-mode
		{
			mode_sym = biari_decode_final(dep_dp);
			if( mode_sym==1 )
			{
				if(bframe)  // B frame
					curr_mb_type = 48;
				else      // P frame
					curr_mb_type = 31;
			}
			else
			{
				act_ctx = 8;
				mode_sym =  biari_decode_symbol(dep_dp, ctx->mb_type_contexts[1] + act_ctx ); // decoding of AC/no AC
				act_sym += mode_sym*12;

				// decoding of cbp: 0,1,2
				act_ctx = 9;
				mode_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[1] + act_ctx );
				if (mode_sym != 0)
				{
					act_sym+=4;
					mode_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[1] + act_ctx );
					if (mode_sym != 0)
						act_sym+=4;
				}

				// decoding of I pred-mode: 0,1,2,3
				act_ctx = 10;
				mode_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[1] + act_ctx );
				act_sym += mode_sym*2;
				mode_sym = biari_decode_symbol(dep_dp, ctx->mb_type_contexts[1] + act_ctx );
				act_sym += mode_sym;
				curr_mb_type = act_sym;
			}
		}
	}
	return curr_mb_type;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode a pair of
*    intra prediction modes of a given MB.
************************************************************************
*/
#if defined(_DEBUG) || !defined(_M_IX86)
int readIntraPredMode_CABAC(Slice *currSlice, DecodingEnvironmentPtr dep_dp)
{
	TextureInfoContexts *ctx     = currSlice->tex_ctx;
	int act_sym;

	// use_most_probable_mode
	act_sym = biari_decode_symbol(dep_dp, ctx->ipr_contexts);

	// remaining_mode_selector
	if (act_sym == 1)
	{
		return -1;
	}
	else
	{
		int pred_mode=0;
		pred_mode |= (biari_decode_symbol(dep_dp, ctx->ipr_contexts+1)     );
		pred_mode |= (biari_decode_symbol(dep_dp, ctx->ipr_contexts+1) << 1);
		pred_mode |= (biari_decode_symbol(dep_dp, ctx->ipr_contexts+1) << 2);
		return pred_mode;
	}
}
#endif
/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the reference
*    parameter of a given MB.
************************************************************************
*/
char readRefFrame_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int list, int x, int y)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	MotionInfoContexts *ctx = currSlice->mot_ctx;
	Macroblock *neighborMB = NULL;

	int   addctx  = 0;
	int   a = 0, b = 0;
	int   act_ctx;
	int   act_sym;
	PicMotion **refframe_array = dec_picture->motion.motion[list];

	PixelPos block_a, block_b;

	p_Vid->getNeighbourPXLuma(currMB, x,     y - 1, &block_b);
	// TODO: this gets called with x << 2 and y << 2, so we can undo the internal >> 2 easily by just passing x and y
	if (block_b.available)
	{
		int b8b=((block_b.x >> 3) & 0x01)+((block_b.y>>2) & 0x02); 
		neighborMB = &p_Vid->mb_data[block_b.mb_addr];
		if (!( (neighborMB->mb_type==IPCM) || IS_DIRECT(neighborMB) || (neighborMB->b8mode[b8b]==0 && neighborMB->b8pdir[b8b]==2)))
		{
			if (currSlice->mb_aff_frame_flag && (currMB->mb_field == FALSE) && (neighborMB->mb_field == TRUE))
				b = (refframe_array[block_b.pos_y>>2][block_b.pos_x>>2].ref_idx > 1 ? 2 : 0);
			else
				b = (refframe_array[block_b.pos_y>>2][block_b.pos_x>>2].ref_idx > 0 ? 2 : 0);
		}
	}

	p_Vid->getNeighbourXPLuma(currMB, x - 1, y    , &block_a);
	if (block_a.available)
	{    
		int b8a=((block_a.x >> 3) & 0x01)+((block_a.y>>2) & 0x02);    
		neighborMB = &p_Vid->mb_data[block_a.mb_addr];
		if (!((neighborMB->mb_type==IPCM) || IS_DIRECT(neighborMB) || (neighborMB->b8mode[b8a]==0 && neighborMB->b8pdir[b8a]==2)))
		{
			if (currSlice->mb_aff_frame_flag && (currMB->mb_field == FALSE) && (neighborMB->mb_field == 1))
				a = (refframe_array[block_a.pos_y>>2][block_a.pos_x>>2].ref_idx > 1 ? 1 : 0);
			else
				a = (refframe_array[block_a.pos_y>>2][block_a.pos_x>>2].ref_idx > 0 ? 1 : 0);
		}
	}

	act_ctx = a + b;

	act_sym = biari_decode_symbol(dep_dp,ctx->ref_no_contexts[addctx] + act_ctx );

	if (act_sym != 0)
	{
		act_ctx = 4;
		act_sym = unary_bin_decode(dep_dp,ctx->ref_no_contexts[addctx] + act_ctx,1);
		++act_sym;
	}
	return act_sym;
}

// x == 0
char readRefFrame_CABAC0(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int list, int y)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	MotionInfoContexts *ctx = currSlice->mot_ctx;
	Macroblock *neighborMB = NULL;

	int   addctx  = 0;
	int   a = 0, b = 0;
	int   act_ctx;
	int   act_sym;
	PicMotion **refframe_array = dec_picture->motion.motion[list];

	PixelPos block_a, block_b;

	p_Vid->getNeighbour0XLuma(currMB, y - 1, &block_b);
	// TODO: this gets called with x << 2 and y << 2, so we can undo the internal >> 2 easily by just passing x and y
	if (block_b.available)
	{
		int b8b=0+((block_b.y>>2) & 0x02); 
		neighborMB = &p_Vid->mb_data[block_b.mb_addr];
		if (!( (neighborMB->mb_type==IPCM) || IS_DIRECT(neighborMB) || (neighborMB->b8mode[b8b]==0 && neighborMB->b8pdir[b8b]==2)))
		{
			if (currSlice->mb_aff_frame_flag && (currMB->mb_field == FALSE) && (neighborMB->mb_field == TRUE))
				b = (refframe_array[block_b.pos_y>>2][block_b.pos_x>>2].ref_idx > 1 ? 2 : 0);
			else
				b = (refframe_array[block_b.pos_y>>2][block_b.pos_x>>2].ref_idx > 0 ? 2 : 0);
		}
	}

	p_Vid->getNeighbourNXLuma(currMB, y    , &block_a);
	if (block_a.available)
	{    
		int b8a=((15 >> 3) & 0x01)+((block_a.y>>2) & 0x02);    
		neighborMB = &p_Vid->mb_data[block_a.mb_addr];
		if (!((neighborMB->mb_type==IPCM) || IS_DIRECT(neighborMB) || (neighborMB->b8mode[b8a]==0 && neighborMB->b8pdir[b8a]==2)))
		{
			if (currSlice->mb_aff_frame_flag && (currMB->mb_field == FALSE) && (neighborMB->mb_field == 1))
				a = (refframe_array[block_a.pos_y>>2][block_a.pos_x>>2].ref_idx > 1 ? 1 : 0);
			else
				a = (refframe_array[block_a.pos_y>>2][block_a.pos_x>>2].ref_idx > 0 ? 1 : 0);
		}
	}

	act_ctx = a + b;

	act_sym = biari_decode_symbol(dep_dp,ctx->ref_no_contexts[addctx] + act_ctx );

	if (act_sym != 0)
	{
		act_ctx = 4;
		act_sym = unary_bin_decode(dep_dp,ctx->ref_no_contexts[addctx] + act_ctx,1);
		++act_sym;
	}
	return act_sym;
}


/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the delta qp
*     of a given MB.
************************************************************************
*/
#if defined(_DEBUG) || !defined(_M_IX86)
short readDquant_CABAC(Slice *currSlice, DecodingEnvironmentPtr dep_dp)
{
	MotionInfoContexts *ctx = currSlice->mot_ctx;
	short dquant;
	int act_ctx = ((currSlice->last_dquant != 0) ? 1 : 0);
	int act_sym = biari_decode_symbol(dep_dp,ctx->delta_qp_contexts + act_ctx );

	if (act_sym != 0)
	{
		act_ctx = 2;
		act_sym = unary_bin_decode(dep_dp,ctx->delta_qp_contexts + act_ctx,1);
		++act_sym;
	}

	dquant = (act_sym + 1) >> 1;
	if((act_sym & 0x01)==0)                           // lsb is signed bit
		dquant = -dquant;

	currSlice->last_dquant = dquant;
	return dquant;
}
#endif
/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the coded
*    block pattern of a given MB.
************************************************************************
*/
int readCBP_CABAC(Macroblock *currMB,  DecodingEnvironmentPtr dep_dp)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	Slice *currSlice = currMB->p_Slice;
	TextureInfoContexts *ctx = currSlice->tex_ctx;  
	Macroblock *neighborMB = NULL;

	int a, b;
	int curr_cbp_ctx;
	int cbp = 0;
	int cbp_bit;
	PixelPos block_a;

	//  coding of luma part (bit by bit)
	neighborMB = currMB->mb_up;
	b = 0;

	if (neighborMB != NULL)
	{
		if(neighborMB->mb_type!=IPCM)
			b = (( (neighborMB->cbp & 4) == 0) ? 2 : 0);
	}

	p_Vid->getNeighbourLeftLuma(currMB, &block_a);
	if (block_a.available)
	{
		if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
			a = 0;
		else
			a = (( (p_Vid->mb_data[block_a.mb_addr].cbp & (1<<(2*(block_a.y>>3)+1))) == 0) ? 1 : 0);
	}
	else
		a=0;

	curr_cbp_ctx = a + b;
	cbp_bit = biari_decode_symbol(dep_dp, ctx->cbp_contexts[0] + curr_cbp_ctx );
	//if (cbp_bit) 
		cbp += cbp_bit;//1;

	if (neighborMB != NULL)
	{
		if(neighborMB->mb_type!=IPCM)
			b = (( (neighborMB->cbp & 8) == 0) ? 2 : 0);
	}

	a = ( ((cbp & 1) == 0) ? 1: 0);

	curr_cbp_ctx = a + b;

	cbp_bit = biari_decode_symbol(dep_dp, ctx->cbp_contexts[0] + curr_cbp_ctx );
	//if (cbp_bit) 
	cbp += (cbp_bit << 1); //2;

	b = ( ((cbp & 1) == 0) ? 2: 0);

	p_Vid->getNeighbourNPLumaNB(currMB, 8, &block_a);
	if (block_a.available)
	{
		if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
			a = 0;
		else
			a = (( (p_Vid->mb_data[block_a.mb_addr].cbp & (1<<(2*(block_a.y>>3)+1))) == 0) ? 1 : 0);
	}
	else
		a=0;

	curr_cbp_ctx = a + b;
	cbp_bit = biari_decode_symbol(dep_dp, ctx->cbp_contexts[0] + curr_cbp_ctx );
	//if (cbp_bit) 
		cbp += (cbp_bit << 2); //4;

	b = ( ((cbp & 2) == 0) ? 2: 0);
	a = ( ((cbp & 4) == 0) ? 1: 0);

	curr_cbp_ctx = a + b;
	cbp_bit = biari_decode_symbol(dep_dp, ctx->cbp_contexts[0] + curr_cbp_ctx );
	//if (cbp_bit) 
			cbp += (cbp_bit << 3); //8;

	if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444)) 
	{
		// coding of chroma part
		// CABAC decoding for BinIdx 0
		b = 0;
		neighborMB = currMB->mb_up;
		if (neighborMB != NULL)
		{
			if (neighborMB->mb_type==IPCM || (neighborMB->cbp > 15))
				b = 2;
		}

		a = 0;
		neighborMB = currMB->mb_left;
		if (neighborMB != NULL)
		{
			if (neighborMB->mb_type==IPCM || (neighborMB->cbp > 15))
				a = 1;
		}

		curr_cbp_ctx = a + b;
		cbp_bit = biari_decode_symbol(dep_dp, ctx->cbp_contexts[1] + curr_cbp_ctx );

		// CABAC decoding for BinIdx 1
		if (cbp_bit) // set the chroma bits
		{
			b = 0;
			neighborMB = currMB->mb_up;
			if (neighborMB != NULL)
			{
				//if ((neighborMB->mb_type == IPCM) || ((neighborMB->cbp > 15) && ((neighborMB->cbp >> 4) == 2)))
				if ((neighborMB->mb_type == IPCM) || ((neighborMB->cbp >> 4) == 2))
					b = 2;
			}


			a = 0;
			neighborMB = currMB->mb_left;
			if (neighborMB != NULL)
			{
				if ((neighborMB->mb_type == IPCM) || ((neighborMB->cbp >> 4) == 2))
					a = 1;
			}

			curr_cbp_ctx = a + b;
			cbp_bit = biari_decode_symbol(dep_dp, ctx->cbp_contexts[2] + curr_cbp_ctx );
			cbp += (16 << cbp_bit); //  ? 32 : 16;
		}
	}


	if (!cbp)
	{
		currSlice->last_dquant = 0;
	}

	return cbp;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the chroma
*    intra prediction mode of a given MB.
************************************************************************
*/
char readCIPredMode_CABAC(Macroblock *currMB, 
													DecodingEnvironmentPtr dep_dp)
{
	Slice *currSlice = currMB->p_Slice;
	TextureInfoContexts *ctx = currSlice->tex_ctx;
	int act_sym;

	Macroblock          *MbUp   = currMB->mb_up;
	Macroblock          *MbLeft = currMB->mb_left;

	int b = (MbUp != NULL)   ? (((MbUp->c_ipred_mode   != 0) && (MbUp->mb_type != IPCM)) ? 1 : 0) : 0;
	int a = (MbLeft != NULL) ? (((MbLeft->c_ipred_mode != 0) && (MbLeft->mb_type != IPCM)) ? 1 : 0) : 0;
	int act_ctx = a + b;

	act_sym = biari_decode_symbol(dep_dp, ctx->cipr_contexts + act_ctx );

	if (act_sym != 0)
		act_sym = unary_bin_max_decode(dep_dp, ctx->cipr_contexts + 3, 0, 1) + 1;
	return act_sym;

}

static const byte maxpos       [] = {15, 14, 63, 31, 31, 15,  3, 14,  7, 15, 15, 14, 63, 31, 31, 15, 15, 14, 63, 31, 31, 15};
static const byte c1isdc       [] = { 1,  0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1};
static const byte type2ctx_bcbp[] = { 0,  1,  2,  3,  3,  4,  5,  6,  5,  5, 10, 11, 12, 13, 13, 14, 16, 17, 18, 19, 19, 20};
static const byte type2ctx_map [] = { 0,  1,  2,  3,  4,  5,  6,  7,  6,  6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21}; // 8
static const byte type2ctx_last[] = { 0,  1,  2,  3,  4,  5,  6,  7,  6,  6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21}; // 8
static const byte type2ctx_one [] = { 0,  1,  2,  3,  3,  4,  5,  6,  5,  5, 10, 11, 12, 13, 13, 14, 16, 17, 18, 19, 19, 20}; // 7
static const byte type2ctx_abs [] = { 0,  1,  2,  3,  3,  4,  5,  6,  5,  5, 10, 11, 12, 13, 13, 14, 16, 17, 18, 19, 19, 20}; // 7
static const byte max_c2       [] = { 4,  4,  4,  4,  4,  4,  3,  4,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4}; // 9



/*!
************************************************************************
* \brief
*    Read CBP4-BIT
************************************************************************
*/
static int read_and_store_CBP_block_bit_444(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int type)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	TextureInfoContexts *tex_ctx = currSlice->tex_ctx;

	int y_ac        = (type==LUMA_16AC || type==LUMA_8x8 || type==LUMA_8x4 || type==LUMA_4x8 || type==LUMA_4x4
		|| type==CB_16AC || type==CB_8x8 || type==CB_8x4 || type==CB_4x8 || type==CB_4x4
		|| type==CR_16AC || type==CR_8x8 || type==CR_8x4 || type==CR_4x8 || type==CR_4x4);
	int y_dc        = (type==LUMA_16DC || type==CB_16DC || type==CR_16DC); 
	int u_ac        = (type==CHROMA_AC && !currMB->is_v_block);
	int v_ac        = (type==CHROMA_AC &&  currMB->is_v_block);
	int chroma_dc   = (type==CHROMA_DC || type==CHROMA_DC_2x4 || type==CHROMA_DC_4x4);
	int u_dc        = (chroma_dc && !currMB->is_v_block);
	int v_dc        = (chroma_dc &&  currMB->is_v_block);
	int j           = (y_ac || u_ac || v_ac ? currMB->subblock_y : 0);
	int i           = (y_ac || u_ac || v_ac ? currMB->subblock_x : 0);
	int bit         = (y_dc ? 0 : y_ac ? 1 : u_dc ? 17 : v_dc ? 18 : u_ac ? 19 : 35);
	int default_bit = (currMB->is_intra_block ? 1 : 0);
	int upper_bit   = default_bit;
	int left_bit    = default_bit;
	int cbp_bit     = 1;  // always one for 8x8 mode
	int ctx;
	int bit_pos_a   = 0;
	int bit_pos_b   = 0;

	PixelPos block_a, block_b;
	if (y_ac)
	{
		get4x4NeighbourLuma(currMB, i - 1, j    , &block_a);
		get4x4NeighbourLuma(currMB, i    , j - 1,  &block_b);
		if (block_a.available)
			bit_pos_a = 4*block_a.y + block_a.x;
		if (block_b.available)
			bit_pos_b = 4*block_b.y + block_b.x;
	}
	else if (y_dc)
	{
		get4x4NeighbourLuma(currMB, i - 1, j    , &block_a);
		get4x4NeighbourLuma(currMB, i    , j - 1, &block_b);
	}
	else if (u_ac||v_ac)
	{
		get4x4Neighbour(currMB, i - 1, j    , p_Vid->mb_size[IS_CHROMA], &block_a);
		get4x4Neighbour(currMB, i    , j - 1, p_Vid->mb_size[IS_CHROMA], &block_b);
		if (block_a.available)
			bit_pos_a = 4*block_a.y + block_a.x;
		if (block_b.available)
			bit_pos_b = 4*block_b.y + block_b.x;
	}
	else
	{
		get4x4Neighbour(currMB, i - 1, j    , p_Vid->mb_size[IS_CHROMA], &block_a);
		get4x4Neighbour(currMB, i    , j - 1, p_Vid->mb_size[IS_CHROMA], &block_b);
	}

	if (dec_picture->chroma_format_idc!=YUV444)
	{
		if (type!=LUMA_8x8)
		{
			//--- get bits from neighboring blocks ---
			if (block_b.available)
			{
				if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
					upper_bit=1;
				else
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[0], bit + bit_pos_b);
			}

			if (block_a.available)
			{
				if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
					left_bit=1;
				else
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[0],bit + bit_pos_a);
			}


			ctx = 2 * upper_bit + left_bit;     
			//===== encode symbol =====
			cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]] + ctx);
		}
	}
	else if( IS_INDEPENDENT(p_Vid) )
	{
		if (type!=LUMA_8x8)
		{
			//--- get bits from neighbouring blocks ---
			if (block_b.available)
			{
				if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
					upper_bit = 1;
				else
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[0],bit+bit_pos_b);
			}


			if (block_a.available)
			{
				if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
					left_bit = 1;
				else
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[0],bit+bit_pos_a);
			}


			ctx = 2 * upper_bit + left_bit;     
			//===== encode symbol =====
			cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]] + ctx);
		}
	}
	else {
		if (block_b.available)
		{
			if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
				upper_bit=1;
			else
			{
				if(type==LUMA_8x8)
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits_8x8[0], bit + bit_pos_b);
				else if (type==CB_8x8)
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits_8x8[1], bit + bit_pos_b);
				else if (type==CR_8x8)
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits_8x8[2], bit + bit_pos_b);
				else if ((type==CB_4x4)||(type==CB_4x8)||(type==CB_8x4)||(type==CB_16AC)||(type==CB_16DC))
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[1],bit+bit_pos_b);
				else if ((type==CR_4x4)||(type==CR_4x8)||(type==CR_8x4)||(type==CR_16AC)||(type==CR_16DC))
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[2],bit+bit_pos_b);
				else
					upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[0],bit+bit_pos_b);
			}
		}


		if (block_a.available)
		{
			if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
				left_bit=1;
			else
			{
				if(type==LUMA_8x8)
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits_8x8[0],bit+bit_pos_a);
				else if (type==CB_8x8)
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits_8x8[1],bit+bit_pos_a);
				else if (type==CR_8x8)
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits_8x8[2],bit+bit_pos_a);
				else if ((type==CB_4x4)||(type==CB_4x8)||(type==CB_8x4)||(type==CB_16AC)||(type==CB_16DC))
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[1],bit+bit_pos_a);
				else if ((type==CR_4x4)||(type==CR_4x8)||(type==CR_8x4)||(type==CR_16AC)||(type==CR_16DC))
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[2],bit+bit_pos_a);
				else
					left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[0],bit+bit_pos_a);
			}
		}

		ctx = 2 * upper_bit + left_bit;
		//===== encode symbol =====
		cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]] + ctx);
	}

	//--- set bits for current block ---
	bit = (y_dc ? 0 : y_ac ? 1 + j + (i >> 2) : u_dc ? 17 : v_dc ? 18 : u_ac ? 19 + j + (i >> 2) : 35 + j + (i >> 2)); 

	if (cbp_bit)
	{  
		if (type==LUMA_8x8) 
		{      
			currMB->cbp_bits[0] |= ((int64) 0x33 << bit   );

			if (dec_picture->chroma_format_idc==YUV444)
			{
				currMB->cbp_bits_8x8[0]   |= ((int64) 0x33 << bit   );
			}
		}
		else if (type==CB_8x8)
		{
			currMB->cbp_bits_8x8[1]   |= ((int64) 0x33 << bit   );      
			currMB->cbp_bits[1]   |= ((int64) 0x33 << bit   );
		}
		else if (type==CR_8x8)
		{
			currMB->cbp_bits_8x8[2]   |= ((int64) 0x33 << bit   );      
			currMB->cbp_bits[2]   |= ((int64) 0x33 << bit   );
		}
		else if (type==LUMA_8x4)
		{
			currMB->cbp_bits[0]   |= ((int64) 0x03 << bit   );
		}
		else if (type==CB_8x4)
		{
			currMB->cbp_bits[1]   |= ((int64) 0x03 << bit   );
		}
		else if (type==CR_8x4)
		{
			currMB->cbp_bits[2]   |= ((int64) 0x03 << bit   );
		}
		else if (type==LUMA_4x8)
		{
			currMB->cbp_bits[0]   |= ((int64) 0x11<< bit   );
		}
		else if (type==CB_4x8)
		{
			currMB->cbp_bits[1]   |= ((int64)0x11<< bit   );
		}
		else if (type==CR_4x8)
		{
			currMB->cbp_bits[2]   |= ((int64)0x11<< bit   );
		}
		else if ((type==CB_4x4)||(type==CB_16AC)||(type==CB_16DC))
		{
			currMB->cbp_bits[1]   |= ((int64)0x01<<bit);
		}
		else if ((type==CR_4x4)||(type==CR_16AC)||(type==CR_16DC))
		{
			currMB->cbp_bits[2]   |= ((int64)0x01<<bit);
		}
		else
		{
			currMB->cbp_bits[0]   |= ((int64)0x01<<bit);
		}
	}
	return cbp_bit;
}



/*!
************************************************************************
* \brief
*    Read CBP4-BIT
************************************************************************
*/
static int read_and_store_CBP_block_bit_normal(Macroblock *currMB, DecodingEnvironmentPtr  dep_dp, int type)
{
	Slice *currSlice = currMB->p_Slice;
	VideoParameters *p_Vid = currMB->p_Vid;
	TextureInfoContexts *tex_ctx = currSlice->tex_ctx;
	int cbp_bit     = 1;  // always one for 8x8 mode

	if (type==LUMA_16DC)
	{

		int upper_bit   = 1;
		int left_bit    = 1;
		int ctx;

		PixelPos block_a, block_b;

		//--- get bits from neighboring blocks ---
		p_Vid->getNeighbour0X(currMB,  -1, p_Vid->mb_size[IS_LUMA], &block_b);
		if (block_b.available)
		{
			if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
				upper_bit=1;
			else
				upper_bit = (int)p_Vid->mb_data[block_b.mb_addr].cbp_bits[0]&1;
		}

		p_Vid->getNeighbourX0(currMB, -1, p_Vid->mb_size[IS_LUMA], &block_a);
		if (block_a.available)
		{
			if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
				left_bit=1;
			else
				left_bit = (int)p_Vid->mb_data[block_a.mb_addr].cbp_bits[0]&1;
		}

		ctx = 2 * upper_bit + left_bit;     
		//===== encode symbol =====
		cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[LUMA_16DC]] + ctx);

		//--- set bits for current block ---

		if (cbp_bit)
		{  
			currMB->cbp_bits[0]   |= 0x01;
		}
	}
	else if (type == LUMA_8x8) 
	{
		int j           = currMB->subblock_y;
		int i           = currMB->subblock_x;

		//--- set bits for current block ---
		int bit = 1 + j + (i >> 2); 

		or_bits(&currMB->cbp_bits[0], 0x33, bit);
	}
	else if (type <= LUMA_4x4) // type==LUMA_16AC ||  type==LUMA_8x4 || type==LUMA_4x8 || type==LUMA_4x4)
	{
		int j           = currMB->subblock_y;
		int i           = currMB->subblock_x;
		int bit;
		int default_bit = (currMB->is_intra_block ? 1 : 0);
		int upper_bit   = default_bit;
		int left_bit    = default_bit;
		int ctx;

		//--- get bits from neighboring blocks ---
		PixelPos block_a, block_b;
		p_Vid->getNeighbourPXLumaNB_NoPos(currMB, j-1, &block_b);
		if (block_b.available)
		{
			int bit_pos_b = (block_b.y&((short)~3)) + (i>>2);   
			if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
				upper_bit=1;
			else
				upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[0], 1 + bit_pos_b);
		}

		p_Vid->getNeighbourXPLumaNB_NoPos(currMB, i-1, j, &block_a);
		if (block_a.available)
		{
			int bit_pos_a = (block_a.y&((short)~3)) + (block_a.x>>2);
			if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
				left_bit=1;
			else
				left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[0],1 + bit_pos_a);
		}

		ctx = 2 * upper_bit + left_bit;     
		//===== encode symbol =====
		cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]] + ctx);


		//--- set bits for current block ---
		bit = 1 + j + (i >> 2); 

		if (cbp_bit)
		{  
			if (type==LUMA_8x4)
			{
				or_bits_low(&currMB->cbp_bits[0], 0x03, bit);
			}
			else if (type==LUMA_4x8)
			{
				or_bits_low(&currMB->cbp_bits[0], 0x011, bit);
			}
			else
			{
				or_bits_low(&currMB->cbp_bits[0], 0x01, bit);
			}
		}
	}
	else if (type == CHROMA_AC)
	{
		int u_ac        = !currMB->is_v_block;

		int default_bit = (currMB->is_intra_block ? 1 : 0);
		int upper_bit   = default_bit;
		int left_bit    = default_bit;
		int ctx;

		PixelPos block_a, block_b;

		int j           = currMB->subblock_y;
		int i           = currMB->subblock_x;
		int bit         = (u_ac ? 19 : 35);
		
		p_Vid->getNeighbourXP_NoPos(currMB, i - 1, j    , p_Vid->mb_size[IS_CHROMA], &block_a);
		p_Vid->getNeighbourPX_NoPos(currMB, i    , j - 1, p_Vid->mb_size[IS_CHROMA], &block_b);

		//--- get bits from neighboring blocks ---
		if (block_b.available)
		{
			if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
				upper_bit=1;
			else
				upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[0], bit + (block_b.y&((short)~3)) + (block_b.x>>2));
		}

		if (block_a.available)
		{
			if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
				left_bit=1;
			else
				left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[0],bit + (block_a.y&((short)~3)) + (block_a.x>>2));
		}

		ctx = 2 * upper_bit + left_bit;     
		//===== encode symbol =====
		cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[CHROMA_AC]] + ctx);


		//--- set bits for current block ---
		if (cbp_bit)
		{  
			or_bits(&currMB->cbp_bits[0], 0x01, bit + j + (i >> 2));
		}

	}
	else if (type <= CHROMA_DC_4x4)
	{
		int v_dc        = currMB->is_v_block;
		int default_bit = (currMB->is_intra_block ? 1 : 0);
		int upper_bit   = default_bit;
		int left_bit    = default_bit;
		int ctx;


		PixelPos block_a, block_b;

		int bit         = (v_dc ? 18 : 17);
		p_Vid->getNeighbourLeft(currMB, p_Vid->mb_size[IS_CHROMA], &block_a);
		p_Vid->getNeighbourUp(currMB, p_Vid->mb_size[IS_CHROMA], &block_b);
		//--- get bits from neighboring blocks ---
		if (block_b.available)
		{
			if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
				upper_bit=1;
			else
				upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[0], bit);
		}

		if (block_a.available)
		{
			if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
				left_bit=1;
			else
				left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[0],bit);
		}

		ctx = 2 * upper_bit + left_bit;     
		//===== encode symbol =====
		cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]] + ctx);


		//--- set bits for current block ---
		if (cbp_bit)
		{  
			or_bits(&currMB->cbp_bits[0], 0x01, bit);
		}


	}
	else
	{
		int default_bit = (currMB->is_intra_block ? 1 : 0);
		int upper_bit   = default_bit;
		int left_bit    = default_bit;
		int ctx;


		PixelPos block_a, block_b;

		p_Vid->getNeighbourLeft(currMB, p_Vid->mb_size[IS_CHROMA], &block_a);
		p_Vid->getNeighbourUp(currMB, p_Vid->mb_size[IS_CHROMA], &block_b);
		//--- get bits from neighboring blocks ---
		if (block_b.available)
		{
			if(p_Vid->mb_data[block_b.mb_addr].mb_type==IPCM)
				upper_bit=1;
			else
				upper_bit = get_bit(p_Vid->mb_data[block_b.mb_addr].cbp_bits[0], 35);
		}

		if (block_a.available)
		{
			if(p_Vid->mb_data[block_a.mb_addr].mb_type==IPCM)
				left_bit=1;
			else
				left_bit = get_bit(p_Vid->mb_data[block_a.mb_addr].cbp_bits[0],35);
		}

		ctx = 2 * upper_bit + left_bit;     
		//===== encode symbol =====
		cbp_bit = biari_decode_symbol (dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]] + ctx);


		//--- set bits for current block ---
		if (cbp_bit)
		{  
			or_bits(&currMB->cbp_bits[0], 0x01, 35);
		}


	}
	return cbp_bit;
}


void set_read_and_store_CBP(Macroblock **currMB, int chroma_format_idc)
{
	if (chroma_format_idc == YUV444)
		(*currMB)->read_and_store_CBP_block_bit = read_and_store_CBP_block_bit_444;
	else
		(*currMB)->read_and_store_CBP_block_bit = read_and_store_CBP_block_bit_normal; 
}





//===== position -> ctx for MAP =====
//--- zig-zag scan ----
static const byte  pos2ctx_map8x8 [] = { 0,  1,  2,  3,  4,  5,  5,  4,  4,  3,  3,  4,  4,  4,  5,  5,
4,  4,  4,  4,  3,  3,  6,  7,  7,  7,  8,  9, 10,  9,  8,  7,
7,  6, 11, 12, 13, 11,  6,  7,  8,  9, 14, 10,  9,  8,  6, 11,
12, 13, 11,  6,  9, 14, 10,  9, 11, 12, 13, 11 ,14, 10, 12, 14}; // 15 CTX
static const byte  pos2ctx_map8x4 [] = { 0,  1,  2,  3,  4,  5,  7,  8,  9, 10, 11,  9,  8,  6,  7,  8,
9, 10, 11,  9,  8,  6, 12,  8,  9, 10, 11,  9, 13, 13, 14, 14}; // 15 CTX
static const byte  pos2ctx_map4x4 [] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 14}; // 15 CTX
static const byte  pos2ctx_map2x4c[] = { 0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2}; // 15 CTX
static const byte  pos2ctx_map4x4c[] = { 0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2}; // 15 CTX
static const byte* pos2ctx_map    [] = {pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8, pos2ctx_map8x4,
pos2ctx_map8x4, pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map4x4,
pos2ctx_map2x4c, pos2ctx_map4x4c, 
pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8,pos2ctx_map8x4,
pos2ctx_map8x4, pos2ctx_map4x4,
pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8,pos2ctx_map8x4,
pos2ctx_map8x4,pos2ctx_map4x4};
//--- interlace scan ----
//taken from ABT
static const byte  pos2ctx_map8x8i[] = { 0,  1,  1,  2,  2,  3,  3,  4,  5,  6,  7,  7,  7,  8,  4,  5,
6,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 11, 12, 11,
9,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 13, 13,  9,
9, 10, 10,  8, 13, 13,  9,  9, 10, 10, 14, 14, 14, 14, 14, 14}; // 15 CTX
static const byte  pos2ctx_map8x4i[] = { 0,  1,  2,  3,  4,  5,  6,  3,  4,  5,  6,  3,  4,  7,  6,  8,
9,  7,  6,  8,  9, 10, 11, 12, 12, 10, 11, 13, 13, 14, 14, 14}; // 15 CTX
static const byte  pos2ctx_map4x8i[] = { 0,  1,  1,  1,  2,  3,  3,  4,  4,  4,  5,  6,  2,  7,  7,  8,
8,  8,  5,  6,  9, 10, 10, 11, 11, 11, 12, 13, 13, 14, 14, 14}; // 15 CTX
static const byte* pos2ctx_map_int[] = {pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
pos2ctx_map4x8i,pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map4x4,
pos2ctx_map2x4c, pos2ctx_map4x4c,
pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
pos2ctx_map8x4i,pos2ctx_map4x4,
pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
pos2ctx_map8x4i,pos2ctx_map4x4};

//===== position -> ctx for LAST =====
static const byte  pos2ctx_last8x8 [] = { 0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,
5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8}; //  9 CTX
static const byte  pos2ctx_last8x4 [] = { 0,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,
3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8}; //  9 CTX

static const byte  pos2ctx_last4x4 [] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15}; // 15 CTX
static const byte  pos2ctx_last2x4c[] = { 0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2}; // 15 CTX
static const byte  pos2ctx_last4x4c[] = { 0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2}; // 15 CTX
static const byte* pos2ctx_last    [] = {pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8, pos2ctx_last8x4,
pos2ctx_last8x4, pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last4x4,
pos2ctx_last2x4c, pos2ctx_last4x4c,
pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8,pos2ctx_last8x4,
pos2ctx_last8x4, pos2ctx_last4x4,
pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8,pos2ctx_last8x4,
pos2ctx_last8x4, pos2ctx_last4x4};



/*!
************************************************************************
* \brief
*    Read Significance MAP
************************************************************************
*/

#if defined(_DEBUG) || defined(_M_X64)
static int read_significance_map(TextureInfoContexts *tex_ctx, const Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int type, int16_t coeff[])
{
	int   i;
	int   coeff_ctr = 0;
	int   i0        = 0;
	int   i1        = maxpos[type];
	const VideoParameters *p_Vid = currMB->p_Vid;

	int               fld       = ( p_Vid->structure!=FRAME || currMB->mb_field );
	const byte *pos2ctx_Map = (fld) ? pos2ctx_map_int[type] : pos2ctx_map[type];
	const byte *last = pos2ctx_last[type];

	BiContextTypePtr  map_ctx   = tex_ctx->map_contexts[fld][type2ctx_map [type]];
	BiContextTypePtr  last_ctx  = tex_ctx->last_contexts[fld][type2ctx_last[type]];

	if (!c1isdc[type])
	{
		pos2ctx_Map++;
		last++;
	}

	for (i=0; i < i1; ++i) // if last coeff is reached, it has to be significant
	{
		//--- read significance symbol ---
		if (biari_decode_symbol   (dep_dp, map_ctx + pos2ctx_Map[i]))
		{
			coeff[i] = 1;
			++coeff_ctr;
			//--- read last coefficient symbol ---
			if (biari_decode_symbol (dep_dp, last_ctx + last[i]))
			{
				while (i++ < i1)
				{
					coeff[i] = 0;
				}
				return coeff_ctr;
				//memset(&coeff[i + 1], 0, (i1 - i) * sizeof(int));
				//i = i1;
			}
		}
		else
		{
			coeff[i] = 0;
		}
	}
	//--- last coefficient must be significant if no last symbol was received ---
	coeff[i] = 1;


	return coeff_ctr+1;
}
#endif
/*!
************************************************************************
* \brief
*    Read Levels
************************************************************************
*/
#if defined(_DEBUG) || defined(_M_X64)
/*!
************************************************************************
* \brief
*    Exp-Golomb decoding for LEVELS
***********************************************************************
*/
unsigned int exp_golomb_decode_eq_prob( DecodingEnvironmentPtr dep_dp, int k);
static unsigned int unary_exp_golomb_level_decode( DecodingEnvironmentPtr dep_dp,
																									BiContextTypePtr ctx)
{
	unsigned int symbol = biari_decode_symbol(dep_dp, ctx );

	if (symbol==0)
		return 0;
	else
	{
		const unsigned int exp_start = 13;

		for (symbol=0;symbol<(exp_start-1);symbol++)
		{
			if (!biari_decode_symbol(dep_dp, ctx))
				return symbol;
		}
		return exp_golomb_decode_eq_prob(dep_dp,0)+13;
	}
}

static void read_significant_coefficients (TextureInfoContexts    *tex_ctx,
																					 DecodingEnvironmentPtr  dep_dp,
																					 int                     type,
																					 int16_t                 coeff[])
{
	static const int plus_one_clip4[5] = { 1, 2, 3, 4, 4 };
	static const int plus_one_clip3[4] = { 1, 2, 3, 3 };
	const int *c2_clip = (max_c2[type]==4)?plus_one_clip4:plus_one_clip3;
	int   i;
	int   c1 = 1;
	int   c2 = 0;
	BiContextType *one_contexts = tex_ctx->one_contexts[type2ctx_one[type]];
	BiContextType *abs_contexts = tex_ctx->abs_contexts[type2ctx_abs[type]];

	for (i=maxpos[type]; i>=0; i--)
	{
		if (coeff[i]!=0)
		{
			coeff[i] += biari_decode_symbol (dep_dp, one_contexts + c1);
			if (coeff[i]==2)
			{
				coeff[i] += unary_exp_golomb_level_decode (dep_dp, abs_contexts + c2);
				c2 = c2_clip[c2];
				c1=0;
			}
			else if (c1)
			{
				c1 = plus_one_clip4[c1];
			}
			if (biari_decode_symbol_eq_prob(dep_dp))
			{
				coeff[i] *= -1;
			}
		}
	}
}
#else
void read_significant_coefficients (TextureInfoContexts    *tex_ctx,
																		DecodingEnvironmentPtr  dep_dp,
																		int                     type,
																		int                     coeff[]);
#endif

/*!
************************************************************************
* \brief
*    Read Block-Transform Coefficients
************************************************************************
*/
#if defined(_DEBUG) || defined(_M_X64)
RunLevel readRunLevel_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int context)
{	
	RunLevel rl;
	Slice *currSlice = currMB->p_Slice;
	//--- read coefficients for whole block ---
	if (currSlice->coeff_ctr < 0)
	{
		//===== decode CBP-BIT =====
		if ((currSlice->coeff_ctr = currMB->read_and_store_CBP_block_bit (currMB, dep_dp, context) )!=0)
		{
			//===== decode significance map =====
			currSlice->coeff_ctr = read_significance_map (currSlice->tex_ctx, currMB, dep_dp, context, currSlice->coeff);

			//===== decode significant coefficients =====
			read_significant_coefficients    (currSlice->tex_ctx, dep_dp, context, currSlice->coeff);
		}
	}

	//--- set run and level ---

	rl.run=0;
	if (currSlice->coeff_ctr--)
	{
		//--- set run and level (coefficient) ---
		for (; currSlice->coeff[currSlice->pos] == 0; ++currSlice->pos, ++rl.run); 
		rl.level = currSlice->coeff[currSlice->pos++];
		//--- decrement coefficient counter and re-set position ---
		if (currSlice->coeff_ctr == 0) 
			currSlice->pos = 0;
		return rl;
	}
	else
	{
		//--- set run and level (EOB) ---
		currSlice->pos = 0;
		rl.level = 0;
		return rl;
	}
}
#endif
/*!
************************************************************************
* \brief
*    arideco_bits_read
************************************************************************
*/
static int arideco_bits_read(const DecodingEnvironmentPtr dep)
{ 
	int tmp = ((*dep->Dcodestrm_len) << 3) - dep->DbitsLeft;

#if (2==TRACE)
	fprintf(p_trace, "tmp: %d\n", tmp);
#endif
	return tmp;
}

/*!
************************************************************************
* \brief
*    decoding of unary binarization using one or 2 distinct
*    models for the first and all remaining bins; no terminating
*    "0" for max_symbol
***********************************************************************
*/
static unsigned int unary_bin_max_decode(DecodingEnvironmentPtr dep_dp,
																				 BiContextTypePtr ctx,
																				 int ctx_offset,
																				 unsigned int max_symbol)
{
	unsigned int symbol =  biari_decode_symbol(dep_dp, ctx );

	if (symbol==0 || (max_symbol == 0))
		return symbol;
	else
	{    
		unsigned int l;
		ctx += ctx_offset;
		symbol = 0;
		do
		{
			l = biari_decode_symbol(dep_dp, ctx);
			++symbol;
		}
		while( (l != 0) && (symbol < max_symbol) );

		if ((l != 0) && (symbol == max_symbol))
			++symbol;
		return symbol;
	}
}


/*!
************************************************************************
* \brief
*    decoding of unary binarization using one or 2 distinct
*    models for the first and all remaining bins
***********************************************************************
*/
static unsigned int unary_bin_decode(DecodingEnvironmentPtr dep_dp,
																		 BiContextTypePtr ctx,
																		 int ctx_offset)
{
	unsigned int symbol = biari_decode_symbol(dep_dp, ctx );

	if (symbol == 0)
		return 0;
	else
	{
		unsigned int l;
		ctx += ctx_offset;;
		symbol = 0;
		do
		{
			l=biari_decode_symbol(dep_dp, ctx);
			++symbol;
		}
		while( l != 0 );
		return symbol;
	}
}


/*!
************************************************************************
* \brief
*    finding end of a slice in case this is not the end of a frame
*
* Unsure whether the "correction" below actually solves an off-by-one
* problem or whether it introduces one in some cases :-(  Anyway,
* with this change the bit stream format works with CABAC again.
* StW, 8.7.02
************************************************************************
*/
int cabac_startcode_follows(Slice *currSlice, int eos_bit)
{
	unsigned int  bit;

	if( eos_bit )
	{
		const byte   *partMap    = assignSE2partition[currSlice->dp_mode];
		DataPartition *dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);  
		DecodingEnvironmentPtr dep_dp = &(dP->de_cabac);

		bit = biari_decode_final (dep_dp); //GB

#if TRACE
		fprintf(p_trace, "@%-6d %-63s (%3d)\n",symbolCount++, "end_of_slice_flag", bit);
		fflush(p_trace);
#endif
	}
	else
	{
		bit = 0;
	}

	return bit;
}

/*!
************************************************************************
* \brief
*    Exp Golomb binarization and decoding of a symbol
*    with prob. of 0.5r
************************************************************************
*/
unsigned int exp_golomb_decode_eq_prob( DecodingEnvironmentPtr dep_dp, int k)
{
	unsigned int l;
	int symbol = 0;
	int binary_symbol = 0;

	do
	{
		l = biari_decode_symbol_eq_prob(dep_dp);
		if (l) // always returns 1 or zero
		{
			symbol += (l<<k); // l is guaranteed to be one
			++k;
		}
	}
	while (l!=0);

	while (k--)                             //next binary part
		if (biari_decode_symbol_eq_prob(dep_dp)==1)
			binary_symbol |= (1<<k);

	return (unsigned int) (symbol + binary_symbol);
}

/*!
************************************************************************
* \brief
*    Exp-Golomb decoding for Motion Vectors
***********************************************************************
*/
#if defined(_DEBUG) || defined(_M_X64)
unsigned int unary_exp_golomb_mv_decode(DecodingEnvironmentPtr dep_dp,
																							 BiContextTypePtr ctx,
																							 unsigned int max_bin)
{
	unsigned int symbol = biari_decode_symbol(dep_dp, ctx );

	if (symbol == 0)
		return 0;
	else
	{
		const unsigned int exp_start = 8;

		++ctx;
		for (symbol=1;symbol<exp_start;)
		{
			if (!biari_decode_symbol(dep_dp, ctx))
				return symbol;
			if ((++symbol)==2) ctx++;
			if (symbol==max_bin) 
				++ctx;
		}

		return exp_start + exp_golomb_decode_eq_prob(dep_dp,3);
	}
}
unsigned int unary_exp_golomb_mv_decode3(DecodingEnvironmentPtr dep_dp,
																							 BiContextTypePtr ctx)
{
																							 unsigned int max_bin = 3;
	unsigned int symbol = biari_decode_symbol(dep_dp, ctx );

	if (symbol == 0)
		return 0;
	else
	{
		const unsigned int exp_start = 8;

		++ctx;
		for (symbol=1;symbol<exp_start;)
		{
			if (!biari_decode_symbol(dep_dp, ctx))
				return symbol;
			if ((++symbol)==2) ctx++;
			if (symbol==max_bin) 
				++ctx;
		}

		return exp_start + exp_golomb_decode_eq_prob(dep_dp,3);
	}
}
#endif

/*!
************************************************************************
* \brief
*    Read I_PCM macroblock 
************************************************************************
*/
void readIPCM_CABAC(Slice *currSlice, struct datapartition *dP)
{
	VideoParameters *p_Vid = currSlice->p_Vid;
	StorablePicture *dec_picture = p_Vid->dec_picture;
	Bitstream* currStream = dP->bitstream;
	DecodingEnvironmentPtr dep = &(dP->de_cabac);
	byte *buf = currStream->streamBuffer;
	int BitstreamLengthInBits = (dP->bitstream->bitstream_length << 3) + 7;

	int val = 0;

	int bits_read = 0;
	int bitoffset, bitdepth;
	int uv, i, j;

	while (dep->DbitsLeft >= 8)
	{
		dep->Dvalue   >>= 8;
		dep->DbitsLeft -= 8;
		(*dep->Dcodestrm_len)--;
	}

	bitoffset = (*dep->Dcodestrm_len) << 3;

	// read luma values
	bitdepth = p_Vid->bitdepth_luma;
	for(i=0;i<MB_BLOCK_SIZE;++i)
	{
		for(j=0;j<MB_BLOCK_SIZE;++j)
		{
			bits_read += GetBits(buf, bitoffset, &val, BitstreamLengthInBits, bitdepth);
			currSlice->ipcm[0][i][j] = val;
			bitoffset += bitdepth;
		}
	}

	// read chroma values
	bitdepth = p_Vid->bitdepth_chroma;
	if ((dec_picture->chroma_format_idc != YUV400) && !IS_INDEPENDENT(p_Vid))
	{
		for (uv=1; uv<3; ++uv)
		{
			for(i=0;i<p_Vid->mb_cr_size_y;++i)
			{
				for(j=0;j<p_Vid->mb_cr_size_x;++j)
				{
					bits_read += GetBits(buf, bitoffset, &val, BitstreamLengthInBits, bitdepth);
					currSlice->ipcm[uv][i][j] = val;
					bitoffset += bitdepth;
				}
			}
		}
	}

	(*dep->Dcodestrm_len) += ( bits_read >> 3);
	if (bits_read & 7)
	{
		++(*dep->Dcodestrm_len);
	}
}

