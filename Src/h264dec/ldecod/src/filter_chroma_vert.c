#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"
#include <emmintrin.h>
static const byte ALPHA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,4,4,5,6,  7,8,9,10,12,13,15,17,  20,22,25,28,32,36,40,45,  50,56,63,71,80,90,101,113,  127,144,162,182,203,226,255,255} ;
static const byte  BETA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,2,2,2,3,  3,3,3, 4, 4, 4, 6, 6,   7, 7, 8, 8, 9, 9,10,10,  11,11,12,12,13,13, 14, 14,   15, 15, 16, 16, 17, 17, 18, 18} ;
static const byte CLIP_TAB[52][5]  =
{
	{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},
	{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},
	{ -1, 0, 0, 0, 0},{ -1, 0, 0, 1, 1},{ -1, 0, 0, 1, 1},{ -1, 0, 0, 1, 1},{ -1, 0, 0, 1, 1},{ -1, 0, 1, 1, 1},{ -1, 0, 1, 1, 1},{ -1, 1, 1, 1, 1},
	{ -1, 1, 1, 1, 1},{ -1, 1, 1, 1, 1},{ -1, 1, 1, 1, 1},{ -1, 1, 1, 2, 2},{ -1, 1, 1, 2, 2},{ -1, 1, 1, 2, 2},{ -1, 1, 1, 2, 2},{ -1, 1, 2, 3, 3},
	{ -1, 1, 2, 3, 3},{ -1, 2, 2, 3, 3},{ -1, 2, 2, 4, 4},{ -1, 2, 3, 4, 4},{ -1, 2, 3, 4, 4},{ -1, 3, 3, 5, 5},{ -1, 3, 4, 6, 6},{ -1, 3, 4, 6, 6},
	{ -1, 4, 5, 7, 7},{ -1, 4, 5, 8, 8},{ -1, 4, 6, 9, 9},{ -1, 5, 7,10,10},{ -1, 6, 8,11,11},{ -1, 6, 8,13,13},{ -1, 7,10,14,14},{ -1, 8,11,16,16},
	{ -1, 9,12,18,18},{ -1,10,13,20,20},{ -1,11,15,23,23},{ -1,13,17,25,25}
} ;

static const int pelnum_cr[2][4] =  {{0,8,16,16}, {0,8, 8,16}};  //[dir:0=vert, 1=hor.][yuv_format]

void EdgeLoopChromaNormal_Vert(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p)
{ 
	// dir == 0
	imgpel** Img = image->img;
	VideoParameters *p_Vid = MbQ->p_Vid;  

	int xQ = edge - 1;
	int yQ = 0;  
	PixelPos pixMB1;

	p_Vid->getNeighbourX0(MbQ, xQ, p_Vid->mb_size[IS_CHROMA], &pixMB1);

	if (pixMB1.available || (MbQ->DFDisableIdc == 0))
	{
		int      bitdepth_scale   = p_Vid->bitdepth_scale[IS_CHROMA];
		int      max_imgpel_value = p_Vid->max_pel_value_comp[uv + 1];

		int AlphaC0Offset = MbQ->DFAlphaC0Offset;
		int BetaOffset = MbQ->DFBetaOffset;
		PixelPos pixP = pixMB1;
		Macroblock *MbP = &(p_Vid->mb_data[pixP.mb_addr]);

		// Average QP of the two blocks
		int QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
		int indexB = iClip3(0, MAX_QP, QP + BetaOffset);

		int Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
		int Beta    = BETA_TABLE [indexB] * bitdepth_scale;

		if (Alpha !=0 && Beta != 0)
		{
			const int PelNum = pelnum_cr[0][p->chroma_format_idc];
			const     byte *ClipTab = CLIP_TAB[indexA];
			int       inc_dim = 1;
			int pel;
			PixelPos pixQ, pixMB2;    

			p_Vid->getNeighbourX0(MbQ, edge, p_Vid->mb_size[IS_CHROMA], &pixMB2);
			pixQ = pixMB2;

			for( pel = 0 ; pel < PelNum ; ++pel )
			{
				int Strng = Strength[(PelNum == 8) ? (((pel >> 1) << 2) + (pel & 0x01)) : pel];

				if( Strng != 0)
				{
					imgpel *SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);
					imgpel  L0  = *SrcPtrP;
					imgpel *SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
					imgpel  R0  = *SrcPtrQ;

					if ( abs( R0 - L0 ) < Alpha ) 
					{
						imgpel R1  = *(SrcPtrQ + inc_dim);
						if ( abs(R0 - R1) < Beta )  
						{
							imgpel L1  = *(SrcPtrP - inc_dim);
							if ( abs(L0 - L1) < Beta )
							{
								if( Strng == 4 )    // INTRA strong filtering
								{
									*SrcPtrP = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
									*SrcPtrQ = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
								}
								else
								{
									int tc0  = ClipTab[ Strng ] * bitdepth_scale + 1;
									int dif = iClip3( -tc0, tc0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

									*SrcPtrP = (imgpel) iClip1 ( max_imgpel_value, L0 + dif) ;
									*SrcPtrQ = (imgpel) iClip1 ( max_imgpel_value, R0 - dif) ;
								}
							}
						}
					}
				}
				pixP.pos_y++;
				pixQ.pos_y++;
			}
		}
	}
}

static void FilterStrongChroma_Vert_sse(int p_step, imgpel *SrcPtrP, int Alpha, int Beta)
{
	__m64 mmx_alpha_minus_one = _mm_set1_pi16(Alpha-1), mmx_beta_minus_one = _mm_set1_pi16(Beta-1);
	__m64 mmx_zero	= _mm_setzero_si64(), mmx_two=_mm_set1_pi16(2);
	__m64 mmx_minus_one;
	__m64 mmx_absdiff, mmx_diff;
	__m64 mmx_L0, mmx_L1, mmx_L1_L0;
	__m64 mmx_R0, mmx_R0_R1, mmx_R1;
	__m64 mmx_load0, mmx_load1, mmx_load2, mmx_load3, mmx_load4, mmx_load5, mmx_load6, mmx_load7, mmx_load8;
	__m64 mmx_match, mmx_L0_new, mmx_R0_new;
	int match;
	int i=0;

	mmx_minus_one = _mm_set1_pi32(-1);
	SrcPtrP -= 1;

	STAGE:
		mmx_load0 = _mm_cvtsi32_si64(*(int *)(SrcPtrP));          // La1 La0 Ra0 Ra1 --- --- --- ---
		mmx_load1 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+=p_step));  // Lb1 Lb0 Rb0 Rb1 --- --- --- ---
		mmx_load4 = _mm_unpacklo_pi8(mmx_load0, mmx_load1);     // La1 Lb1 La0 Lb0 Ra0 Rb0 Ra1 Rb1
		mmx_load2 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+=p_step));  // Lc1 Lc0 Rc0 Rc1 --- --- --- ---
		mmx_load3 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+=p_step));  // Ld1 Ld0 Rd0 Rd1 --- --- --- ---
		SrcPtrP+=p_step;
		mmx_load5 = _mm_unpacklo_pi8(mmx_load2, mmx_load3);     // Lc1 Ld1 Lc0 Ld0 Rc0 Rd0 Rc1 Rd1
		mmx_L1_L0 = _mm_unpacklo_pi16(mmx_load4, mmx_load5);    // La1 Lb1 Lc1 Ld1 La0 Lb0 Lc0 Ld0
		mmx_R0_R1 = _mm_unpackhi_pi16(mmx_load4, mmx_load5);    // Ra0 Rb0 Rc0 Rd0 Ra1 Rb1 Rc1 Rd1

		// abs( R0 - L0 ) < Alpha
		// MMX doesn't have unsigned compare, so we have to go to short
		mmx_L0 = _mm_unpackhi_pi8(mmx_L1_L0, mmx_zero);             // La0 Lb0 Lc0 Ld0
		mmx_R0 = _mm_unpacklo_pi8(mmx_R0_R1, mmx_zero);          // Ra0 Rb0 Rc0 Rd0
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_L0);
		mmx_absdiff =_mm_subs_pu16(mmx_L0, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_match = _mm_cmpgt_pi16(mmx_absdiff, mmx_alpha_minus_one); // 1's in any words we don't have to do
		mmx_match = _mm_xor_si64(mmx_match, mmx_minus_one);
		match = _mm_movemask_pi8(mmx_match); 
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;
				
			goto STAGE; // start the process over from next position
		}

		// abs( R0 - R1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_R0 already populated
		mmx_R1 = _mm_unpackhi_pi8(mmx_R0_R1, mmx_zero);           // Ra1 Rb1 Rc1 Rd1
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_R1);
		mmx_absdiff =_mm_subs_pu16(mmx_R1, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;
				
			goto STAGE; // start the process over from next position
		}

		// abs(L0 - L1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_L0 already populated
		mmx_L1 = _mm_unpacklo_pi8(mmx_L1_L0, mmx_zero);      // La1 Lb1 Lc1 Ld1
		mmx_diff=_mm_subs_pu16(mmx_L0, mmx_L1);
		mmx_absdiff =_mm_subs_pu16(mmx_L1, mmx_L0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;

			goto STAGE; // start the process over from next position
		}

		// ok, now time to performn the actual calculation. hope it was worth it!!
		
				// L0 = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
		mmx_L0_new = mmx_L1;
		mmx_L0_new = _mm_slli_pi16(mmx_L0_new, 1);
		mmx_L0_new = _mm_add_pi16(mmx_L0_new, mmx_L0);
		mmx_L0_new = _mm_add_pi16(mmx_L0_new, mmx_R1);
		mmx_L0_new = _mm_add_pi16(mmx_L0_new, mmx_two);
		mmx_L0_new = _mm_srai_pi16(mmx_L0_new, 2);
		mmx_L0_new = _mm_and_si64(mmx_L0_new, mmx_match);
		mmx_L0 = _mm_andnot_si64(mmx_match, mmx_L0);
		mmx_L0 = _mm_or_si64(mmx_L0, mmx_L0_new);

		// R0 = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
		mmx_R0_new = mmx_R1;
		mmx_R0_new = _mm_slli_pi16(mmx_R0_new, 1);
		mmx_R0_new = _mm_add_pi16(mmx_R0_new, mmx_R0);
		mmx_R0_new = _mm_add_pi16(mmx_R0_new, mmx_L1);
		mmx_R0_new = _mm_add_pi16(mmx_R0_new, mmx_two);
		mmx_R0_new = _mm_srai_pi16(mmx_R0_new, 2);
		mmx_R0_new = _mm_and_si64(mmx_R0_new, mmx_match);
		mmx_R0 = _mm_andnot_si64(mmx_match, mmx_R0);
		mmx_R0 = _mm_or_si64(mmx_R0, mmx_R0_new);

	// now for the super-exciting fun of getting this data back into memory
		SrcPtrP -= 4*p_step;

				// rotate 4x4 matrix
		mmx_load1 = _mm_unpacklo_pi16(mmx_L1, mmx_R0); // 00 20 01 21
		mmx_load3 = _mm_unpackhi_pi16(mmx_L1, mmx_R0); // 02 22 03 23
		mmx_load2 = _mm_unpacklo_pi16(mmx_L0, mmx_R1); // 10 30 11 31
		mmx_load4 = _mm_unpackhi_pi16(mmx_L0, mmx_R1); // 12 32 13 33
		mmx_load5 = _mm_unpacklo_pi16(mmx_load1, mmx_load2); // 00 10 20 30
		mmx_load6 = _mm_unpackhi_pi16(mmx_load1, mmx_load2); // 01 11 21 31
		mmx_load7 = _mm_unpacklo_pi16(mmx_load3, mmx_load4); // 02 12 22 32
		mmx_load8 = _mm_unpackhi_pi16(mmx_load3, mmx_load4); // 03 13 23 33
		mmx_load5 = _mm_packs_pu16(mmx_load5, mmx_load5); 
		mmx_load6 = _mm_packs_pu16(mmx_load6, mmx_load6); 
		mmx_load7 = _mm_packs_pu16(mmx_load7, mmx_load7); 
		mmx_load8 = _mm_packs_pu16(mmx_load8, mmx_load8); 

		//mmx_load1 = _mm_setr_pi16(0x8080, 0x80, 0, 0);
		*(int *)SrcPtrP = _mm_cvtsi64_si32(mmx_load5);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load6);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load7);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load8);

		if (i++ == 1)
			return;

		SrcPtrP += p_step;
		goto STAGE; // next stage
}

static void FilterChroma_Vert_sse(int p_step, imgpel *SrcPtrP, int Alpha, int Beta, const uint8_t Strength[4], const char *ClipTab)
{
	__m64 mmx_alpha_minus_one = _mm_set1_pi16(Alpha-1), mmx_beta_minus_one = _mm_set1_pi16(Beta-1);
	__m64 mmx_zero	= _mm_setzero_si64(), mmx_four=_mm_set1_pi16(4);
	__m64 mmx_minus_one;
	__m64 mmx_absdiff, mmx_diff;
	__m64 mmx_L0, mmx_L1, mmx_L1_L0;
	__m64 mmx_R0, mmx_R0_R1, mmx_R1;
	__m64 mmx_load0, mmx_load1, mmx_load2, mmx_load3, mmx_load4, mmx_load5, mmx_load6, mmx_load7, mmx_load8;
	__m64 mmx_C0, mmx_negative_C0,  mmx_dif,  mmx_match;
	int match;
	int i=0;

	mmx_minus_one = _mm_set1_pi32(-1);
	SrcPtrP -= 1;

	STAGE:

	while (!Strength[i*2] && !Strength[i*2+1])
	{
		SrcPtrP += p_step*4;
		if (i++ == 1) // last stage
			return;
	}

		mmx_load0 = _mm_cvtsi32_si64(*(int *)(SrcPtrP));          // La1 La0 Ra0 Ra1 --- --- --- ---
		mmx_load1 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+=p_step));  // Lb1 Lb0 Rb0 Rb1 --- --- --- ---
		mmx_load4 = _mm_unpacklo_pi8(mmx_load0, mmx_load1);     // La1 Lb1 La0 Lb0 Ra0 Rb0 Ra1 Rb1
		mmx_load2 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+=p_step));  // Lc1 Lc0 Rc0 Rc1 --- --- --- ---
		mmx_load3 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+=p_step));  // Ld1 Ld0 Rd0 Rd1 --- --- --- ---
		SrcPtrP+=p_step;
		mmx_load5 = _mm_unpacklo_pi8(mmx_load2, mmx_load3);     // Lc1 Ld1 Lc0 Ld0 Rc0 Rd0 Rc1 Rd1
		mmx_L1_L0 = _mm_unpacklo_pi16(mmx_load4, mmx_load5);    // La1 Lb1 Lc1 Ld1 La0 Lb0 Lc0 Ld0
		mmx_R0_R1 = _mm_unpackhi_pi16(mmx_load4, mmx_load5);    // Ra0 Rb0 Rc0 Rd0 Ra1 Rb1 Rc1 Rd1

		// abs( R0 - L0 ) < Alpha
		// MMX doesn't have unsigned compare, so we have to go to short
		mmx_L0 = _mm_unpackhi_pi8(mmx_L1_L0, mmx_zero);             // La0 Lb0 Lc0 Ld0
		mmx_R0 = _mm_unpacklo_pi8(mmx_R0_R1, mmx_zero);          // Ra0 Rb0 Rc0 Rd0
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_L0);
		mmx_absdiff =_mm_subs_pu16(mmx_L0, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_match = _mm_cmpgt_pi16(mmx_absdiff, mmx_alpha_minus_one); // 1's in any words we don't have to do
		mmx_match = _mm_xor_si64(mmx_match, mmx_minus_one);
		match = _mm_movemask_pi8(mmx_match); 
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;
				
			goto STAGE; // start the process over from next position
		}

		// abs( R0 - R1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_R0 already populated
		mmx_R1 = _mm_unpackhi_pi8(mmx_R0_R1, mmx_zero);           // Ra1 Rb1 Rc1 Rd1
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_R1);
		mmx_absdiff =_mm_subs_pu16(mmx_R1, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;
				
			goto STAGE; // start the process over from next position
		}

		// abs(L0 - L1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_L0 already populated
		mmx_L1 = _mm_unpacklo_pi8(mmx_L1_L0, mmx_zero);      // La1 Lb1 Lc1 Ld1
		mmx_diff=_mm_subs_pu16(mmx_L0, mmx_L1);
		mmx_absdiff =_mm_subs_pu16(mmx_L1, mmx_L0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;

			goto STAGE; // start the process over from next position
		}

		// ok, now time to performn the actual calculation. hope it was worth it!!

		// tc0  = ClipTab[ Strng ]  + 1
		mmx_C0 = _mm_setr_pi16(ClipTab[Strength[i*2]]+1, ClipTab[Strength[i*2]]+1, ClipTab[Strength[i*2+1]]+1, ClipTab[Strength[i*2+1]]+1);
		mmx_negative_C0 = _mm_sub_pi16(mmx_zero, mmx_C0);

		// dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );
		mmx_dif = mmx_R0;
		mmx_dif = _mm_sub_pi16(mmx_dif, mmx_L0);
		mmx_dif = _mm_slli_pi16(mmx_dif, 2);
		mmx_dif = _mm_add_pi16(mmx_dif, mmx_L1);
		mmx_dif = _mm_sub_pi16(mmx_dif, mmx_R1);
		mmx_dif = _mm_add_pi16(mmx_dif, mmx_four);
		mmx_dif = _mm_srai_pi16(mmx_dif, 3);
		mmx_dif = _mm_min_pi16(mmx_dif, mmx_C0);
		mmx_dif = _mm_max_pi16(mmx_dif, mmx_negative_C0);
		mmx_dif = _mm_and_si64(mmx_dif, mmx_match);

		// L0 = (imgpel) iClip1(max_imgpel_value, L0 + dif);
		mmx_L0 = _mm_add_pi16(mmx_L0, mmx_dif);

		// R0 = (imgpel) iClip1(max_imgpel_value, R0 - dif);
		mmx_R0 = _mm_sub_pi16(mmx_R0, mmx_dif);

	// now for the super-exciting fun of getting this data back into memory
		SrcPtrP -= 4*p_step;

				// rotate 4x4 matrix
		mmx_load1 = _mm_unpacklo_pi16(mmx_L1, mmx_R0); // 00 20 01 21
		mmx_load2 = _mm_unpacklo_pi16(mmx_L0, mmx_R1); // 10 30 11 31
		mmx_load3 = _mm_unpackhi_pi16(mmx_L1, mmx_R0); // 02 22 03 23
		mmx_load4 = _mm_unpackhi_pi16(mmx_L0, mmx_R1); // 12 32 13 33
		mmx_load5 = _mm_unpacklo_pi16(mmx_load1, mmx_load2); // 00 10 20 30
		mmx_load6 = _mm_unpackhi_pi16(mmx_load1, mmx_load2); // 01 11 21 31
		mmx_load7 = _mm_unpacklo_pi16(mmx_load3, mmx_load4); // 02 12 22 32
		mmx_load8 = _mm_unpackhi_pi16(mmx_load3, mmx_load4); // 03 13 23 33
		mmx_load5 = _mm_packs_pu16(mmx_load5, mmx_load5); 
		mmx_load6 = _mm_packs_pu16(mmx_load6, mmx_load6); 
		mmx_load7 = _mm_packs_pu16(mmx_load7, mmx_load7); 
		mmx_load8 = _mm_packs_pu16(mmx_load8, mmx_load8); 

		*(int *)SrcPtrP = _mm_cvtsi64_si32(mmx_load5);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load6);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load7);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load8);

		if (i++ == 1)
			return;

		SrcPtrP += p_step;
		goto STAGE; // next stage
}

static void FilterStrongChroma_Vert_c(int p_step, imgpel *SrcPtrP, int Alpha, int Beta)
{
	int i;
	for (i=0;i<8;i++)
	{
		imgpel  L0  = SrcPtrP[0];
		imgpel  R0  = SrcPtrP[1];
		if ( abs( R0 - L0 ) < Alpha ) 
		{
			imgpel R1  = SrcPtrP[2];
			if ( abs(R0 - R1) < Beta )  
			{
				imgpel L1  = SrcPtrP[-1];
				if ( abs(L0 - L1) < Beta )
				{
					SrcPtrP[0] = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
					SrcPtrP[1] = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
				}
			}
		}
		SrcPtrP+=p_step;
	}
}

static void FilterChroma_Vert_c(int p_step, imgpel *SrcPtrP, int Alpha, int Beta, const uint8_t Strength[4], const byte *ClipTab)
{
	int i;
	for (i=0;i<8;i++)
	{
		if (Strength[i>>1])
		{
		imgpel  L0  = *SrcPtrP;
		imgpel *SrcPtrQ = SrcPtrP + 1;
		imgpel  R0  = *SrcPtrQ;

		if ( abs( R0 - L0 ) < Alpha ) 
		{
			imgpel R1  = *(SrcPtrQ + 1);
			if ( abs(R0 - R1) < Beta )  
			{
				imgpel L1  = *(SrcPtrP - 1);
				if ( abs(L0 - L1) < Beta )
				{
					int tc0  = ClipTab[ Strength[(i*2)/4] ] * 1 + 1;
					int dif = iClip3( -tc0, tc0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

					*SrcPtrP = (imgpel) iClip1 ( 255, L0 + dif) ;
					*SrcPtrQ = (imgpel) iClip1 ( 255, R0 - dif) ;

				}
			}
		}
		}
		SrcPtrP+=p_step;
	}
}

void EdgeLoopChroma_Vert_YUV420(VideoImage *image, const uint8_t Strength[4], Macroblock *MbQ, int uv, PixelPos pixMB1, Macroblock *MbP)
{ 
	// dir == 0
	imgpel** Img = image->img;

	if (pixMB1.available || (MbQ->DFDisableIdc == 0))
	{
		int AlphaC0Offset = MbQ->DFAlphaC0Offset;
		int BetaOffset = MbQ->DFBetaOffset;

		// Average QP of the two blocks
		int QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
		int Alpha   = ALPHA_TABLE[indexA];
		if (Alpha)
		{
			int indexB = iClip3(0, MAX_QP, QP + BetaOffset);
			int Beta    = BETA_TABLE [indexB];

			if (Beta != 0)
			{
				const     byte *ClipTab = CLIP_TAB[indexA];
				const int stride = image->stride;
				imgpel *SrcPtrP = &(Img[pixMB1.pos_y >> 1][pixMB1.pos_x >> 1]);
		
				if (Strength[0] == 4)
				{
					FilterStrongChroma_Vert_sse(stride, SrcPtrP, Alpha, Beta);
				}
				else
				{
					FilterChroma_Vert_sse(stride, SrcPtrP, Alpha, Beta, Strength, ClipTab);
				}
			}
		}
	}
}

void EdgeLoopChromaMBAff_Vert_YUV420(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p)
{
	// dir == 0
	imgpel** Img = image->img;

	int      pel, Strng ;
	int      incP, incQ;
	int      C0, tc0, dif;
	imgpel   L0, R0;
	int      Alpha = 0, Beta = 0;
	const byte* ClipTab = NULL;
	int      indexA, indexB;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      StrengthIdx;
	int      QP;
	int      xQ, yQ;
	PixelPos pixP, pixQ;
	int      bitdepth_scale = p_Vid->bitdepth_scale[IS_CHROMA];
	int      max_imgpel_value = p_Vid->max_pel_value_comp[uv + 1];

	int      AlphaC0Offset = MbQ->DFAlphaC0Offset;
	int      BetaOffset    = MbQ->DFBetaOffset;
	byte fieldModeFilteringFlag;
	Macroblock *MbP;
	imgpel   *SrcPtrP, *SrcPtrQ;
	int      width = image->stride;

	for( pel = 0 ; pel < 8 ; ++pel )
	{
		xQ = edge;
		yQ = pel;
		getAffNeighbour(MbQ, xQ, yQ, p_Vid->mb_size[IS_CHROMA], &pixQ);
		getAffNeighbour(MbQ, xQ - 1, yQ, p_Vid->mb_size[IS_CHROMA], &pixP);    
		MbP = &(p_Vid->mb_data[pixP.mb_addr]);    
		StrengthIdx = ((MbQ->mb_field && !MbP->mb_field) ? pel << 1 :((pel >> 1) << 2) + (pel & 0x01));

		if (pixP.available || (MbQ->DFDisableIdc == 0))
		{
			if( (Strng = Strength[StrengthIdx]) != 0)
			{
				fieldModeFilteringFlag = (byte) (MbQ->mb_field || MbP->mb_field);
				incQ = 1;
				incP = 1;
				SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
				SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);

				// Average QP of the two blocks
				QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

				indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
				indexB = iClip3(0, MAX_QP, QP + BetaOffset);

				Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
				Beta    = BETA_TABLE [indexB] * bitdepth_scale;
				ClipTab = CLIP_TAB[indexA];


				L0  = SrcPtrP[0] ;
				R0  = SrcPtrQ[0] ;      


				if( abs( R0 - L0 ) < Alpha )
				{          
					imgpel L1  = SrcPtrP[-incP];
					imgpel R1  = SrcPtrQ[ incQ];      
					//if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  )
					if( ((abs( R0 - R1) - Beta < 0)  && (abs(L0 - L1) - Beta < 0 ))  )
					{
						if( Strng == 4 )    // INTRA strong filtering
						{
							SrcPtrQ[0] = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
							SrcPtrP[0] = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
						}
						else
						{
							C0  = ClipTab[ Strng ] * bitdepth_scale;
							tc0  = (C0 + 1);
							dif = iClip3( -tc0, tc0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

							SrcPtrP[0] = (imgpel) iClip1 ( max_imgpel_value, L0 + dif );
							SrcPtrQ[0] = (imgpel) iClip1 ( max_imgpel_value, R0 - dif );
						}
					}
				}
			}
		}
	}
}
