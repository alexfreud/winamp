#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"
#include <mmintrin.h>
#include <emmintrin.h>

static const byte ALPHA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,4,4,5,6,  7,8,9,10,12,13,15,17,  20,22,25,28,32,36,40,45,  50,56,63,71,80,90,101,113,  127,144,162,182,203,226,255,255} ;
static const byte  BETA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,2,2,2,3,  3,3,3, 4, 4, 4, 6, 6,   7, 7, 8, 8, 9, 9,10,10,  11,11,12,12,13,13, 14, 14,   15, 15, 16, 16, 17, 17, 18, 18} ;
static const byte CLIP_TAB[52][5]  =
{
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 1, 1, 1, 1},
	{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 2, 3, 3},
	{ 0, 1, 2, 3, 3},{ 0, 2, 2, 3, 3},{ 0, 2, 2, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 3, 3, 5, 5},{ 0, 3, 4, 6, 6},{ 0, 3, 4, 6, 6},
	{ 0, 4, 5, 7, 7},{ 0, 4, 5, 8, 8},{ 0, 4, 6, 9, 9},{ 0, 5, 7,10,10},{ 0, 6, 8,11,11},{ 0, 6, 8,13,13},{ 0, 7,10,14,14},{ 0, 8,11,16,16},
	{ 0, 9,12,18,18},{ 0,10,13,20,20},{ 0,11,15,23,23},{ 0,13,17,25,25}
} ;

static void IntraStrongFilter_Luma_Vert(int p_step, imgpel *SrcPtrP, imgpel *SrcPtrQ, int Alpha, int Beta)
{

	int pel;
	for (pel = 0; pel < BLOCK_SIZE; pel++, SrcPtrP+=p_step, SrcPtrQ+=p_step)
	{
		imgpel  L0 = SrcPtrP[0];
		imgpel  R0 = SrcPtrQ[0];

		if( abs( R0 - L0 ) < Alpha )
		{          
			imgpel  R1 = SrcPtrQ[1];
			imgpel  L1 = SrcPtrP[-1];
			if ((abs( R0 - R1) < Beta)  && (abs(L0 - L1) < Beta))
			{        
				imgpel  R2 = SrcPtrQ[2];
				imgpel  L2 = SrcPtrP[-2];

				int RL0 = L0 + R0;
				int small_gap = (abs( R0 - L0 ) < ((Alpha >> 2) + 2));
				int aq  = ( abs( R0 - R2) < Beta ) & small_gap;
				int ap  = ( abs( L0 - L2) < Beta ) & small_gap;

				if (ap)
				{
					int L1RL0 = L1 + RL0;
					imgpel  L3 = SrcPtrP[-3];
					SrcPtrP[0]              = (imgpel)  (( R1 + ((L1RL0) << 1) +  L2 + 4) >> 3);
					SrcPtrP[-1] = (imgpel)  (( L2 + L1RL0 + 2) >> 2);
					SrcPtrP[-2] = (imgpel) ((((L3 + L2) <<1) + L2 + L1RL0 + 4) >> 3);
				}
				else
				{
					*SrcPtrP = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2) ;
				}

				if (aq)
				{
					imgpel  R3 = SrcPtrQ[3];
					SrcPtrQ[0] = (imgpel) (( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3);
					SrcPtrQ[1] = (imgpel) (( R2 + R0 + L0 + R1 + 2) >> 2);
					SrcPtrQ[2] = (imgpel) ((((R3 + R2) <<1) + R2 + R1 + RL0 + 4) >> 3);
				}
				else
				{
					SrcPtrQ[0] = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
				}
			}
		}
	}
}

static void FilterLuma_Vert(int p_step, imgpel *SrcPtrP, imgpel *SrcPtrQ, int Alpha, int Beta, int C0, int max_imgpel_value)
{
	int pel;
	for (pel = 0; pel < BLOCK_SIZE; pel++, SrcPtrP+=p_step, SrcPtrQ+=p_step)
	{
		imgpel  L0 = SrcPtrP[0];
		imgpel  R0 = SrcPtrQ[0];

		if( abs( R0 - L0 ) < Alpha )
		{          
			imgpel  R1 = SrcPtrQ[1];
			if (abs( R0 - R1) < Beta)
			{
				imgpel  L1 = SrcPtrP[-1];
				if (abs(L0 - L1) < Beta)
				{        
					imgpel  R2 = SrcPtrQ[2];
					imgpel  L2 = SrcPtrP[-2];

					int RL0 = (L0 + R0 + 1) >> 1;
					int aq  = (abs(R0 - R2) < Beta);
					int ap  = (abs(L0 - L2) < Beta);

					//int C0  = ClipTab[ *Strength ] * bitdepth_scale;
					int tc0  = (C0 + ap + aq) ;
					int dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

					if( ap )
						SrcPtrP[-1] += iClip3( -C0,  C0, (L2 + RL0 - (L1<<1)) >> 1 );
					SrcPtrP[0] = (imgpel) iClip1(max_imgpel_value, L0 + dif);

					SrcPtrQ[0] = (imgpel) iClip1(max_imgpel_value, R0 - dif);
					if( aq  )
						SrcPtrQ[1] += iClip3( -C0,  C0, (R2 + RL0 - (R1<<1)) >> 1 );
				}
			}
		}
	}
}

void EdgeLoopLumaNormal_Vert(ColorPlane pl, VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p)
{
	// dir == 0
	imgpel **Img = image->img;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      xQ = edge - 1;

	PixelPos pixMB1;
	p_Vid->getNeighbourX0(MbQ, xQ, p_Vid->mb_size[IS_LUMA], &pixMB1); 

	if (pixMB1.available || (MbQ->DFDisableIdc== 0))
	{   
		int bitdepth_scale   = pl ? p_Vid->bitdepth_scale[IS_CHROMA] : p_Vid->bitdepth_scale[IS_LUMA];
		ptrdiff_t p_step = image->stride;

		Macroblock *MbP  = &(p_Vid->mb_data[pixMB1.mb_addr]);

		// Average QP of the two blocks
		int QP = pl? ((MbP->qpc[pl-1] + MbQ->qpc[pl-1] + 1) >> 1) : (MbP->qp + MbQ->qp + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
		int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);

		int Alpha  = ALPHA_TABLE[indexA] * bitdepth_scale;
		int Beta   = BETA_TABLE [indexB] * bitdepth_scale;

		if (Alpha != 0 && Beta !=0)
		{
			PixelPos pixMB2;
			const byte *ClipTab = CLIP_TAB   [indexA];
			int max_imgpel_value = p_Vid->max_pel_value_comp[pl];
			int pel;
			imgpel *SrcPtrQ;
			imgpel *SrcPtrP = image->base_address + pixMB1.pos_y * image->stride + pixMB1.pos_x;

			p_Vid->getNeighbourX0(MbQ, ++xQ, p_Vid->mb_size[IS_LUMA], &pixMB2);
			SrcPtrQ = image->base_address + pixMB2.pos_y * image->stride + pixMB2.pos_x;

			for( pel = 0 ; pel < MB_BLOCK_SIZE ; pel+=BLOCK_SIZE)
			{
				byte strength = Strength[pel];

				switch(strength)
				{
				case 0:
					break;
				case 4: // INTRA strong
					{
						IntraStrongFilter_Luma_Vert(p_step, SrcPtrP, SrcPtrQ, Alpha, Beta);
					}
					break;
				default:
					{
						int C0  = ClipTab[strength] * bitdepth_scale;
						FilterLuma_Vert(p_step, SrcPtrP, SrcPtrQ, Alpha, Beta, C0, max_imgpel_value);
					}
					break;
				}
				SrcPtrP += p_step * BLOCK_SIZE;
				SrcPtrQ += p_step * BLOCK_SIZE;
			}
		}
	}
}


static void FilterLuma_Vert_sse2(int p_step, imgpel *SrcPtrP, int Alpha, int Beta, const uint8_t Strength[4], const byte *ClipTab)
{

	__m64 mmx_alpha_minus_one = _mm_set1_pi16(Alpha-1), mmx_beta_minus_one = _mm_set1_pi16(Beta-1);
	__m64 mmx_zero	= _mm_setzero_si64(), mmx_one, mmx_four=_mm_set1_pi16(4);
	__m64 mmx_minus_one;
	__m64 mmx_absdiff, mmx_diff;
	__m64 mmx_L0, mmx_L1, mmx_L2, mmx_L0_R0;
	__m64 mmx_R0, mmx_R1_R2, mmx_R1, mmx_R2;
	__m64 mmx_load0, mmx_load1, mmx_load2, mmx_load3, mmx_load4, mmx_load5, mmx_load6, mmx_load7, mmx_load8;
	__m64 mmx_ap, mmx_aq, mmx_C0, mmx_negative_C0, mmx_tc0, mmx_dif, mmx_acc, mmx_match;
	int match;
	int i=0;

	mmx_minus_one = _mm_set1_pi32(-1);
	mmx_one = _mm_sub_pi16(mmx_zero, mmx_minus_one); // dunno if this'll be faster than _mm_set1_pi16 or not
	SrcPtrP -= 2;

	STAGE:

	while (!Strength[i])
	{
		SrcPtrP += p_step << 2;
		if (i++ == 3) // last stage
			return; 
	}

		mmx_load0 = (*(__m64 *)(SrcPtrP));                    // La2 La1 La0 Ra0 Ra1 Ra2 --- ---
		mmx_load1 = (*(__m64 *)(SrcPtrP+=p_step));            // Lb2 Lb1 Lb0 Rb0 Rb1 Rb2 --- ---
		mmx_load4 = _mm_unpacklo_pi8(mmx_load0, mmx_load1);   // La2 Lb2 La1 Lb1 La0 Lb0 Ra0 Rb0 *
		mmx_load2 = (*(__m64 *)(SrcPtrP+=p_step));            // Lc2 Lc1 Lc0 Rc0 Rc1 Rc2 --- ---
		mmx_load3 = (*(__m64 *)(SrcPtrP+=p_step));            // Ld2 Ld1 Ld0 Rd0 Rd1 Rd2 --- ---
		SrcPtrP+=p_step;
		mmx_load5 = _mm_unpacklo_pi8(mmx_load2, mmx_load3);   // Lc2 Ld2 Lc1 Ld1 Lc0 Ld0 Rc0 Rd0 *
		mmx_L0_R0 = _mm_unpackhi_pi16(mmx_load4, mmx_load5);  // La0 Lb0 Lc0 Ld0 Ra0 Rb0 Rc0 Rd0

		// abs( R0 - L0 ) < Alpha
		// MMX doesn't have unsigned compare, so we have to go to short
		mmx_L0 = _mm_unpacklo_pi8(mmx_L0_R0, mmx_zero);             // La0 Lb0 Lc0 Ld0
		mmx_R0 = _mm_unpackhi_pi8(mmx_L0_R0, mmx_zero);          // Ra0 Rb0 Rc0 Rd0
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_L0);
		mmx_absdiff =_mm_subs_pu16(mmx_L0, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_match = _mm_cmpgt_pi16(mmx_absdiff, mmx_alpha_minus_one); // 1's in any words we don't have to do
		mmx_match = _mm_xor_si64(mmx_match, mmx_minus_one);
		match = _mm_movemask_pi8(mmx_match); 
		if (match == 0)
		{
			if (i++ == 3) // last stage
				return;
				
			goto STAGE; // start the process over from next position
		}

		mmx_load6 = _mm_unpackhi_pi8(mmx_load0, mmx_load1);       // Ra1 Rb1 Ra2 Rb2 --- --- --- --- *
		mmx_load7 = _mm_unpackhi_pi8(mmx_load2, mmx_load3);       // Rc1 Rd1 Rc2 Rd2 --- --- --- --- *
		mmx_R1_R2 = _mm_unpacklo_pi16(mmx_load6, mmx_load7);      // Ra1 Rb1 Rc1 Rd1 Ra2 Rb2 Rc2 Rd2

		// abs( R0 - R1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_R0 already populated
		mmx_R1 = _mm_unpacklo_pi8(mmx_R1_R2, mmx_zero);           // Ra1 Rb1 Rc1 Rd1
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_R1);
		mmx_absdiff =_mm_subs_pu16(mmx_R1, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 3) // last stage
				return;
				
			goto STAGE; // start the process over from next position
		}

		// abs(L0 - L1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_L0 already populated
		// mmx_load4: La2 Lb2 La1 Lb1 La0 Lb0 --- ---
		// mmx_load5: Lc2 Ld2 Lc1 Ld1 Lc0 Ld0 --- ---
		mmx_load4 = _mm_unpacklo_pi16(mmx_load4, mmx_load5); // La2 Lb2 Lc2 Ld2 La1 Lb1 Lc1 Ld1 
		mmx_L1 = _mm_unpackhi_pi8(mmx_load4, mmx_zero);      // La1 Lb1 Lc1 Ld1
		mmx_diff=_mm_subs_pu16(mmx_L0, mmx_L1);
		mmx_absdiff =_mm_subs_pu16(mmx_L1, mmx_L0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 3) // last stage
				return;

			goto STAGE; // start the process over from next position
		}

		// ok, now time to performn the actual calculation. hope it was worth it!!
		
		// ap  = (abs(L0 - L2) < Beta);
		// finish loading L2 
		mmx_L2 = _mm_unpacklo_pi8(mmx_load4, mmx_zero);      // La1 Lb1 Lc1 Ld1
		mmx_diff=_mm_subs_pu16(mmx_L0, mmx_L2);
		mmx_absdiff =_mm_subs_pu16(mmx_L2, mmx_L0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_ap = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_tc0 = _mm_add_pi16(mmx_ap, mmx_one); // a clever trick. add one to essential do !mmx_absdiff   (since mmx_diff will == 0xFFFF when true)

		// aq  = (abs(R0 - R2) < Beta);
		// finish loading R2
                                        // 		mmx_R1_R2: Ra1 Rb1 Rc1 Rd1 Ra2 Rb2 Rc2 Rd2
		mmx_R2 = _mm_unpackhi_pi8(mmx_R1_R2, mmx_zero);   // Ra2 Rb2 Rc2 Rd2 
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_R2);
		mmx_absdiff =_mm_subs_pu16(mmx_R2, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_aq = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_tc0 = _mm_add_pi16(mmx_tc0, _mm_add_pi16(mmx_aq, mmx_one)); // a clever trick. add one to essential do !mmx_absdiff   (since mmx_diff will == 0xFFFF when true)

		// tc0  = (C0 + ap + aq) ;
		mmx_C0 = _mm_set1_pi16(ClipTab[Strength[i]]);
		mmx_negative_C0 = _mm_sub_pi16(mmx_zero, mmx_C0);
		mmx_tc0 = _mm_add_pi16(mmx_tc0, mmx_C0);


		// dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );
		mmx_dif = mmx_R0;
		mmx_dif = _mm_sub_pi16(mmx_dif, mmx_L0);
		mmx_dif = _mm_slli_pi16(mmx_dif, 2);
		mmx_dif = _mm_add_pi16(mmx_dif, mmx_L1);
		mmx_dif = _mm_sub_pi16(mmx_dif, mmx_R1);
		mmx_dif = _mm_add_pi16(mmx_dif, mmx_four);
		mmx_dif = _mm_srai_pi16(mmx_dif, 3);
		mmx_dif = _mm_min_pi16(mmx_dif, mmx_tc0);
		mmx_tc0 = _mm_sub_pi16(mmx_zero, mmx_tc0);
		mmx_dif = _mm_max_pi16(mmx_dif, mmx_tc0);
		mmx_dif = _mm_and_si64(mmx_dif, mmx_match);

				// TODO: benski> is it worth checking for_mm_movemask_pi8(ap) to see if we can skip this?
				// if( ap ) L1 += iClip3( -C0,  C0, (L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1 );
		mmx_acc = mmx_L0;
		mmx_acc = _mm_add_pi16(mmx_acc, mmx_R0);
		mmx_acc = _mm_add_pi16(mmx_acc, mmx_one);
		mmx_acc = _mm_srai_pi16(mmx_acc, 1);
		mmx_acc = _mm_sub_pi16(mmx_acc, mmx_L1);
		mmx_acc = _mm_sub_pi16(mmx_acc, mmx_L1);
		mmx_acc = _mm_add_pi16(mmx_acc, mmx_L2);
		mmx_acc = _mm_srai_pi16(mmx_acc, 1);
		mmx_acc = _mm_min_pi16(mmx_acc, mmx_C0);
		mmx_acc = _mm_max_pi16(mmx_acc, mmx_negative_C0);
		mmx_acc = _mm_andnot_si64(mmx_ap, mmx_acc);
		mmx_acc = _mm_and_si64(mmx_acc, mmx_match);
		mmx_L1 = _mm_add_pi16(mmx_L1, mmx_acc);


			//if( aq  )						R1 += iClip3( -C0,  C0, (R2 + RL0 - (R1<<1)) >> 1 );
			mmx_acc = mmx_L0;
		mmx_acc = _mm_add_pi16(mmx_acc, mmx_R0);
		mmx_acc = _mm_add_pi16(mmx_acc, mmx_one);
		mmx_acc = _mm_srai_pi16(mmx_acc, 1);
		mmx_acc = _mm_sub_pi16(mmx_acc, mmx_R1);
		mmx_acc = _mm_sub_pi16(mmx_acc, mmx_R1);
		mmx_acc = _mm_add_pi16(mmx_acc, mmx_R2);
		mmx_acc = _mm_srai_pi16(mmx_acc, 1);
		mmx_acc = _mm_min_pi16(mmx_acc, mmx_C0);
		mmx_acc = _mm_max_pi16(mmx_acc, mmx_negative_C0);
		mmx_acc = _mm_andnot_si64(mmx_aq, mmx_acc);
		mmx_acc = _mm_and_si64(mmx_acc, mmx_match);
		mmx_R1 = _mm_add_pi16(mmx_R1, mmx_acc);

		// L0 = (imgpel) iClip1(max_imgpel_value, L0 + dif);
		mmx_L0 = _mm_add_pi16(mmx_L0, mmx_dif);

		// R0 = (imgpel) iClip1(max_imgpel_value, R0 - dif);
		mmx_R0 = _mm_sub_pi16(mmx_R0, mmx_dif);


	// now for the super-exciting fun of getting this data back into memory
		SrcPtrP -= 4*p_step;
		//SrcPtrQ -= 4*p_step;
		SrcPtrP++;

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

		//mmx_load1 = _mm_setr_pi16(0x8080, 0x80, 0, 0);
		*(int *)SrcPtrP = _mm_cvtsi64_si32(mmx_load5);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load6);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load7);
		*(int *)(SrcPtrP+=p_step) = _mm_cvtsi64_si32(mmx_load8);

		if (i++ == 3)
			return;

		//SrcPtrQ += 2;
		SrcPtrP += p_step;
		//SrcPtrQ += p_step;
		SrcPtrP--;
		goto STAGE; // next stage
}

/* assumptions: YUV 420, getNonAffNeighbour */
void EdgeLoopLuma_Vert_YUV420(VideoImage *image, const uint8_t Strength[4], Macroblock *MbQ, PixelPos pixMB1, Macroblock *MbP)
{
	// dir == 0
	if (MbQ->DFDisableIdc== 0)
	{   
		ptrdiff_t p_step = image->stride;

		// Average QP of the two blocks
		int QP = (MbP->qp + MbQ->qp + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
		int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);

		int Alpha  = ALPHA_TABLE[indexA];
		int Beta   = BETA_TABLE [indexB];

		if (Alpha != 0 && Beta !=0)
		{
			imgpel *SrcPtrP = image->base_address + pixMB1.pos_y * image->stride + pixMB1.pos_x;

			if (Strength[0] == 4) // if strong filter is used, all blocks will be strong
			{
				imgpel *SrcPtrQ = SrcPtrP+1;
				int pel;
				for( pel = 0 ; pel < BLOCK_SIZE ; pel++)
				{
					IntraStrongFilter_Luma_Vert(p_step, SrcPtrP, SrcPtrQ, Alpha, Beta);
					SrcPtrP += p_step * BLOCK_SIZE;
					SrcPtrQ += p_step * BLOCK_SIZE;
				}
			}
			else
			{
				const byte *ClipTab = CLIP_TAB   [indexA];
				FilterLuma_Vert_sse2(p_step, SrcPtrP, Alpha, Beta, Strength, ClipTab);
			}
		}
	}
}

void EdgeLoopLumaMBAff_Vert_YUV420(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, StorablePicture *p)
{
	// dir == 0
	imgpel **Img = image->img;
	int      width = image->stride;
	int      pel, ap = 0, aq = 0, Strng ;

	int      C0, tc0, dif;
	imgpel   L0, R0;
	int      Alpha = 0, Beta = 0 ;
	const byte* ClipTab = NULL;
	int      small_gap;
	int      indexA, indexB;

	int      QP;
	int      xQ, yQ;

	PixelPos pixP, pixQ;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      bitdepth_scale = p_Vid->bitdepth_scale[IS_LUMA];
	int      max_imgpel_value = p_Vid->max_pel_value_comp[PLANE_Y];

	int AlphaC0Offset = MbQ->DFAlphaC0Offset;
	int BetaOffset = MbQ->DFBetaOffset;

	Macroblock *MbP;
	imgpel   *SrcPtrP, *SrcPtrQ;

	for( pel = 0 ; pel < MB_BLOCK_SIZE ; ++pel )
	{
		xQ = edge;
		yQ = pel;
		getAffNeighbourXPLuma(MbQ, xQ - 1, yQ, &pixP);     

		if (pixP.available || (MbQ->DFDisableIdc== 0))
		{
			if( (Strng = Strength[pel]) != 0)
			{
				getAffNeighbourXPLuma(MbQ, xQ, yQ, &pixQ); // TODO: PP

				MbP = &(p_Vid->mb_data[pixP.mb_addr]);

				SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
				SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);

				// Average QP of the two blocks
				QP = (MbP->qp + MbQ->qp + 1) >> 1;

				indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
				indexB = iClip3(0, MAX_QP, QP + BetaOffset);

				Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
				Beta    = BETA_TABLE [indexB] * bitdepth_scale;
				ClipTab = CLIP_TAB[indexA];

				L0  = SrcPtrP[0] ;
				R0  = SrcPtrQ[0] ;      

				if( abs( R0 - L0 ) < Alpha )
				{          
					imgpel L1  = SrcPtrP[-1];
					imgpel R1  = SrcPtrQ[ 1];      
					if ((abs( R0 - R1) < Beta )   && (abs(L0 - L1) < Beta ))
					{
						imgpel L2  = SrcPtrP[-2];
						imgpel R2  = SrcPtrQ[ 2];
						if(Strng == 4 )    // INTRA strong filtering
						{
							int RL0 = L0 + R0;
							small_gap = (abs( R0 - L0 ) < ((Alpha >> 2) + 2));
							aq  = ( abs( R0 - R2) < Beta ) & small_gap;               
							ap  = ( abs( L0 - L2) < Beta ) & small_gap;

							if (ap)
							{
								imgpel L3  = SrcPtrP[-3];
								SrcPtrP[-2] = (imgpel) ((((L3 + L2) << 1) + L2 + L1 + RL0 + 4) >> 3);
								SrcPtrP[-1    ] = (imgpel) (( L2 + L1 + L0 + R0 + 2) >> 2);
								SrcPtrP[    0    ] = (imgpel) (( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3);
							}
							else
							{
								SrcPtrP[     0     ] = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2) ;
							}

							if (aq)
							{
								imgpel R3  = SrcPtrQ[ 3];
								SrcPtrQ[    0     ] = (imgpel) (( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3);
								SrcPtrQ[ 1     ] = (imgpel) (( R2 + R0 + R1 + L0 + 2) >> 2);
								SrcPtrQ[  2 ] = (imgpel) ((((R3 + R2) << 1) + R2 + R1 + RL0 + 4) >> 3);
							}
							else
							{
								SrcPtrQ[    0     ] = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
							}
						}
						else   // normal filtering
						{              
							int RL0 = (L0 + R0 + 1) >> 1;
							aq  = (abs( R0 - R2) < Beta);
							ap  = (abs( L0 - L2) < Beta);

							C0  = ClipTab[ Strng ] * bitdepth_scale;
							tc0  = (C0 + ap + aq) ;
							dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3) ;

							if( ap )
								*(SrcPtrP - 1) += iClip3( -C0,  C0, ( L2 + RL0 - (L1 << 1)) >> 1 ) ;

							*SrcPtrP  = (imgpel) iClip1 (max_imgpel_value, L0 + dif) ;
							*SrcPtrQ  = (imgpel) iClip1 (max_imgpel_value, R0 - dif) ;

							if( aq  )
								*(SrcPtrQ + 1) += iClip3( -C0,  C0, ( R2 + RL0 - (R1 << 1)) >> 1 ) ;
						}            
					}
				}
			}
		}
	}
}
