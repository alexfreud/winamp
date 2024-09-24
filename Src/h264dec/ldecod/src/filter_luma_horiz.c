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

// benski> used for unit testing, not in production code
static int CalculateMatches(int inc_dim, const imgpel *SrcPtrP, const imgpel *SrcPtrQ, int Alpha, int Beta)
{
	int match=0;
	const imgpel *P_L1 = SrcPtrP - inc_dim;
	const imgpel *Q_R1 = SrcPtrQ + inc_dim;


	int pel;
	for (pel = 0; pel < BLOCK_SIZE; pel++, SrcPtrP++, SrcPtrQ++, Q_R1++, P_L1++)
	{
		imgpel  L0 = *SrcPtrP;
		imgpel  R0 = *SrcPtrQ;

		if( abs( R0 - L0 ) < Alpha )
		{          
			imgpel  R1 = *Q_R1;
			if ((abs( R0 - R1) < Beta))
			{
				imgpel  L1 = *P_L1;
				if ((abs(L0 - L1) < Beta))
				{
					match |= (1 << (pel*2));
					match |= (1 << (pel*2+1));
				}
			}
		}
	}
	return match;
}

static void IntraStrongFilter_Luma_Horiz(int inc_dim, imgpel *SrcPtrP, imgpel *SrcPtrQ, int Alpha, int Beta)
{

	imgpel *P_L1 = SrcPtrP - inc_dim;
	imgpel *P_L2 = P_L1 - inc_dim;
	const imgpel *P_L3 = P_L2 - inc_dim;

	imgpel *Q_R1 = SrcPtrQ + inc_dim;
	imgpel *Q_R2 = Q_R1 + inc_dim;
	const imgpel *Q_R3 = Q_R2 + inc_dim;


	int pel;
	for (pel = 0; pel < BLOCK_SIZE; pel++, SrcPtrP++, SrcPtrQ++, Q_R1++, P_L1++, Q_R2++, P_L2++, Q_R3++, P_L3++)
	{
		imgpel  L0 = *SrcPtrP;
		imgpel  R0 = *SrcPtrQ;

		if( abs( R0 - L0 ) < Alpha )
		{          
			imgpel  R1 = *Q_R1;
			if ((abs( R0 - R1) < Beta))
			{
				imgpel  L1 = *P_L1;
				if ((abs(L0 - L1) < Beta))
				{        
					imgpel  R2 = *Q_R2;
					imgpel  L2 = *P_L2;

					int RL0 = L0 + R0;
					int small_gap = (abs( R0 - L0 ) < ((Alpha >> 2) + 2));
					int aq  = ( abs( R0 - R2) < Beta ) & small_gap;
					int ap  = ( abs( L0 - L2) < Beta ) & small_gap;

					if (ap)
					{
						int L1RL0 = L1 + RL0;
						imgpel  L3 = *P_L3;
						*SrcPtrP              = (imgpel)  (( R1 + ((L1RL0) << 1) +  L2 + 4) >> 3);
						*P_L1 = (imgpel)  (( L2 + L1RL0 + 2) >> 2);
						*P_L2 = (imgpel) ((((L3 + L2) <<1) + L2 + L1RL0 + 4) >> 3);
					}
					else
					{
						*SrcPtrP = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2) ;                
					}

					if (aq)
					{
						imgpel  R3 = *Q_R3;
						*(SrcPtrQ            ) = (imgpel) (( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3);
						*Q_R1 = (imgpel) (( R2 + R0 + L0 + R1 + 2) >> 2);
						*Q_R2 = (imgpel) ((((R3 + R2) <<1) + R2 + R1 + RL0 + 4) >> 3);
					}
					else
					{
						*SrcPtrQ = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
					}
				}
			}
		}
	}
}

#define LOAD_LINE_EPI16(reg, ptr) { reg = _mm_loadl_epi64((__m128i *)(ptr));	reg = _mm_unpacklo_epi8(reg, xmm_zero); }
static void IntraStrongFilter_Luma_Horiz_sse2(int inc_dim, imgpel *SrcPtrP, imgpel *SrcPtrQ, __m128i xmm_alpha, __m128i xmm_beta, __m128i xmm_match)
{
	__m128i xmm_zero = _mm_setzero_si128();
	__m128i xmm_smallgap;
	__m128i xmm_ap, xmm_aq;
	__m128i xmm_L3, xmm_L2, xmm_L1, xmm_L0, xmm_R0, xmm_R1, xmm_R2, xmm_R3;
	__m128i xmm_4 = _mm_set1_epi16(4), xmm_2 = _mm_set1_epi16(2);
	__m128i xmm_add, xmm_add2, xmm_acc, xmm_match_and_an;
	__m128i xmm_absdiff, xmm_diff;

	LOAD_LINE_EPI16(xmm_L0, SrcPtrP);
	LOAD_LINE_EPI16(xmm_R0, SrcPtrQ);

	// small_gap = (abs( R0 - L0 ) < ((Alpha >> 2) + 2));
	xmm_alpha = _mm_srai_epi16(xmm_alpha, 2);
	xmm_alpha = _mm_add_epi16(xmm_alpha, xmm_2);
	xmm_diff=_mm_subs_epu16(xmm_R0, xmm_L0);
	xmm_absdiff =_mm_subs_epu16(xmm_L0, xmm_R0);
	xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
	xmm_smallgap = _mm_cmplt_epi16(xmm_absdiff, xmm_alpha);

	LOAD_LINE_EPI16(xmm_R2, SrcPtrQ + 2*inc_dim);

	// (abs(R0 - R2) < Beta) & small_gap;
	xmm_diff=_mm_subs_epu16(xmm_R0, xmm_R2);
	xmm_absdiff =_mm_subs_epu16(xmm_R2, xmm_R0);
	xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
	xmm_aq = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
	xmm_aq = _mm_and_si128(xmm_aq, xmm_smallgap);

	LOAD_LINE_EPI16(xmm_L2, SrcPtrP - 2*inc_dim);

	//  (abs(L0 - L2) < Beta) & small_gap;
	xmm_diff=_mm_subs_epu16(xmm_L0, xmm_L2);
	xmm_absdiff =_mm_subs_epu16(xmm_L2, xmm_L0);
	xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
	xmm_ap = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
	xmm_ap = _mm_and_si128(xmm_ap, xmm_smallgap);

	LOAD_LINE_EPI16(xmm_L1, SrcPtrP - inc_dim);
	LOAD_LINE_EPI16(xmm_R1, SrcPtrQ + inc_dim);
	LOAD_LINE_EPI16(xmm_L3, SrcPtrP - 3*inc_dim);
	LOAD_LINE_EPI16(xmm_R3, SrcPtrQ + 3*inc_dim);

	xmm_match_and_an=_mm_and_si128(xmm_match, xmm_ap);

	// if(ap) SrcPtrP   = (imgpel)  (( R1 + ((L1 + L0 + R0) << 1) +  L2 + 4) >> 3)
	xmm_add = xmm_L1;
	xmm_add = _mm_add_epi16(xmm_add, xmm_L0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_slli_epi16(xmm_add, 1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_4);
	xmm_add = _mm_srai_epi16(xmm_add, 3);
	xmm_acc = _mm_and_si128(xmm_add, xmm_match_and_an);

	// if (ap) *P_L1 = (imgpel)  (( L2 + L1 + L0 + R0 + 2) >> 2);
	xmm_add = xmm_L2;
	xmm_add = _mm_add_epi16(xmm_add, xmm_L1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_2);
	xmm_add = _mm_srai_epi16(xmm_add, 2);
	xmm_add = _mm_and_si128(xmm_add, xmm_match_and_an);
	xmm_add2= xmm_L1;
	xmm_add2= _mm_andnot_si128(xmm_match_and_an, xmm_add2);
	xmm_add=_mm_add_epi16(xmm_add, xmm_add2);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrP-inc_dim), xmm_add); 


	// if (ap) *P_L2 = (imgpel) ((((L3 + L2) <<1) + L2 + L1 + L0 + R0 + 4) >> 3);
	xmm_add = xmm_L3;
	xmm_add = _mm_add_epi16(xmm_add, xmm_L2);
	xmm_add = _mm_slli_epi16(xmm_add, 1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_4);
	xmm_add = _mm_srai_epi16(xmm_add, 3);
	xmm_add = _mm_and_si128(xmm_add, xmm_match_and_an);
	xmm_add2= xmm_L2;
	xmm_add2= _mm_andnot_si128(xmm_match_and_an, xmm_add2);
	xmm_add=_mm_add_epi16(xmm_add, xmm_add2);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrP-2*inc_dim), xmm_add); 

	// if (!ap) *SrcPtrP = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2) ; 
	xmm_add = xmm_L1;
	xmm_add = _mm_slli_epi16(xmm_add, 1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_2);
	xmm_add = _mm_srai_epi16(xmm_add, 2);
	xmm_add = _mm_and_si128(xmm_add, xmm_match);
	xmm_add = _mm_andnot_si128(xmm_ap, xmm_add);
	xmm_add2= xmm_L0;
	//xmm_match_and_an=_mm_or_si128(xmm_match, xmm_ap);
	xmm_add2=_mm_andnot_si128(xmm_match, xmm_add2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_add2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_acc);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrP), xmm_add); 

	xmm_match_and_an=_mm_and_si128(xmm_match, xmm_aq);

	// if (aq) *(SrcPtrQ            ) = (imgpel) (( L1 + ((R1 + L0 + R0) << 1) +  R2 + 4) >> 3);
	xmm_add = xmm_R1;
	xmm_add = _mm_add_epi16(xmm_add, xmm_L0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_slli_epi16(xmm_add, 1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_4);
	xmm_add = _mm_srai_epi16(xmm_add, 3);
	xmm_acc = _mm_and_si128(xmm_add, xmm_match_and_an);

	// if (aq) *Q_R1 = (imgpel) (( R2 + R0 + L0 + R1 + 2) >> 2);
	xmm_add = xmm_R2;
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_2);
	xmm_add = _mm_srai_epi16(xmm_add, 2);
	xmm_add = _mm_and_si128(xmm_add, xmm_match_and_an);
	xmm_add2= xmm_R1;
	xmm_add2= _mm_andnot_si128(xmm_match_and_an, xmm_add2);
	xmm_add=_mm_add_epi16(xmm_add, xmm_add2);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrQ+inc_dim), xmm_add); 

	// if (aq) *Q_R2 = (imgpel) ((((R3 + R2) <<1) + R2 + R1 + L0 + R0 + 4) >> 3);
	xmm_add = xmm_R3;
	xmm_add = _mm_add_epi16(xmm_add, xmm_R2);
	xmm_add = _mm_slli_epi16(xmm_add, 1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_4);
	xmm_add = _mm_srai_epi16(xmm_add, 3);
	xmm_add = _mm_and_si128(xmm_add, xmm_match_and_an);
	xmm_add2= xmm_R2;
	xmm_add2= _mm_andnot_si128(xmm_match_and_an, xmm_add2);
	xmm_add=_mm_add_epi16(xmm_add, xmm_add2);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);

	_mm_storel_epi64((__m128i *)(SrcPtrQ+2*inc_dim), xmm_add); 

	// if (!aq) *SrcPtrQ = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
	xmm_add = xmm_R1;
	xmm_add = _mm_slli_epi16(xmm_add, 1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_2);
	xmm_add = _mm_srai_epi16(xmm_add, 2);
	xmm_add = _mm_and_si128(xmm_add, xmm_match);
	xmm_add = _mm_andnot_si128(xmm_aq, xmm_add);
	xmm_add2= xmm_R0;
	//xmm_match_and_an=_mm_or_si128(xmm_match, xmm_aq);
	xmm_add2=_mm_andnot_si128(xmm_match, xmm_add2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_add2);
	xmm_add = _mm_add_epi16(xmm_add, xmm_acc);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);

	_mm_storel_epi64((__m128i *)(SrcPtrQ), xmm_add); 


}

// benski> for some reason, Visual Studio 2008 only allows for 3 __m128i parameters, or else we'd pass a whole lot more for optimization reasons
// we could put this function straight into EdgeLoopLumaNormal_Horiz_sse2 if we think it's worth it
static void FilterLuma_Horiz_sse2(int inc_dim, imgpel *SrcPtrP, imgpel *SrcPtrQ, __m128i xmm_beta, int C0[2], __m128i xmm_match)
{
	__m128i xmm_zero = _mm_setzero_si128();
	__m128i xmm_C0 = _mm_setr_epi16(C0[0], C0[0], C0[0], C0[0], C0[1], C0[1], C0[1], C0[1]); // TODO: benski> probably a better way to do this.
	__m128i xmm_negative_C0;
	__m128i xmm_tc0;
	__m128i xmm_L2, xmm_L1, xmm_L0, xmm_R0, xmm_R1, xmm_R2;
	__m128i xmm_absdiff, xmm_diff;
	__m128i xmm_dif;
	__m128i xmm_4 = _mm_set1_epi16(4), xmm_1 = _mm_set1_epi16(1);
	__m128i xmm_add;
	__m128i xmm_ap, xmm_aq;

	xmm_negative_C0 = _mm_sub_epi16(xmm_zero, xmm_C0);
	xmm_tc0 = xmm_C0;

	xmm_R2 = _mm_loadl_epi64((__m128i *)(SrcPtrQ + 2*inc_dim));
	xmm_R2 = _mm_unpacklo_epi8(xmm_R2, xmm_zero);

	xmm_R0 = _mm_loadl_epi64((__m128i *)(SrcPtrQ));
	xmm_R0 = _mm_unpacklo_epi8(xmm_R0, xmm_zero);

	// (abs(R0 - R2) < Beta);
	xmm_diff=_mm_subs_epu16(xmm_R0, xmm_R2);
	xmm_absdiff =_mm_subs_epu16(xmm_R2, xmm_R0);
	xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
	xmm_aq = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
	xmm_add = _mm_srli_epi16(xmm_aq, 15); // convert 0xFFFF to 1 and 0x0000 to 0
	xmm_tc0 = _mm_adds_epu16(xmm_tc0, xmm_add); 	// tc0  = (C0 + ap + aq) ;

	xmm_L2 = _mm_loadl_epi64((__m128i *)(SrcPtrP - 2*inc_dim));
	xmm_L2 = _mm_unpacklo_epi8(xmm_L2, xmm_zero);

	xmm_L0 = _mm_loadl_epi64((__m128i *)(SrcPtrP));
	xmm_L0 = _mm_unpacklo_epi8(xmm_L0, xmm_zero);

	//  (abs(L0 - L2) < Beta);
	xmm_diff=_mm_subs_epu16(xmm_L2, xmm_L0);
	xmm_absdiff =_mm_subs_epu16(xmm_L0, xmm_L2);
	xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
	xmm_ap = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
	xmm_add = _mm_srli_epi16(xmm_ap, 15); // convert 0xFFFF to 1 and 0x0000 to 0
	xmm_tc0 = _mm_adds_epu16(xmm_tc0, xmm_add); 	// tc0  = (C0 + ap + aq) ;

	xmm_L1 = _mm_loadl_epi64((__m128i *)(SrcPtrP - inc_dim));
	xmm_L1 = _mm_unpacklo_epi8(xmm_L1, xmm_zero);

	xmm_R1 = _mm_loadl_epi64((__m128i *)(SrcPtrQ + inc_dim));
	xmm_R1 = _mm_unpacklo_epi8(xmm_R1, xmm_zero);

	// dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + L1 - R1 + 4) >> 3 );
	xmm_dif = xmm_R0;
	xmm_dif = _mm_sub_epi16(xmm_dif, xmm_L0);
	xmm_dif = _mm_slli_epi16(xmm_dif, 2);
	xmm_dif = _mm_add_epi16(xmm_dif, xmm_L1);
	xmm_dif = _mm_sub_epi16(xmm_dif, xmm_R1);
	xmm_dif = _mm_add_epi16(xmm_dif, xmm_4);
	xmm_dif = _mm_srai_epi16(xmm_dif, 3);
	xmm_dif = _mm_min_epi16(xmm_dif, xmm_tc0);
	xmm_tc0 = _mm_sub_epi16(xmm_zero, xmm_tc0);
	xmm_dif = _mm_max_epi16(xmm_dif, xmm_tc0);
	xmm_dif = _mm_and_si128(xmm_dif, xmm_match);

	//	if( ap ) *P_L1 += iClip3( -C0,  C0, (L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1 );
	xmm_add = xmm_L0;
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_1);
	xmm_add = _mm_srai_epi16(xmm_add, 1);
	xmm_add = _mm_sub_epi16(xmm_add, xmm_L1);
	xmm_add = _mm_sub_epi16(xmm_add, xmm_L1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L2);
	xmm_add = _mm_srai_epi16(xmm_add, 1);
	xmm_add = _mm_min_epi16(xmm_add, xmm_C0);
	xmm_add = _mm_max_epi16(xmm_add, xmm_negative_C0);
	xmm_add = _mm_and_si128(xmm_add, xmm_ap);
	xmm_add = _mm_and_si128(xmm_add, xmm_match);
	xmm_add = _mm_add_epi16(xmm_add, xmm_L1);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrP-inc_dim), xmm_add); 

	// *SrcPtrP = (imgpel) iClip1(max_imgpel_value, L0 + dif);
	xmm_add = _mm_add_epi16(xmm_dif, xmm_L0);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrP), xmm_add);

	// *SrcPtrQ = (imgpel) iClip1(max_imgpel_value, R0 - dif);
	xmm_add = _mm_sub_epi16(xmm_R0, xmm_dif);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrQ), xmm_add); 

	// if (aq) *Q_R1 += iClip3( -C0,  C0, (R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );
	xmm_add = xmm_L0;
	xmm_add = _mm_add_epi16(xmm_add, xmm_R0);
	xmm_add = _mm_add_epi16(xmm_add, xmm_1);
	xmm_add = _mm_srai_epi16(xmm_add, 1);
	xmm_add = _mm_sub_epi16(xmm_add, xmm_R1);
	xmm_add = _mm_sub_epi16(xmm_add, xmm_R1);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R2);
	xmm_add = _mm_srai_epi16(xmm_add, 1);
	xmm_add = _mm_min_epi16(xmm_add, xmm_C0);
	xmm_add = _mm_max_epi16(xmm_add, xmm_negative_C0);
	xmm_add = _mm_and_si128(xmm_add, xmm_aq);
	xmm_add = _mm_and_si128(xmm_add, xmm_match);
	xmm_add = _mm_add_epi16(xmm_add, xmm_R1);
	xmm_add = _mm_packus_epi16(xmm_add, xmm_add);
	_mm_storel_epi64((__m128i *)(SrcPtrQ+inc_dim), xmm_add);
}

static void FilterLuma_Horiz(int inc_dim, imgpel *SrcPtrP, imgpel *SrcPtrQ, int Alpha, int Beta, int C0, int max_imgpel_value)
{
	imgpel *P_L1 = SrcPtrP - inc_dim;
	const imgpel *P_L2 = P_L1 - inc_dim;
	imgpel *Q_R1 = SrcPtrQ + inc_dim;
	const imgpel *Q_R2 = Q_R1 + inc_dim;

	int pel;
	for (pel = 0; pel < BLOCK_SIZE; pel++, SrcPtrP++, SrcPtrQ++, Q_R1++, P_L1++, Q_R2++, P_L2++)
	{
		imgpel  L0 = *SrcPtrP;
		imgpel  R0 = *SrcPtrQ;
		if( abs( R0 - L0 ) < Alpha )
		{          
			imgpel  R1 = *Q_R1;
			if (abs( R0 - R1) < Beta)
			{
				imgpel  L1 = *P_L1;
				if (abs(L0 - L1) < Beta)
				{        
					imgpel  R2 = *Q_R2;
					imgpel  L2 = *P_L2;

					int RL0 = (L0 + R0 + 1) >> 1;
					int aq  = (abs(R0 - R2) < Beta);
					int ap  = (abs(L0 - L2) < Beta);

					//int C0  = ClipTab[ strength ] * bitdepth_scale;
					int tc0  = (C0 + ap + aq) ;
					int dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

					if( ap )
						*P_L1 += iClip3( -C0,  C0, (L2 + RL0 - (L1<<1)) >> 1 );
					*SrcPtrP = (imgpel) iClip1(max_imgpel_value, L0 + dif);

					*SrcPtrQ = (imgpel) iClip1(max_imgpel_value, R0 - dif);
					if( aq  )
						*Q_R1 += iClip3( -C0,  C0, (R2 + RL0 - (R1<<1)) >> 1 );
				}     
			}
		}
	}
}


/* benski> this exists for unit testing, not used in production code */
static int CalculateMatches_sse2(int inc_dim, const imgpel *SrcPtrP, const imgpel *SrcPtrQ, int Alpha, int Beta, __m128i *xmm_result)
{
	int match;
	__m128i xmm_L0, xmm_R0, xmm_R1, xmm_L1;
	__m128i xmm_absdiff, xmm_diff, xmm_alpha, xmm_beta, xmm_zero, xmm_strength;

	xmm_zero = _mm_setzero_si128();
	xmm_alpha = _mm_set1_epi16((uint16_t)Alpha);
	xmm_beta= _mm_set1_epi16((uint16_t)Beta);

	// abs( R0 - L0 )
	LOAD_LINE_EPI16(xmm_L0, SrcPtrP);
	LOAD_LINE_EPI16(xmm_R0, SrcPtrQ);

	xmm_diff=_mm_subs_epu16(xmm_R0, xmm_L0);
	xmm_absdiff=_mm_subs_epu16(xmm_L0, xmm_R0);
	xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

	// if( abs( R0 - L0 ) < Alpha )
	xmm_strength = _mm_cmplt_epi16(xmm_absdiff, xmm_alpha);
	match = _mm_movemask_epi8(xmm_strength);
	if (match == 0)
		return 0;

	// abs(R0 - R1)
	LOAD_LINE_EPI16(xmm_R1, SrcPtrQ+inc_dim);
	xmm_diff=_mm_subs_epu16(xmm_R0, xmm_R1);
	xmm_absdiff=_mm_subs_epu16(xmm_R1, xmm_R0);
	xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

	// 			if (abs( R0 - R1) < Beta)
	xmm_absdiff = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
	xmm_strength = _mm_and_si128(xmm_strength, xmm_absdiff);
	match = _mm_movemask_epi8(xmm_strength);
	if (match == 0)
		return 0;

	// abs(L0 - L1)
	LOAD_LINE_EPI16(xmm_L1, SrcPtrP-inc_dim);
	xmm_diff=_mm_subs_epu16(xmm_L1, xmm_L0);
	xmm_absdiff=_mm_subs_epu16(xmm_L0, xmm_L1);
	xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

	// if ((abs(L0 - L1) < Beta))
	xmm_absdiff = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
	xmm_strength = _mm_and_si128(xmm_strength, xmm_absdiff);
	match = _mm_movemask_epi8(xmm_strength);
	if (match == 0)
		return 0;

	*xmm_result = xmm_strength;
	return match;
}

void EdgeLoopLumaNormal_Horiz_sse2(ColorPlane pl, VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p)
{
	// dir == 1
	__m128i xmm_L0, xmm_R0, xmm_R1, xmm_L1;
	__m128i xmm_absdiff, xmm_diff, xmm_alpha, xmm_beta, xmm_comphi, xmm_complo, xmm_zero, xmm_127;
	__m128i xmm_strength;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      yQ = (edge < MB_BLOCK_SIZE ? edge - 1: 0);
	int pelmatch;

	PixelPos pixMB1;
	p_Vid->getNeighbour0X(MbQ, yQ, p_Vid->mb_size[IS_LUMA], &pixMB1); 

	if (pixMB1.available || (MbQ->DFDisableIdc== 0))
	{   
		int bitdepth_scale   = pl ? p_Vid->bitdepth_scale[IS_CHROMA] : p_Vid->bitdepth_scale[IS_LUMA];

		Macroblock *MbP  = &(p_Vid->mb_data[pixMB1.mb_addr]);

		// Average QP of the two blocks
		int QP = pl? ((MbP->qpc[pl-1] + MbQ->qpc[pl-1] + 1) >> 1) : (MbP->qp + MbQ->qp + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
		int Alpha  = ALPHA_TABLE[indexA] * bitdepth_scale;
		if (Alpha)
		{
			int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);
			int Beta   = BETA_TABLE [indexB] * bitdepth_scale;

			if (Beta !=0)
			{
				int match;
				PixelPos pixMB2;
				const byte *ClipTab = CLIP_TAB   [indexA];
				int max_imgpel_value = p_Vid->max_pel_value_comp[pl];
				int inc_dim = image->stride;
				imgpel *SrcPtrQ;
				imgpel *SrcPtrP = image->base_address + pixMB1.pos_y * image->stride + pixMB1.pos_x;

				p_Vid->getNeighbour0X(MbQ, yQ+1, p_Vid->mb_size[IS_LUMA], &pixMB2);
				SrcPtrQ = image->base_address + pixMB2.pos_y * image->stride + pixMB2.pos_x;

				xmm_strength = _mm_load_si128((__m128i *)Strength);
				xmm_127 = _mm_set1_epi8(127);
				xmm_strength = _mm_adds_epu8(xmm_strength, xmm_127);
				xmm_strength = _mm_srai_epi16(xmm_strength, 15); // shift so it's all 0xFFFF or 0x0000

				// abs( R0 - L0 )
				xmm_R0 = _mm_loadu_si128((__m128i *)SrcPtrQ);
				xmm_L0 = _mm_loadu_si128((__m128i *)SrcPtrP);
				xmm_diff=_mm_subs_epu8(xmm_R0, xmm_L0);
				xmm_absdiff=_mm_subs_epu8(xmm_L0, xmm_R0);
				xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

				// if( abs( R0 - L0 ) < Alpha )
				// SSE2 doesn't have unsigned <, so we have to go to short
				xmm_zero = _mm_setzero_si128();
				xmm_comphi = _mm_unpackhi_epi8(xmm_absdiff, xmm_zero);
				xmm_complo = _mm_unpacklo_epi8(xmm_absdiff, xmm_zero);
				xmm_alpha = _mm_set1_epi16((uint16_t)Alpha);
				xmm_comphi = _mm_cmplt_epi16(xmm_comphi, xmm_alpha);
				xmm_complo = _mm_cmplt_epi16(xmm_complo, xmm_alpha);
				xmm_complo = _mm_packs_epi16(xmm_complo, xmm_comphi);
				xmm_strength = _mm_and_si128(xmm_strength, xmm_complo);
				match = _mm_movemask_epi8(xmm_strength);
				if (match == 0)
					return;

				// abs(R0 - R1)
				xmm_R1 = _mm_loadu_si128((__m128i *)(SrcPtrQ+inc_dim));
				xmm_diff=_mm_subs_epu8(xmm_R0, xmm_R1);
				xmm_absdiff=_mm_subs_epu8(xmm_R1, xmm_R0);
				xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

				// 			if (abs( R0 - R1) < Beta)
				// SSE2 doesn't have unsigned <, so we have to go to short
				xmm_comphi = _mm_unpackhi_epi8(xmm_absdiff, xmm_zero);
				xmm_complo = _mm_unpacklo_epi8(xmm_absdiff, xmm_zero);
				xmm_beta= _mm_set1_epi16((uint16_t)Beta);
				xmm_comphi = _mm_cmplt_epi16(xmm_comphi, xmm_beta);
				xmm_complo = _mm_cmplt_epi16(xmm_complo, xmm_beta);
				xmm_complo = _mm_packs_epi16(xmm_complo, xmm_comphi);
				xmm_strength = _mm_and_si128(xmm_strength, xmm_complo);
				match = _mm_movemask_epi8(xmm_strength);
				if (match == 0)
					return;

				// abs(L0 - L1)
				xmm_L1 = _mm_loadu_si128((__m128i *)(SrcPtrP-inc_dim));
				xmm_diff=_mm_subs_epu8(xmm_L1, xmm_L0);
				xmm_absdiff=_mm_subs_epu8(xmm_L0, xmm_L1);
				xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

				// if ((abs(L0 - L1) < Beta))
				// SSE2 doesn't have unsigned <, so we have to go to short
				xmm_comphi = _mm_unpackhi_epi8(xmm_absdiff, xmm_zero);
				xmm_complo = _mm_unpacklo_epi8(xmm_absdiff, xmm_zero);
				xmm_comphi = _mm_cmplt_epi16(xmm_comphi, xmm_beta);
				xmm_complo = _mm_cmplt_epi16(xmm_complo, xmm_beta);
				xmm_complo = _mm_packs_epi16(xmm_complo, xmm_comphi);
				xmm_strength = _mm_and_si128(xmm_strength, xmm_complo);
				match = _mm_movemask_epi8(xmm_strength);
				if (match == 0)
					return;

				pelmatch = match & 0xFF;
				if (pelmatch)
				{
					byte strength = Strength[0];

					xmm_complo = _mm_unpacklo_epi8(xmm_strength, xmm_strength);

					switch(strength)
					{
					case 4: // INTRA strong
						{
							assert(Strength[4] == 4);
							IntraStrongFilter_Luma_Horiz_sse2(inc_dim, SrcPtrP, SrcPtrQ, xmm_alpha, xmm_beta, xmm_complo);
						}
						break;
					default:
						{
							int C[2]  = { ClipTab[strength] * bitdepth_scale, ClipTab[Strength[4]] * bitdepth_scale };
							FilterLuma_Horiz_sse2(inc_dim, SrcPtrP, SrcPtrQ, xmm_beta, C, xmm_complo);
						}
						break;
					}
				}
				pelmatch = match & 0xFF00;
				if (pelmatch)
				{
					byte strength = Strength[8];

					xmm_comphi = _mm_unpackhi_epi8(xmm_strength, xmm_strength);

					switch(strength)
					{
					case 4: // INTRA strong
						{
							assert(Strength[12] == 4);
							IntraStrongFilter_Luma_Horiz_sse2(inc_dim, SrcPtrP+8, SrcPtrQ+8, xmm_alpha, xmm_beta, xmm_comphi);
						}
						break;
					default:
						{
							int C[2]  = { ClipTab[strength] * bitdepth_scale, ClipTab[Strength[12]] * bitdepth_scale };
							FilterLuma_Horiz_sse2(inc_dim, SrcPtrP+8, SrcPtrQ+8, xmm_beta, C, xmm_comphi);
						}
						break;
					}
				}
			}
		}
	}
}


void EdgeLoopLuma_Horiz_YUV420(VideoImage *image, const byte strength[4], Macroblock *MbQ, PixelPos pixMB1, Macroblock *MbP)
{
	// dir == 1
	__m128i xmm_L0, xmm_R0, xmm_R1, xmm_L1;
	__m128i xmm_absdiff, xmm_diff, xmm_alpha, xmm_beta, xmm_comphi, xmm_complo, xmm_zero, xmm_127;
	__m128i xmm_strength;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int pelmatch;
		int i;
__declspec(align(32)) uint8_t Strength[16];

	for (i=0;i<16;i++)
	{
Strength[i] = strength[i/4];
	}

	if (pixMB1.available || (MbQ->DFDisableIdc== 0))
	{   
		// Average QP of the two blocks
		int QP =  (MbP->qp + MbQ->qp + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
		int Alpha  = ALPHA_TABLE[indexA];
		if (Alpha)
		{
			int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);
			int Beta   = BETA_TABLE [indexB];

			if (Beta !=0)
			{
				int match;
				const byte *ClipTab = CLIP_TAB   [indexA];
				int inc_dim = image->stride;
				
				imgpel *SrcPtrP = image->base_address + pixMB1.pos_y * inc_dim + pixMB1.pos_x;
				imgpel *SrcPtrQ = SrcPtrP + inc_dim;

				xmm_strength = _mm_load_si128((__m128i *)Strength);
				xmm_127 = _mm_set1_epi8(127);
				xmm_strength = _mm_adds_epu8(xmm_strength, xmm_127);
				xmm_strength = _mm_srai_epi16(xmm_strength, 15); // shift so it's all 0xFFFF or 0x0000

				// abs( R0 - L0 )
				xmm_L0 = _mm_loadu_si128((__m128i *)SrcPtrP);
				xmm_R0 = _mm_loadu_si128((__m128i *)SrcPtrQ);				
				xmm_diff=_mm_subs_epu8(xmm_R0, xmm_L0);
				xmm_absdiff=_mm_subs_epu8(xmm_L0, xmm_R0);
				xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

				// if( abs( R0 - L0 ) < Alpha )
				// SSE2 doesn't have unsigned <, so we have to go to short
				xmm_zero = _mm_setzero_si128();
				xmm_comphi = _mm_unpackhi_epi8(xmm_absdiff, xmm_zero);
				xmm_complo = _mm_unpacklo_epi8(xmm_absdiff, xmm_zero);
				xmm_alpha = _mm_set1_epi16((uint16_t)Alpha);
				xmm_comphi = _mm_cmplt_epi16(xmm_comphi, xmm_alpha);
				xmm_complo = _mm_cmplt_epi16(xmm_complo, xmm_alpha);
				xmm_complo = _mm_packs_epi16(xmm_complo, xmm_comphi);
				xmm_strength = _mm_and_si128(xmm_strength, xmm_complo);
				match = _mm_movemask_epi8(xmm_strength);
				if (match == 0)
					return;

				// abs(R0 - R1)
				xmm_R1 = _mm_loadu_si128((__m128i *)(SrcPtrQ+inc_dim));
				xmm_diff=_mm_subs_epu8(xmm_R0, xmm_R1);
				xmm_absdiff=_mm_subs_epu8(xmm_R1, xmm_R0);
				xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

				// 			if (abs( R0 - R1) < Beta)
				// SSE2 doesn't have unsigned <, so we have to go to short
				xmm_comphi = _mm_unpackhi_epi8(xmm_absdiff, xmm_zero);
				xmm_complo = _mm_unpacklo_epi8(xmm_absdiff, xmm_zero);
				xmm_beta= _mm_set1_epi16((uint16_t)Beta);
				xmm_comphi = _mm_cmplt_epi16(xmm_comphi, xmm_beta);
				xmm_complo = _mm_cmplt_epi16(xmm_complo, xmm_beta);
				xmm_complo = _mm_packs_epi16(xmm_complo, xmm_comphi);
				xmm_strength = _mm_and_si128(xmm_strength, xmm_complo);
				match = _mm_movemask_epi8(xmm_strength);
				if (match == 0)
					return;

				// abs(L0 - L1)
				xmm_L1 = _mm_loadu_si128((__m128i *)(SrcPtrP-inc_dim));
				xmm_diff=_mm_subs_epu8(xmm_L1, xmm_L0);
				xmm_absdiff=_mm_subs_epu8(xmm_L0, xmm_L1);
				xmm_absdiff=_mm_or_si128(xmm_absdiff, xmm_diff);

				// if ((abs(L0 - L1) < Beta))
				// SSE2 doesn't have unsigned <, so we have to go to short
				xmm_comphi = _mm_unpackhi_epi8(xmm_absdiff, xmm_zero);
				xmm_complo = _mm_unpacklo_epi8(xmm_absdiff, xmm_zero);
				xmm_comphi = _mm_cmplt_epi16(xmm_comphi, xmm_beta);
				xmm_complo = _mm_cmplt_epi16(xmm_complo, xmm_beta);
				xmm_complo = _mm_packs_epi16(xmm_complo, xmm_comphi);
				xmm_strength = _mm_and_si128(xmm_strength, xmm_complo);
				match = _mm_movemask_epi8(xmm_strength);
				if (match == 0)
					return;

				pelmatch = match & 0xFF;
				if (pelmatch)
				{
					byte strength = Strength[0];

					xmm_complo = _mm_unpacklo_epi8(xmm_strength, xmm_strength);

					switch(strength)
					{
					case 4: // INTRA strong
						{
							assert(Strength[4] == 4);
							IntraStrongFilter_Luma_Horiz_sse2(inc_dim, SrcPtrP, SrcPtrQ, xmm_alpha, xmm_beta, xmm_complo);
						}
						break;
					default:
						{
							int C[2]  = { ClipTab[strength], ClipTab[Strength[4]] };
							FilterLuma_Horiz_sse2(inc_dim, SrcPtrP, SrcPtrQ, xmm_beta, C, xmm_complo);
						}
						break;
					}
				}
				pelmatch = match & 0xFF00;
				if (pelmatch)
				{
					byte strength = Strength[8];

					xmm_comphi = _mm_unpackhi_epi8(xmm_strength, xmm_strength);

					switch(strength)
					{
					case 4: // INTRA strong
						{
							assert(Strength[12] == 4);
							IntraStrongFilter_Luma_Horiz_sse2(inc_dim, SrcPtrP+8, SrcPtrQ+8, xmm_alpha, xmm_beta, xmm_comphi);
						}
						break;
					default:
						{
							int C[2]  = { ClipTab[strength], ClipTab[Strength[12]] };
							FilterLuma_Horiz_sse2(inc_dim, SrcPtrP+8, SrcPtrQ+8, xmm_beta, C, xmm_comphi);
						}
						break;
					}
				}
			}
		}
	}
}


void EdgeLoopLumaNormal_Horiz(ColorPlane pl, VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p)
{
	// dir == 1
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      yQ = (edge < MB_BLOCK_SIZE ? edge - 1: 0);

	PixelPos pixMB1;
	p_Vid->getNeighbour0X(MbQ, yQ, p_Vid->mb_size[IS_LUMA], &pixMB1); 

	if (pixMB1.available || (MbQ->DFDisableIdc== 0))
	{   
		int bitdepth_scale   = pl ? p_Vid->bitdepth_scale[IS_CHROMA] : p_Vid->bitdepth_scale[IS_LUMA];

		Macroblock *MbP  = &(p_Vid->mb_data[pixMB1.mb_addr]);

		// Average QP of the two blocks
		int QP = pl? ((MbP->qpc[pl-1] + MbQ->qpc[pl-1] + 1) >> 1) : (MbP->qp + MbQ->qp + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
		int Alpha  = ALPHA_TABLE[indexA] * bitdepth_scale;
		if (Alpha)
		{
			int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);
			int Beta   = BETA_TABLE [indexB] * bitdepth_scale;

			if (Beta !=0)
			{
				PixelPos pixMB2;
				const byte *ClipTab = CLIP_TAB   [indexA];
				int max_imgpel_value = p_Vid->max_pel_value_comp[pl];
				int inc_dim = image->stride;
				int pel;
				imgpel *SrcPtrQ;
				imgpel *SrcPtrP = image->base_address + pixMB1.pos_y * image->stride + pixMB1.pos_x;

				p_Vid->getNeighbour0X(MbQ, ++yQ, p_Vid->mb_size[IS_LUMA], &pixMB2);
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
							IntraStrongFilter_Luma_Horiz(inc_dim, SrcPtrP+pel, SrcPtrQ+pel, Alpha, Beta);
						}
						break;
					default:
						{
							int C0  = ClipTab[strength] * bitdepth_scale;
							FilterLuma_Horiz(inc_dim, SrcPtrP+pel, SrcPtrQ+pel, Alpha, Beta, C0, max_imgpel_value);
						}
						break;
					}

				}
			}
		}
	}
}
